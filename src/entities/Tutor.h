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
        Tutor(const string& personID,
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
          const string& achievements);

    void displayInfo() const override;
    string toString() const override;

    const vector<string>& getSubjects() const;
    const string& getDegrees() const;
    int getYearsOfExperience() const;
    const string& getExperienceDetails() const;
    const string& getRatePerHour() const;
    const string& getAvailability() const;
    bool getIsAvailable() const;
    int getCurrentStudents() const;
    const vector<string>& getTeachingClasses() const;
    const vector<string>& getTeachingAreas() const;
    const string& getAchievements() const;

    void setSubjects(const vector<string>& value);
    void setDegrees(const string& value);
    void setYearsOfExperience(int value);
    void setExperienceDetails(const string& value);
    void setRatePerHour(const string& value);
    void setAvailability(const string& value);
    void setIsAvailable(bool value);
    void setCurrentStudents(int value);
    void setTeachingClasses(const vector<string>& value);
    void setTeachingAreas(const vector<string>& value);
    void setAchievements(const string& value);

private:
    TutorProfile profile_;
};
