#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "Student.h"
#include "Tutor.h"
#include "TutoringClass.h"
#include "Contract.h"
#include "Registration.h"
#include "Vector.h"

#include <string>
#include <vector>

using namespace std;

class FileManager {
public:
    static Vector<Student*> loadStudents();
    static void saveStudents(Vector<Student*>& students);

    static Vector<Tutor*> loadTutors();
    static void saveTutors(Vector<Tutor*>& tutors);

    static Vector<TutoringClass*> loadClasses();
    static void saveClasses(Vector<TutoringClass*>& classes);

    static Vector<Contract*> loadContracts();
    static void saveContracts(Vector<Contract*>& contracts);

    static Vector<Registration*> loadRegistrations();
    static void saveRegistrations(Vector<Registration*>& registrations);

    static bool validateAdminLogin(string username, string password);

private:
    static string getDataPath(string fileName);
    static vector<string> splitString(string text, char delimiter);
};

#endif
