#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "FileManager.h"
#include "Student.h"
#include "Tutor.h"

#include <QMainWindow>
#include <QStackedWidget>
#include <QTableWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    QStackedWidget* stackedWidget = nullptr;
    Vector<Student*> students;
    Vector<Tutor*> tutors;

    QWidget* createDashboardPage();
    QWidget* createStudentPage();
    QWidget* createTutorPage();
    QWidget* createMatchingPage();
    QWidget* createClassPage();
    QWidget* createContractPage();
    QWidget* createStatisticsPage();
    QWidget* createSidebar();
    void switchPage(int index);
    void populateStudentTable(QTableWidget* table);
    void populateTutorTable(QTableWidget* table);
    static QString joinStrings(const std::vector<std::string>& items);
};

#endif
