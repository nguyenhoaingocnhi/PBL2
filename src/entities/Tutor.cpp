#include "Tutor.h"

#include <iostream>
#include <sstream>

Tutor::Tutor() = default;

Tutor::Tutor(const std::string& personID,
             const std::string& fullName,
             const std::string& phone,
             const std::string& email,
             const std::string& address,
             const std::string& gender,
             const std::string& dateOfBirth,
             const std::vector<std::string>& subjects,
             const std::string& degrees,
             int yearsOfExperience,
             const std::string& experienceDetails,
             const std::string& ratePerHour,
             const std::string& availability,
             bool isAvailable,
             int currentStudents,
             const std::vector<std::string>& teachingClasses,
             const std::vector<std::string>& teachingAreas,
             const std::string& achievements)
    : Person(personID, fullName, phone, email, address, gender, dateOfBirth),
      subjects(subjects),
      degrees(degrees),
      yearsOfExperience(yearsOfExperience),
      experienceDetails(experienceDetails),
      ratePerHour(ratePerHour),
      availability(availability),
      isAvailable(isAvailable),
      currentStudents(currentStudents),
      teachingClasses(teachingClasses),
      teachingAreas(teachingAreas),
      achievements(achievements) {}

void Tutor::displayInfo() const {
    std::cout << "Tutor: " << personID << " - " << fullName << std::endl;
}

std::string Tutor::toString() const {
    std::ostringstream oss;
    oss << personID << "|" << fullName << "|" << phone << "|" << email << "|" << address << "|"
        << gender << "|" << dateOfBirth << "|";

    for (std::size_t i = 0; i < subjects.size(); ++i) {
        if (i > 0) oss << ",";
        oss << subjects[i];
    }

    oss << "|" << degrees << "|" << yearsOfExperience << "|" << ratePerHour << "|" << availability << "|";
    for (std::size_t i = 0; i < teachingAreas.size(); ++i) {
        if (i > 0) oss << ",";
        oss << teachingAreas[i];
    }
    oss << "|" << (isAvailable ? "true" : "false") << "|" << currentStudents;
    return oss.str();
}

const std::vector<std::string>& Tutor::getSubjects() const { return subjects; }
const std::string& Tutor::getDegrees() const { return degrees; }
int Tutor::getYearsOfExperience() const { return yearsOfExperience; }
const std::string& Tutor::getExperienceDetails() const { return experienceDetails; }
const std::string& Tutor::getRatePerHour() const { return ratePerHour; }
const std::string& Tutor::getAvailability() const { return availability; }
bool Tutor::getIsAvailable() const { return isAvailable; }
int Tutor::getCurrentStudents() const { return currentStudents; }
const std::vector<std::string>& Tutor::getTeachingClasses() const { return teachingClasses; }
const std::vector<std::string>& Tutor::getTeachingAreas() const { return teachingAreas; }
const std::string& Tutor::getAchievements() const { return achievements; }

void Tutor::setSubjects(const std::vector<std::string>& value) { subjects = value; }
void Tutor::setDegrees(const std::string& value) { degrees = value; }
void Tutor::setYearsOfExperience(int value) { yearsOfExperience = value; }
void Tutor::setExperienceDetails(const std::string& value) { experienceDetails = value; }
void Tutor::setRatePerHour(const std::string& value) { ratePerHour = value; }
void Tutor::setAvailability(const std::string& value) { availability = value; }
void Tutor::setIsAvailable(bool value) { isAvailable = value; }
void Tutor::setCurrentStudents(int value) { currentStudents = value; }
void Tutor::setTeachingClasses(const std::vector<std::string>& value) { teachingClasses = value; }
void Tutor::setTeachingAreas(const std::vector<std::string>& value) { teachingAreas = value; }
void Tutor::setAchievements(const std::string& value) { achievements = value; }
