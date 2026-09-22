#include "Contract.h"

using namespace std;

Contract::Contract() = default;

Contract::Contract(string contractID,
                   string classID,
                   string studentID,
                   string tutorID,
                   string createdAt,
                   string startDate,
                   string endDate,
                   string totalFee,
                   string status)
    : data_{contractID, classID, studentID, tutorID, createdAt, startDate, endDate, totalFee, status} {}

string Contract::getContractID() {
    return data_.contractID;
}

string Contract::getClassID() {
    return data_.classID;
}

string Contract::getStudentID() {
    return data_.studentID;
}

string Contract::getTutorID() {
    return data_.tutorID;
}

string Contract::getCreatedAt() {
    return data_.createdAt;
}

string Contract::getStartDate() {
    return data_.startDate;
}

string Contract::getEndDate() {
    return data_.endDate;
}

string Contract::getTotalFee() {
    return data_.totalFee;
}

string Contract::getStatus() {
    return data_.status;
}

void Contract::setContractID(string value) {
    data_.contractID = value;
}

void Contract::setClassID(string value) {
    data_.classID = value;
}

void Contract::setStudentID(string value) {
    data_.studentID = value;
}

void Contract::setTutorID(string value) {
    data_.tutorID = value;
}

void Contract::setCreatedAt(string value) {
    data_.createdAt = value;
}

void Contract::setStartDate(string value) {
    data_.startDate = value;
}

void Contract::setEndDate(string value) {
    data_.endDate = value;
}

void Contract::setTotalFee(string value) {
    data_.totalFee = value;
}

void Contract::setStatus(string value) {
    data_.status = value;
}
