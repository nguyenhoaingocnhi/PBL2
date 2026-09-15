#include "Student.h"

#include <iostream>
#include <sstream>

using namespace std;

Student::Student() = default;

Student::Student(const string& personID,
                 const string& fullName,
                 const string& phone,
                 const string& email,
                 const string& address,
                 const string& gender,
                 const string& dateOfBirth,
                 const string& grade,
                 const vector<string>& needSubjects,
                 const string& availableSchedule,
                 const string& learningLevel,
                 const string& preferredTutorGender,
                 const string& budgetPerHour,
                 const string& notes,
                 bool isMatched,
                 const vector<string>& enrolledClasses)
        : Person(personID, fullName, phone, email, address, gender, dateOfBirth),
            profile_{grade, needSubjects, availableSchedule, learningLevel,
                             preferredTutorGender, budgetPerHour, notes, isMatched, enrolledClasses} {}

void Student::displayInfo() const {
    cout << "Student: " << getPersonID() << " - " << getFullName() << endl;
}

string Student::toString() const {
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

const string& Student::getGrade() const {
    return profile_.grade;
}

const vector<string>& Student::getNeedSubjects() const {
    return profile_.needSubjects;
}

const string& Student::getAvailableSchedule() const {
    return profile_.availableSchedule;
}

const string& Student::getLearningLevel() const {
    return profile_.learningLevel;
}

const string& Student::getPreferredTutorGender() const {
    return profile_.preferredTutorGender;
}

const string& Student::getBudgetPerHour() const {
    return profile_.budgetPerHour;
}

const string& Student::getNotes() const {
    return profile_.notes;
}

bool Student::getIsMatched() const {
    return profile_.isMatched;
}

const vector<string>& Student::getEnrolledClasses() const {
    return profile_.enrolledClasses;
}

void Student::setGrade(const string& value) {
    profile_.grade = value;
}

void Student::setNeedSubjects(const vector<string>& value) {
    profile_.needSubjects = value;
}

void Student::setAvailableSchedule(const string& value) {
    profile_.availableSchedule = value;
}

void Student::setLearningLevel(const string& value) {
    profile_.learningLevel = value;
}

void Student::setPreferredTutorGender(const string& value) {
    profile_.preferredTutorGender = value;
}

void Student::setBudgetPerHour(const string& value) {
    profile_.budgetPerHour = value;
}

void Student::setNotes(const string& value) {
    profile_.notes = value;
}

void Student::setIsMatched(bool value) {
    profile_.isMatched = value;
}

void Student::setEnrolledClasses(const vector<string>& value) {
    profile_.enrolledClasses = value;
}

