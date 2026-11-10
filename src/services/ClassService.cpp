#include "ClassService.h"
#include "ContractService.h"

#include "FileManager.h"
#include "MatchingService.h"
#include "TextUtils.h"
#include "Validator.h"
#include "SubjectUtils.h"
#include "ScheduleUtils.h"

#include <set>
#include <string>

using namespace std;

namespace {
bool validStatus(const string& status) {
    return status == "Đang học" || status == "Chờ xếp lịch" ||
           status == "Cần gia sư" || status == "Hoàn thành";
}

// Parse legacy display titles exactly once. Never test a subject by substring.
Vector<string> subjectsCoveredByClass(const string& title, Student& student) {
    Vector<string> covered;
    const auto requested = SubjectUtils::parseClassSubjects(title);
    for (size_t i = 0; i < requested.size(); ++i)
        if (SubjectUtils::contains(student.getNeedSubjects(), requested[i]))
            covered.push_back(requested[i]);
    return covered;
}

bool tutorTeachesAll(Vector<string>& covered, Tutor& tutor) {
    Vector<string>& teachings = tutor.getSubjects();
    for (size_t i = 0; i < covered.size(); ++i) {
        bool found = false;
        for (size_t j = 0; j < teachings.size(); ++j) {
            if (SubjectUtils::equals(teachings[j], covered[i])) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}
}

ClassService::ClassService(Vector<Student*>& students, Vector<Tutor*>& tutors,
                           Vector<TutoringClass*>& classes)
    : students_(students), tutors_(tutors), classes_(classes) {}

bool ClassService::isAssigned(const TutoringClass& item) {
    return !item.getTutorID().empty() &&
           (item.getStatus() == "Đang học" || item.getStatus() == "Chờ xếp lịch");
}

bool ClassService::canReceiveStudents(Tutor& tutor) {
    return tutor.getIsAvailable() &&
           tutor.getCurrentStudents() < MatchingService::kMaxStudentsPerTutor;
}

Student* ClassService::studentById(const string& id) const {
    for (size_t i = 0; i < students_.size(); ++i)
        if (students_[i]->getPersonID() == id) return students_[i];
    return nullptr;
}

Tutor* ClassService::tutorById(const string& id) const {
    for (size_t i = 0; i < tutors_.size(); ++i)
        if (tutors_[i]->getPersonID() == id) return tutors_[i];
    return nullptr;
}

TutoringClass* ClassService::classById(const string& id) const {
    for (size_t i = 0; i < classes_.size(); ++i)
        if (classes_[i]->getClassID() == id) return classes_[i];
    return nullptr;
}

void ClassService::synchronize() {
    for (size_t i = 0; i < students_.size(); ++i) {
        students_[i]->setIsMatched(false);
        students_[i]->setEnrolledClasses({});
    }
    for (size_t i = 0; i < tutors_.size(); ++i) {
        tutors_[i]->setCurrentStudents(0);
        tutors_[i]->setTeachingClasses({});
    }

    // Do not count finished/unassigned classes; count a student just once per tutor.
    std::set<std::pair<string, string>> distinct;
    for (size_t i = 0; i < classes_.size(); ++i) {
        TutoringClass* c = classes_[i];
        if (!isAssigned(*c)) continue;
        Student* student = studentById(c->getStudentID());
        Tutor* tutor = tutorById(c->getTutorID());
        if (!student || !tutor) continue;  // preserve legacy records, flag in audit
        student->setIsMatched(true);
        student->getEnrolledClasses().push_back(c->getClassID());
        tutor->getTeachingClasses().push_back(c->getClassID());
        if (distinct.insert({c->getTutorID(), c->getStudentID()}).second)
            tutor->setCurrentStudents(tutor->getCurrentStudents() + 1);
    }
}

bool ClassService::validate(const TutoringClass& c, const string& replacingId,
                            string& error) const {
    if (c.getClassID().empty() || (!replacingId.empty() && c.getClassID() != replacingId)) {
        error = "Mã lớp không hợp lệ hoặc không được phép thay đổi.";
        return false;
    }
    for (size_t i = 0; i < classes_.size(); ++i) {
        if (classes_[i]->getClassID() == c.getClassID() && c.getClassID() != replacingId) {
            error = "Mã lớp đã tồn tại.";
            return false;
        }
    }
    Student* student = studentById(c.getStudentID());
    if (!student) { error = "Học sinh không tồn tại."; return false; }
    Tutor* tutor = c.getTutorID().empty() ? nullptr : tutorById(c.getTutorID());
    if (!c.getTutorID().empty() && !tutor) { error = "Gia sư không tồn tại."; return false; }
    if (TextUtils::trim(c.getSubject()).empty()) { error = "Môn học không được để trống."; return false; }
    if (!validStatus(c.getStatus())) { error = "Trạng thái lớp không hợp lệ."; return false; }
    const TutoringClass* previous = replacingId.empty() ? nullptr : classById(replacingId);
    if (previous && previous->getStatus() == "Hoàn thành" && c.getStatus() != "Hoàn thành") {
        error = "Lớp đã hoàn thành không thể tự kích hoạt lại. Hãy tạo lớp mới.";
        return false;
    }
    if ((c.getStatus() == "Đang học" || c.getStatus() == "Chờ xếp lịch") && !tutor) {
        error = "Lớp đang học / chờ xếp lịch phải có gia sư."; return false;
    }
    if (c.getStatus() == "Cần gia sư" && tutor) {
        error = "Lớp cần gia sư không được gán gia sư. Hãy đổi trạng thái lớp.";
        return false;
    }
    if (c.getStatus() == "Hoàn thành" && !tutor) {
        error = "Không thể hoàn thành lớp chưa từng có gia sư."; return false;
    }
    if (!Validator::isValidDate(c.getStartDate())) {
        error = "Ngày bắt đầu không hợp lệ (dd/MM/yyyy)."; return false;
    }
    if (!Validator::isNonNegativeNumber(c.getRatePerHour())) {
        error = "Học phí phải là số không âm."; return false;
    }
    const Vector<string> declared = SubjectUtils::parseClassSubjects(c.getSubject());
    Vector<string> covered = subjectsCoveredByClass(c.getSubject(), *student);
    if (c.getStatus() != "Hoàn thành" && (covered.empty() || covered.size() != declared.size())) {
        error = "Môn của lớp chưa có trong danh sách môn học sinh đăng ký.";
        return false;
    }
    if (!ScheduleUtils::valid(c.getSchedule(), c.getStatus() != "Đang học")) {
        error = "Lịch học phải gồm các ngày T2..T7 hoặc CN, ngăn cách bằng dấu phẩy; lớp đang học cần lịch.";
        return false;
    }
    if (!isAssigned(c)) return true;

    if (!tutorTeachesAll(covered, *tutor)) {
        error = "Gia sư phải dạy được tất cả môn được ghi trong lớp.";
        return false;
    }
    if (!ScheduleUtils::valid(student->getAvailableSchedule()) ||
        !ScheduleUtils::valid(tutor->getAvailability()) ||
        (!c.getSchedule().empty() &&
         (!ScheduleUtils::isSubset(c.getSchedule(), student->getAvailableSchedule()) ||
          !ScheduleUtils::isSubset(c.getSchedule(), tutor->getAvailability())))) {
        error = "Lịch lớp phải là tập con các ngày rảnh của cả học sinh và gia sư.";
        return false;
    }
    long long budget = TextUtils::toLongLong(student->getBudgetPerHour(), 0);
    long long fee = TextUtils::toLongLong(c.getRatePerHour(), 0);
    if (budget > 0 && fee > budget) {
        error = "Học phí lớp vượt ngân sách của học sinh."; return false;
    }

    // A student/tutor pair represents one live assignment. Different subjects
    // for the same pair should be consolidated in the same class.
    std::set<string> assignedStudents;
    for (size_t i = 0; i < classes_.size(); ++i) {
        TutoringClass* current = classes_[i];
        if (current->getClassID() == replacingId || !isAssigned(*current)) continue;
        if (current->getTutorID() == tutor->getPersonID())
            assignedStudents.insert(current->getStudentID());
        // A day-only calendar cannot distinguish two lessons on one day.
        // Treat all scheduled live classes as reserving those days.
        if (!c.getSchedule().empty() && !current->getSchedule().empty() &&
            ScheduleUtils::intersects(c.getSchedule(), current->getSchedule()) &&
            (current->getStudentID() == c.getStudentID() ||
             current->getTutorID() == c.getTutorID())) {
            error = "Trùng ngày học với lớp " + current->getClassID() +
                    " của học sinh hoặc gia sư. Hãy chọn ngày khác.";
            return false;
        }
        if (current->getStudentID() != c.getStudentID()) continue;
        if (current->getTutorID() == c.getTutorID()) {
            error = "Học sinh đã có lớp đang hiệu lực với gia sư này (không ghép trùng).";
            return false;
        }
        Vector<string> oldCovered = SubjectUtils::parseClassSubjects(current->getSubject());
        for (size_t subjectIndex = 0; subjectIndex < covered.size(); ++subjectIndex) {
            if (SubjectUtils::contains(oldCovered, covered[subjectIndex])) {
                error = "Môn này đã được học sinh ghép ở một lớp khác. Hãy chọn môn chưa ghép.";
                return false;
            }
        }
    }
    if (!assignedStudents.count(c.getStudentID()) &&
        assignedStudents.size() >= static_cast<size_t>(MatchingService::kMaxStudentsPerTutor)) {
        error = "Gia sư đã đạt giới hạn số học sinh."; return false;
    }

    // Administrators can finish or edit an existing assignment even if the
    // tutor is marked unavailable. New assignments require explicit availability.
    const TutoringClass* old = replacingId.empty() ? nullptr : classById(replacingId);
    const bool sameExisting = old && isAssigned(*old) && old->getTutorID() == c.getTutorID() &&
                              old->getStudentID() == c.getStudentID();
    if (!sameExisting && !tutor->getIsAvailable()) {
        error = "Gia sư hiện không nhận học sinh mới."; return false;
    }
    return true;
}

bool ClassService::persist(string& error) {
    if (!FileManager::saveAll(students_, tutors_, classes_)) {
        error = "Không thể lưu đồng bộ dữ liệu. Các thay đổi đã được hoàn tác.";
        return false;
    }
    return true;
}

bool ClassService::create(const TutoringClass& candidate, string& error) {
    if (!validate(candidate, "", error)) return false;
    auto* item = new TutoringClass(candidate);
    classes_.push_back(item);
    synchronize();
    if (!persist(error)) {
        classes_.pop_back();
        delete item;
        synchronize();
        return false;
    }
    return true;
}

bool ClassService::update(const string& id, const TutoringClass& candidate, string& error) {
    TutoringClass* current = classById(id);
    if (!current) { error = "Không tìm thấy lớp học."; return false; }
    if (!validate(candidate, id, error)) return false;
    if (ContractService::hasSavedReference("CLASS", id) &&
        (current->getStudentID() != candidate.getStudentID() ||
         current->getTutorID() != candidate.getTutorID() ||
         current->getSubject() != candidate.getSubject() ||
         current->getRatePerHour() != candidate.getRatePerHour() ||
         current->getStartDate() != candidate.getStartDate())) {
        error = "Lớp đã có hợp đồng; không thể thay đổi các điều khoản đã ký. "
                "Hãy xử lý hợp đồng trước.";
        return false;
    }
    TutoringClass before = *current;
    *current = candidate;
    synchronize();
    if (!persist(error)) {
        *current = before;
        synchronize();
        return false;
    }
    return true;
}

bool ClassService::remove(const string& id, string& error) {
    if (ContractService::hasSavedReference("CLASS", id)) {
        error = "Lớp đã có hợp đồng (kể cả đã hủy). Hãy xử lý hợp đồng trước khi xóa lớp.";
        return false;
    }
    for (size_t i = 0; i < classes_.size(); ++i) {
        if (classes_[i]->getClassID() != id) continue;
        TutoringClass* item = classes_[i];
        const Vector<TutoringClass*> originalOrder = classes_;
        classes_.erase(i);
        synchronize();
        if (!persist(error)) {
            classes_ = originalOrder;
            synchronize();
            return false;
        }
        delete item;
        return true;
    }
    error = "Không tìm thấy lớp học.";
    return false;
}

Vector<string> ClassService::availableSlots(Student& student, Tutor& tutor) const {
    Vector<string> result;
    const auto studentDays = ScheduleUtils::parse(student.getAvailableSchedule());
    const auto tutorDays = ScheduleUtils::parse(tutor.getAvailability());
    if (!ScheduleUtils::valid(student.getAvailableSchedule(), false) ||
        !ScheduleUtils::valid(tutor.getAvailability(), false)) return result;
    for (size_t i = 0; i < studentDays.size(); ++i) {
        const string& day = studentDays[i];
        if (!ScheduleUtils::contains(tutorDays, day)) continue;
        bool busy = false;
        for (size_t j = 0; j < classes_.size(); ++j) {
            const auto* c = classes_[j];
            if (!isAssigned(*c) || c->getSchedule().empty()) continue;
            if ((c->getStudentID() == student.getPersonID() ||
                 c->getTutorID() == tutor.getPersonID()) &&
                ScheduleUtils::contains(ScheduleUtils::parse(c->getSchedule()), day)) {
                busy = true;
                break;
            }
        }
        if (!busy) result.push_back(day);
    }
    return result;
}

bool ClassService::updateStudent(Student& student, Student proposed, string& error) {
    if (student.getPersonID() != proposed.getPersonID()) {
        error = "Không được đổi mã học sinh sau khi tạo."; return false;
    }
    if (proposed.getNeedSubjects().empty() ||     !ScheduleUtils::valid(proposed.getAvailableSchedule()) ||
    !Validator::isValidDate(proposed.getDateOfBirth()) ||
    !Validator::isValidPhone(proposed.getPhone()) ||
    !Validator::isNonNegativeNumber(proposed.getBudgetPerHour())) {
        error = "Hồ sơ học sinh chứa môn, lịch, ngày sinh, số điện thoại hoặc ngân sách không hợp lệ.";
        return false;
    }
    Student original = student;
    student = proposed;
    for (size_t i = 0; i < classes_.size(); ++i) {
        const auto& cls = *classes_[i];
        if (cls.getStudentID() == student.getPersonID() && cls.getStatus() != "Hoàn thành" &&
            !validate(cls, cls.getClassID(), error)) {
            student = original;
            synchronize();
            error = "Hồ sơ mới làm lớp " + cls.getClassID() + " không hợp lệ: " + error;
            return false;
        }
    }
    synchronize();
    if (!persist(error)) { student = original; synchronize(); return false; }
    return true;
}

bool ClassService::updateTutor(Tutor& tutor, Tutor proposed, string& error) {
    if (tutor.getPersonID() != proposed.getPersonID()) {
        error = "Không được đổi mã gia sư sau khi tạo."; return false;
    }
    if (proposed.getSubjects().empty() ||     !ScheduleUtils::valid(proposed.getAvailability()) ||
    !Validator::isValidDate(proposed.getDateOfBirth()) ||
    !Validator::isValidPhone(proposed.getPhone()) ||
    !Validator::isNonNegativeNumber(proposed.getRatePerHour()) ||
        proposed.getYearsOfExperience() < 0) {
        error = "Hồ sơ gia sư chứa môn, lịch, ngày sinh, số điện thoại hoặc học phí không hợp lệ.";
        return false;
    }
    Tutor original = tutor;
    tutor = proposed;
    for (size_t i = 0; i < classes_.size(); ++i) {
        const auto& cls = *classes_[i];
        if (cls.getTutorID() == tutor.getPersonID() && isAssigned(cls) &&
            !validate(cls, cls.getClassID(), error)) {
            tutor = original;
            synchronize();
            error = "Hồ sơ mới làm lớp " + cls.getClassID() + " không hợp lệ: " + error;
            return false;
        }
    }
    synchronize();
    if (!persist(error)) { tutor = original; synchronize(); return false; }
    return true;
}

bool ClassService::canDeleteStudent(const string& id) const {
    if (ContractService::hasSavedReference("STUDENT", id)) return false;
    for (size_t i = 0; i < classes_.size(); ++i)
        if (classes_[i]->getStudentID() == id) return false;
    return true;
}

bool ClassService::canDeleteTutor(const string& id) const {
    if (ContractService::hasSavedReference("TUTOR", id)) return false;
    for (size_t i = 0; i < classes_.size(); ++i)
        if (classes_[i]->getTutorID() == id) return false;
    return true;
}

bool ClassService::hasAssignedSubject(const string& studentId, const string& subject) const {
    if (SubjectUtils::canonical(subject).empty()) return false;
    for (size_t i = 0; i < classes_.size(); ++i) {
        const TutoringClass& c = *classes_[i];
        if (c.getStudentID() == studentId && isAssigned(c) &&
            SubjectUtils::contains(SubjectUtils::parseClassSubjects(c.getSubject()), subject))
            return true;
    }
    return false;
}

bool ClassService::isAssignedTo(const string& studentId, const string& tutorId) const {
    for (size_t i = 0; i < classes_.size(); ++i) {
        const TutoringClass& c = *classes_[i];
        if (c.getStudentID() == studentId && c.getTutorID() == tutorId && isAssigned(c))
            return true;
    }
    return false;
}

int ClassService::countDuplicateAssignments() const {
    int duplicates = 0;
    std::set<std::pair<string, string>> seen;
    for (size_t i = 0; i < classes_.size(); ++i) {
        const TutoringClass& c = *classes_[i];
        if (!isAssigned(c)) continue;
        if (!seen.insert({c.getStudentID(), c.getTutorID()}).second)
            ++duplicates;
    }
    return duplicates;
}

int ClassService::countBrokenReferences() const {
    int invalid = 0;
    for (size_t i = 0; i < classes_.size(); ++i) {
        const TutoringClass& c = *classes_[i];
        if (!studentById(c.getStudentID()) ||
            (!c.getTutorID().empty() && !tutorById(c.getTutorID()))) ++invalid;
    }
    return invalid;
}

int ClassService::countInvalidClasses() const {
    int invalid = 0;
    for (size_t i = 0; i < classes_.size(); ++i) {
        string error;
        if (!validate(*classes_[i], classes_[i]->getClassID(), error)) ++invalid;
    }
    return invalid;
}

bool ClassService::validateExisting(const TutoringClass& item, std::string& error) const {
    return validate(item, item.getClassID(), error);
}
