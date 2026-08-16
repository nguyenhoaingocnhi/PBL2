#ifndef TUTOR_H
#define TUTOR_H

#include "Person.h"
#include <string>
#include <vector>

class Tutor : public Person {
public:
    Tutor();
    Tutor(const std::string& personID,
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
          const std::string& achievements);

    void displayInfo() const override;
    std::string toString() const override;

    const std::vector<std::string>& getSubjects() const;
    const std::string& getDegrees() const;
    int getYearsOfExperience() const;
    const std::string& getExperienceDetails() const;
    const std::string& getRatePerHour() const;
    const std::string& getAvailability() const;
    bool getIsAvailable() const;
    int getCurrentStudents() const;
    const std::vector<std::string>& getTeachingClasses() const;
    const std::vector<std::string>& getTeachingAreas() const;
    const std::string& getAchievements() const;

    void setSubjects(const std::vector<std::string>& value);
    void setDegrees(const std::string& value);
    void setYearsOfExperience(int value);
    void setExperienceDetails(const std::string& value);
    void setRatePerHour(const std::string& value);
    void setAvailability(const std::string& value);
    void setIsAvailable(bool value);
    void setCurrentStudents(int value);
    void setTeachingClasses(const std::vector<std::string>& value);
    void setTeachingAreas(const std::vector<std::string>& value);
    void setAchievements(const std::string& value);

private:
    std::vector<std::string> subjects;
    std::string degrees;
    int yearsOfExperience = 0;
    std::string experienceDetails;
    std::string ratePerHour;
    std::string availability;
    bool isAvailable = true;
    int currentStudents = 0;
    std::vector<std::string> teachingClasses;
    std::vector<std::string> teachingAreas;
    std::string achievements;
};

#endif
