#pragma once

#include <string>

using namespace std;

struct PersonInfo {
    string personID;
    string fullName;
    string phone;
    string email;
    string address;
    string gender;
    string dateOfBirth;
};

class Person {
public:
    Person();
        Person(const string& personID,
            const string& fullName,
            const string& phone,
            const string& email,
            const string& address,
            const string& gender,
            const string& dateOfBirth);

    virtual ~Person() = default;

    virtual void displayInfo() const = 0;
    virtual string toString() const = 0;

    const string& getPersonID() const;
    const string& getFullName() const;
    const string& getPhone() const;
    const string& getEmail() const;
    const string& getAddress() const;
    const string& getGender() const;
    const string& getDateOfBirth() const;

    void setPersonID(const string& id);
    void setFullName(const string& name);
    void setPhone(const string& phone);
    void setEmail(const string& email);
    void setAddress(const string& address);
    void setGender(const string& gender);
    void setDateOfBirth(const string& dob);

protected:
    PersonInfo info_;
};
