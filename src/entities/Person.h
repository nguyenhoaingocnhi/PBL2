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
    Person(string personID,
           string fullName,
           string phone,
           string email,
           string address,
           string gender,
           string dateOfBirth);

    virtual ~Person() = default;

    virtual void displayInfo() = 0;
    virtual string toString() = 0;

    string getPersonID();
    string getFullName();
    string getPhone();
    string getEmail();
    string getAddress();
    string getGender();
    string getDateOfBirth();

    void setPersonID(string id);
    void setFullName(string name);
    void setPhone(string phone);
    void setEmail(string email);
    void setAddress(string address);
    void setGender(string gender);
    void setDateOfBirth(string dob);

protected:
    PersonInfo info_;
};
