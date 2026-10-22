// Bộ kiểm thử backend: chạy bằng `ctest` hoặc trực tiếp `backend_tests`.
// Trả về mã khác 0 nếu có bất kỳ kiểm tra nào thất bại.

#include "Algorithms.h"
#include "FileManager.h"
#include "HashMap.h"
#include "IdGenerator.h"
#include "MatchingService.h"
#include "Registration.h"
#include "Sha256.h"
#include "Student.h"
#include "TextUtils.h"
#include "Tutor.h"
#include "Validator.h"
#include "Vector.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

static int g_failed = 0;
static int g_total = 0;

#define CHECK(cond)                                                                   \
    do {                                                                              \
        ++g_total;                                                                    \
        if (!(cond)) {                                                                \
            ++g_failed;                                                               \
            cerr << "  [FAIL] " << __FILE__ << ":" << __LINE__ << "  " #cond << endl; \
        }                                                                             \
    } while (0)

static void section(const char* name) { cout << "- " << name << endl; }

static Tutor* makeTutor(const string& id, const string& gender, Vector<string> subjects, int years,
                        const string& rate, const string& schedule, Vector<string> areas, int current = 0,
                        bool available = true) {
    return new Tutor(id, "Gia su " + id, "0905000000", id + "@mail.com", "Hai Chau", gender, "01/01/1995",
                     subjects, "DH", years, "Kinh nghiem", rate, schedule, available, current, {}, areas, "");
}

static Student makeStudent(Vector<string> subjects, const string& area, const string& schedule,
                           const string& budget, const string& preferredGender = "Khong") {
    return Student("SV900", "Hoc Sinh", "0905111111", "hs@mail.com", area, "Nam", "01/01/2010", "10", subjects,
                   schedule, "Kha", preferredGender, budget, "", false);
}

static void testVector() {
    section("Vector");
    Vector<int> v;
    for (int i = 0; i < 100; ++i) v.push_back(i);
    CHECK(v.size() == 100);
    CHECK(v[99] == 99);
    v.erase(static_cast<size_t>(0));
    CHECK(v.size() == 99 && v[0] == 1);
    Vector<int> copy = v;
    copy[0] = 42;
    CHECK(v[0] == 1);  // sao chép sâu
    bool threw = false;
    try {
        v[1000] = 1;
    } catch (const out_of_range&) {
        threw = true;
    }
    CHECK(threw);
}

static void testHashMap() {
    section("HashMap");
    HashMap<string, int> m;
    for (int i = 0; i < 1000; ++i) m.insert("K" + to_string(i), i);  // trước đây: "HashMap is full" sau 16 phần tử
    CHECK(m.size() == 1000);
    bool allFound = true;
    for (int i = 0; i < 1000; ++i) {
        int* p = m.get("K" + to_string(i));
        if (!p || *p != i) allFound = false;
    }
    CHECK(allFound);

    m.insert("K5", 555);  // cập nhật, không tăng size
    CHECK(m.size() == 1000 && *m.get("K5") == 555);

    // Xóa không được làm đứt chuỗi dò (lỗi cũ: get(17) NOT FOUND sau remove(1)).
    HashMap<int, int> t(16);
    t.insert(1, 10);
    t.insert(17, 20);
    t.insert(33, 30);
    CHECK(t.remove(1));
    CHECK(t.get(17) != nullptr && *t.get(17) == 20);
    CHECK(t.get(33) != nullptr && *t.get(33) == 30);
    CHECK(!t.contains(1));
    CHECK(!t.remove(1));
    t.insert(1, 11);  // tái sử dụng ô đã xóa
    CHECK(*t.get(1) == 11 && t.size() == 3);

    // Thêm/xóa liên tục không được làm bảng đầy ô "đã xóa".
    HashMap<int, int> churn(8);
    for (int round = 0; round < 5000; ++round) {
        churn.insert(round, round);
        churn.remove(round);
    }
    churn.insert(7, 7);
    CHECK(churn.size() == 1 && churn.contains(7));
    CHECK(churn.bucketCount() < 64);

    const HashMap<int, int>& cref = t;
    CHECK(cref.contains(17));
    t.clear();
    CHECK(t.size() == 0 && t.empty());
}

