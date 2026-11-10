#include "Student.h"

#include "TextUtils.h"
#include "SubjectUtils.h"

#include <iostream>
#include <sstream>

using namespace std;

Student::Student() = default;

Student::Student(string personID,
                 string fullName,
                 string phone,
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
    : Person(personID, fullName, phone, address, gender, dateOfBirth),
      profile_{grade, needSubjects, availableSchedule, learningLevel,
               preferredTutorGender, budgetPerHour, notes, isMatched, enrolledClasses} {}

void Student::displayInfo() {
    cout << "Student: " << getPersonID() << " - " << getFullName() << endl;
}

string Student::toString() {
    // Thứ tự cột phải khớp với FileManager::loadStudents.
    ostringstream oss;
    oss << TextUtils::cleanField(getPersonID()) << "|" << TextUtils::cleanField(getFullName()) << "|"
        << TextUtils::cleanField(getPhone()) << "|"
        << TextUtils::cleanField(getAddress()) << "|" << TextUtils::cleanField(getGender()) << "|"
        << TextUtils::cleanField(getDateOfBirth()) << "|"
        << TextUtils::cleanField(profile_.grade) << "|";

    for (size_t i = 0; i < profile_.needSubjects.size(); ++i) {
        if (i > 0) oss << ",";
        oss << TextUtils::cleanField(profile_.needSubjects[i]);
    }

    oss << "|" << TextUtils::cleanField(profile_.availableSchedule) << "|"
        << TextUtils::cleanField(profile_.learningLevel) << "|"
        << TextUtils::cleanField(profile_.preferredTutorGender) << "|"
        << TextUtils::cleanField(profile_.budgetPerHour) << "|"
        << TextUtils::cleanField(profile_.notes) << "|" << (profile_.isMatched ? "true" : "false");
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

string Student::getPhoneNumber() {
    return getPhone();
}

string Student::getAcademicPerformance() {
    return getLearningLevel();
}

void Student::setPhoneNumber(string value) {
    setPhone(value);
}

void Student::setAcademicPerformance(string value) {
    setLearningLevel(value);
}

void Student::setGrade(string value) {
    profile_.grade = value;
}

void Student::setNeedSubjects(Vector<string> value) {
    profile_.needSubjects.clear();
    registerMultipleSubjects(value);  // remove duplicates across case / Vietnamese accents
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
    subject = TextUtils::trim(subject);
    const string normalized = SubjectUtils::canonical(subject);
    if (normalized.empty()) return;
    for (size_t i = 0; i < profile_.needSubjects.size(); ++i) {
        if (SubjectUtils::canonical(profile_.needSubjects[i]) == normalized) return;
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
    const string normalized = SubjectUtils::canonical(subject);
    for (size_t i = 0; i < profile_.needSubjects.size(); ++i) {
        if (SubjectUtils::canonical(profile_.needSubjects[i]) == normalized) {
            profile_.needSubjects.erase(i);
            return true;
        }
    }
    return false;
}

Vector<string>& Student::getRegisteredSubjects() {
    return profile_.needSubjects;
}
