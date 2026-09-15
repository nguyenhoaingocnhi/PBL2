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
    TutoringClass(const string& classID,
                  const string& studentID,
                  const string& tutorID,
                  const string& subject,
                  const string& schedule,
                  const string& startDate,
                  const string& endDate,
                  const string& status,
                  const string& ratePerHour,
                  const string& createdAt,
                  const string& updatedAt);

    const string& getClassID() const;
    const string& getStudentID() const;
    const string& getTutorID() const;
    const string& getSubject() const;
    const string& getSchedule() const;
    const string& getStartDate() const;
    const string& getEndDate() const;
    const string& getStatus() const;
    const string& getRatePerHour() const;
    const string& getCreatedAt() const;
    const string& getUpdatedAt() const;

    void setClassID(const string& value);
    void setStudentID(const string& value);
    void setTutorID(const string& value);
    void setSubject(const string& value);
    void setSchedule(const string& value);
    void setStartDate(const string& value);
    void setEndDate(const string& value);
    void setStatus(const string& value);
    void setRatePerHour(const string& value);
    void setCreatedAt(const string& value);
    void setUpdatedAt(const string& value);

private:
    TutoringClassData data_;
};
