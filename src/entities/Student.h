#ifndef STUDENT_H
#define STUDENT_H

#include "Person.h"
#include <string>
#include <vector>

class Student : public Person {
public:
    Student();
    Student(const std::string& personID,
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
            const std::vector<std::string>& enrolledClasses = {});

    void displayInfo() const override;
    std::string toString() const override;

    const std::string& getGrade() const;
    const std::vector<std::string>& getNeedSubjects() const;
    const std::string& getAvailableSchedule() const;
    const std::string& getLearningLevel() const;
    const std::string& getPreferredTutorGender() const;
    const std::string& getBudgetPerHour() const;
    const std::string& getNotes() const;
    bool getIsMatched() const;
    const std::vector<std::string>& getEnrolledClasses() const;

    void setGrade(const std::string& value);
    void setNeedSubjects(const std::vector<std::string>& value);
    void setAvailableSchedule(const std::string& value);
    void setLearningLevel(const std::string& value);
    void setPreferredTutorGender(const std::string& value);
    void setBudgetPerHour(const std::string& value);
    void setNotes(const std::string& value);
    void setIsMatched(bool value);
    void setEnrolledClasses(const std::vector<std::string>& value);

private:
    std::string grade;
    std::vector<std::string> needSubjects;
    std::string availableSchedule;
    std::string learningLevel;
    std::string preferredTutorGender;
    std::string budgetPerHour;
    std::string notes;
    bool isMatched = false;
    std::vector<std::string> enrolledClasses;
};

#endif
