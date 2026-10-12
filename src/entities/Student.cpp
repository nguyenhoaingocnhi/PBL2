#include "Student.h"

#include <iostream>
#include <sstream>

using namespace std;

Student::Student() = default;

Student::Student(string personID,
                 string fullName,
                 string phone,
                 string email,
                 string address,
                 string gender,
                 string dateOfBirth,
                 string grade,
                 Vector<string> needSubjects,
                 string availableSchedule,
                 string learningLevel,
                 string preferredTutorGender,
                 string budgetPerHour,
                 string notes,
                 bool isMatched,
                 Vector<string> enrolledClasses)
    : Person(personID, fullName, phone, email, address, gender, dateOfBirth),
      profile_{grade, needSubjects, availableSchedule, learningLevel,
               preferredTutorGender, budgetPerHour, notes, isMatched, enrolledClasses} {}

void Student::displayInfo() {
    cout << "Student: " << getPersonID() << " - " << getFullName() << endl;
}

string Student::toString() {
    ostringstream oss;
    oss << getPersonID() << "|" << getFullName() << "|" << getPhone() << "|" << getEmail() << "|"
        << getAddress() << "|" << getGender() << "|" << getDateOfBirth() << "|";

    for (size_t i = 0; i < profile_.needSubjects.size(); ++i) {
        if (i > 0) oss << ",";
        oss << profile_.needSubjects[i];
    }

    oss << "|" << profile_.availableSchedule << "|" << profile_.learningLevel << "|"
        << profile_.preferredTutorGender << "|" << profile_.budgetPerHour << "|"
        << profile_.notes << "|" << (profile_.isMatched ? "true" : "false");
    return oss.str();
}

string Student::getGrade() {
    return profile_.grade;
}

Vector<string>& Student::getNeedSubjects() {
    return profile_.needSubjects;
}

string Student::getAvailableSchedule() {
    return profile_.availableSchedule;
}

string Student::getLearningLevel() {
    return profile_.learningLevel;
}

string Student::getPreferredTutorGender() {
    return profile_.preferredTutorGender;
}

string Student::getBudgetPerHour() {
    return profile_.budgetPerHour;
}

string Student::getNotes() {
    return profile_.notes;
}

bool Student::getIsMatched() {
    return profile_.isMatched;
}

Vector<string>& Student::getEnrolledClasses() {
    return profile_.enrolledClasses;
}

void Student::setGrade(string value) {
    profile_.grade = value;
}

void Student::setNeedSubjects(Vector<string> value) {
    profile_.needSubjects = value;
}

void Student::setAvailableSchedule(string value) {
    profile_.availableSchedule = value;
}

void Student::setLearningLevel(string value) {
    profile_.learningLevel = value;
}

void Student::setPreferredTutorGender(string value) {
    profile_.preferredTutorGender = value;
}

void Student::setBudgetPerHour(string value) {
    profile_.budgetPerHour = value;
}

void Student::setNotes(string value) {
    profile_.notes = value;
}

void Student::setIsMatched(bool value) {
    profile_.isMatched = value;
}

void Student::setEnrolledClasses(Vector<string> value) {
    profile_.enrolledClasses = value;
}

// 1 sinh viên có thể đăng ký thêm môn học
void Student::registerSubject(string subject) {
    if (subject.empty()) return;
    for (size_t i = 0; i < profile_.needSubjects.size(); ++i) {
        if (profile_.needSubjects[i] == subject) {
            return;
        }
    }
    profile_.needSubjects.push_back(subject);
}

// 1 sinh viên có thể đăng ký nhiều môn học cùng lúc
void Student::registerMultipleSubjects(Vector<string> subjects) {
    for (auto& subj : subjects) {
        registerSubject(subj);
    }
}

// Hủy đăng ký môn học
bool Student::unregisterSubject(string subject) {
    for (size_t i = 0; i < profile_.needSubjects.size(); ++i) {
        if (profile_.needSubjects[i] == subject) {
            profile_.needSubjects.erase(i);
            return true;
        }
    }
    return false;
}

Vector<string>& Student::getRegisteredSubjects() {
    return profile_.needSubjects;
}
