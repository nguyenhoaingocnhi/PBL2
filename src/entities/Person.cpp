#include "Person.h"

Person::Person() = default;

Person::Person(const std::string& personID,
               const std::string& fullName,
               const std::string& phone,
               const std::string& email,
               const std::string& address,
               const std::string& gender,
               const std::string& dateOfBirth)
    : personID(personID),
      fullName(fullName),
      phone(phone),
      email(email),
      address(address),
      gender(gender),
      dateOfBirth(dateOfBirth) {}

const std::string& Person::getPersonID() const { return personID; }
const std::string& Person::getFullName() const { return fullName; }
const std::string& Person::getPhone() const { return phone; }
const std::string& Person::getEmail() const { return email; }
const std::string& Person::getAddress() const { return address; }
const std::string& Person::getGender() const { return gender; }
const std::string& Person::getDateOfBirth() const { return dateOfBirth; }

void Person::setPersonID(const std::string& id) { personID = id; }
void Person::setFullName(const std::string& name) { fullName = name; }
void Person::setPhone(const std::string& phoneValue) { phone = phoneValue; }
void Person::setEmail(const std::string& emailValue) { email = emailValue; }
void Person::setAddress(const std::string& addressValue) { address = addressValue; }
void Person::setGender(const std::string& genderValue) { gender = genderValue; }
void Person::setDateOfBirth(const std::string& dob) { dateOfBirth = dob; }
