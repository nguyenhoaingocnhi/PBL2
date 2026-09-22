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
    Contract(string contractID,
             string classID,
             string studentID,
             string tutorID,
             string createdAt,
             string startDate,
             string endDate,
             string totalFee,
             string status);

    string getContractID();
    string getClassID();
    string getStudentID();
    string getTutorID();
    string getCreatedAt();
    string getStartDate();
    string getEndDate();
    string getTotalFee();
    string getStatus();

    void setContractID(string value);
    void setClassID(string value);
    void setStudentID(string value);
    void setTutorID(string value);
    void setCreatedAt(string value);
    void setStartDate(string value);
    void setEndDate(string value);
    void setTotalFee(string value);
    void setStatus(string value);

private:
    ContractData data_;
};
