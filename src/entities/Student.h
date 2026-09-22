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
    Student(string personID,
            string fullName,
            string phone,
            string email,
            string address,
            string gender,
            string dateOfBirth,
            string grade,
            vector<string> needSubjects,
            string availableSchedule,
            string learningLevel,
            string preferredTutorGender,
            string budgetPerHour,
            string notes,
            bool isMatched,
            vector<string> enrolledClasses = {});

    void displayInfo() override;
    string toString() override;

    string getGrade();
    vector<string>& getNeedSubjects();
    string getAvailableSchedule();
    string getLearningLevel();
    string getPreferredTutorGender();
    string getBudgetPerHour();
    string getNotes();
    bool getIsMatched();
    vector<string>& getEnrolledClasses();

    void setGrade(string value);
    void setNeedSubjects(vector<string> value);
    void setAvailableSchedule(string value);
    void setLearningLevel(string value);
    void setPreferredTutorGender(string value);
    void setBudgetPerHour(string value);
    void setNotes(string value);
    void setIsMatched(bool value);
    void setEnrolledClasses(vector<string> value);

    // Tiện ích đăng ký nhiều môn học cho sinh viên
    void registerSubject(string subject);
    void registerMultipleSubjects(vector<string> subjects);
    bool unregisterSubject(string subject);
    vector<string>& getRegisteredSubjects();

private:
    StudentProfile profile_;
};
