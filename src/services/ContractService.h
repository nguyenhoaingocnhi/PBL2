#pragma once

#include "Student.h"
#include "Tutor.h"
#include "TutoringClass.h"
#include "Vector.h"

#include <cstdint>
#include <string>

// Contracts use explicit references instead of duplicating student/tutor/class
// state. Payment history is immutable; revenue is the sum of recorded payments.
struct ContractRecord {
    std::string id;
    std::string type;          // CLASS, STUDENT, TUTOR
    std::string referenceId;   // ID of the selected entity
    std::string issuedDate;    // dd/MM/yyyy
    std::int64_t hourlyRate = 0;  // snapshot for class agreements (VND/hour)
    std::int64_t commission = 0;  // agreed service/commission fee (VND)
    bool active = true;
    std::string notes;
};

struct PaymentRecord {
    std::string id;
    std::string contractId;
    std::string paidDate;
    std::int64_t amount = 0;
    std::string notes;
};

class ContractService {
public:
    ContractService(Vector<Student*>& students, Vector<Tutor*>& tutors,
                    Vector<TutoringClass*>& classes);

    bool reload(std::string& error);
    bool create(ContractRecord record, std::string& error);
    bool update(const std::string& id, ContractRecord record, std::string& error);
    bool remove(const std::string& id, std::string& error);
    bool addPayment(PaymentRecord payment, std::string& error);
    bool cancel(const std::string& id, std::string& error);

    const Vector<ContractRecord>& contracts() const { return contracts_; }
    const Vector<PaymentRecord>& payments() const { return payments_; }
    const ContractRecord* find(const std::string& id) const;
    std::int64_t totalPaid(const std::string& id) const;
    std::int64_t totalRevenue() const;
    std::int64_t revenueByMonth(int year, int month) const;
    std::int64_t outstanding(const std::string& id) const;
    bool hasReference(const std::string& type, const std::string& ref) const;
    static bool hasSavedReference(const std::string& type, const std::string& ref);
    static std::string makeNextId(const Vector<ContractRecord>& records,
                                  const std::string& type);
    static std::string makeNextPaymentId(const Vector<PaymentRecord>& records);

private:
    bool validRecord(const ContractRecord& record, const std::string& replacing,
                     std::string& error) const;
    bool persist(std::string& error) const;
    static bool parsePositiveId(const std::string& value, const std::string& prefix,
                                std::uint64_t& number);

    Vector<Student*>& students_;
    Vector<Tutor*>& tutors_;
    Vector<TutoringClass*>& classes_;
    Vector<ContractRecord> contracts_;
    Vector<PaymentRecord> payments_;
    bool loadOk_ = false;
};
