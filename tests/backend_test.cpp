#include "Student.h"
#include "Tutor.h"
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

    cout << "Student count: " << students.size() << endl;
    cout << "Tutor count: " << tutors.size() << endl;

    if (!students.empty()) {
        cout << "Student found: " << students[0]->getPersonID() << " - " << students[0]->getFullName() << endl;
    }

    if (!tutors.empty()) {
        cout << "Tutor found: " << tutors[0]->getPersonID() << " - " << tutors[0]->getFullName() << endl;
    }

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

    return 0;
}
