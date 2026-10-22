#include "FileManager.h"

#include "Sha256.h"
#include "TextUtils.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace std;

namespace {

string g_dataDir;       // thư mục dữ liệu do người dùng đặt (rỗng = tự dò)
int g_skippedLines = 0;  // số dòng sai định dạng ở lần đọc gần nhất

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

}  // namespace

void FileManager::setDataDirectory(const string& dir) { g_dataDir = dir; }

int FileManager::skippedLineCount() { return g_skippedLines; }

string FileManager::getDataPath(string fileName) {
    namespace fs = filesystem;
    if (!g_dataDir.empty()) return (fs::path(g_dataDir) / fileName).string();
    if (fs::exists(fs::path("data") / fileName)) return (fs::path("data") / fileName).string();
    if (fs::exists(fs::path("../data") / fileName)) return (fs::path("../data") / fileName).string();
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
    error_code ec;
    fs::rename(tmp, path, ec);
    if (ec) {
        // Một số hệ thống không cho ghi đè khi đổi tên -> xóa file đích rồi thử lại.
        fs::remove(path, ec);
        ec.clear();
        fs::rename(tmp, path, ec);
        if (ec) {
            fs::remove(tmp, ec);
            return false;
        }
    }
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
    return lines;
}

}  // namespace

Vector<Student*> FileManager::loadStudents() {
    g_skippedLines = 0;
    Vector<Student*> students;
    Vector<string> lines = readLines(getDataPath("student.txt"));
    for (size_t i = 0; i < lines.size(); ++i) {
        auto parts = splitString(lines[i], '|');
        if (parts.size() < 15) {
            ++g_skippedLines;
            continue;
        }
        students.push_back(new Student(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5], parts[6],
                                       parts[7], splitString(parts[8], ','), parts[9], parts[10], parts[11],
                                       parts[12], parts[13], parts[14] == "true"));
    }
    return students;
}

void FileManager::saveStudents(Vector<Student*>& students) {
    string content;
    for (size_t i = 0; i < students.size(); ++i) content += students[i]->toString() + "\n";
    writeFileAtomically(getDataPath("student.txt"), content);
}

Vector<Tutor*> FileManager::loadTutors() {
    Vector<Tutor*> tutors;
    Vector<string> lines = readLines(getDataPath("tutor.txt"));
    for (size_t i = 0; i < lines.size(); ++i) {
        auto parts = splitString(lines[i], '|');
        if (parts.size() < 16) {
            ++g_skippedLines;
            continue;
        }
        Vector<string> subjects = splitString(parts[7], ',');
        Vector<string> areas = splitString(parts[13], ',');
        tutors.push_back(new Tutor(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5], parts[6], subjects,
                                   parts[8], TextUtils::toInt(parts[9], 0), parts[10], parts[11], parts[12],
                                   parts[14] == "true", TextUtils::toInt(parts[15], 0), {}, areas, ""));
    }
    return tutors;
}

void FileManager::saveTutors(Vector<Tutor*>& tutors) {
    string content;
    for (size_t i = 0; i < tutors.size(); ++i) content += tutors[i]->toString() + "\n";
    writeFileAtomically(getDataPath("tutor.txt"), content);
}

Vector<TutoringClass*> FileManager::loadClasses() {
    Vector<TutoringClass*> classes;
    Vector<string> lines = readLines(getDataPath("class.txt"));
    for (size_t i = 0; i < lines.size(); ++i) {
        auto parts = splitString(lines[i], '|');
        if (parts.size() < 11) {
            ++g_skippedLines;
            continue;
        }
        classes.push_back(new TutoringClass(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5], parts[6],
                                            parts[7], parts[8], parts[9], parts[10]));
    }
    return classes;
}

void FileManager::saveClasses(Vector<TutoringClass*>& classes) {
    ostringstream out;
    for (size_t i = 0; i < classes.size(); ++i) {
        TutoringClass* c = classes[i];
        out << TextUtils::cleanField(c->getClassID()) << "|" << TextUtils::cleanField(c->getStudentID()) << "|"
            << TextUtils::cleanField(c->getTutorID()) << "|" << TextUtils::cleanField(c->getSubject()) << "|"
            << TextUtils::cleanField(c->getSchedule()) << "|" << TextUtils::cleanField(c->getStartDate()) << "|"
            << TextUtils::cleanField(c->getEndDate()) << "|" << TextUtils::cleanField(c->getStatus()) << "|"
            << TextUtils::cleanField(c->getRatePerHour()) << "|" << TextUtils::cleanField(c->getCreatedAt()) << "|"
            << TextUtils::cleanField(c->getUpdatedAt()) << "\n";
    }
    writeFileAtomically(getDataPath("class.txt"), out.str());
}

Vector<Contract*> FileManager::loadContracts() {
    Vector<Contract*> contracts;
    Vector<string> lines = readLines(getDataPath("contract.txt"));
    for (size_t i = 0; i < lines.size(); ++i) {
        auto parts = splitString(lines[i], '|');
        if (parts.size() < 9) {
            ++g_skippedLines;
            continue;
        }
        contracts.push_back(new Contract(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5], parts[6],
                                         parts[7], parts[8]));
    }
    return contracts;
}

void FileManager::saveContracts(Vector<Contract*>& contracts) {
    ostringstream out;
    for (size_t i = 0; i < contracts.size(); ++i) {
        Contract* c = contracts[i];
        out << TextUtils::cleanField(c->getContractID()) << "|" << TextUtils::cleanField(c->getClassID()) << "|"
            << TextUtils::cleanField(c->getStudentID()) << "|" << TextUtils::cleanField(c->getTutorID()) << "|"
            << TextUtils::cleanField(c->getCreatedAt()) << "|" << TextUtils::cleanField(c->getStartDate()) << "|"
            << TextUtils::cleanField(c->getEndDate()) << "|" << TextUtils::cleanField(c->getTotalFee()) << "|"
            << TextUtils::cleanField(c->getStatus()) << "\n";
    }
    writeFileAtomically(getDataPath("contract.txt"), out.str());
}

Vector<Registration*> FileManager::loadRegistrations() {
    Vector<Registration*> registrations;
    Vector<string> lines = readLines(getDataPath("registration.txt"));
    for (size_t i = 0; i < lines.size(); ++i) {
        auto parts = splitString(lines[i], '|');
        if (parts.size() < 7) {
            ++g_skippedLines;
            continue;
        }
        registrations.push_back(new Registration(parts[0], parts[1], parts[2], splitString(parts[3], ','), parts[4],
                                                 parts[5], parts[6]));
    }
    return registrations;
}

void FileManager::saveRegistrations(Vector<Registration*>& registrations) {
    string content;
    for (size_t i = 0; i < registrations.size(); ++i) content += registrations[i]->toString() + "\n";
    writeFileAtomically(getDataPath("registration.txt"), content);
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