static void testMergeSort() {
    section("mergeSort");
    Vector<int> v;
    for (int i = 0; i < 500; ++i) v.push_back((i * 7919) % 503);
    mergeSort(v, [](const int& a, const int& b) { return a < b; });
    bool sorted = true;
    for (size_t i = 1; i < v.size(); ++i) {
        if (v[i - 1] > v[i]) sorted = false;
    }
    CHECK(sorted);

    // Ổn định: phần tử cùng khóa giữ nguyên thứ tự ban đầu.
    struct P {
        int key = 0;
        int order = 0;
    };
    Vector<P> ps;
    for (int i = 0; i < 50; ++i) ps.push_back(P{i % 3, i});
    mergeSort(ps, [](const P& a, const P& b) { return a.key < b.key; });
    bool stable = true;
    for (size_t i = 1; i < ps.size(); ++i) {
        if (ps[i - 1].key == ps[i].key && ps[i - 1].order > ps[i].order) stable = false;
    }
    CHECK(stable);

    Vector<int> empty;
    mergeSort(empty, [](const int& a, const int& b) { return a < b; });
    CHECK(empty.empty());
}

static void testTextUtils() {
    section("TextUtils");
    CHECK(TextUtils::normalize("  Hải  Châu ") == "hai chau");
    CHECK(TextUtils::normalize("ĐÀ NẴNG") == "da nang");
    CHECK(TextUtils::normalize("Toán") == "toan");
    CHECK(TextUtils::normalize("Tiếng Anh") == "tieng anh");
    CHECK(TextUtils::normalize("Lý") == "ly");
    CHECK(TextUtils::normalize("Thanh Khê") == TextUtils::normalize("thanh khe"));
    // Dạng NFD (dấu tách rời) cũng được chuẩn hóa.
    CHECK(TextUtils::normalize("Toa\xCC\x81n") == "toan");

    CHECK(TextUtils::cleanField("a|b\nc") == "a/b c");
    CHECK(TextUtils::toInt("12", -1) == 12);
    CHECK(TextUtils::toInt("abc", -1) == -1);
    CHECK(TextUtils::toInt("12x", -1) == -1);
    CHECK(TextUtils::toLongLong("", 7) == 7);

    Vector<string> parts = TextUtils::splitTrimmed(" T2 , T4 ,, T6 ", ',');
    CHECK(parts.size() == 3 && parts[1] == "T4");
}

static void testValidator() {
    section("Validator");
    CHECK(Validator::isValidDate("29/02/2024"));
    CHECK(!Validator::isValidDate("29/02/2023"));
    CHECK(!Validator::isValidDate("31/04/2026"));
    CHECK(!Validator::isValidDate("1/1/2026"));
    CHECK(!Validator::isValidDate("ab/cd/efgh"));
    CHECK(Validator::compareDates("01/09/2026", "31/12/2026") < 0);
    CHECK(Validator::compareDates("01/01/2027", "31/12/2026") > 0);
    CHECK(Validator::isValidPhone("0905123456"));
    CHECK(Validator::isValidPhone("0905 123 456"));
    CHECK(!Validator::isValidPhone("905123456"));
    CHECK(!Validator::isValidPhone("09051abc56"));
    CHECK(Validator::isValidEmail(""));
    CHECK(Validator::isValidEmail("a@b.com"));
    CHECK(!Validator::isValidEmail("a@b"));
    CHECK(!Validator::isValidEmail("a b@c.com"));
    CHECK(Validator::isNonNegativeNumber("200000"));
    CHECK(!Validator::isNonNegativeNumber("-5"));
    CHECK(!Validator::isNonNegativeNumber(""));
}

