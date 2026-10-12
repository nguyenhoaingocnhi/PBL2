#pragma once

#include "Person.h"
#include "Vector.h"
#include <string>

using namespace std;

struct TutorProfile {
    Vector<string> subjects;
    string degrees;
    int yearsOfExperience = 0;
    string experienceDetails;
    string ratePerHour;
    string availability;
    bool isAvailable = true;
    int currentStudents = 0;
    Vector<string> teachingClasses;
    Vector<string> teachingAreas;
    string achievements;
};

class Tutor : public Person {
public:
    Tutor();
    Tutor(string personID,
          string fullName,
          string phone,
          string email,
          string address,
          string gender,
          string dateOfBirth,
          Vector<string> subjects,
          string degrees,
          int yearsOfExperience,
          string experienceDetails,
          string ratePerHour,
          string availability,
          bool isAvailable,
          int currentStudents,
          Vector<string> teachingClasses,
          Vector<string> teachingAreas,
          string achievements);

    void displayInfo() override;
    string toString() override;

    Vector<string>& getSubjects();
    string getDegrees();
    int getYearsOfExperience();
    string getExperienceDetails();
    string getRatePerHour();
    string getAvailability();
    bool getIsAvailable();
    int getCurrentStudents();
    Vector<string>& getTeachingClasses();
    Vector<string>& getTeachingAreas();
    string getAchievements();

    void setSubjects(Vector<string> value);
    void setDegrees(string value);
    void setYearsOfExperience(int value);
    void setExperienceDetails(string value);
    void setRatePerHour(string value);
    void setAvailability(string value);
    void setIsAvailable(bool value);
    void setCurrentStudents(int value);
    void setTeachingClasses(Vector<string> value);
    void setTeachingAreas(Vector<string> value);
    void setAchievements(string value);

private:
    TutorProfile profile_;
};
