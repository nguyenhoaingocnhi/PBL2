#pragma once

#include "Person.h"
#include <string>
#include <vector>

using namespace std;

struct TutorProfile {
    vector<string> subjects;
    string degrees;
    int yearsOfExperience = 0;
    string experienceDetails;
    string ratePerHour;
    string availability;
    bool isAvailable = true;
    int currentStudents = 0;
    vector<string> teachingClasses;
    vector<string> teachingAreas;
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
          string achievements);

    void displayInfo() override;
    string toString() override;

    vector<string>& getSubjects();
    string getDegrees();
    int getYearsOfExperience();
    string getExperienceDetails();
    string getRatePerHour();
    string getAvailability();
    bool getIsAvailable();
    int getCurrentStudents();
    vector<string>& getTeachingClasses();
    vector<string>& getTeachingAreas();
    string getAchievements();

    void setSubjects(vector<string> value);
    void setDegrees(string value);
    void setYearsOfExperience(int value);
    void setExperienceDetails(string value);
    void setRatePerHour(string value);
    void setAvailability(string value);
    void setIsAvailable(bool value);
    void setCurrentStudents(int value);
    void setTeachingClasses(vector<string> value);
    void setTeachingAreas(vector<string> value);
    void setAchievements(string value);

private:
    TutorProfile profile_;
};
