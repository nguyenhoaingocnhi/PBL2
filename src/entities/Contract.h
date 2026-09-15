#pragma once

#include <string>

using namespace std;

struct ContractData {
    string contractID;
    string classID;
    string studentID;
    string tutorID;
    string createdAt;
    string startDate;
    string endDate;
    string totalFee;
    string status;
};

class Contract {
public:
    Contract();
    Contract(const string& contractID,
             const string& classID,
             const string& studentID,
             const string& tutorID,
             const string& createdAt,
             const string& startDate,
             const string& endDate,
             const string& totalFee,
             const string& status);

    const string& getContractID() const;
    const string& getClassID() const;
    const string& getStudentID() const;
    const string& getTutorID() const;
    const string& getCreatedAt() const;
    const string& getStartDate() const;
    const string& getEndDate() const;
    const string& getTotalFee() const;
    const string& getStatus() const;

    void setContractID(const string& value);
    void setClassID(const string& value);
    void setStudentID(const string& value);
    void setTutorID(const string& value);
    void setCreatedAt(const string& value);
    void setStartDate(const string& value);
    void setEndDate(const string& value);
    void setTotalFee(const string& value);
    void setStatus(const string& value);

private:
    ContractData data_;
};
