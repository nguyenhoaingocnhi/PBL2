#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "FileManager.h"
#include "Student.h"
#include "Tutor.h"
#include "TutoringClass.h"
#include "Contract.h"
#include "Registration.h"
#include "MatchingService.h"

#include <QMainWindow>
#include <QStackedWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

using namespace std;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    QStackedWidget* stackedWidget = nullptr;
    Vector<Student*> students;
    Vector<Tutor*> tutors;
    Vector<TutoringClass*> classes;
    Vector<Contract*> contracts;
    Vector<Registration*> registrations;

    // Table references
    QTableWidget* studentTable = nullptr;
    QTableWidget* tutorTable = nullptr;
    QTableWidget* classTable = nullptr;
    QTableWidget* contractTable = nullptr;
    QTableWidget* registrationTable = nullptr;
    QTableWidget* dashboardRecentTable = nullptr;

    // Filter controls references
    QLineEdit* studentSearchEdit = nullptr;
    QComboBox* studentGradeCombo = nullptr;
    QComboBox* studentStatusCombo = nullptr;

    QLineEdit* tutorSearchEdit = nullptr;
    QComboBox* tutorSubjectCombo = nullptr;
    QComboBox* tutorAreaCombo = nullptr;
    QComboBox* tutorStatusCombo = nullptr;

    QLineEdit* classSearchEdit = nullptr;
    QComboBox* classSubjectCombo = nullptr;
    QComboBox* classStatusCombo = nullptr;

    QLineEdit* contractSearchEdit = nullptr;
    QComboBox* contractStatusCombo = nullptr;

    QLineEdit* regSearchEdit = nullptr;
    QComboBox* regStatusCombo = nullptr;

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

    // Contract Page summary tiles
    QLabel* contractSumTotal = nullptr;
    QLabel* contractSumActive = nullptr;
    QLabel* contractSumPending = nullptr;
    QLabel* contractSumCompleted = nullptr;

    // Matching UI references
    QComboBox* matchStudentCombo = nullptr;
    QLabel* matchReqSubject = nullptr;
    QLabel* matchReqGrade = nullptr;
    QLabel* matchReqArea = nullptr;
    QLabel* matchReqBudget = nullptr;
    QWidget* matchReqScheduleContainer = nullptr;

    // Candidate card references
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

    // Page creation
    QWidget* createDashboardPage();
    QWidget* createStudentPage();
    QWidget* createTutorPage();
    QWidget* createMatchingPage();
    QWidget* createClassPage();
    QWidget* createContractPage();
    QWidget* createStatisticsPage();
    QWidget* createRegistrationPage();
    QWidget* createSidebar();

    void switchPage(int index);

    // Helpers
    static QString joinStrings(Vector<string> items);
    Student* findStudent(const string& id);
    Tutor* findTutor(const string& id);
    TutoringClass* findClass(const string& id);

    // Populate & Refresh methods
    void populateStudentTable();
    void populateTutorTable();
    void populateClassTable();
    void populateContractTable();
    void populateRegistrationTable();
    void populateDashboardRecentTable();
    void refreshDashboardMetrics();
    void refreshStatisticsMetrics();
    void refreshTutorSummary();
    void refreshClassSummary();
    void refreshContractSummary();
    void refreshAll();

    // Matching methods
    void updateMatchingStudentReqs(int studentIndex);
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

    void onAddContract();
    void onEditContract(Contract* c);
    void onDeleteContract(Contract* c);

    void onAddRegistration();
    void onEditRegistration(Registration* r);
    void onDeleteRegistration(Registration* r);
};

#endif
