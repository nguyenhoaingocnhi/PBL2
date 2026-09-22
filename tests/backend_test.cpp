#include "Student.h"
#include "Tutor.h"
#include "Registration.h"
#include "FileManager.h"
#include "MatchingService.h"
#include "HashMap.h"
#include "Vector.h"

#include <iostream>
#include <vector>

using namespace std;

int main() {
    Vector<Student*> students = FileManager::loadStudents();
    Vector<Tutor*> tutors = FileManager::loadTutors();
    Vector<Registration*> registrations = FileManager::loadRegistrations();

    cout << "Student count: " << students.size() << endl;
    cout << "Tutor count: " << tutors.size() << endl;
    cout << "Registration count from file: " << registrations.size() << endl;

    if (!students.empty()) {
        cout << "Student found: " << students[0]->getPersonID() << " - " << students[0]->getFullName() << endl;
    }

    if (!tutors.empty()) {
        cout << "Tutor found: " << tutors[0]->getPersonID() << " - " << tutors[0]->getFullName() << endl;
    }

    cout << "\n--- TEST DANG KY NHIEU MON HOC CHO 1 SINH VIEN ---" << endl;
    // Tạo 1 phiếu đăng ký cho sinh viên SV001 với môn ban đầu là Toán
    Registration reg("DK099", "SV001", "Nguyen Minh Anh", {"Toan"}, "22/09/2026", "Cho ghep", "Hoc offline");
    cout << "Ban dau dang ky: " << reg.getSubjectCount() << " mon (" << reg.getSubjectsSummary() << ")" << endl;

    // Sinh viên đăng ký thêm nhiều môn học khác
    reg.addSubject("Vat Ly");
    reg.addSubject("Hoa Hoc");
    reg.addSubject("Tieng Anh");
    // Thêm lại môn Toan -> tu dong khong bi trung
    reg.addSubject("Toan");

    cout << "Sau khi dang ky them nhieu mon: " << reg.getSubjectCount() << " mon (" << reg.getSubjectsSummary() << ")" << endl;
    reg.displayInfo();

    // Sinh viên hủy 1 môn học
    reg.removeSubject("Hoa Hoc");
    cout << "Sau khi huy mon Hoa Hoc con: " << reg.getSubjectCount() << " mon (" << reg.getSubjectsSummary() << ")" << endl;

    // Kiểm tra đăng ký môn học trực tiếp trên đối tượng Student
    if (!students.empty()) {
        cout << "\n--- TEST DANG KY MON TRUC TIEP TREN STUDENT ---" << endl;
        students[0]->registerMultipleSubjects({"Lap Trinh C++", "Co So Du Lieu"});
        cout << "Danh sach mon SV001 can hoc: ";
        for (auto& s : students[0]->getRegisteredSubjects()) {
            cout << s << " | ";
        }
        cout << endl;
    }

    cout << "\n--- TEST HASHMAP & MATCHING SERVICE (KHONG DUNG CONST) ---" << endl;
    HashMap<string, Student*> studentMap;
    for (size_t i = 0; i < students.size(); ++i) {
        studentMap.insert(students[i]->getPersonID(), students[i]);
    }
    cout << "Contains SV001: " << studentMap.contains("SV001") << endl;

    HashMap<string, Tutor*> tutorMap;
    for (size_t i = 0; i < tutors.size(); ++i) {
        tutorMap.insert(tutors[i]->getPersonID(), tutors[i]);
    }
    cout << "Contains GS001: " << tutorMap.contains("GS001") << endl;

    if (!students.empty() && !tutors.empty()) {
        vector<Tutor*> tutorsVector;
        for (size_t i = 0; i < tutors.size(); ++i) {
            tutorsVector.push_back(tutors[i]);
        }
        auto matches = MatchingService::matchStudentsToTutors(*students[0], tutorsVector);
        cout << "Match count: " << matches.size() << endl;
        if (!matches.empty()) {
            cout << "Top match: " << matches[0].tutor->getPersonID() << " - " << matches[0].totalScore << endl;
        }
    }

    cout << "\n--- DANH SACH PHIEU DANG KY DOC TU FILE ---" << endl;
    for (size_t i = 0; i < registrations.size(); ++i) {
        registrations[i]->displayInfo();
    }

    return 0;
}