static void testIdGenerator() {
    section("IdGenerator");
    Vector<string> ids;
    CHECK(IdGenerator::next("SV", ids) == "SV001");
    ids.push_back("SV001");
    ids.push_back("SV002");
    ids.push_back("SV005");  // đã xóa SV003, SV004
    CHECK(IdGenerator::next("SV", ids) == "SV006");
    ids.push_back("SV099");
    ids.push_back("SV100");
    CHECK(IdGenerator::next("SV", ids) == "SV101");
    ids.push_back("GS999");  // tiền tố khác không ảnh hưởng
    CHECK(IdGenerator::next("SV", ids) == "SV101");
}

static void testSha256() {
    section("SHA-256 / mật khẩu");
    CHECK(Sha256::hashHex("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    CHECK(Sha256::hashHex("") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    string salt = Sha256::generateSalt();
    CHECK(salt.size() == 32);
    CHECK(Sha256::hashPassword(salt, "123456") == Sha256::hashPassword(salt, "123456"));
    CHECK(Sha256::hashPassword(salt, "123456") != Sha256::hashPassword(salt, "123457"));
    CHECK(Sha256::hashPassword(salt, "123456") != Sha256::hashPassword("khac", "123456"));
}

static void testMatching() {
    section("MatchingService");
    CHECK(MatchingService::calculatePriceScore(100000, 0) < 45.0);  // lỗi cũ: ngân sách 0 được 45 điểm
    CHECK(MatchingService::calculatePriceScore(0, 100000) < 45.0);
    CHECK(MatchingService::calculatePriceScore(80000, 100000) == 45.0);
    CHECK(MatchingService::calculatePriceScore(200000, 100000) == 10.0);

    Vector<string> areas = {"Hải Châu", "Thanh Khê"};
    CHECK(MatchingService::calculateLocationScore("Hai Chau", areas) == 10.0);  // khác dấu vẫn khớp
    CHECK(MatchingService::calculateLocationScore("Liên Chiểu", areas) < 10.0);
    CHECK(MatchingService::calculateScheduleScore("T2,T4,T6", "T2, T4 ,T6,T7") == 20.0);
    CHECK(MatchingService::calculateScheduleScore("T2,T4,T6", "T3,T5") == 0.0);
    CHECK(MatchingService::calculateScheduleScore("T2,T4", "T2") == 10.0);

    Vector<Tutor*> tutors;
    tutors.push_back(makeTutor("GS001", "Nam", {"Toán", "Lý"}, 8, "170000", "T2,T4,T6", {"Hải Châu"}));
    tutors.push_back(makeTutor("GS002", "Nu", {"Hóa"}, 12, "100000", "T2,T4,T6", {"Hải Châu"}));          // sai môn
    tutors.push_back(makeTutor("GS003", "Nam", {"Toan"}, 2, "400000", "T3,T5", {"Liên Chiểu"}));           // đắt, lệch lịch
    tutors.push_back(makeTutor("GS004", "Nam", {"Toan"}, 9, "150000", "T2,T4,T6", {"Hải Châu"}, 0, false));  // không nhận
    tutors.push_back(makeTutor("GS005", "Nam", {"Toan"}, 9, "150000", "T2,T4,T6", {"Hải Châu"}, 8));        // đủ học sinh (tối đa 8)

    Student s = makeStudent({"Toan"}, "Hai Chau", "T2,T4,T6", "200000");
    Vector<MatchResult> res = MatchingService::matchStudentsToTutors(s, tutors);
    CHECK(res.size() == 2);  // chỉ GS001 và GS003
    if (res.size() == 2) {
        CHECK(res[0].tutor->getPersonID() == "GS001");
        CHECK(res[0].totalScore > res[1].totalScore);
        CHECK(res[0].totalScore <= 100.0 && res[1].totalScore >= 0.0);
        CHECK(!res[0].reasons.empty());
        CHECK(res[0].subjectCoverage == 1.0);
    }

    // Ngân sách = 0 không được đẩy gia sư lên đầu một cách vô lý.
    Student noBudget = makeStudent({"Toan"}, "Hai Chau", "T2,T4,T6", "0");
    Vector<MatchResult> res2 = MatchingService::matchStudentsToTutors(noBudget, tutors);
    CHECK(res2.size() == 2);
    if (!res2.empty()) CHECK(res2[0].priceScore < 45.0);

    // Giới tính mong muốn.
    Vector<Tutor*> two;
    two.push_back(makeTutor("GS010", "Nam", {"Toan"}, 5, "150000", "T2", {"Hai Chau"}));
    two.push_back(makeTutor("GS011", "Nữ", {"Toan"}, 5, "150000", "T2", {"Hai Chau"}));
    Student wantFemale = makeStudent({"Toan"}, "Hai Chau", "T2", "200000", "Nữ");
    Vector<MatchResult> res3 = MatchingService::matchStudentsToTutors(wantFemale, two);
    CHECK(res3.size() == 2 && res3[0].tutor->getPersonID() == "GS011");

    // matchTopN
    CHECK(MatchingService::matchTopN(s, tutors, 1).size() == 1);
    CHECK(MatchingService::matchTopN(s, tutors, 10).size() == 2);

    // Không yêu cầu môn -> mọi gia sư đang nhận đều hợp lệ.
    Student anySubject = makeStudent({}, "Hai Chau", "T2,T4,T6", "200000");
    CHECK(MatchingService::matchStudentsToTutors(anySubject, tutors).size() == 3);

    for (size_t i = 0; i < tutors.size(); ++i) delete tutors[i];
    for (size_t i = 0; i < two.size(); ++i) delete two[i];
}

// Lỗi nghiêm trọng đã sửa: sau khi lưu, toàn bộ học sinh/gia sư biến mất khi đọc lại.
static void testFileRoundTrip() {
    section("FileManager: lưu rồi đọc lại");
    namespace fs = filesystem;
    fs::path dir = fs::temp_directory_path() / "pbl2_test_data";
    fs::remove_all(dir);
    fs::create_directories(dir);
    FileManager::setDataDirectory(dir.string());

    Vector<Student*> students;
    students.push_back(new Student("SV001", "Nguyễn Minh Anh", "0905123456", "a@b.com", "Hải Châu", "Nam",
                                   "10/05/2010", "10", {"Toán", "Lý"}, "T2,T4,T6", "Khá", "Khong", "200000",
                                   "ghi|chú\nxấu", true));
    FileManager::saveStudents(students);
    Vector<Student*> s2 = FileManager::loadStudents();
    CHECK(s2.size() == 1);
    if (s2.size() == 1) {
        CHECK(s2[0]->getPersonID() == "SV001");
        CHECK(s2[0]->getGrade() == "10");
        CHECK(s2[0]->getNeedSubjects().size() == 2);
        CHECK(s2[0]->getAvailableSchedule() == "T2,T4,T6");
        CHECK(s2[0]->getBudgetPerHour() == "200000");
        CHECK(s2[0]->getIsMatched());
        CHECK(s2[0]->getNotes() == "ghi/chú xấu");  // '|' và xuống dòng không phá định dạng
    }

    Vector<Tutor*> tutors;
    tutors.push_back(new Tutor("GS001", "Trần Quốc Huy", "0905111222", "h@b.com", "Hải Châu", "Nam", "10/05/2000",
                               {"Toán", "Lý"}, "ĐHBK", 8, "Dạy 5 năm", "170000", "T2,T4,T6", true, 2, {},
                               {"Hải Châu", "Thanh Khê"}, ""));
    FileManager::saveTutors(tutors);
    Vector<Tutor*> t2 = FileManager::loadTutors();
    CHECK(t2.size() == 1);
    if (t2.size() == 1) {
        CHECK(t2[0]->getYearsOfExperience() == 8);
        CHECK(t2[0]->getExperienceDetails() == "Dạy 5 năm");
        CHECK(t2[0]->getRatePerHour() == "170000");
        CHECK(t2[0]->getTeachingAreas().size() == 2);
        CHECK(t2[0]->getCurrentStudents() == 2);
        CHECK(t2[0]->getIsAvailable());
    }

    Vector<Registration*> regs;
    regs.push_back(new Registration("DK001", "SV001", "Nguyễn Minh Anh", {"Toan", "Ly"}, "20/09/2026", "Cho ghep",
                                    ""));  // ghi chú rỗng ở cuối dòng
    FileManager::saveRegistrations(regs);
    Vector<Registration*> r2 = FileManager::loadRegistrations();
    CHECK(r2.size() == 1 && r2[0]->getSubjectCount() == 2);

    // File có dòng hỏng / BOM / dòng trống: không crash, bỏ qua dòng hỏng và đếm lại.
    {
        ofstream out(dir / "tutor.txt", ios::binary);
        out << "\xEF\xBB\xBF" << tutors[0]->toString() << "\r\n\r\n";
        out << "GS002|thieu cot\r\n";
        out << "GS003|A|0|a@b.c|x|Nam|01/01/2000|Toan|DH|khongphaiso|kn|100000|T2|x|true|abc\r\n";
    }
    Vector<Tutor*> t3 = FileManager::loadTutors();
    CHECK(t3.size() == 2);  // GS001 + GS003 (số sai -> mặc định 0, không ném lỗi)
    CHECK(FileManager::skippedLineCount() == 1);
    if (t3.size() == 2) {
        CHECK(t3[1]->getYearsOfExperience() == 0 && t3[1]->getCurrentStudents() == 0);
    }

    for (size_t i = 0; i < students.size(); ++i) delete students[i];
    for (size_t i = 0; i < s2.size(); ++i) delete s2[i];
    for (size_t i = 0; i < tutors.size(); ++i) delete tutors[i];
    for (size_t i = 0; i < t2.size(); ++i) delete t2[i];
    for (size_t i = 0; i < t3.size(); ++i) delete t3[i];
    for (size_t i = 0; i < regs.size(); ++i) delete regs[i];
    for (size_t i = 0; i < r2.size(); ++i) delete r2[i];
    fs::remove_all(dir);
}

static void testAdminLogin() {
    section("Đăng nhập admin (băm mật khẩu)");
    namespace fs = filesystem;
    fs::path dir = fs::temp_directory_path() / "pbl2_test_admin";
    fs::remove_all(dir);
    fs::create_directories(dir);
    FileManager::setDataDirectory(dir.string());

    // Dòng cũ dạng thô vẫn đăng nhập được và được nâng cấp thành dạng băm.
    {
        ofstream out(dir / "admin.txt");
        out << "admin|123456\n";
    }
    CHECK(!FileManager::validateAdminLogin("admin", "sai"));
    CHECK(FileManager::validateAdminLogin("admin", "123456"));
    string content;
    {
        ifstream in(dir / "admin.txt");
        getline(in, content);
    }
    CHECK(content.find("123456") == string::npos);  // không còn mật khẩu thô trong file
    CHECK(content.size() > 64);
    CHECK(FileManager::validateAdminLogin("admin", "123456"));  // vẫn đúng sau khi băm
    CHECK(!FileManager::validateAdminLogin("admin", "12345"));
    CHECK(!FileManager::validateAdminLogin("root", "123456"));

    CHECK(!FileManager::changeAdminPassword("admin", "sai", "matkhaumoi"));
    CHECK(!FileManager::changeAdminPassword("admin", "123456", "123"));  // quá ngắn
    CHECK(FileManager::changeAdminPassword("admin", "123456", "matkhaumoi"));
    CHECK(!FileManager::validateAdminLogin("admin", "123456"));
    CHECK(FileManager::validateAdminLogin("admin", "matkhaumoi"));

    fs::remove_all(dir);
}

int main() {
    cout << "=== Backend tests ===" << endl;
    testVector();
    testHashMap();
    testMergeSort();
    testTextUtils();
    testValidator();
    testIdGenerator();
    testSha256();
    testMatching();
    testFileRoundTrip();
    testAdminLogin();

    cout << "\n" << (g_total - g_failed) << "/" << g_total << " kiem tra dat" << endl;
    if (g_failed > 0) {
        cerr << g_failed << " KIEM TRA THAT BAI" << endl;
        return 1;
    }
    cout << "TAT CA DEU DAT" << endl;
    return 0;
}
