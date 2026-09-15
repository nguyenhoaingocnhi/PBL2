#include "Person.h"

using namespace std;

Person::Person() = default;

Person::Person(const string& personID,
               const string& fullName,
               const string& phone,
               const string& email,
               const string& address,
               const string& gender,
               const string& dateOfBirth)
    : info_{personID, fullName, phone, email, address, gender, dateOfBirth} {}

const string& Person::getPersonID() const {
    return info_.personID;
}

const string& Person::getFullName() const {
    return info_.fullName;
}

const string& Person::getPhone() const {
    return info_.phone;
}

const string& Person::getEmail() const {
    return info_.email;
}

const string& Person::getAddress() const {
    return info_.address;
}

const string& Person::getGender() const {
    return info_.gender;
}

const string& Person::getDateOfBirth() const {
    return info_.dateOfBirth;
}

void Person::setPersonID(const string& id) {
    info_.personID = id;
}

void Person::setFullName(const string& name) {
    info_.fullName = name;
}

void Person::setPhone(const string& phoneValue) {
    info_.phone = phoneValue;
}

void Person::setEmail(const string& emailValue) {
    info_.email = emailValue;
}

void Person::setAddress(const string& addressValue) {
    info_.address = addressValue;
}

void Person::setGender(const string& genderValue) {
    info_.gender = genderValue;
}

void Person::setDateOfBirth(const string& dob) {
    info_.dateOfBirth = dob;
}

