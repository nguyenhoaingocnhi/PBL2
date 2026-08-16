#include "Student.h"

#include <iostream>
#include <sstream>

Student::Student() = default;

Student::Student(const std::string& personID,
                 const std::string& fullName,
                 const std::string& phone,
                 const std::string& email,
                 const std::string& address,
                 const std::string& gender,
                 const std::string& dateOfBirth,
                 const std::string& grade,
                 const std::vector<std::string>& needSubjects,
                 const std::string& availableSchedule,
                 const std::string& learningLevel,
                 const std::string& preferredTutorGender,
                 const std::string& budgetPerHour,
                 const std::string& notes,
                 bool isMatched,
                 const std::vector<std::string>& enrolledClasses)
    : Person(personID, fullName, phone, email, address, gender, dateOfBirth),
      grade(grade),
      needSubjects(needSubjects),
      availableSchedule(availableSchedule),
      learningLevel(learningLevel),
      preferredTutorGender(preferredTutorGender),
      budgetPerHour(budgetPerHour),
      notes(notes),
      isMatched(isMatched),
      enrolledClasses(enrolledClasses) {}

void Student::displayInfo() const {
    std::cout << "Student: " << personID << " - " << fullName << std::endl;
}

std::string Student::toString() const {
    std::ostringstream oss;
    oss << personID << "|" << fullName << "|" << phone << "|" << email << "|" << address << "|"
        << gender << "|" << dateOfBirth << "|";

    for (std::size_t i = 0; i < needSubjects.size(); ++i) {
        if (i > 0) oss << ",";
        oss << needSubjects[i];
    }

    oss << "|" << availableSchedule << "|" << learningLevel << "|" << preferredTutorGender << "|"
        << budgetPerHour << "|" << notes << "|" << (isMatched ? "true" : "false");
    return oss.str();
}

const std::string& Student::getGrade() const { return grade; }
const std::vector<std::string>& Student::getNeedSubjects() const { return needSubjects; }
const std::string& Student::getAvailableSchedule() const { return availableSchedule; }
const std::string& Student::getLearningLevel() const { return learningLevel; }
const std::string& Student::getPreferredTutorGender() const { return preferredTutorGender; }
const std::string& Student::getBudgetPerHour() const { return budgetPerHour; }
const std::string& Student::getNotes() const { return notes; }
bool Student::getIsMatched() const { return isMatched; }
const std::vector<std::string>& Student::getEnrolledClasses() const { return enrolledClasses; }

void Student::setGrade(const std::string& value) { grade = value; }
void Student::setNeedSubjects(const std::vector<std::string>& value) { needSubjects = value; }
void Student::setAvailableSchedule(const std::string& value) { availableSchedule = value; }
void Student::setLearningLevel(const std::string& value) { learningLevel = value; }
void Student::setPreferredTutorGender(const std::string& value) { preferredTutorGender = value; }
void Student::setBudgetPerHour(const std::string& value) { budgetPerHour = value; }
void Student::setNotes(const std::string& value) { notes = value; }
void Student::setIsMatched(bool value) { isMatched = value; }
void Student::setEnrolledClasses(const std::vector<std::string>& value) { enrolledClasses = value; }
