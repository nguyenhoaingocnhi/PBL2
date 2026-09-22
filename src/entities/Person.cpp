#include "Person.h"

using namespace std;

Person::Person() = default;

Person::Person(string personID,
               string fullName,
               string phone,
               string email,
               string address,
               string gender,
               string dateOfBirth)
    : info_{personID, fullName, phone, email, address, gender, dateOfBirth} {}

string Person::getPersonID() {
    return info_.personID;
}

string Person::getFullName() {
    return info_.fullName;
}

string Person::getPhone() {
    return info_.phone;
}

string Person::getEmail() {
    return info_.email;
}

string Person::getAddress() {
    return info_.address;
}

string Person::getGender() {
    return info_.gender;
}

string Person::getDateOfBirth() {
    return info_.dateOfBirth;
}

void Person::setPersonID(string id) {
    info_.personID = id;
}

void Person::setFullName(string name) {
    info_.fullName = name;
}

void Person::setPhone(string phoneValue) {
    info_.phone = phoneValue;
}

void Person::setEmail(string emailValue) {
    info_.email = emailValue;
}

void Person::setAddress(string addressValue) {
    info_.address = addressValue;
}

void Person::setGender(string genderValue) {
    info_.gender = genderValue;
}

void Person::setDateOfBirth(string dob) {
    info_.dateOfBirth = dob;
}
