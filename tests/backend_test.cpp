#include "Student.h"
#include "Tutor.h"
#include "FileManager.h"
#include "MatchingService.h"
#include "HashMap.h"
#include "Vector.h"

#include <iostream>
#include <vector>

int main() {
    Vector<Student*> students = FileManager::loadStudents();
    Vector<Tutor*> tutors = FileManager::loadTutors();

    std::cout << "Student count: " << students.size() << std::endl;
    std::cout << "Tutor count: " << tutors.size() << std::endl;

    if (!students.empty()) {
        std::cout << "Student found: " << students[0]->getPersonID() << " - " << students[0]->getFullName() << std::endl;
    }

    if (!tutors.empty()) {
        std::cout << "Tutor found: " << tutors[0]->getPersonID() << " - " << tutors[0]->getFullName() << std::endl;
    }

    HashMap<std::string, Student*> studentMap;
    for (std::size_t i = 0; i < students.size(); ++i) {
        studentMap.insert(students[i]->getPersonID(), students[i]);
    }
    std::cout << "Contains SV001: " << studentMap.contains("SV001") << std::endl;

    HashMap<std::string, Tutor*> tutorMap;
    for (std::size_t i = 0; i < tutors.size(); ++i) {
        tutorMap.insert(tutors[i]->getPersonID(), tutors[i]);
    }
    std::cout << "Contains GS001: " << tutorMap.contains("GS001") << std::endl;

    if (!students.empty() && !tutors.empty()) {
        std::vector<Tutor*> tutorsVector;
        for (std::size_t i = 0; i < tutors.size(); ++i) {
            tutorsVector.push_back(tutors[i]);
        }
        auto matches = MatchingService::matchStudentsToTutors(*students[0], tutorsVector);
        std::cout << "Match count: " << matches.size() << std::endl;
        if (!matches.empty()) {
            std::cout << "Top match: " << matches[0].tutor->getPersonID() << " - " << matches[0].totalScore << std::endl;
        }
    }

    return 0;
}
