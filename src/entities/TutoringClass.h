#pragma once

#include <string>

using namespace std;

struct TutoringClassData {
    string classID;
    string studentID;
    string tutorID;
    string subject;
    string schedule;
    string startDate;
    string status;
    string ratePerHour;
    string createdAt;
    string updatedAt;
};

class TutoringClass {
public:
    TutoringClass();
    TutoringClass(string classID,
                  string studentID,
                  string tutorID,
                  string subject,
                  string schedule,
                  string startDate,
                  string status,
                  string ratePerHour,
                  string createdAt,
                  string updatedAt);

    string getClassID() const;
    string getStudentID() const;
    string getTutorID() const;
    string getSubject() const;
    string getSchedule() const;
    string getStartDate() const;
    string getStatus() const;
    string getRatePerHour() const;
    string getCreatedAt() const;
    string getUpdatedAt() const;

    void setClassID(string value);
    void setStudentID(string value);
    void setTutorID(string value);
    void setSubject(string value);
    void setSchedule(string value);
    void setStartDate(string value);
    void setStatus(string value);
    void setRatePerHour(string value);
    void setCreatedAt(string value);
    void setUpdatedAt(string value);

private:
    TutoringClassData data_;
};
