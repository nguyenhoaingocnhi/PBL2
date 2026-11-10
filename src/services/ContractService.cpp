#include "ContractService.h"

#include "FileManager.h"
#include "TextUtils.h"
#include "Validator.h"

#include <filesystem>
#include <fstream>
#include <limits>
#include <set>
#include <sstream>
#include <vector>

using namespace std;

namespace {
vector<string> fields(const string& line) {
    vector<string> result;
    string token;
    istringstream input(line);
    while (getline(input, token, '|')) result.push_back(token);
    if (!line.empty() && line.back() == '|') result.emplace_back();
    return result;
}

bool wholeNonnegative(const string& value, int64_t& out) {
    if (value.empty() || value.size() > 18) return false;
    for (char c : value) if (c < '0' || c > '9') return false;
    try {
        size_t consumed = 0;
        out = stoll(value, &consumed, 10);
        return consumed == value.size() && out >= 0;
    } catch (...) { return false; }
}

bool validType(const string& type) {
    return type == "CLASS" || type == "STUDENT" || type == "TUTOR";
}

string prefixFor(const string& type) {
    if (type == "CLASS") return "CTC";
    if (type == "STUDENT") return "CTS";
    return "CTT";
}

int dateMonth(const string& date) {
    if (!Validator::isValidDate(date)) return -1;
    return stoi(date.substr(3, 2));
}
int dateYear(const string& date) {
    if (!Validator::isValidDate(date)) return -1;
    return stoi(date.substr(6, 4));
}
int dateNumber(const string& date) {
    if (!Validator::isValidDate(date)) return -1;
    return dateYear(date) * 10000 + dateMonth(date) * 100 + stoi(date.substr(0, 2));
}

bool sumSafe(int64_t a, int64_t b) {
    return b >= 0 && a <= numeric_limits<int64_t>::max() - b;
}
}

ContractService::ContractService(Vector<Student*>& students, Vector<Tutor*>& tutors,
                                 Vector<TutoringClass*>& classes)
    : students_(students), tutors_(tutors), classes_(classes) {
    string error;
    reload(error);
}

