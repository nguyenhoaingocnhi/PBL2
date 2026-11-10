#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "FileManager.h"

#include "Sha256.h"
#include "TextUtils.h"
#include "Validator.h"
#include "ScheduleUtils.h"

#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <set>

using namespace std;

namespace {

string g_dataDir;       // thư mục dữ liệu do người dùng đặt (rỗng = tự dò)
int g_skippedLines = 0;
string g_recoveryError;
string g_storageError; // refuse subsequent writes after an incomplete/invalid load

void refuseUnsafeData(const string& message) {
    if (g_storageError.empty()) g_storageError = message;
    ++g_skippedLines;
}

// Bỏ BOM UTF-8 nếu file được lưu bằng Notepad.
string stripBom(const string& s) {
    if (s.size() >= 3 && static_cast<unsigned char>(s[0]) == 0xEF &&
        static_cast<unsigned char>(s[1]) == 0xBB && static_cast<unsigned char>(s[2]) == 0xBF) {
        return s.substr(3);
    }
    return s;
}

bool isHex64(const string& s) {
    if (s.size() != 64) return false;
    for (char c : s) {
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return false;
    }
    return true;
}

// Cho phép so sánh thời gian cố định để tránh lộ thông tin qua thời gian phản hồi.
bool constantTimeEquals(const string& a, const string& b) {
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (size_t i = 0; i < a.size(); ++i) diff |= static_cast<unsigned char>(a[i] ^ b[i]);
    return diff == 0;
}

// Journal v1: a marker exists only after every .backup and .stage was prepared.
// PENDING + absent COMMITTED => restore originals; COMMITTED => keep new files.
// This provides startup recovery for an interrupted commit (not physical disk
// flush guarantees, which require platform-specific fsync/FlushFileBuffers).
bool recoverDirectory(const filesystem::path& dir) {
    namespace fs = filesystem;
    const fs::path pending = dir / ".tutorflow_pending";
    const fs::path committed = dir / ".tutorflow_committed";
    error_code ec;
    if (!fs::exists(pending)) {
        if (fs::exists(committed)) fs::remove(committed, ec);
        return true;
    }
    ifstream input(pending, ios::binary);
    string flags;
    getline(input, flags);
    input.close();
    if (!flags.empty() && flags.back() == '\r') flags.pop_back();
    if (flags.size() != 5 || flags.substr(0, 2) != "v1" ||
        flags.find_first_not_of("01", 2) != string::npos) {
        g_recoveryError = "Nhật ký giao dịch không hợp lệ. Hãy sao lưu thư mục data và kiểm tra .tutorflow_pending.";
        return false;
    }
    const array<string, 3> names = {"student.txt", "tutor.txt", "class.txt"};
    if (!fs::exists(committed)) {
        // Pending = pre-commit state. Verify *all* necessary backups before
        // changing anything, so incomplete metadata cannot delete valid data.
        for (size_t i = 0; i < names.size(); ++i) {
            if (flags[i + 2] == '1' && !fs::is_regular_file(dir / (names[i] + ".backup"))) {
                g_recoveryError = "Thiếu bản sao phục hồi " + names[i] + ".backup. Dừng ghi để tránh mất dữ liệu.";
                return false;
            }
        }
        for (size_t i = 0; i < names.size(); ++i) {
            const fs::path target = dir / names[i];
            if (flags[i + 2] == '1')
                fs::copy_file(dir / (names[i] + ".backup"), target,
                              fs::copy_options::overwrite_existing, ec);
            else fs::remove(target, ec);
            if (ec) {
                g_recoveryError = "Không thể phục hồi " + names[i] + ": " + ec.message();
                return false;
            }
        }
    }
    // Remove journal last so any crash during restoration remains recoverable.
    // COMMITTED state keeps its marker until the PENDING file is removed.
    for (const auto& name : names) {
        fs::remove(dir / (name + ".stage"), ec);
        fs::remove(dir / (name + ".backup"), ec);
    }
    fs::remove(pending, ec);
    if (!ec) fs::remove(committed, ec);
    g_recoveryError.clear();
    return !ec;
}

}  // namespace

void FileManager::setDataDirectory(const string& dir) {
    g_dataDir = dir;
    g_storageError.clear();
    g_skippedLines = 0;
    g_recoveryError.clear();
}

