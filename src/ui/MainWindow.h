#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "FileManager.h"
#include "Student.h"
#include "Tutor.h"
#include "Registration.h"

#include <QMainWindow>
#include <QStackedWidget>
#include <QTableWidget>

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
    Vector<Registration*> registrations;

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
    void populateStudentTable(QTableWidget* table);
    void populateTutorTable(QTableWidget* table);
    void populateRegistrationTable(QTableWidget* table);
    static QString joinStrings(vector<string> items);
};

#endif
