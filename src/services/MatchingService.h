#ifndef MATCHINGSERVICE_H
#define MATCHINGSERVICE_H

#include "Student.h"
#include "Tutor.h"
#include <vector>
#include <string>

struct MatchResult {
    Tutor* tutor = nullptr;
    double experienceScore = 0.0;
    double locationScore = 0.0;
    double priceScore = 0.0;
    double scheduleScore = 0.0;
    double totalScore = 0.0;
};

class MatchingService {
public:
    static std::vector<MatchResult> matchStudentsToTutors(const Student& student, const std::vector<Tutor*>& tutors);
    static double calculateExperienceScore(int yearsOfExperience);
    static double calculateLocationScore(const std::string& studentArea, const std::vector<std::string>& tutorAreas);
    static double calculatePriceScore(long long tutorRate, long long studentBudget);
    static double calculateScheduleScore(const std::string& studentSchedule, const std::string& tutorAvailability);
};

#endif
