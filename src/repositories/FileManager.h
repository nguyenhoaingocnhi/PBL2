#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "Student.h"
#include "Tutor.h"
#include "TutoringClass.h"
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

    // Consistent persistence for business transactions. Stages all three files,
    // keeps rollback copies and reports I/O failure (unlike legacy save* APIs).
    // Multi-file replacement is NOT guaranteed crash-atomic; keep backups.
    static bool saveAll(Vector<Student*>& students, Vector<Tutor*>& tutors,
                        Vector<TutoringClass*>& classes);

    // Thư mục dữ liệu: nếu chưa đặt, tự dò "data" hoặc "../data" quanh thư mục chạy.
    static void setDataDirectory(const string& dir);
    // Storage helpers for other modules; all paths honor the same data directory.
    static string dataPath(const string& filename);
    static bool saveTextFile(const string& filename, const string& content);

    // Kiểm tra đăng nhập admin. admin.txt lưu dạng: tên|muối|băm.
    // Dòng cũ dạng tên|mật_khẩu_thô vẫn đăng nhập được và tự động nâng cấp sang dạng băm.
    static bool validateAdminLogin(string username, string password);
    static bool changeAdminPassword(string username, string oldPassword, string newPassword);

    static Vector<string> splitString(string text, char delimiter);

    // Số dòng bị bỏ qua (sai định dạng) ở lần đọc file gần nhất; để giao diện cảnh báo.
    static int skippedLineCount();

    // Recover a multi-file update interrupted before its completion marker.
    // If recovery fails, writes are refused and the files are left available
    // for manual repair instead of silently overwriting the data.
    static bool recoverIfNeeded();
    static string recoveryError();
    // Sticky fail-closed error set when a student/tutor/class TXT could not be
    // parsed without dropping records. Restart after restoring the source file.
    static string storageError();

private:
    static string getDataPath(string fileName);
    static bool writeFileAtomically(const string& path, const string& content);
};

#endif
