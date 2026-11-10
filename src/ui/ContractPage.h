#pragma once

#include "ContractService.h"

#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

// Independent Qt panel. Its only write path is ContractService, never the PBL
// archive's global mutable lists or its hard-coded Windows file locations.
class ContractPage : public QWidget {
public:
    ContractPage(Vector<Student*>& students, Vector<Tutor*>& tutors,
                 Vector<TutoringClass*>& classes, QWidget* parent = nullptr);
    void refresh();
    int contractCount() const;
    std::int64_t totalPaid() const;
    std::int64_t paidThisMonth(int year, int month) const;
    std::int64_t monthlyRevenue(int year, int month) const;

private:
    void renderTables();
    void editContract(bool creating);
    void recordPayment();
    void viewDetails();
    void exportPdf();
    void deleteContract();
    void cancelContract();
    const ContractRecord* selected() const;
    QString entityName(const ContractRecord& record) const;
    QString contractTypeTitle(const std::string& type) const;
    QTableWidget* createTable(const QString& type);

    Vector<Student*>& students_;
    Vector<Tutor*>& tutors_;
    Vector<TutoringClass*>& classes_;
    ContractService service_;
    QTabWidget* tabs_ = nullptr;
    QTableWidget* tables_[3] = {nullptr,nullptr,nullptr};
    QLineEdit* search_ = nullptr;
    QComboBox* paymentState_ = nullptr;
    QLabel* summary_ = nullptr;
    QPushButton* add_ = nullptr;
    QPushButton* edit_ = nullptr;
    QPushButton* detail_ = nullptr;
    QPushButton* pay_ = nullptr;
    QPushButton* pdf_ = nullptr;
    QPushButton* cancel_ = nullptr;
    QPushButton* remove_ = nullptr;
};
