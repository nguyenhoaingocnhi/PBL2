#include "Registration.h"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;

Registration::Registration()
    : status_("Chờ ghép gia sư") {}

Registration::Registration(string registrationID,
                           string studentID,
                           string studentName,
                           vector<string> subjects,
                           string registrationDate,
                           string status,
                           string notes)
    : registrationID_(registrationID),
      studentID_(studentID),
      studentName_(studentName),
      subjects_(subjects),
      registrationDate_(registrationDate),
      status_(status),
      notes_(notes) {}

string Registration::getRegistrationID() {
    return registrationID_;
}

string Registration::getStudentID() {
    return studentID_;
}

string Registration::getStudentName() {
    return studentName_;
}

vector<string>& Registration::getSubjects() {
    return subjects_;
}

string Registration::getRegistrationDate() {
    return registrationDate_;
}

string Registration::getStatus() {
    return status_;
}

string Registration::getNotes() {
    return notes_;
}

void Registration::setRegistrationID(string value) {
    registrationID_ = value;
}

void Registration::setStudentID(string value) {
    studentID_ = value;
}

void Registration::setStudentName(string value) {
    studentName_ = value;
}

void Registration::setSubjects(vector<string> value) {
    subjects_ = value;
}

void Registration::setRegistrationDate(string value) {
    registrationDate_ = value;
}

void Registration::setStatus(string value) {
    status_ = value;
}

void Registration::setNotes(string value) {
    notes_ = value;
}

// 1 sinh viên có thể đăng ký nhiều môn học: thêm môn nếu chưa có
void Registration::addSubject(string subject) {
    if (subject.empty()) return;
    for (size_t i = 0; i < subjects_.size(); ++i) {
        if (subjects_[i] == subject) {
            return; // Đã đăng ký môn này rồi
        }
    }
    subjects_.push_back(subject);
}

// Hủy môn học đã đăng ký
bool Registration::removeSubject(string subject) {
    for (auto it = subjects_.begin(); it != subjects_.end(); ++it) {
        if (*it == subject) {
            subjects_.erase(it);
            return true;
        }
    }
    return false;
}

// Kiểm tra môn học đã đăng ký chưa
bool Registration::hasSubject(string subject) {
    for (size_t i = 0; i < subjects_.size(); ++i) {
        if (subjects_[i] == subject) {
            return true;
        }
    }
    return false;
}

int Registration::getSubjectCount() {
    return static_cast<int>(subjects_.size());
}

string Registration::getSubjectsSummary() {
    ostringstream oss;
    for (size_t i = 0; i < subjects_.size(); ++i) {
        if (i > 0) oss << ",";
        oss << subjects_[i];
    }
    return oss.str();
}

void Registration::displayInfo() {
    cout << "Phiếu đăng ký: " << registrationID_
         << " | Sinh viên: " << studentID_ << " - " << studentName_
         << " | Số môn: " << getSubjectCount()
         << " (" << getSubjectsSummary() << ")"
         << " | Ngày: " << registrationDate_
         << " | Trạng thái: " << status_ << endl;
}

string Registration::toString() {
    ostringstream oss;
    oss << registrationID_ << "|"
        << studentID_ << "|"
        << studentName_ << "|"
        << getSubjectsSummary() << "|"
        << registrationDate_ << "|"
        << status_ << "|"
        << notes_;
    return oss.str();
}