string FileManager::dataPath(const string& filename) { return getDataPath(filename); }

bool FileManager::saveTextFile(const string& filename, const string& content) {
    if (!g_storageError.empty() || !recoverIfNeeded()) return false;
    return writeFileAtomically(getDataPath(filename), content);
}

int FileManager::skippedLineCount() { return g_skippedLines; }
string FileManager::storageError() { return g_storageError; }

bool FileManager::recoverIfNeeded() {
    return recoverDirectory(filesystem::path(getDataPath("student.txt")).parent_path());
}
string FileManager::recoveryError() { return g_recoveryError; }

string FileManager::getDataPath(string fileName) {
    namespace fs = filesystem;
    if (!g_dataDir.empty()) return (fs::path(g_dataDir) / fileName).string();
    // Select ONE directory for the whole dataset. Per-file probing can mix
    // student.txt from ./data with tutor.txt from ../data, corrupting links.
    if (fs::is_directory("data")) return (fs::path("data") / fileName).string();
    if (fs::is_directory("../data")) return (fs::path("../data") / fileName).string();
    return (fs::path("data") / fileName).string();
}

// Ghi ra file tạm rồi đổi tên: nếu chương trình tắt giữa chừng thì file cũ vẫn nguyên vẹn.
bool FileManager::writeFileAtomically(const string& path, const string& content) {
    namespace fs = filesystem;
    string tmp = path + ".tmp";
    {
        ofstream out(tmp, ios::binary | ios::trunc);
        if (!out) return false;
        out << content;
        out.flush();
        if (!out) return false;
    }
    // Never delete the original before successful replacement. On Windows,
    // std::filesystem::rename cannot reliably replace an existing destination.
#ifdef _WIN32
    const bool replaced = MoveFileExW(fs::path(tmp).c_str(), fs::path(path).c_str(),
                                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
    if (!replaced) {
        error_code ec;
        fs::remove(tmp, ec);
        return false;
    }
#else
    error_code ec;
    fs::rename(tmp, path, ec);
    if (ec) {
        fs::remove(tmp, ec);
        return false;
    }
#endif
    return true;
}

Vector<string> FileManager::splitString(string text, char delimiter) {
    Vector<string> tokens;
    string current;
    for (size_t i = 0; i <= text.size(); ++i) {
        if (i == text.size() || text[i] == delimiter) {
            tokens.push_back(TextUtils::trim(current));
            current.clear();
        } else {
            current.push_back(text[i]);
        }
    }
    // Chuỗi rỗng không sinh phần tử nào.
    if (text.empty()) tokens.clear();
    // Danh sách phân tách bằng dấu phẩy (môn học, khu vực, lịch): bỏ phần tử rỗng.
    if (delimiter == ',') {
        Vector<string> filtered;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (!tokens[i].empty()) filtered.push_back(tokens[i]);
        }
        return filtered;
    }
    return tokens;
}

namespace {

// Đọc toàn bộ dòng không rỗng của file (đã bỏ BOM, bỏ \r).
Vector<string> readLines(const string& path) {
    Vector<string> lines;
    ifstream in(path, ios::binary);
    std::error_code ec;
    if (!in && filesystem::exists(path, ec)) {
        refuseUnsafeData("Không thể đọc file " + path + ". Không lưu để tránh mất dữ liệu.");
        return lines;
    }
    if (ec) {
        refuseUnsafeData("Không thể kiểm tra file " + path + ": " + ec.message());
        return lines;
    }
    string line;
    bool first = true;
    while (getline(in, line)) {
        if (first) {
            line = stripBom(line);
            first = false;
        }
        string t = TextUtils::trim(line);
        if (!t.empty()) lines.push_back(t);
    }
    if (in.bad()) refuseUnsafeData("Lỗi đọc dữ liệu " + path + ".");
    return lines;
}

}  // namespace

