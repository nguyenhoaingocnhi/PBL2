#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "Student.h"
#include "Tutor.h"
#include "TutoringClass.h"
#include "Contract.h"
#include "Vector.h"

#include <string>
#include <vector>

using namespace std;

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

    static bool validateAdminLogin(const string& username, const string& password);

private:
    static string getDataPath(const string& fileName);
    static vector<string> splitString(const string& text, char delimiter);
};

#endif
