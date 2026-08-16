#include "TutoringClass.h"

TutoringClass::TutoringClass() = default;

TutoringClass::TutoringClass(const std::string& classID,
                             const std::string& studentID,
                             const std::string& tutorID,
                             const std::string& subject,
                             const std::string& schedule,
                             const std::string& startDate,
                             const std::string& endDate,
                             const std::string& status,
                             const std::string& ratePerHour,
                             const std::string& createdAt,
                             const std::string& updatedAt)
    : classID(classID),
      studentID(studentID),
      tutorID(tutorID),
      subject(subject),
      schedule(schedule),
      startDate(startDate),
      endDate(endDate),
      status(status),
      ratePerHour(ratePerHour),
      createdAt(createdAt),
      updatedAt(updatedAt) {}

const std::string& TutoringClass::getClassID() const { return classID; }
const std::string& TutoringClass::getStudentID() const { return studentID; }
const std::string& TutoringClass::getTutorID() const { return tutorID; }
const std::string& TutoringClass::getSubject() const { return subject; }
const std::string& TutoringClass::getSchedule() const { return schedule; }
const std::string& TutoringClass::getStartDate() const { return startDate; }
const std::string& TutoringClass::getEndDate() const { return endDate; }
const std::string& TutoringClass::getStatus() const { return status; }
const std::string& TutoringClass::getRatePerHour() const { return ratePerHour; }
const std::string& TutoringClass::getCreatedAt() const { return createdAt; }
const std::string& TutoringClass::getUpdatedAt() const { return updatedAt; }

void TutoringClass::setClassID(const std::string& value) { classID = value; }
void TutoringClass::setStudentID(const std::string& value) { studentID = value; }
void TutoringClass::setTutorID(const std::string& value) { tutorID = value; }
void TutoringClass::setSubject(const std::string& value) { subject = value; }
void TutoringClass::setSchedule(const std::string& value) { schedule = value; }
void TutoringClass::setStartDate(const std::string& value) { startDate = value; }
void TutoringClass::setEndDate(const std::string& value) { endDate = value; }
void TutoringClass::setStatus(const std::string& value) { status = value; }
void TutoringClass::setRatePerHour(const std::string& value) { ratePerHour = value; }
void TutoringClass::setCreatedAt(const std::string& value) { createdAt = value; }
void TutoringClass::setUpdatedAt(const std::string& value) { updatedAt = value; }