Vector<Student*> FileManager::loadStudents() {
    g_skippedLines = 0;
    if (!recoverIfNeeded()) return {};
    Vector<Student*> students;
    std::set<string> seen;
    Vector<string> lines = readLines(getDataPath("student.txt"));
    for (size_t i = 0; i < lines.size(); ++i) {
        auto parts = splitString(lines[i], '|');
        if ((parts.size() != 14 && parts.size() != 15) ||
            parts[0].empty() || !seen.insert(parts[0]).second) {
            refuseUnsafeData("student.txt: dòng " + to_string(i + 1) +
                             " sai số cột, trống ID hoặc trùng ID. Hãy sửa file trước khi lưu.");
            continue;
        }
        if (!Validator::isNonNegativeNumber(parts[11]) ||
            !Validator::isValidDate(parts[5]) ||
            (parts[13] != "true" && parts[13] != "false")) {
            refuseUnsafeData("student.txt: dòng " + to_string(i + 1) +
                             " có ngày, ngân sách hoặc trạng thái không hợp lệ.");
        }
        auto* student = new Student(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5],
                                       parts[6], splitString(parts[7], ','), parts[8], parts[9], parts[10],
                                       parts[11], parts[12], parts[13] == "true");
        students.push_back(student);
    }
    return students;
}

void FileManager::saveStudents(Vector<Student*>& students) {
    if (!g_storageError.empty() || !recoverIfNeeded()) return;
    string content;
    for (size_t i = 0; i < students.size(); ++i) content += students[i]->toString() + "\n";
    writeFileAtomically(getDataPath("student.txt"), content);
}

Vector<Tutor*> FileManager::loadTutors() {
    Vector<Tutor*> tutors;
    if (!recoverIfNeeded()) return {};
    std::set<string> seen;
    Vector<string> lines = readLines(getDataPath("tutor.txt"));
    for (size_t i = 0; i < lines.size(); ++i) {
        auto parts = splitString(lines[i], '|');
        if ((parts.size() != 15 && parts.size() != 16) ||
            parts[0].empty() || !seen.insert(parts[0]).second) {
            refuseUnsafeData("tutor.txt: dòng " + to_string(i + 1) +
                             " sai số cột, trống ID hoặc trùng ID. Hãy sửa file trước khi lưu.");
            continue;
        }
        if (!Validator::isNonNegativeNumber(parts[8]) ||
            (TextUtils::toLongLong(parts[8], -1) < 0 || TextUtils::toLongLong(parts[8], -1) > 1000) ||
            !Validator::isNonNegativeNumber(parts[14]) ||
            (TextUtils::toLongLong(parts[14], -1) < 0 || TextUtils::toLongLong(parts[14], -1) > 1000000) ||
            !Validator::isNonNegativeNumber(parts[10]) ||
            !Validator::isValidDate(parts[5]) ||
            (parts[13] != "true" && parts[13] != "false")) {
            refuseUnsafeData("tutor.txt: dòng " + to_string(i + 1) +
                             " có số, ngày hoặc trạng thái không hợp lệ.");
        }
        Vector<string> subjects = splitString(parts[6], ',');
        Vector<string> areas = splitString(parts[12], ',');
        auto* tutor = new Tutor(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5], subjects,
                                   parts[7], TextUtils::toInt(parts[8], 0), parts[9], parts[10], parts[11],
                                   parts[13] == "true", TextUtils::toInt(parts[14], 0), {}, areas, "");
        tutors.push_back(tutor);
    }
    return tutors;
}

void FileManager::saveTutors(Vector<Tutor*>& tutors) {
    if (!g_storageError.empty() || !recoverIfNeeded()) return;
    string content;
    for (size_t i = 0; i < tutors.size(); ++i) content += tutors[i]->toString() + "\n";
    writeFileAtomically(getDataPath("tutor.txt"), content);
}

