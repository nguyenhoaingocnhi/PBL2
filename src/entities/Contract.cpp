#include "Contract.h"

using namespace std;

Contract::Contract() = default;

Contract::Contract(const string& contractID,
                   const string& classID,
                   const string& studentID,
                   const string& tutorID,
                   const string& createdAt,
                   const string& startDate,
                   const string& endDate,
                   const string& totalFee,
                   const string& status)
    : data_{contractID, classID, studentID, tutorID, createdAt, startDate, endDate, totalFee, status} {}

const string& Contract::getContractID() const {
    return data_.contractID;
}

const string& Contract::getClassID() const {
    return data_.classID;
}

const string& Contract::getStudentID() const {
    return data_.studentID;
}

const string& Contract::getTutorID() const {
    return data_.tutorID;
}

const string& Contract::getCreatedAt() const {
    return data_.createdAt;
}

const string& Contract::getStartDate() const {
    return data_.startDate;
}

const string& Contract::getEndDate() const {
    return data_.endDate;
}

const string& Contract::getTotalFee() const {
    return data_.totalFee;
}

const string& Contract::getStatus() const {
    return data_.status;
}

void Contract::setContractID(const string& value) {
    data_.contractID = value;
}

void Contract::setClassID(const string& value) {
    data_.classID = value;
}

void Contract::setStudentID(const string& value) {
    data_.studentID = value;
}

void Contract::setTutorID(const string& value) {
    data_.tutorID = value;
}

void Contract::setCreatedAt(const string& value) {
    data_.createdAt = value;
}

void Contract::setStartDate(const string& value) {
    data_.startDate = value;
}

void Contract::setEndDate(const string& value) {
    data_.endDate = value;
}

void Contract::setTotalFee(const string& value) {
    data_.totalFee = value;
}

void Contract::setStatus(const string& value) {
    data_.status = value;
}

