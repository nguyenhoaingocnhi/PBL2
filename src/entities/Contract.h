#ifndef CONTRACT_H
#define CONTRACT_H

#include <string>

class Contract {
public:
    Contract();
    Contract(const std::string& contractID,
             const std::string& classID,
             const std::string& studentID,
             const std::string& tutorID,
             const std::string& createdAt,
             const std::string& startDate,
             const std::string& endDate,
             const std::string& totalFee,
             const std::string& status);

    const std::string& getContractID() const;
    const std::string& getClassID() const;
    const std::string& getStudentID() const;
    const std::string& getTutorID() const;
    const std::string& getCreatedAt() const;
    const std::string& getStartDate() const;
    const std::string& getEndDate() const;
    const std::string& getTotalFee() const;
    const std::string& getStatus() const;

    void setContractID(const std::string& value);
    void setClassID(const std::string& value);
    void setStudentID(const std::string& value);
    void setTutorID(const std::string& value);
    void setCreatedAt(const std::string& value);
    void setStartDate(const std::string& value);
    void setEndDate(const std::string& value);
    void setTotalFee(const std::string& value);
    void setStatus(const std::string& value);

private:
    std::string contractID;
    std::string classID;
    std::string studentID;
    std::string tutorID;
    std::string createdAt;
    std::string startDate;
    std::string endDate;
    std::string totalFee;
    std::string status;
};

#endif
