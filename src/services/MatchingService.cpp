#include "MatchingService.h"

#include "Algorithms.h"
#include "TextUtils.h"

#include <cmath>
#include <string>

using namespace std;

namespace {

double round1(double v) { return std::round(v * 10.0) / 10.0; }

bool containsValue(const Vector<string>& list, const string& value) {
    for (size_t i = 0; i < list.size(); ++i) {
        if (list[i] == value) return true;
    }
    return false;
}

// Loại bỏ phần tử trùng nhau (giữ thứ tự xuất hiện).
Vector<string> unique(const Vector<string>& list) {
    Vector<string> out;
    for (size_t i = 0; i < list.size(); ++i) {
        if (!containsValue(out, list[i])) out.push_back(list[i]);
    }
    return out;
}

// Số mục trong lịch học sinh mà gia sư cũng rảnh (so sánh sau khi chuẩn hóa).
int countScheduleOverlap(const Vector<string>& studentSlots, const Vector<string>& tutorSlots) {
    int matched = 0;
    for (size_t i = 0; i < studentSlots.size(); ++i) {
        if (containsValue(tutorSlots, studentSlots[i])) ++matched;
    }
    return matched;
}

string formatMoney(long long amount) {
    string digits = to_string(amount);
    string out;
    int count = 0;
    for (size_t i = digits.size(); i > 0; --i) {
        out.insert(out.begin(), digits[i - 1]);
        if (++count % 3 == 0 && i > 1) out.insert(out.begin(), '.');
    }
    return out;
}

// Học sinh có yêu cầu giới tính gia sư không? Trả về "nam" / "nu" hoặc rỗng nếu không yêu cầu.
// (Dữ liệu cũ dùng "Khong"/"Co" nên coi như không yêu cầu.)
string genderPreference(Student& student) {
    string pref = TextUtils::normalize(student.getPreferredTutorGender());
    if (pref == "nam" || pref == "nu") return pref;
    return "";
}

// Số môn học sinh cần mà gia sư dạy được.
int countSubjectMatches(const Vector<string>& needed, Vector<string>& tutorSubjects) {
    Vector<string> tutorNorm;
    for (size_t i = 0; i < tutorSubjects.size(); ++i) {
        string n = TextUtils::normalize(tutorSubjects[i]);
        if (!n.empty()) tutorNorm.push_back(n);
    }
    int matched = 0;
    for (size_t i = 0; i < needed.size(); ++i) {
        if (containsValue(tutorNorm, needed[i])) ++matched;
    }
    return matched;
}

MatchResult createMatchResult(Student& student, Tutor& tutor, const Vector<string>& neededSubjects,
                              int matchedSubjects) {
    MatchResult r;
    r.tutor = &tutor;

    long long rate = TextUtils::toLongLong(tutor.getRatePerHour(), 0);
    long long budget = TextUtils::toLongLong(student.getBudgetPerHour(), 0);

    r.experienceScore = MatchingService::calculateExperienceScore(tutor.getYearsOfExperience());
    r.locationScore = MatchingService::calculateLocationScore(student.getAddress(), tutor.getTeachingAreas());
    r.priceScore = MatchingService::calculatePriceScore(rate, budget);
    r.scheduleScore = MatchingService::calculateScheduleScore(student.getAvailableSchedule(), tutor.getAvailability());

    r.subjectCoverage = neededSubjects.empty()
                            ? 1.0
                            : static_cast<double>(matchedSubjects) / static_cast<double>(neededSubjects.size());

    double base = r.experienceScore + r.locationScore + r.priceScore + r.scheduleScore;
    double adjustment = 0.0;

    string pref = genderPreference(student);
    string tutorGender = TextUtils::normalize(tutor.getGender());
    if (!pref.empty() && !tutorGender.empty()) {
        adjustment += (pref == tutorGender) ? 3.0 : -8.0;
    }
    // Phạt nhẹ gia sư đang dạy nhiều học sinh để chia đều tải.
    adjustment -= static_cast<double>(tutor.getCurrentStudents());
    r.adjustment = round1(adjustment);

    double total = base * (0.85 + 0.15 * r.subjectCoverage) + adjustment;
    if (total < 0.0) total = 0.0;
    if (total > 100.0) total = 100.0;
    r.totalScore = round1(total);

    // ---- Lý do ghép (hiển thị trên giao diện) ----
    if (!neededSubjects.empty()) {
        r.reasons.push_back("Dạy được " + to_string(matchedSubjects) + "/" + to_string(neededSubjects.size()) +
                            " môn học sinh cần");
    }
    if (r.locationScore >= 8.0) {
        r.reasons.push_back("Dạy tại khu vực của học sinh");
    } else {
        r.reasons.push_back("Chưa dạy ở khu vực của học sinh");
    }
    Vector<string> studentSlots = unique(TextUtils::splitNormalized(student.getAvailableSchedule(), ','));
    Vector<string> tutorSlots = TextUtils::splitNormalized(tutor.getAvailability(), ',');
    if (!studentSlots.empty()) {
        r.reasons.push_back("Trùng " + to_string(countScheduleOverlap(studentSlots, tutorSlots)) + "/" +
                            to_string(studentSlots.size()) + " buổi rảnh của học sinh");
    }
    if (rate > 0 && budget > 0) {
        if (rate <= budget) {
            r.reasons.push_back("Học phí " + formatMoney(rate) + "đ/buổi trong ngân sách " + formatMoney(budget) + "đ");
        } else {
            r.reasons.push_back("Học phí " + formatMoney(rate) + "đ/buổi vượt ngân sách " + formatMoney(budget) + "đ");
        }
    }
    r.reasons.push_back("Kinh nghiệm " + to_string(tutor.getYearsOfExperience()) + " năm");
    if (!pref.empty() && !tutorGender.empty() && pref != tutorGender) {
        r.reasons.push_back("Không đúng giới tính gia sư học sinh mong muốn");
    }
    return r;
}

}  // namespace