bool ContractService::reload(string& error) {
    Vector<ContractRecord> newContracts;
    Vector<PaymentRecord> newPayments;
    namespace fs = std::filesystem;
    const fs::path path(FileManager::dataPath("contracts.txt"));
    if (!fs::exists(path)) {
        contracts_ = newContracts; payments_ = newPayments; loadOk_ = true;
        return true;
    }
    ifstream input(path, ios::binary);
    if (!input) { loadOk_ = false; error = "Không thể đọc contracts.txt."; return false; }
    string line;
    int lineNumber = 0;
    set<string> ids, paymentIds;
    while (getline(input, line)) {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        const auto f = fields(line);
        if (f.size() == 9 && f[0] == "C") {
            ContractRecord record;
            int64_t fee = 0, commission = 0;
            uint64_t contractNumber=0;
            if (!wholeNonnegative(f[5], fee) || !wholeNonnegative(f[6], commission) ||
                !validType(f[2]) || !parsePositiveId(f[1],prefixFor(f[2]),contractNumber) ||
                !Validator::isValidDate(f[4]) ||
                (f[7] != "0" && f[7] != "1") ||
                !ids.insert(f[1]).second || f[1].empty() || f[3].empty()) {
                error = "contracts.txt: hợp đồng không hợp lệ tại dòng " + to_string(lineNumber);
                loadOk_ = false; return false;
            }
            record.id=f[1]; record.type=f[2]; record.referenceId=f[3]; record.issuedDate=f[4];
            record.hourlyRate=fee; record.commission=commission; record.active=f[7]=="1";
            record.notes=f[8]; newContracts.push_back(record);
        } else if (f.size() == 6 && f[0] == "P") {
            PaymentRecord payment;
            int64_t amount = 0;
            uint64_t paymentNumber=0;
            if (!wholeNonnegative(f[4], amount) || !amount ||
                !parsePositiveId(f[1],"PAY",paymentNumber) ||
                !Validator::isValidDate(f[3]) || !paymentIds.insert(f[1]).second ||
                f[1].empty() || f[2].empty()) {
                error = "contracts.txt: thanh toán không hợp lệ tại dòng " + to_string(lineNumber);
                loadOk_ = false; return false;
            }
            payment.id=f[1]; payment.contractId=f[2]; payment.paidDate=f[3];
            payment.amount=amount; payment.notes=f[5]; newPayments.push_back(payment);
        } else {
            error = "contracts.txt: không nhận diện được dòng " + to_string(lineNumber);
            loadOk_ = false; return false;
        }
    }
    if (!input.eof()) { loadOk_=false; error="contracts.txt: lỗi đọc file."; return false; }
    for (size_t i=0;i<newPayments.size();++i) {
        const auto& p=newPayments[i];
        bool linked=false;
        for (size_t j=0;j<newContracts.size();++j) {
            if (newContracts[j].id != p.contractId) continue;
            linked=true;
            if (dateNumber(p.paidDate)<dateNumber(newContracts[j].issuedDate)) {
                loadOk_=false; error="contracts.txt: thanh toán xảy ra trước ngày lập hợp đồng."; return false;
            }
            break;
        }
        if (!linked) { loadOk_=false; error="contracts.txt: thanh toán tham chiếu hợp đồng không tồn tại."; return false; }
    }
    int64_t revenue=0;
    for (size_t j=0;j<newPayments.size();++j) {
        if (!sumSafe(revenue,newPayments[j].amount)) {
            loadOk_=false;error="Tổng thanh toán vượt miền giá trị hỗ trợ.";return false;
        }
        revenue+=newPayments[j].amount;
    }
    for (size_t i=0;i<newContracts.size();++i) {
        int64_t paid=0;
        for (size_t j=0;j<newPayments.size();++j) if (newPayments[j].contractId == newContracts[i].id) {
            if (!sumSafe(paid, newPayments[j].amount)) { loadOk_=false; error="Lỗi tràn số thanh toán."; return false; }
            paid+=newPayments[j].amount;
        }
        if (paid>newContracts[i].commission) { loadOk_=false; error="Thanh toán vượt phí hợp đồng."; return false; }
    }
    // Preserve every byte on disk and reject writes if the separate student,
    // tutor, or class store is missing an entity referenced by an agreement.
    for (size_t i=0;i<newContracts.size();++i) {
        const auto& record=newContracts[i];
        bool linked=false;
        if (record.type=="STUDENT") {
            for(size_t j=0;j<students_.size();++j)
                if(students_[j]->getPersonID()==record.referenceId) linked=true;
        } else if(record.type=="TUTOR") {
            for(size_t j=0;j<tutors_.size();++j)
                if(tutors_[j]->getPersonID()==record.referenceId) linked=true;
        } else {
            for(size_t j=0;j<classes_.size();++j)
                if(classes_[j]->getClassID()==record.referenceId) linked=true;
        }
        if(!linked) {
            loadOk_=false;
            error="contracts.txt: hợp đồng " + record.id +
                " tham chiếu đến đối tượng không tồn tại. Hãy phục hồi dữ liệu trước khi ghi.";
            return false;
        }
    }
    contracts_=newContracts; payments_=newPayments; loadOk_=true;
    return true;
}

const ContractRecord* ContractService::find(const string& id) const {
    for (size_t i=0;i<contracts_.size();++i) if (contracts_[i].id==id) return &contracts_[i];
    return nullptr;
}

bool ContractService::hasReference(const string& type, const string& ref) const {
    for (size_t i=0;i<contracts_.size();++i)
        if (contracts_[i].type == type && contracts_[i].referenceId == ref) return true;
    return false;
}

