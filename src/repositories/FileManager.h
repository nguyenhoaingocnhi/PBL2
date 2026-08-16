#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "Student.h"
#include "Tutor.h"
#include "TutoringClass.h"
#include "Contract.h"
#include "Vector.h"

#include <string>

class FileManager {
public:
    static Vector<Student*> loadStudents();
    static void saveStudents(const Vector<Student*>& students);

    static Vector<Tutor*> loadTutors();
    static void saveTutors(const Vector<Tutor*>& tutors);

    static Vector<TutoringClass*> loadClasses();
    static void saveClasses(const Vector<TutoringClass*>& classes);

    static Vector<Contract*> loadContracts();
    static void saveContracts(const Vector<Contract*>& contracts);

    static bool validateAdminLogin(const std::string& username, const std::string& password);

private:
    static std::string getDataPath(const std::string& fileName);
    static std::vector<std::string> splitString(const std::string& text, char delimiter);
};

#endif
