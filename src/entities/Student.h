#pragma once

#include "Person.h"
#include "Vector.h"
#include <string>

using namespace std;

struct StudentProfile {
    string grade;
    Vector<string> needSubjects;
    string availableSchedule;
    string learningLevel;
    string preferredTutorGender;
    string budgetPerHour;
    string notes;
    bool isMatched = false;
    Vector<string> enrolledClasses;
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
            Vector<string> needSubjects,
            string availableSchedule,
            string learningLevel,
            string preferredTutorGender,
            string budgetPerHour,
            string notes,
            bool isMatched,
            Vector<string> enrolledClasses = {});

    void displayInfo() override;
    string toString() override;

    string getGrade();
    Vector<string>& getNeedSubjects();
    string getAvailableSchedule();
    string getLearningLevel();
    string getPreferredTutorGender();
    string getBudgetPerHour();
    string getNotes();
    bool getIsMatched();
    Vector<string>& getEnrolledClasses();

    void setGrade(string value);
    void setNeedSubjects(Vector<string> value);
    void setAvailableSchedule(string value);
    void setLearningLevel(string value);
    void setPreferredTutorGender(string value);
    void setBudgetPerHour(string value);
    void setNotes(string value);
    void setIsMatched(bool value);
    void setEnrolledClasses(Vector<string> value);

    // Tiện ích đăng ký nhiều môn học cho sinh viên
    void registerSubject(string subject);
    void registerMultipleSubjects(Vector<string> subjects);
    bool unregisterSubject(string subject);
    Vector<string>& getRegisteredSubjects();

private:
    StudentProfile profile_;
};
