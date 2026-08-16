#include "FileManager.h"

#include <fstream>
#include <sstream>
#include <filesystem>

namespace {
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \r\n\t");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \r\n\t");
    return s.substr(start, end - start + 1);
}
}

std::string FileManager::getDataPath(const std::string& fileName) {
    return (std::filesystem::path("data") / fileName).string();
}

std::vector<std::string> FileManager::splitString(const std::string& text, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(text);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

Vector<Student*> FileManager::loadStudents() {
    Vector<Student*> students;
    std::ifstream in(getDataPath("student.txt"));
    std::string line;
    while (std::getline(in, line)) {
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
    std::ofstream out(getDataPath("student.txt"), std::ios::trunc);
    for (std::size_t i = 0; i < students.size(); ++i) {
        out << students[i]->toString() << "\n";
    }
}

Vector<Tutor*> FileManager::loadTutors() {
    Vector<Tutor*> tutors;
    std::ifstream in(getDataPath("tutor.txt"));
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto parts = splitString(line, '|');
        if (parts.size() < 15) continue;
        std::vector<std::string> subjects = splitString(parts[7], ',');
        std::vector<std::string> areas = splitString(parts[13], ',');
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
            std::stoi(parts[9]),
            parts[10],
            parts[11],
            parts[12],
            parts[14] == "true",
            std::stoi(parts[15]),
            {},
            areas,
            "");
        tutors.push_back(t);
    }
    return tutors;
}

void FileManager::saveTutors(const Vector<Tutor*>& tutors) {
    std::ofstream out(getDataPath("tutor.txt"), std::ios::trunc);
    for (std::size_t i = 0; i < tutors.size(); ++i) {
        out << tutors[i]->toString() << "\n";
    }
}

Vector<TutoringClass*> FileManager::loadClasses() {
    Vector<TutoringClass*> classes;
    std::ifstream in(getDataPath("class.txt"));
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto parts = splitString(line, '|');
        if (parts.size() < 11) continue;
        classes.push_back(new TutoringClass(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5], parts[6], parts[7], parts[8], parts[9], parts[10]));
    }
    return classes;
}

void FileManager::saveClasses(const Vector<TutoringClass*>& classes) {
    std::ofstream out(getDataPath("class.txt"), std::ios::trunc);
    for (std::size_t i = 0; i < classes.size(); ++i) {
        out << classes[i]->getClassID() << "|" << classes[i]->getStudentID() << "|" << classes[i]->getTutorID() << "|"
            << classes[i]->getSubject() << "|" << classes[i]->getSchedule() << "|" << classes[i]->getStartDate() << "|"
            << classes[i]->getEndDate() << "|" << classes[i]->getStatus() << "|" << classes[i]->getRatePerHour() << "|"
            << classes[i]->getCreatedAt() << "|" << classes[i]->getUpdatedAt() << "\n";
    }
}

Vector<Contract*> FileManager::loadContracts() {
    Vector<Contract*> contracts;
    std::ifstream in(getDataPath("contract.txt"));
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto parts = splitString(line, '|');
        if (parts.size() < 9) continue;
        contracts.push_back(new Contract(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5], parts[6], parts[7], parts[8]));
    }
    return contracts;
}

void FileManager::saveContracts(const Vector<Contract*>& contracts) {
    std::ofstream out(getDataPath("contract.txt"), std::ios::trunc);
    for (std::size_t i = 0; i < contracts.size(); ++i) {
        out << contracts[i]->getContractID() << "|" << contracts[i]->getClassID() << "|" << contracts[i]->getStudentID() << "|"
            << contracts[i]->getTutorID() << "|" << contracts[i]->getCreatedAt() << "|" << contracts[i]->getStartDate() << "|"
            << contracts[i]->getEndDate() << "|" << contracts[i]->getTotalFee() << "|" << contracts[i]->getStatus() << "\n";
    }
}

bool FileManager::validateAdminLogin(const std::string& username, const std::string& password) {
    std::ifstream in(getDataPath("admin.txt"));
    std::string line;
    while (std::getline(in, line)) {
        auto parts = splitString(line, '|');
        if (parts.size() >= 2 && trim(parts[0]) == username && trim(parts[1]) == password) {
            return true;
        }
    }
    return false;
}