Vector<TutoringClass*> FileManager::loadClasses() {
    Vector<TutoringClass*> classes;
    if (!recoverIfNeeded()) return {};
    std::set<string> seen;
    Vector<string> lines = readLines(getDataPath("class.txt"));
    for (size_t i = 0; i < lines.size(); ++i) {
        auto parts = splitString(lines[i], '|');
        // Current schema: 10 fields; legacy schema: 11 fields (column 7 was end date).
        // Accept both formats to avoid silently losing old projects during upgrade.
        if ((parts.size() != 10 && parts.size() != 11) ||
            parts[0].empty() || !seen.insert(parts[0]).second) {
            refuseUnsafeData("class.txt: dòng " + to_string(i + 1) +
                             " sai số cột, trống ID hoặc trùng ID. Hãy sửa file trước khi lưu.");
            continue;
        }
        const size_t statusColumn = parts.size() == 11 ? 7 : 6;
        if (!Validator::isValidDate(parts[5]) ||
            !Validator::isNonNegativeNumber(parts[statusColumn + 1]) ||
            !ScheduleUtils::valid(parts[4]) ||
            (parts[statusColumn] != "Đang học" && parts[statusColumn] != "Chờ xếp lịch" &&
             parts[statusColumn] != "Cần gia sư" && parts[statusColumn] != "Hoàn thành")) {
            refuseUnsafeData("class.txt: dòng " + to_string(i + 1) +
                             " có ngày, học phí, lịch hoặc trạng thái không hợp lệ.");
        }
        classes.push_back(new TutoringClass(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5],
                                            parts[statusColumn], parts[statusColumn + 1],
                                            parts[statusColumn + 2], parts[statusColumn + 3]));
    }
    return classes;
}

void FileManager::saveClasses(Vector<TutoringClass*>& classes) {
    if (!g_storageError.empty() || !recoverIfNeeded()) return;
    ostringstream out;
    for (size_t i = 0; i < classes.size(); ++i) {
        TutoringClass* c = classes[i];
        out << TextUtils::cleanField(c->getClassID()) << "|" << TextUtils::cleanField(c->getStudentID()) << "|"
            << TextUtils::cleanField(c->getTutorID()) << "|" << TextUtils::cleanField(c->getSubject()) << "|"
            << TextUtils::cleanField(c->getSchedule()) << "|" << TextUtils::cleanField(c->getStartDate()) << "|"
            << TextUtils::cleanField(c->getStatus()) << "|"
            << TextUtils::cleanField(c->getRatePerHour()) << "|" << TextUtils::cleanField(c->getCreatedAt()) << "|"
            << TextUtils::cleanField(c->getUpdatedAt()) << "\n";
    }
    writeFileAtomically(getDataPath("class.txt"), out.str());
}


