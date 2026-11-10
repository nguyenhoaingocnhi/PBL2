#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "FileManager.h"
#include "Student.h"
#include "Tutor.h"
#include "TutoringClass.h"
#include "MatchingService.h"

#include <QMainWindow>
#include <QStackedWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVector>

class QButtonGroup;
class ContractPage;

using namespace std;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    QStackedWidget* stackedWidget = nullptr;
    QButtonGroup* navGroup = nullptr;
    ContractPage* contractPage = nullptr;
    Vector<Student*> students;
    Vector<Tutor*> tutors;
    Vector<TutoringClass*> classes;
    // Table references
    QTableWidget* studentTable = nullptr;
    QTableWidget* tutorTable = nullptr;
    QTableWidget* classTable = nullptr;
    QTableWidget* dashboardRecentTable = nullptr;

    // Bộ lọc theo từng cột; nút Tìm kiếm áp dụng, nút Xóa khôi phục.
    QComboBox* studentIdCombo = nullptr;
    QLineEdit *studentNameEdit = nullptr, *studentPhoneEdit = nullptr;
    QLineEdit *studentSubjectEdit = nullptr, *studentAddressEdit = nullptr;
    QComboBox* studentStatusCombo = nullptr;

    QComboBox* tutorIdCombo = nullptr;
    QLineEdit *tutorNameEdit = nullptr, *tutorPhoneEdit = nullptr;
    QLineEdit *tutorSubjectEdit = nullptr, *tutorAddressEdit = nullptr;
    QComboBox *tutorExperienceCombo = nullptr, *tutorStatusCombo = nullptr;

    QComboBox* classIdCombo = nullptr;
    QLineEdit *classStudentEdit = nullptr, *classTutorEdit = nullptr;
    QLineEdit *classSubjectEdit = nullptr, *classScheduleEdit = nullptr;
    QComboBox* classStatusCombo = nullptr;

    // Dashboard Metric / Status labels
    QLabel* dashTotalStudentsLabel = nullptr;
    QLabel* dashTotalTutorsLabel = nullptr;
    QLabel* dashActiveClassesLabel = nullptr;
    QLabel* dashMatchRateLabel = nullptr;
    QLabel* dashMatchedStudentsLabel = nullptr;
    QLabel* dashUnmatchedStudentsLabel = nullptr;
    QLabel* dashAvailTutorsLabel = nullptr;
    QLabel* dashBusyTutorsLabel = nullptr;

    // Statistics Metric labels
    QLabel* statContractsCount = nullptr;
    QLabel* statMonthlyRevenue = nullptr;
    QWidget* statChartRevenue = nullptr;
    QWidget* statChartStudentSubject = nullptr;
    QLabel* statTotalStudents = nullptr;
    QLabel* statTotalTutors = nullptr;
    QLabel* statAvailTutors = nullptr;
    QLabel* statActiveClasses = nullptr;
    QLabel* statMatchRate = nullptr;
    QLabel* statMatchedClasses = nullptr;
    QLabel* statPendingClasses = nullptr;

    // Tutor Page summary tiles
    QLabel* tutorSumTotal = nullptr;
    QLabel* tutorSumAvail = nullptr;
    QLabel* tutorSumBusy = nullptr;
    QLabel* tutorSumAvgRate = nullptr;

    // Class Page summary tiles
    QLabel* classSumTotal = nullptr;
    QLabel* classSumActive = nullptr;
    QLabel* classSumPending = nullptr;
    QLabel* classSumNeedTutor = nullptr;

    // Biểu đồ (kiểu cụ thể nằm trong MainWindow.cpp)
    QWidget* dashChartTutorSubject = nullptr;
    QWidget* statChartMatch = nullptr;
    QWidget* statChartClass = nullptr;
    QWidget* statChartGrade = nullptr;
    QWidget* statChartSubject = nullptr;
    QWidget* statChartMonthly = nullptr;

    // Student summary tiles
    QLabel* stuSumTotal = nullptr;
    QLabel* stuSumMatched = nullptr;
    QLabel* stuSumUnmatched = nullptr;
    QLabel* stuSumAvgBudget = nullptr;
    // Thanh chào mừng và tab trạng thái
    QVector<QLabel*> welcomeLabels;
    QVector<QPushButton*> stuTabs, tutorTabs, classTabs;

    // Matching UI references
    QComboBox* matchStudentCombo = nullptr;
    QComboBox* matchSubjectCombo = nullptr; // all registered subjects or one subject
    QTableWidget* matchCandidatesTable = nullptr; // all eligible tutors, score descending
    QLabel* matchCandidatesSummary = nullptr;
    Vector<MatchResult> matchCandidateResults;
    QLabel* matchReqSubject = nullptr;
    QLabel* matchReqGrade = nullptr;
    QLabel* matchReqArea = nullptr;
    QLabel* matchReqBudget = nullptr;
    QWidget* matchReqScheduleContainer = nullptr;

    // Candidate card references
    QLabel* candidateRankBadge = nullptr;
    QLabel* candidateName = nullptr;
    QLabel* candidateId = nullptr;
    QLabel* candidateDegree = nullptr;
    QLabel* candidateExp = nullptr;
    QLabel* candidateArea = nullptr;
    QLabel* candidateFee = nullptr;
    QLabel* candidateSchedule = nullptr;
    QLabel* candidateMatchPercent = nullptr;
    QWidget* candidateDonutContainer = nullptr;
    QWidget* candidateScoreBreakdownContainer = nullptr;
    QWidget* candidateWhyContainer = nullptr;
    QPushButton* btnAcceptCandidate = nullptr;
    Student* currentMatchedStudent = nullptr;
    Tutor* currentMatchedTutor = nullptr;
    string currentMatchedSubject; // empty => all registered subjects

    // Page creation
    QWidget* createDashboardPage();
    QWidget* createStudentPage();
    QWidget* createTutorPage();
    QWidget* createMatchingPage();
    QWidget* createClassPage();
    QWidget* createStatisticsPage();
    QWidget* createSidebar();

    void switchPage(int index);

    // Helpers
    static QString joinStrings(Vector<string> items);
    Student* findStudent(const string& id);
    Tutor* findTutor(const string& id);
    TutoringClass* findClass(const string& id);

    // Populate & Refresh methods
    void resetStudentFilters();
    void resetTutorFilters();
    void resetClassFilters();
    void populateStudentTable();
    void populateTutorTable();
    void populateClassTable();
    void populateDashboardRecentTable();
    void refreshDashboardMetrics();
    void refreshStatisticsMetrics();
    void refreshTutorSummary();
    void refreshClassSummary();
    void refreshStudentSummary();
    void refreshOverviewWidgets();
    void refreshAll();
    // Chuyển sang trang quản lý tương ứng, bỏ bộ lọc và chọn/cuộn tới bản ghi vừa thêm.
    void focusNewRecord(int pageIndex, QTableWidget* table, const QString& id);

    // Matching methods
    void updateMatchingStudentReqs(int studentIndex);
    void clearMatchingCandidates();
    void selectMatchingCandidate(int row);
    void runSmartMatching();
    void acceptCandidateMatch();

    // CRUD Dialog slots
    void onAddStudent();
    void onEditStudent(Student* s);
    void onDeleteStudent(Student* s);
    void onViewStudent(Student* s);

    void onAddTutor();
    void onEditTutor(Tutor* t);
    void onDeleteTutor(Tutor* t);
    void onViewTutor(Tutor* t);

    void onAddClass();
    void onEditClass(TutoringClass* c);
    void onDeleteClass(TutoringClass* c);

};

#endif
