// Bộ kiểm thử backend: chạy bằng `ctest` hoặc trực tiếp `backend_tests`.
// Trả về mã khác 0 nếu có bất kỳ kiểm tra nào thất bại.

#include "Algorithms.h"
#include "FileManager.h"
#include "ClassService.h"
#include "ContractService.h"
#include "HashMap.h"
#include "IdGenerator.h"
#include "MatchingService.h"
#include "Sha256.h"
#include "Student.h"
#include "TextUtils.h"
#include "SubjectUtils.h"
#include "ScheduleUtils.h"
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
    return new Tutor(id, "Gia su " + id, "0905000000", "Hai Chau", gender, "01/01/1995",
                     subjects, "DH", years, "Kinh nghiem", rate, schedule, available, current, {}, areas, "");
}

static Student makeStudent(Vector<string> subjects, const string& area, const string& schedule,
                           const string& budget, const string& preferredGender = "Khong") {
    return Student("SV900", "Hoc Sinh", "0905111111", area, "Nam", "01/01/2010", "10", subjects,
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
    CHECK(Validator::isValidDate("29/02/2000"));
    CHECK(!Validator::isValidDate("29/02/1900"));
    CHECK(!Validator::isValidDate("00/01/2010"));
    CHECK(!Validator::isValidDate("01/13/2010"));
    CHECK(!Validator::isValidDate("01-01-2010"));
    CHECK(!Validator::isValidDate("01/1/2010"));
    CHECK(!Validator::isValidDate("01/01/2010x"));
    CHECK(!Validator::isValidDate(""));
    CHECK(!Validator::isValidDate("ab/cd/efgh"));
    CHECK(Validator::compareDates("01/09/2026", "31/12/2026") < 0);
    CHECK(Validator::compareDates("01/01/2027", "31/12/2026") > 0);
    CHECK(Validator::isValidPhone("0905123456"));
    CHECK(Validator::isValidPhone("0905 123 456"));
    CHECK(!Validator::isValidPhone("905123456"));
    CHECK(!Validator::isValidPhone("09051abc56"));
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
    CHECK(res.size() == 1);  // gia sư đắt, lệch lịch bị loại
    if (res.size() == 1) {
        CHECK(res[0].tutor->getPersonID() == "GS001");
        CHECK(res[0].totalScore <= 100.0 && res[0].totalScore >= 0.0);
        CHECK(!res[0].reasons.empty());
        CHECK(res[0].subjectCoverage == 1.0);
    }

    // Ngân sách = 0 không được đẩy gia sư lên đầu một cách vô lý.
    Student noBudget = makeStudent({"Toan"}, "Hai Chau", "T2,T4,T6", "0");
    Vector<MatchResult> res2 = MatchingService::matchStudentsToTutors(noBudget, tutors);
    CHECK(res2.size() == 1);
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
    CHECK(MatchingService::matchTopN(s, tutors, 10).size() == 1);

    // Không yêu cầu môn -> mọi gia sư đang nhận đều hợp lệ.
    Student anySubject = makeStudent({}, "Hai Chau", "T2,T4,T6", "200000");
    CHECK(MatchingService::matchStudentsToTutors(anySubject, tutors).size() == 2);

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
    students.push_back(new Student("SV001", "Nguyễn Minh Anh", "0905123456", "Hải Châu", "Nam",
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
    tutors.push_back(new Tutor("GS001", "Trần Quốc Huy", "0905111222", "Hải Châu", "Nam", "10/05/2000",
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

    // File có dòng hỏng / BOM / dòng trống: không crash, bỏ qua dòng hỏng và đếm lại.
    {
        ofstream out(dir / "tutor.txt", ios::binary);
        out << "\xEF\xBB\xBF" << tutors[0]->toString() << "\r\n\r\n";
        out << "GS002|thieu cot\r\n";
        out << "GS003|A|0|a@b.c|x|Nam|01/01/2000|Toan|DH|khongphaiso|kn|100000|T2|x|true|abc\r\n";
    }
    Vector<Tutor*> t3 = FileManager::loadTutors();
    CHECK(t3.size() == 2);  // GS001 + GS003 (số sai -> mặc định 0, không ném lỗi)
    CHECK(FileManager::skippedLineCount() == 2);  // one malformed row + one bad numeric row
    CHECK(!FileManager::storageError().empty());  // never overwrite coerced numeric data
    if (t3.size() == 2) {
        CHECK(t3[1]->getYearsOfExperience() == 0 && t3[1]->getCurrentStudents() == 0);
    }

    for (size_t i = 0; i < students.size(); ++i) delete students[i];
    for (size_t i = 0; i < s2.size(); ++i) delete s2[i];
    for (size_t i = 0; i < tutors.size(); ++i) delete tutors[i];
    for (size_t i = 0; i < t2.size(); ++i) delete t2[i];
    for (size_t i = 0; i < t3.size(); ++i) delete t3[i];
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


static TutoringClass makeClass(const string& id, const string& student,
                              const string& tutor, const string& status = "Đang học",
                              const string& subject = "Toán Lớp 10",
                              const string& schedule = "T2,T4", const string& rate = "150000") {
    return TutoringClass(id, student, tutor, subject, schedule, "01/09/2026",
                         status, rate, "01/09/2026", "01/09/2026");
}

// A user may reject rank #1 and choose any later eligible tutor.
// This verifies descending ranking and creation for the explicitly selected row.
static void testRankedCandidateChoice() {
    section("Matching: ranked list and choose a non-top tutor");
    namespace fs = filesystem;
    const fs::path dir = fs::temp_directory_path() / "pbl2_test_ranked_choice";
    fs::remove_all(dir);
    fs::create_directories(dir);
    FileManager::setDataDirectory(dir.string());

    Vector<Student*> students;
    auto* student = new Student(makeStudent({"Toán"}, "Hai Chau", "T2,T4", "250000"));
    student->setPersonID("SV123");
    students.push_back(student);
    Vector<Tutor*> tutors;
    tutors.push_back(makeTutor("GS901", "Nam", {"Toán"}, 12, "150000", "T2,T4", {"Hai Chau"}));
    tutors.push_back(makeTutor("GS902", "Nam", {"Toán"}, 4, "170000", "T2,T4", {"Hai Chau"}));
    tutors.push_back(makeTutor("GS903", "Nam", {"Toán"}, 2, "190000", "T2,T4", {"Hai Chau"}));
    tutors.push_back(makeTutor("GS904", "Nam", {"Hóa"}, 10, "120000", "T2,T4", {"Hai Chau"}));
    Vector<TutoringClass*> classes;
    ClassService service(students, tutors, classes);
    service.synchronize();

    Vector<MatchResult> candidates = MatchingService::matchStudentsToTutors(*student, tutors);
    CHECK(candidates.size() == 3);  // ineligible subject is excluded
    if (candidates.size() >= 3) {
        CHECK(candidates[0].totalScore >= candidates[1].totalScore);
        CHECK(candidates[1].totalScore >= candidates[2].totalScore);
        CHECK(candidates[0].tutor != candidates[1].tutor);
        CHECK(candidates[1].tutor->getPersonID() == "GS902");

        // Simulate UI selection of the SECOND row instead of the default top.
        Tutor* chosen = candidates[1].tutor;
        string error;
        CHECK(MatchingService::meetsMandatoryRequirements(*student, *chosen));
        CHECK(service.create(makeClass("LH901", student->getPersonID(),
                                       chosen->getPersonID(), "Đang học", "Toán Lớp 10",
                                       "T2", chosen->getRatePerHour()), error));
        CHECK(service.isAssignedTo("SV123", "GS902"));
        CHECK(!service.isAssignedTo("SV123", "GS901"));
        CHECK(student->getIsMatched());
        CHECK(chosen->getCurrentStudents() == 1);
        CHECK(tutors[0]->getCurrentStudents() == 0);
        CHECK(!service.create(makeClass("LH902", "SV123", "GS901"), error));
    }
    for (size_t i = 0; i < students.size(); ++i) delete students[i];
    for (size_t i = 0; i < tutors.size(); ++i) delete tutors[i];
    for (size_t i = 0; i < classes.size(); ++i) delete classes[i];
    fs::remove_all(dir);
}

static void testClassBusinessLogic() {
    section("ClassService: vòng đời lớp và đồng bộ dữ liệu");
    namespace fs = filesystem;
    fs::path dir = fs::temp_directory_path() / "pbl2_test_class_logic";
    fs::remove_all(dir);
    fs::create_directories(dir);
    FileManager::setDataDirectory(dir.string());

    Vector<Student*> students;
    auto* first = new Student(makeStudent({"Toán"}, "Hai Chau", "T2,T4", "200000"));
    first->setPersonID("SV001");
    students.push_back(first);
    auto* second = new Student(makeStudent({"Toán"}, "Hai Chau", "T2,T4", "200000"));
    second->setPersonID("SV002");
    students.push_back(second);
    Vector<Tutor*> tutors;
    tutors.push_back(makeTutor("GS001", "Nam", {"Toan"}, 5, "150000", "T2,T4", {"Hai Chau"}));
    tutors.push_back(makeTutor("GS002", "Nam", {"Toan"}, 5, "150000", "T2,T4", {"Hai Chau"}));
    Vector<TutoringClass*> classes;
    ClassService service(students, tutors, classes);
    string error;

    // Create unassigned class, then assign a tutor; both routes are identical.
    CHECK(service.create(makeClass("LH001", "SV001", "", "Cần gia sư"), error));
    CHECK(!first->getIsMatched());
    CHECK(tutors[0]->getCurrentStudents() == 0);
    CHECK(!service.canDeleteStudent("SV001"));  // retain historical integrity
    CHECK(service.countBrokenReferences() == 0);
    CHECK(!service.create(makeClass("LH001", "SV002", ""), error)); // duplicate ID
    CHECK(!service.create(makeClass("LHbad", "missing", "GS001"), error));
    CHECK(!service.create(makeClass("LH002", "SV002", "missing"), error));
    CHECK(!service.create(makeClass("LH002", "SV002", "", "Đang học"), error));
    CHECK(!service.create(makeClass("LH002", "SV002", "GS001", "Cần gia sư"), error));
    CHECK(!service.create(makeClass("LH002", "SV002", "GS001", "Đang học", "Hóa Lớp 10"), error));
    CHECK(!service.create(makeClass("LH002", "SV002", "GS001", "Đang học", "Toán Lớp 10", "T3"), error));
    CHECK(!service.create(makeClass("LH002", "SV002", "GS001", "Đang học", "Toán Lớp 10", "T2", "300000"), error));
    CHECK(classes.size() == 1 && !first->getIsMatched());

    TutoringClass assigned = *classes[0];
    assigned.setTutorID("GS001");
    assigned.setStatus("Đang học");
    assigned.setSchedule("T2");
    CHECK(service.update("LH001", assigned, error));
    CHECK(first->getIsMatched() && tutors[0]->getCurrentStudents() == 1);
    CHECK(first->getEnrolledClasses().size() == 1 && tutors[0]->getTeachingClasses().size() == 1);
    CHECK(service.isAssignedTo("SV001", "GS001"));
    CHECK(!service.canDeleteTutor("GS001"));

    // Exact pair cannot be assigned twice; different student may use same tutor.
    CHECK(!service.create(makeClass("LH002", "SV001", "GS001"), error));
    CHECK(service.create(makeClass("LH002", "SV002", "GS001", "Đang học", "Toán Lớp 10", "T4"), error));
    CHECK(tutors[0]->getCurrentStudents() == 2);
    CHECK(second->getIsMatched());

    // Move another student's class to tutor GS002; counts recomputed both sides.
    TutoringClass reassigned = *classes[1];
    reassigned.setTutorID("GS002");
    CHECK(service.update("LH002", reassigned, error));
    CHECK(tutors[0]->getCurrentStudents() == 1 && tutors[1]->getCurrentStudents() == 1);
    CHECK(second->getIsMatched());
    CHECK(service.isAssignedTo("SV002", "GS002"));

    // A finished class no longer consumes capacity or marks the student matched.
    reassigned.setStatus("Hoàn thành");
    CHECK(service.update("LH002", reassigned, error));
    CHECK(tutors[1]->getCurrentStudents() == 0 && !second->getIsMatched());
    CHECK(!service.canDeleteTutor("GS002")); // historical class still references tutor

    // Deletion of the remaining assigned class unmatches the student.
    CHECK(service.remove("LH001", error));
    CHECK(!first->getIsMatched());
    CHECK(tutors[0]->getCurrentStudents() == 0);
    CHECK(service.canDeleteStudent("SV001") && service.canDeleteTutor("GS001"));
    CHECK(!service.remove("LH001", error));

    // Files agree with derived state after a successful transaction.
    auto loadedStudents = FileManager::loadStudents();
    auto loadedTutors = FileManager::loadTutors();
    auto loadedClasses = FileManager::loadClasses();
    CHECK(loadedStudents.size() == 2 && loadedTutors.size() == 2 && loadedClasses.size() == 1);
    if (loadedStudents.size() == 2 && loadedTutors.size() == 2) {
        CHECK(!loadedStudents[0]->getIsMatched() && !loadedStudents[1]->getIsMatched());
        CHECK(loadedTutors[0]->getCurrentStudents() == 0 && loadedTutors[1]->getCurrentStudents() == 0);
    }

    for (size_t i = 0; i < loadedStudents.size(); ++i) delete loadedStudents[i];
    for (size_t i = 0; i < loadedTutors.size(); ++i) delete loadedTutors[i];
    for (size_t i = 0; i < loadedClasses.size(); ++i) delete loadedClasses[i];
    for (size_t i = 0; i < students.size(); ++i) delete students[i];
    for (size_t i = 0; i < tutors.size(); ++i) delete tutors[i];
    for (size_t i = 0; i < classes.size(); ++i) delete classes[i];
    fs::remove_all(dir);
}

static void testClassCapacityAndRollback() {
    section("ClassService: giới hạn sức chứa và rollback khi lưu lỗi");
    namespace fs = filesystem;
    fs::path dir = fs::temp_directory_path() / "pbl2_test_capacity_logic";
    fs::remove_all(dir);
    fs::create_directories(dir);
    FileManager::setDataDirectory(dir.string());
    Vector<Student*> students;
    Vector<Tutor*> tutors;
    Vector<TutoringClass*> classes;
    tutors.push_back(makeTutor("GS001", "Nam", {"Toan"}, 3, "150000", "T2,T4", {"Hai Chau"}));
    for (int i = 0; i < MatchingService::kMaxStudentsPerTutor + 1; ++i) {
        auto* student = new Student(makeStudent({"Toán"}, "Hai Chau", "T2,T4", "200000"));
        student->setPersonID("SV" + to_string(i + 1));
        students.push_back(student);
    }
    ClassService service(students, tutors, classes);
    string error;
    for (int i = 0; i < MatchingService::kMaxStudentsPerTutor; ++i) {
        CHECK(service.create(makeClass("LH" + to_string(i + 1), "SV" + to_string(i + 1), "GS001", "Chờ xếp lịch", "Toán Lớp 10", ""), error));
    }
    CHECK(tutors[0]->getCurrentStudents() == MatchingService::kMaxStudentsPerTutor);
    CHECK(!ClassService::canReceiveStudents(*tutors[0]));
    CHECK(!service.create(makeClass("LH99", "SV9", "GS001", "Chờ xếp lịch", "Toán Lớp 10", ""), error));
    CHECK(classes.size() == MatchingService::kMaxStudentsPerTutor);

    CHECK(service.remove("LH1", error));
    CHECK(ClassService::canReceiveStudents(*tutors[0]));
    CHECK(service.create(makeClass("LH99", "SV9", "GS001", "Chờ xếp lịch", "Toán Lớp 10", ""), error));

    // Simulate an unwritable/missing destination and verify no in-memory class
    // or workload is changed after the failed transaction.
    const size_t beforeCount = classes.size();
    const int beforeStudents = tutors[0]->getCurrentStudents();
    FileManager::setDataDirectory((dir / "missing" / "subdir").string());
    CHECK(!service.remove("LH99", error));
    CHECK(classes.size() == beforeCount);
    CHECK(tutors[0]->getCurrentStudents() == beforeStudents);
    CHECK(service.isAssignedTo("SV9", "GS001"));
    FileManager::setDataDirectory(dir.string());
    auto saved = FileManager::loadClasses();
    CHECK(saved.size() == beforeCount);
    for (size_t i = 0; i < saved.size(); ++i) delete saved[i];
    for (size_t i = 0; i < students.size(); ++i) delete students[i];
    for (size_t i = 0; i < tutors.size(); ++i) delete tutors[i];
    for (size_t i = 0; i < classes.size(); ++i) delete classes[i];
    fs::remove_all(dir);
}


static void testSubjectRegistrationAndMatching() {
    section("Đăng ký môn -> ghép theo môn -> nhiều lớp -> hoàn thành");
    namespace fs = filesystem;
    const fs::path dir = fs::temp_directory_path() / "pbl2_test_multi_subject";
    fs::remove_all(dir);
    fs::create_directories(dir);
    FileManager::setDataDirectory(dir.string());

    Vector<Student*> students;
    auto* s = new Student(makeStudent({"Toán", "Anh"}, "Hải Châu", "T2,T4", "250000"));
    s->setPersonID("SV001");
    CHECK(s->getRegisteredSubjects().size() == 2);
    s->registerSubject(" toan ");
    CHECK(s->getRegisteredSubjects().size() == 2); // trim/normalize; no duplicated subject
    CHECK(s->unregisterSubject("ANH"));
    CHECK(s->getRegisteredSubjects().size() == 1);
    s->registerSubject("Anh");
    CHECK(s->getRegisteredSubjects().size() == 2);
    students.push_back(s);

    Vector<Tutor*> tutors;
    tutors.push_back(makeTutor("GS001", "Nam", {"Toán"}, 3, "150000", "T2,T4", {"Hải Châu"}));
    tutors.push_back(makeTutor("GS002", "Nữ", {"Anh"}, 5, "200000", "T2,T4", {"Hải Châu"}));
    tutors.push_back(makeTutor("GS003", "Nam", {"Toan"}, 7, "150000", "T2,T4", {"Hải Châu"}));
    Vector<TutoringClass*> classes;
    ClassService service(students, tutors, classes);
    string error;

    CHECK(MatchingService::matchStudentsToTutors(*s, tutors).empty());  // no tutor covers BOTH
    Student mathRequest = *s;
    mathRequest.setNeedSubjects({"Toan"});
    CHECK(MatchingService::matchStudentsToTutors(mathRequest, tutors).size() == 2);
    Student englishRequest = *s;
    englishRequest.setNeedSubjects({"Anh"});
    CHECK(MatchingService::matchStudentsToTutors(englishRequest, tutors).size() == 1);
    CHECK(!service.create(makeClass("LH00", "SV001", "", "Cần gia sư", "Sinh Lớp 10"), error));
    CHECK(service.create(makeClass("LH01", "SV001", "GS001", "Đang học", "Toán Lớp 10", "T2"), error));
    CHECK(service.hasAssignedSubject("SV001", "toan"));
    CHECK(!service.hasAssignedSubject("SV001", "anh"));
    CHECK(!service.create(makeClass("LH03", "SV001", "GS003", "Đang học", "Toán Lớp 10"), error));
    CHECK(service.create(makeClass("LH02", "SV001", "GS002", "Đang học", "Anh Lớp 10", "T4"), error));
    CHECK(s->getIsMatched());
    CHECK(s->getEnrolledClasses().size() == 2);
    CHECK(tutors[0]->getCurrentStudents() == 1 && tutors[1]->getCurrentStudents() == 1);
    CHECK(service.hasAssignedSubject("SV001", "anh"));

    TutoringClass mathClass = *classes[0];
    mathClass.setStatus("Hoàn thành");
    CHECK(service.update("LH01", mathClass, error));
    CHECK(s->getIsMatched()); // English is still assigned
    CHECK(tutors[0]->getCurrentStudents() == 0 && tutors[1]->getCurrentStudents() == 1);
    TutoringClass englishClass = *classes[1];
    englishClass.setStatus("Hoàn thành");
    CHECK(service.update("LH02", englishClass, error));
    CHECK(!s->getIsMatched());
    CHECK(tutors[0]->getCurrentStudents() == 0 && tutors[1]->getCurrentStudents() == 0);
    CHECK(!service.hasAssignedSubject("SV001", "toan"));
    CHECK(!service.canDeleteStudent("SV001")); // historical class still retained

    for (size_t i = 0; i < students.size(); ++i) delete students[i];
    for (size_t i = 0; i < tutors.size(); ++i) delete tutors[i];
    for (size_t i = 0; i < classes.size(); ++i) delete classes[i];
    fs::remove_all(dir);
}

// Legacy 11-column class files remain importable, but new files have no end date.
static void testClassDataMigration() {
    section("FileManager: class schema v2 and v1 backward compatibility");
    namespace fs = filesystem;
    const fs::path dir = fs::temp_directory_path() / "pbl2_test_class_migration";
    fs::remove_all(dir);
    fs::create_directories(dir);
    FileManager::setDataDirectory(dir.string());
    const string legacy =
        "LH001|SV001|GS001|Toan Lop 10|T2,T4|01/09/2026|31/12/2026|Đang học|150000|01/09/2026|01/09/2026\n";
    {
        ofstream out(dir / "class.txt", ios::binary | ios::trunc);
        out << legacy;
    }
    Vector<TutoringClass*> classes = FileManager::loadClasses();
    CHECK(classes.size() == 1);
    if (classes.size() == 1) {
        CHECK(classes[0]->getStatus() == "Đang học");
        CHECK(classes[0]->getRatePerHour() == "150000");
        CHECK(classes[0]->getStartDate() == "01/09/2026");
    }
    FileManager::saveClasses(classes);
    string line;
    { ifstream in(dir / "class.txt"); getline(in, line); }
    CHECK(FileManager::splitString(line, '|').size() == 10);
    CHECK(line.find("31/12/2026") == string::npos);
    Vector<TutoringClass*> fresh = FileManager::loadClasses();
    CHECK(fresh.size() == 1);
    if (fresh.size() == 1) {
        CHECK(fresh[0]->getStatus() == "Đang học");
        CHECK(fresh[0]->getRatePerHour() == "150000");
    }
    for (size_t i = 0; i < classes.size(); ++i) delete classes[i];
    for (size_t i = 0; i < fresh.size(); ++i) delete fresh[i];
    fs::remove_all(dir);
}


static void testSubjectAndScheduleRules() {
    section("Môn chính xác + lịch rảnh + chống đặt trùng ngày");
    CHECK(SubjectUtils::equals("Tiếng Anh", "Anh"));
    CHECK(SubjectUtils::equals("Hóa học", "Hoa"));
    CHECK(!SubjectUtils::equals("Anh", "Anh nâng cao"));
    CHECK(SubjectUtils::parseClassSubjects("10 - Toán, Lý").size() == 2);
    CHECK(SubjectUtils::parseClassSubjects("Toán Lớp 10").size() == 1);
    CHECK(!SubjectUtils::contains(SubjectUtils::parseClassSubjects("Anh văn 2 Lớp 10"), "Anh"));
    CHECK(ScheduleUtils::valid("T2,T4,CN", false));
    CHECK(!ScheduleUtils::valid("T2,T9", false));
    CHECK(!ScheduleUtils::valid("T2,,T3", false));
    CHECK(!ScheduleUtils::valid("", false));
    CHECK(ScheduleUtils::intersects("T2,T4", "T4,T6"));
    CHECK(!ScheduleUtils::intersects("T2", "T3"));
    CHECK(ScheduleUtils::isSubset("T2,T4", "T2,T4,T5"));
    CHECK(!ScheduleUtils::isSubset("T2,T4", "T2,T5"));

    namespace fs = filesystem;
    const fs::path dir = fs::temp_directory_path() / "tutorflow_conflict_rules";
    fs::remove_all(dir);
    fs::create_directories(dir);
    FileManager::setDataDirectory(dir.string());
    auto* a = new Student(makeStudent({"Toán", "Anh"}, "Hai Chau", "T2,T4,T6", "300000"));
    a->setPersonID("SV101");
    auto* b = new Student(makeStudent({"Toán"}, "Hai Chau", "T2,T4,T6", "300000"));
    b->setPersonID("SV102");
    Vector<Student*> students = {a, b};
    Vector<Tutor*> tutors = {
        makeTutor("GS101", "Nam", {"Toán", "Anh"}, 6, "150000", "T2,T4,T6", {"Hai Chau"}),
        makeTutor("GS102", "Nữ", {"Toán", "Anh"}, 6, "150000", "T2,T4,T6", {"Hai Chau"})
    };
    Vector<TutoringClass*> classes;
    ClassService service(students, tutors, classes);
    string error;
    CHECK(service.create(makeClass("LH101", "SV101", "GS101", "Đang học", "Toán Lớp 10", "T2"), error));
    CHECK(!service.create(makeClass("LH102", "SV102", "GS101", "Đang học", "Toán Lớp 10", "T2"), error)); // same tutor
    CHECK(service.create(makeClass("LH102", "SV102", "GS101", "Đang học", "Toán Lớp 10", "T4"), error));
    CHECK(!service.create(makeClass("LH103", "SV101", "GS102", "Đang học", "Anh Lớp 10", "T2"), error)); // same student
    CHECK(!service.create(makeClass("LH103", "SV101", "GS102", "Đang học", "Toán Lớp 10", "T6"), error)); // same subject
    CHECK(service.create(makeClass("LH103", "SV101", "GS102", "Đang học", "Anh Lớp 10", "T4"), error));
    CHECK(!service.create(makeClass("LH104", "SV102", "GS102", "Đang học", "Toán Lớp 10", "T3"), error)); // outside availability
    CHECK(service.availableSlots(*a, *tutors[0]).size() == 1); // T6 only; both occupied T2/T4
    CHECK(!service.hasAssignedSubject("SV101", "Anh nâng cao"));
    CHECK(service.hasAssignedSubject("SV101", "Tiếng Anh"));

    Student alteredStudent = *a;
    alteredStudent.setNeedSubjects({"Toán"});
    CHECK(!service.updateStudent(*a, alteredStudent, error)); // cannot remove English with live class
    CHECK(a->getNeedSubjects().size() == 2);
    alteredStudent = *a;
    alteredStudent.setAvailableSchedule("T6");
    CHECK(!service.updateStudent(*a, alteredStudent, error)); // cannot drop scheduled days
    CHECK(a->getAvailableSchedule() == "T2,T4,T6");
    alteredStudent = *a;
    alteredStudent.setBudgetPerHour("100000");
    CHECK(!service.updateStudent(*a, alteredStudent, error)); // lower than class fee
    alteredStudent = *a;
    alteredStudent.setFullName("Học sinh đã chỉnh");
    CHECK(service.updateStudent(*a, alteredStudent, error));
    CHECK(a->getFullName() == "Học sinh đã chỉnh");

    Tutor alteredTutor = *tutors[0];
    alteredTutor.setSubjects({"Anh"});
    CHECK(!service.updateTutor(*tutors[0], alteredTutor, error));
    CHECK(SubjectUtils::contains(tutors[0]->getSubjects(), "Toán"));
    alteredTutor = *tutors[0];
    alteredTutor.setAvailability("T6");
    CHECK(!service.updateTutor(*tutors[0], alteredTutor, error));
    alteredTutor = *tutors[0];
    alteredTutor.setIsAvailable(false); // no NEW students, existing classes unaffected
    CHECK(service.updateTutor(*tutors[0], alteredTutor, error));
    CHECK(!tutors[0]->getIsAvailable());
    CHECK(service.countInvalidClasses() == 0);

    for (auto* item : classes) delete item;
    for (auto* item : tutors) delete item;
    for (auto* item : students) delete item;
    fs::remove_all(dir);
}

static void testTransactionJournalRecovery() {
    section("Giao dịch nhiều file + khôi phục sau khi bị ngắt");
    namespace fs = filesystem;
    const fs::path dir = fs::temp_directory_path() / "tutorflow_txn_recovery";
    fs::remove_all(dir);
    fs::create_directories(dir);
    FileManager::setDataDirectory(dir.string());
    const char* names[] = {"student.txt", "tutor.txt", "class.txt"};
    for (const auto* name : names) {
        ofstream out(dir / name);
        out << "old-" << name << "\n";
        ofstream backup(dir / (string(name) + ".backup"));
        backup << "old-" << name << "\n";
    }
    // Emulate a failure after replacing the first two files but BEFORE COMMITTED.
    { ofstream out(dir / "student.txt"); out << "new-student\n"; }
    { ofstream out(dir / "tutor.txt"); out << "new-tutor\n"; }
    { ofstream journal(dir / ".tutorflow_pending"); journal << "v1111\n"; }
    CHECK(FileManager::recoverIfNeeded());
    for (const auto* name : names) {
        string line;
        ifstream in(dir / name); getline(in, line);
        CHECK(line == string("old-") + name);
        CHECK(!fs::exists(dir / (string(name) + ".backup")));
    }
    CHECK(!fs::exists(dir / ".tutorflow_pending"));

    // COMMITTED exists => do not undo the transaction, just remove backup.
    for (const auto* name : names) {
        { ofstream out(dir / name); out << "new-" << name << "\n"; }
        { ofstream backup(dir / (string(name) + ".backup")); backup << "old-" << name << "\n"; }
    }
    { ofstream journal(dir / ".tutorflow_pending"); journal << "v1111\n"; }
    { ofstream journal(dir / ".tutorflow_committed"); journal << "committed\n"; }
    CHECK(FileManager::recoverIfNeeded());
    for (const auto* name : names) {
        string line; ifstream in(dir / name); getline(in, line);
        CHECK(line == string("new-") + name);
    }
    CHECK(!fs::exists(dir / ".tutorflow_pending"));

    // Broken journal/backups => refuse to overwrite or remove existing data.
    { ofstream journal(dir / ".tutorflow_pending"); journal << "v1111\n"; }
    CHECK(!FileManager::recoverIfNeeded());
    CHECK(!FileManager::recoveryError().empty());
    CHECK(fs::exists(dir / "student.txt"));
    fs::remove_all(dir);
}

static void testCleanSampleData() {
    section("Sample data: no broken references or duplicate active assignments");
    namespace fs = filesystem;
    const fs::path data = fs::path(__FILE__).parent_path().parent_path() / "data";
    CHECK(fs::exists(data / "class.txt"));
    FileManager::setDataDirectory(data.string());
    Vector<Student*> students = FileManager::loadStudents();
    Vector<Tutor*> tutors = FileManager::loadTutors();
    Vector<TutoringClass*> classes = FileManager::loadClasses();
    CHECK(FileManager::storageError().empty());  // bundled data must remain writable
    ClassService service(students, tutors, classes);
    service.synchronize();
    CHECK(classes.size() == 22);
    CHECK(service.countDuplicateAssignments() == 0);
    CHECK(service.countBrokenReferences() == 0);
    CHECK(service.countInvalidClasses() == 0);
    for (size_t i = 0; i < students.size(); ++i) delete students[i];
    for (size_t i = 0; i < tutors.size(); ++i) delete tutors[i];
    for (size_t i = 0; i < classes.size(); ++i) delete classes[i];
}

static void testLegacyProfileColumnCompatibility() {
    section("Profile TXT compatibility: ignore legacy trailing columns");
    namespace fs = filesystem;
    const fs::path dir = fs::temp_directory_path() / "tutorflow_profile_compatibility_20261110";
    fs::remove_all(dir);
    fs::create_directories(dir);
    FileManager::setDataDirectory(dir.string());

    Student student = makeStudent({"Toan"}, "Hai Chau", "T2", "200000");
    student.setPersonID("SV731");
    Tutor* tutor = makeTutor("GS731", "Nam", {"Toan"}, 4, "120000", "T2", {"Hai Chau"});
    const string studentRecord = student.toString();
    const string tutorRecord = tutor->toString();
    {
        ofstream out(dir / "student.txt", ios::binary);
        out << studentRecord << "|legacy-extra\n";
    }
    {
        ofstream out(dir / "tutor.txt", ios::binary);
        out << tutorRecord << "|legacy-extra\n";
    }
    { ofstream out(dir / "class.txt"); }
    delete tutor;

    Vector<Student*> learners = FileManager::loadStudents();
    Vector<Tutor*> teachers = FileManager::loadTutors();
    Vector<TutoringClass*> classes = FileManager::loadClasses();
    CHECK(learners.size() == 1);
    CHECK(teachers.size() == 1);
    if (learners.size() == 1 && teachers.size() == 1) {
        CHECK(learners[0]->toString() == studentRecord);
        CHECK(teachers[0]->toString() == tutorRecord);
        CHECK(FileManager::saveAll(learners, teachers, classes));
    }
    for (auto* p : learners) delete p;
    for (auto* p : teachers) delete p;
    learners = FileManager::loadStudents();
    teachers = FileManager::loadTutors();
    CHECK(learners.size() == 1);
    CHECK(teachers.size() == 1);
    if (learners.size() == 1 && teachers.size() == 1) {
        CHECK(learners[0]->toString() == studentRecord);
        CHECK(teachers[0]->toString() == tutorRecord);
    }
    for (auto* p : learners) delete p;
    for (auto* p : teachers) delete p;
    fs::remove_all(dir);
}

static void testContractModule() {
    section("PBL integration: three contract types, payments, revenue, integrity");
    namespace fs = filesystem;
    const fs::path dir = fs::temp_directory_path() / "tutorflow_contracts_test_20261110";
    fs::remove_all(dir);
    fs::create_directories(dir);
    FileManager::setDataDirectory(dir.string());

    Vector<Student*> students;
    auto* s = new Student(makeStudent({"Toan"}, "Hai Chau", "T2,T4", "200000"));
    s->setPersonID("SV710");
    students.push_back(s);
    auto* otherStudent = new Student(makeStudent({"Ly"}, "Hai Chau", "T3", "200000"));
    otherStudent->setPersonID("SV711");
    students.push_back(otherStudent);
    Vector<Tutor*> tutors;
    tutors.push_back(makeTutor("GS710", "Nam", {"Toan"}, 4, "150000", "T2,T4", {"Hai Chau"}));
    Vector<TutoringClass*> classes;
    classes.push_back(new TutoringClass("LH710", "SV710", "GS710", "Toan", "T2",
        "01/11/2026", "Đang học", "150000", "01/11/2026", "01/11/2026"));

    ContractService contracts(students, tutors, classes);
    CHECK(contracts.contracts().empty());
    CHECK(contracts.totalRevenue() == 0);
    ContractRecord item;
    item.type = "CLASS";
    item.referenceId = "LH710";
    item.issuedDate = "01/11/2026";
    item.hourlyRate = 150000;
    item.commission = 400000;
    item.notes = "Hop dong lop Toan";
    string error;
    CHECK(contracts.create(item, error));
    CHECK(contracts.contracts().size() == 1);
    const string classContract = contracts.contracts()[0].id;
    CHECK(classContract == "CTC1");
    CHECK(contracts.outstanding(classContract) == 400000);
    CHECK(!contracts.create(item, error));  // one agreement per class
    CHECK(contracts.hasReference("CLASS", "LH710"));
    CHECK(ContractService::hasSavedReference("CLASS", "LH710"));
    CHECK(!ClassService(students, tutors, classes).remove("LH710", error));
    CHECK(classes.size() == 1);

    item.type="STUDENT"; item.referenceId="SV711"; item.hourlyRate=0;
    item.commission=50000; item.notes="Hop dong hoc vien";
    CHECK(contracts.create(item,error));
    CHECK(contracts.contracts().size()==2);
    CHECK(contracts.contracts()[1].id=="CTS1");
    CHECK(!ClassService(students,tutors,classes).canDeleteStudent("SV711"));
    CHECK(contracts.hasReference("STUDENT","SV711"));

    item.type="TUTOR"; item.referenceId="GS710"; item.commission=0;
    CHECK(contracts.create(item,error));
    CHECK(contracts.contracts().size()==3);
    CHECK(contracts.contracts()[2].id=="CTT1");
    CHECK(!ClassService(students,tutors,classes).canDeleteTutor("GS710"));

    ContractRecord invalid = item;
    invalid.referenceId="GS_missing";
    CHECK(!contracts.create(invalid,error));
    invalid.referenceId="GS710";
    invalid.issuedDate="31/02/2026";
    CHECK(!contracts.create(invalid,error));

    PaymentRecord payment;
    payment.contractId=classContract;
    payment.paidDate="02/11/2026";
    payment.amount=150000;
    payment.notes="Chuyen khoan";
    CHECK(contracts.addPayment(payment,error));
    CHECK(contracts.payments().size()==1);
    CHECK(contracts.totalPaid(classContract)==150000);
    CHECK(contracts.outstanding(classContract)==250000);
    CHECK(contracts.totalRevenue()==150000);
    CHECK(contracts.revenueByMonth(2026,11)==150000);
    CHECK(contracts.revenueByMonth(2026,10)==0);
    CHECK(!contracts.remove(classContract,error));  // ledger preserved

    payment.amount=300000;
    CHECK(!contracts.addPayment(payment,error));  // exceeds due
    payment.amount=250000;
    payment.paidDate="31/10/2026";
    CHECK(!contracts.addPayment(payment,error));  // before issuance
    payment.paidDate="03/11/2026";
    CHECK(contracts.addPayment(payment,error));
    CHECK(contracts.outstanding(classContract)==0);
    CHECK(contracts.totalRevenue()==400000);
    CHECK(contracts.revenueByMonth(2026,11)==400000);
    CHECK(!contracts.addPayment(payment,error)); // paid in full

    ContractRecord change=*contracts.find(classContract);
    change.commission=300000;
    CHECK(!contracts.update(classContract,change,error)); // may not reduce below paid
    change.commission=400000;
    change.referenceId="LH777";
    CHECK(!contracts.update(classContract,change,error)); // immutable link
    CHECK(contracts.cancel(classContract,error));
    CHECK(!contracts.find(classContract)->active);
    CHECK(!contracts.addPayment(payment,error)); // cancelled
    change=*contracts.find(classContract);
    change.active=true;
    CHECK(!contracts.update(classContract,change,error));

    ContractService reopened(students,tutors,classes);
    CHECK(reopened.contracts().size()==3);
    CHECK(reopened.payments().size()==2);
    CHECK(reopened.totalRevenue()==400000);
    CHECK(reopened.find(classContract) && !reopened.find(classContract)->active);
    CHECK(reopened.remove("CTS1",error)); // no payment: can remove
    CHECK(!reopened.hasReference("STUDENT","SV711"));
    CHECK(ClassService(students,tutors,classes).canDeleteStudent("SV711"));

    // Bad legacy/corrupt data must not be silently overwritten.
    { ofstream f(dir / "contracts.txt",ios::trunc);f << "C|bad|record\n"; }
    ContractService corrupt(students,tutors,classes);
    CHECK(!corrupt.reload(error));
    CHECK(!corrupt.create(item,error));
    {
        ifstream f(dir / "contracts.txt");string raw;
        getline(f,raw);CHECK(raw=="C|bad|record");
    }

    for(size_t i=0;i<students.size();++i) delete students[i];
    for(size_t i=0;i<tutors.size();++i) delete tutors[i];
    for(size_t i=0;i<classes.size();++i) delete classes[i];
    fs::remove_all(dir);
    FileManager::setDataDirectory("");
}


// Regression: a corrupt line must never be dropped and replaced by partial data.
static void testFailClosedStorageAndOrdering() {
    section("Safety regression: corrupt TXT, duplicate IDs and rollback ordering");
    namespace fs = filesystem;
    const fs::path dir = fs::temp_directory_path() / "tutorflow_fail_closed_regression";
    fs::remove_all(dir);
    fs::create_directories(dir);
    FileManager::setDataDirectory(dir.string());
    Vector<Student*> students;
    for (int i = 1; i <= 2; ++i) {
        auto* s = new Student(makeStudent({"Toan"}, "Hai Chau", "T2,T4", "300000"));
        s->setPersonID("SV" + to_string(i));
        students.push_back(s);
    }
    Vector<Tutor*> tutors;
    tutors.push_back(makeTutor("GS1", "Nam", {"Toan"}, 4, "150000", "T2,T4", {"Hai Chau"}));
    Vector<TutoringClass*> classes;
    ClassService service(students, tutors, classes);
    string error;
    CHECK(service.create(makeClass("LH1", "SV1", "GS1", "Đang học", "Toan", "T2"),error));
    CHECK(service.create(makeClass("LH2", "SV2", "GS1", "Đang học", "Toan", "T4"),error));

    auto contents = [](const fs::path& path) {
        ifstream in(path, ios::binary);
        return string((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    };
    // Existing insertion order must remain unchanged after a failed transaction.
    FileManager::setDataDirectory((dir / "missing" / "nested").string());
    CHECK(!service.remove("LH1",error));
    CHECK(classes.size() == 2 && classes[0]->getClassID() == "LH1" &&
          classes[1]->getClassID() == "LH2");
    FileManager::setDataDirectory(dir.string());

    const string originalStudents = contents(dir / "student.txt");
    const string originalTutors = contents(dir / "tutor.txt");
    const string originalClasses = contents(dir / "class.txt");
    { ofstream out(dir / "student.txt", ios::app); out << "INVALID|ROW\n"; }
    const string corruptStudents = contents(dir / "student.txt");
    auto partiallyLoaded = FileManager::loadStudents();
    CHECK(partiallyLoaded.size() == 2);
    CHECK(!FileManager::storageError().empty());
    CHECK(!FileManager::saveAll(students, tutors, classes));
    FileManager::saveStudents(students);
    CHECK(contents(dir / "student.txt") == corruptStudents);
    for (auto* student : partiallyLoaded) delete student;

    // Same protection for a duplicate identifier rather than malformed columns.
    { ofstream out(dir / "student.txt", ios::binary|ios::trunc);
      out << originalStudents << students[0]->toString() << "\n"; }
    FileManager::setDataDirectory(dir.string());
    partiallyLoaded = FileManager::loadStudents();
    CHECK(partiallyLoaded.size() == 2);
    CHECK(!FileManager::storageError().empty());
    CHECK(!FileManager::saveAll(students, tutors, classes));
    for (auto* student : partiallyLoaded) delete student;

    // An invalid tutor row cannot disappear during a later save.
    { ofstream out(dir / "student.txt", ios::binary|ios::trunc); out << originalStudents; }
    { ofstream out(dir / "tutor.txt", ios::app); out << "GS999|BAD\n"; }
    const string corruptTutors = contents(dir / "tutor.txt");
    FileManager::setDataDirectory(dir.string());
    auto partialTutors = FileManager::loadTutors();
    CHECK(partialTutors.size() == 1);
    CHECK(!FileManager::storageError().empty());
    CHECK(!FileManager::saveAll(students, tutors, classes));
    CHECK(contents(dir / "tutor.txt") == corruptTutors);
    for (auto* tutor : partialTutors) delete tutor;

    // An invalid class row cannot disappear during a later save.
    { ofstream out(dir / "tutor.txt", ios::binary|ios::trunc); out << originalTutors; }
    { ofstream out(dir / "class.txt", ios::app); out << "LH999|BAD\n"; }
    const string corruptClasses = contents(dir / "class.txt");
    FileManager::setDataDirectory(dir.string());
    auto partialClasses = FileManager::loadClasses();
    CHECK(partialClasses.size() == 2);
    CHECK(!FileManager::storageError().empty());
    CHECK(!FileManager::saveAll(students, tutors, classes));
    CHECK(contents(dir / "class.txt") == corruptClasses);
    for (auto* cls : partialClasses) delete cls;

    // Restoring the source bytes and reopening clears the deliberate read-only lock.
    { ofstream out(dir / "class.txt", ios::binary|ios::trunc); out << originalClasses; }
    FileManager::setDataDirectory(dir.string());
    auto recovered = FileManager::loadClasses();
    CHECK(recovered.size() == 2 && FileManager::storageError().empty());
    CHECK(FileManager::saveAll(students, tutors, classes));
    for (auto* cls : recovered) delete cls;
    for (auto* cls : classes) delete cls;
    for (auto* tutor : tutors) delete tutor;
    for (auto* student : students) delete student;
    fs::remove_all(dir);
    FileManager::setDataDirectory("");
}

static void testContractFailureRollbackAndFailClosedReference() {
    section("Contracts regression: preserve order and block corrupt references");
    namespace fs = filesystem;
    const fs::path dir = fs::temp_directory_path() / "tutorflow_contract_rollback_regression";
    fs::remove_all(dir);
    fs::create_directories(dir);
    FileManager::setDataDirectory(dir.string());
    Vector<Student*> students;
    auto* s = new Student(makeStudent({"Toan"}, "Hai Chau", "T2", "200000"));
    s->setPersonID("SV1");
    students.push_back(s);
    Vector<Tutor*> tutors;
    Vector<TutoringClass*> classes;
    ContractService contracts(students,tutors,classes);
    ContractRecord c;
    c.type="STUDENT"; c.referenceId="SV1"; c.issuedDate="01/11/2026";
    c.commission=100000;
    string error;
    CHECK(contracts.create(c,error));
    CHECK(contracts.create(c,error));
    CHECK(contracts.create(c,error));
    CHECK(contracts.contracts().size()==3);
    CHECK(contracts.contracts()[0].id=="CTS1" && contracts.contracts()[1].id=="CTS2" &&
          contracts.contracts()[2].id=="CTS3");
    FileManager::setDataDirectory((dir / "missing").string());
    CHECK(!contracts.remove("CTS1",error));
    CHECK(contracts.contracts().size()==3);
    CHECK(contracts.contracts()[0].id=="CTS1" && contracts.contracts()[1].id=="CTS2" &&
          contracts.contracts()[2].id=="CTS3");
    FileManager::setDataDirectory(dir.string());
    CHECK(ContractService::hasSavedReference("STUDENT","SV1"));

    // A row with the right number of fields but an invalid contract ID cannot
    // be treated as a clean ledger and used to authorize deleting a student.
    { ofstream out(dir / "contracts.txt",ios::trunc);
      out << "# corrupt-but-nine-columns\n"
          << "C|BAD_ID|STUDENT|SV1|01/11/2026|0|100000|1|note\n"; }
    CHECK(ContractService::hasSavedReference("STUDENT","SV1"));
    CHECK(ContractService::hasSavedReference("STUDENT","OTHER")); // fail closed
    CHECK(!ClassService(students,tutors,classes).canDeleteStudent("SV1"));
    for (auto* student : students) delete student;
    fs::remove_all(dir);
    FileManager::setDataDirectory("");
}


static void testImplicitDirectoryNeverMixesDatasets() {
    section("Storage regression: implicit data directory is consistent");
    namespace fs = filesystem;
    const auto originalWorking = fs::current_path();
    const auto root = fs::temp_directory_path() / "tutorflow_directory_consistency";
    fs::remove_all(root);
    fs::create_directories(root / "data");
    fs::create_directories(root / "nested" / "data");
    { ofstream out(root / "data" / "tutor.txt"); out << "other-dataset"; }
    { ofstream out(root / "nested" / "data" / "student.txt"); out << "local-dataset"; }
    FileManager::setDataDirectory("");
    fs::current_path(root / "nested");
    CHECK(fs::path(FileManager::dataPath("student.txt")).parent_path() ==
          fs::path(FileManager::dataPath("tutor.txt")).parent_path());
    CHECK(fs::path(FileManager::dataPath("tutor.txt")).parent_path() ==
          fs::path(FileManager::dataPath("class.txt")).parent_path());
    CHECK(fs::path(FileManager::dataPath("tutor.txt")).parent_path() == fs::path("data"));
    fs::current_path(originalWorking);
    FileManager::setDataDirectory("");
    fs::remove_all(root);
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
    testRankedCandidateChoice();
    testClassBusinessLogic();
    testClassCapacityAndRollback();
    testSubjectRegistrationAndMatching();
    testFileRoundTrip();
    testAdminLogin();
    testClassDataMigration();
    testContractModule();
    testLegacyProfileColumnCompatibility();
    testCleanSampleData();
    testSubjectAndScheduleRules();
    testTransactionJournalRecovery();
    testFailClosedStorageAndOrdering();
    testContractFailureRollbackAndFailClosedReference();
    testImplicitDirectoryNeverMixesDatasets();

    cout << "\n" << (g_total - g_failed) << "/" << g_total << " kiem tra dat" << endl;
    if (g_failed > 0) {
        cerr << g_failed << " KIEM TRA THAT BAI" << endl;
        return 1;
    }
    cout << "TAT CA DEU DAT" << endl;
    return 0;
}
