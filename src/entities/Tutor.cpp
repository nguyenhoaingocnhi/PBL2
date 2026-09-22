#include "Tutor.h"

#include <iostream>
#include <sstream>

using namespace std;

Tutor::Tutor() = default;

Tutor::Tutor(string personID,
             string fullName,
             string phone,
             string email,
             string address,
             string gender,
             string dateOfBirth,
             vector<string> subjects,
             string degrees,
             int yearsOfExperience,
             string experienceDetails,
             string ratePerHour,
             string availability,
             bool isAvailable,
             int currentStudents,
             vector<string> teachingClasses,
             vector<string> teachingAreas,
             string achievements)
    : Person(personID, fullName, phone, email, address, gender, dateOfBirth),
      profile_{subjects, degrees, yearsOfExperience, experienceDetails, ratePerHour,
               availability, isAvailable, currentStudents, teachingClasses, teachingAreas,
               achievements} {}

void Tutor::displayInfo() {
    cout << "Tutor: " << getPersonID() << " - " << getFullName() << endl;
}

string Tutor::toString() {
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

vector<string>& Tutor::getSubjects() {
    return profile_.subjects;
}

string Tutor::getDegrees() {
    return profile_.degrees;
}

int Tutor::getYearsOfExperience() {
    return profile_.yearsOfExperience;
}

string Tutor::getExperienceDetails() {
    return profile_.experienceDetails;
}

string Tutor::getRatePerHour() {
    return profile_.ratePerHour;
}

string Tutor::getAvailability() {
    return profile_.availability;
}

bool Tutor::getIsAvailable() {
    return profile_.isAvailable;
}

int Tutor::getCurrentStudents() {
    return profile_.currentStudents;
}

vector<string>& Tutor::getTeachingClasses() {
    return profile_.teachingClasses;
}

vector<string>& Tutor::getTeachingAreas() {
    return profile_.teachingAreas;
}

string Tutor::getAchievements() {
    return profile_.achievements;
}

void Tutor::setSubjects(vector<string> value) {
    profile_.subjects = value;
}

void Tutor::setDegrees(string value) {
    profile_.degrees = value;
}

void Tutor::setYearsOfExperience(int value) {
    profile_.yearsOfExperience = value;
}

void Tutor::setExperienceDetails(string value) {
    profile_.experienceDetails = value;
}

void Tutor::setRatePerHour(string value) {
    profile_.ratePerHour = value;
}

void Tutor::setAvailability(string value) {
    profile_.availability = value;
}

void Tutor::setIsAvailable(bool value) {
    profile_.isAvailable = value;
}

void Tutor::setCurrentStudents(int value) {
    profile_.currentStudents = value;
}

void Tutor::setTeachingClasses(vector<string> value) {
    profile_.teachingClasses = value;
}

void Tutor::setTeachingAreas(vector<string> value) {
    profile_.teachingAreas = value;
}

void Tutor::setAchievements(string value) {
    profile_.achievements = value;
}
