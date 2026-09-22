#include "TutoringClass.h"

using namespace std;

TutoringClass::TutoringClass() = default;

TutoringClass::TutoringClass(string classID,
                             string studentID,
                             string tutorID,
                             string subject,
                             string schedule,
                             string startDate,
                             string endDate,
                             string status,
                             string ratePerHour,
                             string createdAt,
                             string updatedAt)
    : data_{classID, studentID, tutorID, subject, schedule, startDate, endDate,
            status, ratePerHour, createdAt, updatedAt} {}

string TutoringClass::getClassID() {
    return data_.classID;
}

string TutoringClass::getStudentID() {
    return data_.studentID;
}

string TutoringClass::getTutorID() {
    return data_.tutorID;
}

string TutoringClass::getSubject() {
    return data_.subject;
}

string TutoringClass::getSchedule() {
    return data_.schedule;
}

string TutoringClass::getStartDate() {
    return data_.startDate;
}

string TutoringClass::getEndDate() {
    return data_.endDate;
}

string TutoringClass::getStatus() {
    return data_.status;
}

string TutoringClass::getRatePerHour() {
    return data_.ratePerHour;
}

string TutoringClass::getCreatedAt() {
    return data_.createdAt;
}

string TutoringClass::getUpdatedAt() {
    return data_.updatedAt;
}

void TutoringClass::setClassID(string value) {
    data_.classID = value;
}

void TutoringClass::setStudentID(string value) {
    data_.studentID = value;
}

void TutoringClass::setTutorID(string value) {
    data_.tutorID = value;
}

void TutoringClass::setSubject(string value) {
    data_.subject = value;
}

void TutoringClass::setSchedule(string value) {
    data_.schedule = value;
}

void TutoringClass::setStartDate(string value) {
    data_.startDate = value;
}

void TutoringClass::setEndDate(string value) {
    data_.endDate = value;
}

void TutoringClass::setStatus(string value) {
    data_.status = value;
}

void TutoringClass::setRatePerHour(string value) {
    data_.ratePerHour = value;
}

void TutoringClass::setCreatedAt(string value) {
    data_.createdAt = value;
}

void TutoringClass::setUpdatedAt(string value) {
    data_.updatedAt = value;
}
