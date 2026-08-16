#ifndef PERSON_H
#define PERSON_H

#include <string>

class Person {
public:
    Person();
    Person(const std::string& personID,
           const std::string& fullName,
           const std::string& phone,
           const std::string& email,
           const std::string& address,
           const std::string& gender,
           const std::string& dateOfBirth);

    virtual ~Person() = default;

    virtual void displayInfo() const = 0;
    virtual std::string toString() const = 0;

    const std::string& getPersonID() const;
    const std::string& getFullName() const;
    const std::string& getPhone() const;
    const std::string& getEmail() const;
    const std::string& getAddress() const;
    const std::string& getGender() const;
    const std::string& getDateOfBirth() const;

    void setPersonID(const std::string& id);
    void setFullName(const std::string& name);
    void setPhone(const std::string& phone);
    void setEmail(const std::string& email);
    void setAddress(const std::string& address);
    void setGender(const std::string& gender);
    void setDateOfBirth(const std::string& dob);

protected:
    std::string personID;
    std::string fullName;
    std::string phone;
    std::string email;
    std::string address;
    std::string gender;
    std::string dateOfBirth;
};

#endif
