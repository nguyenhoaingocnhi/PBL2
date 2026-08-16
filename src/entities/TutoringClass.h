#ifndef TUTORINGCLASS_H
#define TUTORINGCLASS_H

#include <string>

class TutoringClass {
public:
    TutoringClass();
    TutoringClass(const std::string& classID,
                  const std::string& studentID,
                  const std::string& tutorID,
                  const std::string& subject,
                  const std::string& schedule,
                  const std::string& startDate,
                  const std::string& endDate,
                  const std::string& status,
                  const std::string& ratePerHour,
                  const std::string& createdAt,
                  const std::string& updatedAt);

    const std::string& getClassID() const;
    const std::string& getStudentID() const;
    const std::string& getTutorID() const;
    const std::string& getSubject() const;
    const std::string& getSchedule() const;
    const std::string& getStartDate() const;
    const std::string& getEndDate() const;
    const std::string& getStatus() const;
    const std::string& getRatePerHour() const;
    const std::string& getCreatedAt() const;
    const std::string& getUpdatedAt() const;

    void setClassID(const std::string& value);
    void setStudentID(const std::string& value);
    void setTutorID(const std::string& value);
    void setSubject(const std::string& value);
    void setSchedule(const std::string& value);
    void setStartDate(const std::string& value);
    void setEndDate(const std::string& value);
    void setStatus(const std::string& value);
    void setRatePerHour(const std::string& value);
    void setCreatedAt(const std::string& value);
    void setUpdatedAt(const std::string& value);

private:
    std::string classID;
    std::string studentID;
    std::string tutorID;
    std::string subject;
    std::string schedule;
    std::string startDate;
    std::string endDate;
    std::string status;
    std::string ratePerHour;
    std::string createdAt;
    std::string updatedAt;
};

#endif
