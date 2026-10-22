#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "Student.h"
#include "Tutor.h"
#include "TutoringClass.h"
#include "Contract.h"
#include "Registration.h"
#include "Vector.h"

#include <string>

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

    // Thư mục dữ liệu: nếu chưa đặt, tự dò "data" hoặc "../data" quanh thư mục chạy.
    static void setDataDirectory(const string& dir);

    // Kiểm tra đăng nhập admin. admin.txt lưu dạng: tên|muối|băm.
    // Dòng cũ dạng tên|mật_khẩu_thô vẫn đăng nhập được và tự động nâng cấp sang dạng băm.
    static bool validateAdminLogin(string username, string password);
    static bool changeAdminPassword(string username, string oldPassword, string newPassword);

    static Vector<string> splitString(string text, char delimiter);

    // Số dòng bị bỏ qua (sai định dạng) ở lần đọc file gần nhất; để giao diện cảnh báo.
    static int skippedLineCount();

private:
    static string getDataPath(string fileName);
    static bool writeFileAtomically(const string& path, const string& content);
};

#endif
