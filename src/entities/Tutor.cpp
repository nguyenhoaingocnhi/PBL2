#include "Tutor.h"

#include <iostream>
#include <sstream>

using namespace std;

Tutor::Tutor() = default;

Tutor::Tutor(const string& personID,
             const string& fullName,
             const string& phone,
             const string& email,
             const string& address,
             const string& gender,
             const string& dateOfBirth,
             const vector<string>& subjects,
             const string& degrees,
             int yearsOfExperience,
             const string& experienceDetails,
             const string& ratePerHour,
             const string& availability,
             bool isAvailable,
             int currentStudents,
             const vector<string>& teachingClasses,
             const vector<string>& teachingAreas,
             const string& achievements)
    : Person(personID, fullName, phone, email, address, gender, dateOfBirth),
      profile_{subjects, degrees, yearsOfExperience, experienceDetails, ratePerHour,
               availability, isAvailable, currentStudents, teachingClasses, teachingAreas,
               achievements} {}

void Tutor::displayInfo() const {
    cout << "Tutor: " << getPersonID() << " - " << getFullName() << endl;
}

string Tutor::toString() const {
    ostringstream oss;
    oss << getPersonID() << "|" << getFullName() << "|" << getPhone() << "|" << getEmail() << "|"
        << getAddress() << "|" << getGender() << "|" << getDateOfBirth() << "|";

    for (size_t i = 0; i < profile_.subjects.size(); ++i) {
        if (i > 0) oss << ",";
        oss << profile_.subjects[i];
    }

    oss << "|" << profile_.degrees << "|" << profile_.yearsOfExperience << "|"
        << profile_.ratePerHour << "|" << profile_.availability << "|";
    for (size_t i = 0; i < profile_.teachingAreas.size(); ++i) {
        if (i > 0) oss << ",";
        oss << profile_.teachingAreas[i];
    }
    oss << "|" << (profile_.isAvailable ? "true" : "false") << "|" << profile_.currentStudents;
    return oss.str();
}

const vector<string>& Tutor::getSubjects() const {
    return profile_.subjects;
}

const string& Tutor::getDegrees() const {
    return profile_.degrees;
}

int Tutor::getYearsOfExperience() const {
    return profile_.yearsOfExperience;
}

const string& Tutor::getExperienceDetails() const {
    return profile_.experienceDetails;
}

const string& Tutor::getRatePerHour() const {
    return profile_.ratePerHour;
}

const string& Tutor::getAvailability() const {
    return profile_.availability;
}

bool Tutor::getIsAvailable() const {
    return profile_.isAvailable;
}

int Tutor::getCurrentStudents() const {
    return profile_.currentStudents;
}

const vector<string>& Tutor::getTeachingClasses() const {
    return profile_.teachingClasses;
}

const vector<string>& Tutor::getTeachingAreas() const {
    return profile_.teachingAreas;
}

const string& Tutor::getAchievements() const {
    return profile_.achievements;
}

void Tutor::setSubjects(const vector<string>& value) {
    profile_.subjects = value;
}

void Tutor::setDegrees(const string& value) {
    profile_.degrees = value;
}

void Tutor::setYearsOfExperience(int value) {
    profile_.yearsOfExperience = value;
}

void Tutor::setExperienceDetails(const string& value) {
    profile_.experienceDetails = value;
}

void Tutor::setRatePerHour(const string& value) {
    profile_.ratePerHour = value;
}

void Tutor::setAvailability(const string& value) {
    profile_.availability = value;
}

void Tutor::setIsAvailable(bool value) {
    profile_.isAvailable = value;
}

void Tutor::setCurrentStudents(int value) {
    profile_.currentStudents = value;
}

void Tutor::setTeachingClasses(const vector<string>& value) {
    profile_.teachingClasses = value;
}

void Tutor::setTeachingAreas(const vector<string>& value) {
    profile_.teachingAreas = value;
}

void Tutor::setAchievements(const string& value) {
    profile_.achievements = value;
}

