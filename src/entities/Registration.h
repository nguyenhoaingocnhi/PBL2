#pragma once

#include <string>
#include <vector>
#include <iostream>

using namespace std;

class Registration {
public:
    Registration();
    Registration(string registrationID,
                 string studentID,
                 string studentName,
                 vector<string> subjects,
                 string registrationDate,
                 string status,
                 string notes);

    // Getters (không dùng const)
    string getRegistrationID();
    string getStudentID();
    string getStudentName();
    vector<string>& getSubjects();
    string getRegistrationDate();
    string getStatus();
    string getNotes();

    // Setters (không dùng const)
    void setRegistrationID(string value);
    void setStudentID(string value);
    void setStudentName(string value);
    void setSubjects(vector<string> value);
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
    vector<string> subjects_;
    string registrationDate_;
    string status_;
    string notes_;
};