// Multi-file commit: prepare three stages and three backups, record a journal,
// replace every file, write a COMMITTED marker, then clean up. Startup recovery
// restores the *entire* old set if power loss occurs before COMMITTED.
bool FileManager::saveAll(Vector<Student*>& students, Vector<Tutor*>& tutors,
                          Vector<TutoringClass*>& classes) {
    namespace fs = filesystem;
    const array<fs::path, 3> paths = {getDataPath("student.txt"), getDataPath("tutor.txt"),
                                      getDataPath("class.txt")};
    const fs::path dir = paths[0].parent_path();
    if (!g_storageError.empty() || !recoverIfNeeded() || !fs::is_directory(dir)) return false;
    array<string, 3> values;
    for (size_t i = 0; i < students.size(); ++i) values[0] += students[i]->toString() + "\n";
    for (size_t i = 0; i < tutors.size(); ++i) values[1] += tutors[i]->toString() + "\n";
    for (size_t i = 0; i < classes.size(); ++i) {
        TutoringClass* c = classes[i];
        values[2] += TextUtils::cleanField(c->getClassID()) + "|" +
                     TextUtils::cleanField(c->getStudentID()) + "|" +
                     TextUtils::cleanField(c->getTutorID()) + "|" +
                     TextUtils::cleanField(c->getSubject()) + "|" +
                     TextUtils::cleanField(c->getSchedule()) + "|" +
                     TextUtils::cleanField(c->getStartDate()) + "|" +
                     TextUtils::cleanField(c->getStatus()) + "|" +
                     TextUtils::cleanField(c->getRatePerHour()) + "|" +
                     TextUtils::cleanField(c->getCreatedAt()) + "|" +
                     TextUtils::cleanField(c->getUpdatedAt()) + "\n";
    }
    const fs::path pending = dir / ".tutorflow_pending";
    const fs::path committed = dir / ".tutorflow_committed";
    array<fs::path, 3> stage, backup;
    string flags = "v1";
    error_code ec;
    for (size_t i = 0; i < paths.size(); ++i) {
        stage[i] = paths[i].string() + ".stage";
        backup[i] = paths[i].string() + ".backup";
        if (fs::exists(stage[i]) || fs::exists(backup[i])) return false;
        flags += fs::exists(paths[i]) ? '1' : '0';
    }
    auto cleanupUncommitted = [&]() {
        for (size_t i = 0; i < paths.size(); ++i) {
            fs::remove(stage[i], ec);
            fs::remove(backup[i], ec);
        }
        fs::remove(pending, ec);
        fs::remove(committed, ec);
    };
    for (size_t i = 0; i < paths.size(); ++i) {
        ofstream output(stage[i], ios::binary | ios::trunc);
        if (!output) { cleanupUncommitted(); return false; }
        output.write(values[i].data(), static_cast<streamsize>(values[i].size()));
        output.flush();
        if (!output) { cleanupUncommitted(); return false; }
        output.close();
        if (!output) { cleanupUncommitted(); return false; }
    }
    for (size_t i = 0; i < paths.size(); ++i) {
        if (flags[i + 2] != '1') continue;
        ec.clear();
        fs::copy_file(paths[i], backup[i], fs::copy_options::none, ec);
        if (ec) { cleanupUncommitted(); return false; }
    }
    {
        ofstream marker(pending, ios::binary | ios::trunc);
        if (!marker) { cleanupUncommitted(); return false; }
        marker << flags << "\n";
        marker.flush();
        if (!marker) { cleanupUncommitted(); return false; }
    }
    for (size_t i = 0; i < paths.size(); ++i) {
        ec.clear();
        fs::rename(stage[i], paths[i], ec);
        if (ec) {
            ec.clear();
            fs::remove(paths[i], ec);
            if (!ec) fs::rename(stage[i], paths[i], ec);
        }
        if (ec) return recoverIfNeeded() && false;
    }
    {
        ofstream marker(committed, ios::binary | ios::trunc);
        if (!marker) return recoverIfNeeded() && false;
        marker << "committed\n";
        marker.flush();
        if (!marker) {
            marker.close();
            fs::remove(committed, ec);
            return recoverIfNeeded() && false;
        }
    }
    // A failed cleanup is non-fatal: next startup finishes it. Data is committed.
    recoverIfNeeded();
    return true;
}

// ---------------------------------------------------------------------------
// Tài khoản admin: tên|muối|băm  (hoặc dòng cũ tên|mật_khẩu_thô)
// ---------------------------------------------------------------------------

bool FileManager::validateAdminLogin(string username, string password) {
    string path = getDataPath("admin.txt");
    Vector<string> lines = readLines(path);
    bool ok = false;
    bool upgrade = false;
    string content;

    for (size_t i = 0; i < lines.size(); ++i) {
        auto parts = splitString(lines[i], '|');
        string line = lines[i];
        if (parts.size() >= 3 && isHex64(parts[2])) {
            if (parts[0] == username &&
                constantTimeEquals(Sha256::hashPassword(parts[1], password), parts[2])) {
                ok = true;
            }
        } else if (parts.size() >= 2) {
            // Định dạng cũ: mật khẩu thô -> nếu khớp thì nâng cấp thành dạng băm.
            if (parts[0] == username && constantTimeEquals(parts[1], password)) {
                ok = true;
                string salt = Sha256::generateSalt();
                line = parts[0] + "|" + salt + "|" + Sha256::hashPassword(salt, password);
                upgrade = true;
            }
        }
        content += line + "\n";
    }
    if (ok && upgrade) writeFileAtomically(path, content);
    return ok;
}

bool FileManager::changeAdminPassword(string username, string oldPassword, string newPassword) {
    if (newPassword.size() < 6 || newPassword.find('|') != string::npos) return false;
    if (!validateAdminLogin(username, oldPassword)) return false;

    string path = getDataPath("admin.txt");
    Vector<string> lines = readLines(path);
    string content;
    for (size_t i = 0; i < lines.size(); ++i) {
        auto parts = splitString(lines[i], '|');
        if (!parts.empty() && parts[0] == username) {
            string salt = Sha256::generateSalt();
            content += username + "|" + salt + "|" + Sha256::hashPassword(salt, newPassword) + "\n";
        } else {
            content += lines[i] + "\n";
        }
    }
    return writeFileAtomically(path, content);
}
