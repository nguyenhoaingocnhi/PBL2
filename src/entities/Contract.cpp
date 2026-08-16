#include "Contract.h"

Contract::Contract() = default;

Contract::Contract(const std::string& contractID,
                   const std::string& classID,
                   const std::string& studentID,
                   const std::string& tutorID,
                   const std::string& createdAt,
                   const std::string& startDate,
                   const std::string& endDate,
                   const std::string& totalFee,
                   const std::string& status)
    : contractID(contractID),
      classID(classID),
      studentID(studentID),
      tutorID(tutorID),
      createdAt(createdAt),
      startDate(startDate),
      endDate(endDate),
      totalFee(totalFee),
      status(status) {}

const std::string& Contract::getContractID() const { return contractID; }
const std::string& Contract::getClassID() const { return classID; }
const std::string& Contract::getStudentID() const { return studentID; }
const std::string& Contract::getTutorID() const { return tutorID; }
const std::string& Contract::getCreatedAt() const { return createdAt; }
const std::string& Contract::getStartDate() const { return startDate; }
const std::string& Contract::getEndDate() const { return endDate; }
const std::string& Contract::getTotalFee() const { return totalFee; }
const std::string& Contract::getStatus() const { return status; }

void Contract::setContractID(const std::string& value) { contractID = value; }
void Contract::setClassID(const std::string& value) { classID = value; }
void Contract::setStudentID(const std::string& value) { studentID = value; }
void Contract::setTutorID(const std::string& value) { tutorID = value; }
void Contract::setCreatedAt(const std::string& value) { createdAt = value; }
void Contract::setStartDate(const std::string& value) { startDate = value; }
void Contract::setEndDate(const std::string& value) { endDate = value; }
void Contract::setTotalFee(const std::string& value) { totalFee = value; }
void Contract::setStatus(const std::string& value) { status = value; }
