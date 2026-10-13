#pragma once

#include <string>
#include <iostream>
#include "Vector.h"

using namespace std;

class Registration {
public:
    Registration();
    Registration(string registrationID,
                 string studentID,
                 string studentName,
                 Vector<string> subjects,
                 string registrationDate,
                 string status,
                 string notes);

    string getRegistrationID();
    string getStudentID();
    string getStudentName();
    Vector<string>& getSubjects();
    string getRegistrationDate();
    string getStatus();
    string getNotes();

    void setRegistrationID(string value);
    void setStudentID(string value);
    void setStudentName(string value);
    void setSubjects(Vector<string> value);
    void setRegistrationDate(string value);
    void setStatus(string value);
    void setNotes(string value);

    // Nghiệp vụ: 1 sinh viên có thể đăng ký nhiều môn học
    void addSubject(string subject);
    bool removeSubject(string subject);
    bool hasSubject(string subject);
    int getSubjectCount();
    string getSubjectsSummary();

    void displayInfo();
    string toString();

private:
    string registrationID_;
    string studentID_;
    string studentName_;
    Vector<string> subjects_;
    string registrationDate_;
    string status_;
    string notes_;
};
