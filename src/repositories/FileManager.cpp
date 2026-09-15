#include "FileManager.h"

#include <fstream>
#include <sstream>
#include <filesystem>

using namespace std;

namespace {
string trim(const string& s) {
    size_t start = s.find_first_not_of(" \r\n\t");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \r\n\t");
    return s.substr(start, end - start + 1);
}
}

string FileManager::getDataPath(const string& fileName) {
    if (filesystem::exists(filesystem::path("data") / fileName)) {
        return (filesystem::path("data") / fileName).string();
    }
    if (filesystem::exists(filesystem::path("../data") / fileName)) {
        return (filesystem::path("../data") / fileName).string();
    }
    return (filesystem::path("data") / fileName).string();
}

vector<string> FileManager::splitString(const string& text, char delimiter) {
    vector<string> tokens;
    stringstream ss(text);
    string token;
    while (getline(ss, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

Vector<Student*> FileManager::loadStudents() {
    Vector<Student*> students;
    ifstream in(getDataPath("student.txt"));
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        auto parts = splitString(line, '|');
        if (parts.size() < 15) continue;
        Student* s = new Student(
            parts[0],
            parts[1],
            parts[2],
            parts[3],
            parts[4],
            parts[5],
            parts[6],
            parts[7],
            splitString(parts[8], ','),
            parts[9],
            parts[10],
            parts[11],
            parts[12],
            parts[13],
            parts[14] == "true");
        students.push_back(s);
    }
    return students;
}

void FileManager::saveStudents(const Vector<Student*>& students) {
    ofstream out(getDataPath("student.txt"), ios::trunc);
    for (size_t i = 0; i < students.size(); ++i) {
        out << students[i]->toString() << "\n";
    }
}

Vector<Tutor*> FileManager::loadTutors() {
    Vector<Tutor*> tutors;
    ifstream in(getDataPath("tutor.txt"));
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        auto parts = splitString(line, '|');
        if (parts.size() < 16) continue;
        vector<string> subjects = splitString(parts[7], ',');
        vector<string> areas = splitString(parts[13], ',');
        Tutor* t = new Tutor(
            parts[0],
            parts[1],
            parts[2],
            parts[3],
            parts[4],
            parts[5],
            parts[6],
            subjects,
            parts[8],
            stoi(parts[9]),
            parts[10],
            parts[11],
            parts[12],
            parts[14] == "true",
            stoi(parts[15]),
            {},
            areas,
            "");
        tutors.push_back(t);
    }
    return tutors;
}

void FileManager::saveTutors(const Vector<Tutor*>& tutors) {
    ofstream out(getDataPath("tutor.txt"), ios::trunc);
    for (size_t i = 0; i < tutors.size(); ++i) {
        out << tutors[i]->toString() << "\n";
    }
}

Vector<TutoringClass*> FileManager::loadClasses() {
    Vector<TutoringClass*> classes;
    ifstream in(getDataPath("class.txt"));
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        auto parts = splitString(line, '|');
        if (parts.size() < 11) continue;
        classes.push_back(new TutoringClass(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5], parts[6], parts[7], parts[8], parts[9], parts[10]));
    }
    return classes;
}

void FileManager::saveClasses(const Vector<TutoringClass*>& classes) {
    ofstream out(getDataPath("class.txt"), ios::trunc);
    for (size_t i = 0; i < classes.size(); ++i) {
        out << classes[i]->getClassID() << "|" << classes[i]->getStudentID() << "|" << classes[i]->getTutorID() << "|"
            << classes[i]->getSubject() << "|" << classes[i]->getSchedule() << "|" << classes[i]->getStartDate() << "|"
            << classes[i]->getEndDate() << "|" << classes[i]->getStatus() << "|" << classes[i]->getRatePerHour() << "|"
            << classes[i]->getCreatedAt() << "|" << classes[i]->getUpdatedAt() << "\n";
    }
}

Vector<Contract*> FileManager::loadContracts() {
    Vector<Contract*> contracts;
    ifstream in(getDataPath("contract.txt"));
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        auto parts = splitString(line, '|');
        if (parts.size() < 9) continue;
        contracts.push_back(new Contract(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5], parts[6], parts[7], parts[8]));
    }
    return contracts;
}

void FileManager::saveContracts(const Vector<Contract*>& contracts) {
    ofstream out(getDataPath("contract.txt"), ios::trunc);
    for (size_t i = 0; i < contracts.size(); ++i) {
        out << contracts[i]->getContractID() << "|" << contracts[i]->getClassID() << "|" << contracts[i]->getStudentID() << "|"
            << contracts[i]->getTutorID() << "|" << contracts[i]->getCreatedAt() << "|" << contracts[i]->getStartDate() << "|"
            << contracts[i]->getEndDate() << "|" << contracts[i]->getTotalFee() << "|" << contracts[i]->getStatus() << "\n";
    }
}

bool FileManager::validateAdminLogin(const string& username, const string& password) {
    ifstream in(getDataPath("admin.txt"));
    string line;
    while (getline(in, line)) {
        auto parts = splitString(line, '|');
        if (parts.size() >= 2 && trim(parts[0]) == username && trim(parts[1]) == password) {
            return true;
        }
    }
    return false;
}
