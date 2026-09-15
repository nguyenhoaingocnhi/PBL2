#pragma once

#include "Person.h"
#include <string>
#include <vector>

using namespace std;

struct StudentProfile {
    string grade;
    vector<string> needSubjects;
    string availableSchedule;
    string learningLevel;
    string preferredTutorGender;
    string budgetPerHour;
    string notes;
    bool isMatched = false;
    vector<string> enrolledClasses;
};

class Student : public Person {
public:
    Student();
        Student(const string& personID,
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
            const vector<string>& enrolledClasses = {});

    void displayInfo() const override;
    string toString() const override;

    const string& getGrade() const;
    const vector<string>& getNeedSubjects() const;
    const string& getAvailableSchedule() const;
    const string& getLearningLevel() const;
    const string& getPreferredTutorGender() const;
    const string& getBudgetPerHour() const;
    const string& getNotes() const;
    bool getIsMatched() const;
    const vector<string>& getEnrolledClasses() const;

    void setGrade(const string& value);
    void setNeedSubjects(const vector<string>& value);
    void setAvailableSchedule(const string& value);
    void setLearningLevel(const string& value);
    void setPreferredTutorGender(const string& value);
    void setBudgetPerHour(const string& value);
    void setNotes(const string& value);
    void setIsMatched(bool value);
    void setEnrolledClasses(const vector<string>& value);

private:
    StudentProfile profile_;
};
