#include "TutoringClass.h"

using namespace std;

TutoringClass::TutoringClass() = default;

TutoringClass::TutoringClass(const string& classID,
                             const string& studentID,
                             const string& tutorID,
                             const string& subject,
                             const string& schedule,
                             const string& startDate,
                             const string& endDate,
                             const string& status,
                             const string& ratePerHour,
                             const string& createdAt,
                             const string& updatedAt)
    : data_{classID, studentID, tutorID, subject, schedule, startDate, endDate,
            status, ratePerHour, createdAt, updatedAt} {}

const string& TutoringClass::getClassID() const {
    return data_.classID;
}

const string& TutoringClass::getStudentID() const {
    return data_.studentID;
}

const string& TutoringClass::getTutorID() const {
    return data_.tutorID;
}

const string& TutoringClass::getSubject() const {
    return data_.subject;
}

const string& TutoringClass::getSchedule() const {
    return data_.schedule;
}

const string& TutoringClass::getStartDate() const {
    return data_.startDate;
}

const string& TutoringClass::getEndDate() const {
    return data_.endDate;
}

const string& TutoringClass::getStatus() const {
    return data_.status;
}

const string& TutoringClass::getRatePerHour() const {
    return data_.ratePerHour;
}

const string& TutoringClass::getCreatedAt() const {
    return data_.createdAt;
}

const string& TutoringClass::getUpdatedAt() const {
    return data_.updatedAt;
}

void TutoringClass::setClassID(const string& value) {
    data_.classID = value;
}

void TutoringClass::setStudentID(const string& value) {
    data_.studentID = value;
}

void TutoringClass::setTutorID(const string& value) {
    data_.tutorID = value;
}

void TutoringClass::setSubject(const string& value) {
    data_.subject = value;
}

void TutoringClass::setSchedule(const string& value) {
    data_.schedule = value;
}

void TutoringClass::setStartDate(const string& value) {
    data_.startDate = value;
}

void TutoringClass::setEndDate(const string& value) {
    data_.endDate = value;
}

void TutoringClass::setStatus(const string& value) {
    data_.status = value;
}

void TutoringClass::setRatePerHour(const string& value) {
    data_.ratePerHour = value;
}

void TutoringClass::setCreatedAt(const string& value) {
    data_.createdAt = value;
}

void TutoringClass::setUpdatedAt(const string& value) {
    data_.updatedAt = value;
}