Vector<MatchResult> MatchingService::matchStudentsToTutors(Student& student, Vector<Tutor*>& tutors) {
    Vector<MatchResult> results;
    Vector<string> needed = unique(TextUtils::splitNormalized(
        [&]() {
            string joined;
            Vector<string>& subs = student.getNeedSubjects();
            for (size_t i = 0; i < subs.size(); ++i) {
                if (i > 0) joined += ",";
                joined += subs[i];
            }
            return joined;
        }(),
        ','));

    for (size_t i = 0; i < tutors.size(); ++i) {
        Tutor* tutor = tutors[i];
        if (tutor == nullptr || !tutor->getIsAvailable()) continue;
        if (tutor->getCurrentStudents() >= kMaxStudentsPerTutor) continue;

        int matched = countSubjectMatches(needed, tutor->getSubjects());
        // Có yêu cầu môn học thì gia sư phải dạy được ít nhất một môn.
        if (!needed.empty() && matched == 0) continue;

        results.push_back(createMatchResult(student, *tutor, needed, matched));
    }

    mergeSort(results, [](const MatchResult& a, const MatchResult& b) {
        if (a.totalScore != b.totalScore) return a.totalScore > b.totalScore;
        long long ra = TextUtils::toLongLong(a.tutor->getRatePerHour(), 0);
        long long rb = TextUtils::toLongLong(b.tutor->getRatePerHour(), 0);
        if (ra != rb) return ra < rb;  // cùng điểm: học phí thấp hơn đứng trước
        return a.tutor->getPersonID() < b.tutor->getPersonID();
    });
    return results;
}

Vector<MatchResult> MatchingService::matchTopN(Student& student, Vector<Tutor*>& tutors, size_t n) {
    Vector<MatchResult> all = matchStudentsToTutors(student, tutors);
    while (all.size() > n) all.pop_back();
    return all;
}

double MatchingService::calculateExperienceScore(int yearsOfExperience) {
    if (yearsOfExperience >= 10) return 25.0;
    if (yearsOfExperience >= 5) return 20.0;
    if (yearsOfExperience >= 3) return 15.0;
    if (yearsOfExperience >= 1) return 10.0;
    return 5.0;
}

double MatchingService::calculateLocationScore(string studentArea, Vector<string>& tutorAreas) {
    if (tutorAreas.empty()) return 5.0;  // chưa biết khu vực -> điểm trung tính
    string area = TextUtils::normalize(studentArea);
    if (area.empty()) return 5.0;
    double best = 3.0;  // khác khu vực
    for (size_t i = 0; i < tutorAreas.size(); ++i) {
        string t = TextUtils::normalize(tutorAreas[i]);
        if (t.empty()) continue;
        if (t == area) return 10.0;
        if (t.find(area) != string::npos || area.find(t) != string::npos) best = 8.0;  // vd "quan hai chau"
    }
    return best;
}

double MatchingService::calculatePriceScore(long long tutorRate, long long studentBudget) {
    // Thiếu dữ liệu (ngân sách hoặc học phí = 0) -> điểm trung tính, không được thưởng tối đa.
    if (studentBudget <= 0 || tutorRate <= 0) return 22.0;
    double r = static_cast<double>(tutorRate) / static_cast<double>(studentBudget);
    if (r <= 0.8) return 45.0;
    if (r <= 1.0) return 40.0;
    if (r <= 1.2) return 30.0;
    if (r <= 1.5) return 20.0;
    return 10.0;
}

double MatchingService::calculateScheduleScore(string studentSchedule, string tutorAvailability) {
    Vector<string> s = unique(TextUtils::splitNormalized(studentSchedule, ','));
    Vector<string> t = TextUtils::splitNormalized(tutorAvailability, ',');
    if (s.empty()) return 10.0;  // học sinh chưa nêu lịch -> trung tính
    double ratio = static_cast<double>(countScheduleOverlap(s, t)) / static_cast<double>(s.size());
    return round1(20.0 * ratio);
}
