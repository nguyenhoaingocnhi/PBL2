#pragma once

#include "Student.h"
#include "Tutor.h"
#include "Vector.h"
#include <string>

using namespace std;

// Kết quả ghép của một gia sư với một học sinh.
// Điểm thành phần giữ thang cũ để giao diện hiển thị đúng:
//   kinh nghiệm /25, khu vực /10, học phí /45, lịch /20  (tổng nền tối đa 100).
// totalScore = tổng nền x hệ số phủ môn + điều chỉnh giới tính - phạt quá tải, giới hạn 0..100.
struct MatchResult {
    Tutor* tutor = nullptr;
    double experienceScore = 0.0;
    double locationScore = 0.0;
    double priceScore = 0.0;
    double scheduleScore = 0.0;
    double subjectCoverage = 0.0;  // 0..1: tỉ lệ môn học sinh cần mà gia sư dạy được
    double adjustment = 0.0;       // điều chỉnh giới tính (+/-) và phạt số học sinh hiện có
    double totalScore = 0.0;
    Vector<string> reasons;        // lý do ghép, hiển thị cho người dùng
};

class MatchingService {
public:
    // Số học sinh tối đa một gia sư nhận; đủ số này gia sư không còn xuất hiện trong kết quả.
    static constexpr int kMaxStudentsPerTutor = 8;

    // Trả về danh sách gia sư phù hợp, sắp xếp điểm giảm dần (merge sort, O(n log n)).
    static Vector<MatchResult> matchStudentsToTutors(Student& student, Vector<Tutor*>& tutors);

    // Chỉ lấy n kết quả đầu.
    static Vector<MatchResult> matchTopN(Student& student, Vector<Tutor*>& tutors, size_t n);

    static double calculateExperienceScore(int yearsOfExperience);
    static double calculateLocationScore(string studentArea, Vector<string>& tutorAreas);
    static double calculatePriceScore(long long tutorRate, long long studentBudget);
    static double calculateScheduleScore(string studentSchedule, string tutorAvailability);
};