bool ContractService::hasSavedReference(const string& type, const string& ref) {
    namespace fs = std::filesystem;
    const fs::path path(FileManager::dataPath("contracts.txt"));
    std::error_code ec;
    if (!fs::exists(path, ec)) return static_cast<bool>(ec);  // missing is valid; I/O error is not
    ifstream input(path, ios::binary);
    if (!input) return true;  // fail closed
    string line;
    std::set<string> contractIds, paymentIds;
    while (getline(input,line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        auto f=fields(line);
        std::uint64_t number = 0;
        std::int64_t value = 0, extra = 0;
        if (f.size() == 9 && f[0] == "C") {
            if (!validType(f[2]) || f[3].empty() ||
                !parsePositiveId(f[1], prefixFor(f[2]), number) ||
                !Validator::isValidDate(f[4]) ||
                !wholeNonnegative(f[5], value) || !wholeNonnegative(f[6], extra) ||
                (f[7] != "0" && f[7] != "1") ||
                !contractIds.insert(f[1]).second) return true;
            if (f[2] == type && f[3] == ref) return true;
        } else if (f.size() == 6 && f[0] == "P") {
            if (!parsePositiveId(f[1], "PAY", number) ||
                !Validator::isValidDate(f[3]) ||
                !wholeNonnegative(f[4], value) || value == 0 ||
                f[2].empty() || !paymentIds.insert(f[1]).second) return true;
        } else return true;  // malformed row: cannot prove deletion is safe
    }
    return !input.eof();
}

bool ContractService::parsePositiveId(const string& value, const string& prefix,
                                      uint64_t& number) {
    if (value.size()<=prefix.size() || value.substr(0,prefix.size())!=prefix) return false;
    const string tail=value.substr(prefix.size());
    if (tail.size()>16) return false;
    for (char c:tail) if (c<'0'||c>'9') return false;
    try { number=stoull(tail); return number>0; } catch (...) { return false; }
}

string ContractService::makeNextId(const Vector<ContractRecord>& records,const string& type) {
    string prefix=prefixFor(type);
    uint64_t maxNumber=0, number;
    for (size_t i=0;i<records.size();++i)
        if (parsePositiveId(records[i].id,prefix,number) && number>maxNumber) maxNumber=number;
    return prefix+to_string(maxNumber+1);
}

string ContractService::makeNextPaymentId(const Vector<PaymentRecord>& records) {
    uint64_t maxNumber=0,number;
    for (size_t i=0;i<records.size();++i)
        if (parsePositiveId(records[i].id,"PAY",number) && number>maxNumber) maxNumber=number;
    return "PAY"+to_string(maxNumber+1);
}

bool ContractService::validRecord(const ContractRecord& r, const string& replacing,
                                  string& error) const {
    if (!loadOk_) { error="Dữ liệu hợp đồng chưa được đọc an toàn."; return false; }
    if (!validType(r.type) || r.referenceId.empty() ||
        !Validator::isValidDate(r.issuedDate) || r.hourlyRate<0 || r.commission<0) {
        error="Loại, đối tượng, ngày lập hoặc phí hợp đồng không hợp lệ."; return false;
    }
    uint64_t num;
    if (!parsePositiveId(r.id,prefixFor(r.type),num)) {
        error="Mã hợp đồng không đúng loại."; return false;
    }
    const ContractRecord* old=find(r.id);
    if (!replacing.empty() && replacing!=r.id) {
        error="Không được đổi mã hợp đồng."; return false;
    }
    if (replacing.empty() && old) { error="Mã hợp đồng đã tồn tại."; return false; }
    if (!replacing.empty() && !old) { error="Không tìm thấy hợp đồng cần sửa."; return false; }
    if (!replacing.empty() && (old->type!=r.type || old->referenceId!=r.referenceId)) {
        error="Không thể đổi đối tượng liên kết. Hãy lập hợp đồng mới."; return false;
    }
    bool linked=false;
    if (r.type=="STUDENT") {
        for (size_t i=0;i<students_.size();++i)
            if (students_[i]->getPersonID()==r.referenceId) linked=true;
    } else if (r.type=="TUTOR") {
        for (size_t i=0;i<tutors_.size();++i)
            if (tutors_[i]->getPersonID()==r.referenceId) linked=true;
    } else {
        for (size_t i=0;i<classes_.size();++i) {
            const auto* c=classes_[i];
            if (c->getClassID()!=r.referenceId) continue;
            linked=!c->getTutorID().empty();
            if (TextUtils::toLongLong(c->getRatePerHour(),-1)<0) linked=false;
        }
        if (!replacing.empty() && !r.active && old && old->active) {
            // Allowed to cancel a signed agreement; class is historical.
        }
    }
    if (!linked) { error="Đối tượng liên kết không tồn tại hoặc lớp chưa có gia sư."; return false; }
    if (r.type=="CLASS") {
        for (size_t i=0;i<contracts_.size();++i) {
            if (contracts_[i].id!=replacing && contracts_[i].type=="CLASS" &&
                contracts_[i].referenceId==r.referenceId) {
                error="Lớp này đã có hợp đồng (kể cả hợp đồng đã hủy)."; return false;
            }
        }
    }
    if (old && !old->active && r.active) { error="Không thể kích hoạt lại hợp đồng đã hủy."; return false; }
    if (totalPaid(r.id)>r.commission) { error="Phí mới thấp hơn tổng tiền đã thu."; return false; }
    for (size_t i=0;i<payments_.size();++i) if (payments_[i].contractId==r.id &&
        dateNumber(payments_[i].paidDate)<dateNumber(r.issuedDate)) {
        error="Ngày lập sau một khoản thanh toán đã ghi nhận."; return false;
    }
    return true;
}

bool ContractService::persist(string& error) const {
    if (!loadOk_) { error="Không thể lưu: dữ liệu hợp đồng chưa được đọc đúng."; return false; }
    ostringstream out;
    out << "# TutorFlow contracts and payment history v1\n";
    for (size_t i=0;i<contracts_.size();++i) {
        const auto& c=contracts_[i];
        out << "C|" << c.id << '|' << c.type << '|' << c.referenceId << '|'
            << c.issuedDate << '|' << c.hourlyRate << '|' << c.commission << '|'
            << (c.active?1:0) << '|' << TextUtils::cleanField(c.notes) << '\n';
    }
    for (size_t i=0;i<payments_.size();++i) {
        const auto& p=payments_[i];
        out << "P|" << p.id << '|' << p.contractId << '|' << p.paidDate << '|'
            << p.amount << '|' << TextUtils::cleanField(p.notes) << '\n';
    }
    if (!FileManager::saveTextFile("contracts.txt",out.str())) {
        error="Không ghi được hợp đồng/thanh toán; mọi thay đổi đã được hoàn tác.";
        return false;
    }
    return true;
}

bool ContractService::create(ContractRecord r,string& error) {
    if (r.id.empty()) r.id=makeNextId(contracts_,r.type);
    if (!validRecord(r,"",error)) return false;
    r.notes=TextUtils::cleanField(r.notes);
    contracts_.push_back(r);
    if (!persist(error)) { contracts_.pop_back(); return false; }
    return true;
}

bool ContractService::update(const string& id,ContractRecord r,string& error) {
    if (!validRecord(r,id,error)) return false;
    for (size_t i=0;i<contracts_.size();++i) if (contracts_[i].id==id) {
        ContractRecord old=contracts_[i];
        r.notes=TextUtils::cleanField(r.notes);
        contracts_[i]=r;
        if (!persist(error)) { contracts_[i]=old; return false; }
        return true;
    }
    error="Không tìm thấy hợp đồng."; return false;
}

bool ContractService::remove(const string& id,string& error) {
    if (!loadOk_) { error="Không thể xóa khi dữ liệu bị lỗi."; return false; }
    for (size_t i=0;i<payments_.size();++i) if (payments_[i].contractId==id) {
        error="Hợp đồng đã có thanh toán. Hãy hủy hợp đồng để giữ lịch sử, không được xóa.";
        return false;
    }
    for (size_t i=0;i<contracts_.size();++i) if (contracts_[i].id==id) {
        const Vector<ContractRecord> originalOrder=contracts_;
        contracts_.erase(i);
        if (!persist(error)) { contracts_=originalOrder; return false; }
        return true;
    }
    error="Không tìm thấy hợp đồng."; return false;
}

bool ContractService::cancel(const string& id,string& error) {
    const ContractRecord* old=find(id);
    if (!old) { error="Không tìm thấy hợp đồng."; return false; }
    ContractRecord newer=*old; newer.active=false;
    return update(id,newer,error);
}

bool ContractService::addPayment(PaymentRecord p,string& error) {
    if (!loadOk_) { error="Không thể thanh toán khi dữ liệu bị lỗi."; return false; }
    const ContractRecord* record=find(p.contractId);
    if (!record || !record->active) { error="Hợp đồng không tồn tại hoặc đã hủy."; return false; }
    if (!Validator::isValidDate(p.paidDate) ||
        dateNumber(p.paidDate)<dateNumber(record->issuedDate) || p.amount<=0 ||
        p.amount>outstanding(record->id) || !sumSafe(totalRevenue(),p.amount)) {
        error="Ngày thanh toán hoặc số tiền không hợp lệ / vượt phí còn phải thu."; return false;
    }
    if (p.id.empty()) p.id=makeNextPaymentId(payments_);
    uint64_t number;
    if (!parsePositiveId(p.id,"PAY",number)) { error="Mã thanh toán không hợp lệ."; return false; }
    for (size_t i=0;i<payments_.size();++i) if (payments_[i].id==p.id) {
        error="Mã thanh toán đã tồn tại."; return false;
    }
    p.notes=TextUtils::cleanField(p.notes);
    payments_.push_back(p);
    if (!persist(error)) { payments_.pop_back(); return false; }
    return true;
}

int64_t ContractService::totalPaid(const string& id) const {
    int64_t result=0;
    for (size_t i=0;i<payments_.size();++i) if (payments_[i].contractId==id) result+=payments_[i].amount;
    return result;
}
int64_t ContractService::outstanding(const string& id) const {
    const ContractRecord* c=find(id);
    return c ? c->commission-totalPaid(id) : 0;
}
int64_t ContractService::totalRevenue() const {
    int64_t result=0;
    for (size_t i=0;i<payments_.size();++i) result+=payments_[i].amount;
    return result;
}
int64_t ContractService::revenueByMonth(int year,int month) const {
    int64_t result=0;
    for (size_t i=0;i<payments_.size();++i) {
        const auto& p=payments_[i];
        if (dateYear(p.paidDate)==year && dateMonth(p.paidDate)==month) result+=p.amount;
    }
    return result;
}
