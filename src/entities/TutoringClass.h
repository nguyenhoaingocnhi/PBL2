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
    string endDate;
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
                  string endDate,
                  string status,
                  string ratePerHour,
                  string createdAt,
                  string updatedAt);

    string getClassID();
    string getStudentID();
    string getTutorID();
    string getSubject();
    string getSchedule();
    string getStartDate();
    string getEndDate();
    string getStatus();
    string getRatePerHour();
    string getCreatedAt();
    string getUpdatedAt();

    void setClassID(string value);
    void setStudentID(string value);
    void setTutorID(string value);
    void setSubject(string value);
    void setSchedule(string value);
    void setStartDate(string value);
    void setEndDate(string value);
    void setStatus(string value);
    void setRatePerHour(string value);
    void setCreatedAt(string value);
    void setUpdatedAt(string value);

private:
    TutoringClassData data_;
};
