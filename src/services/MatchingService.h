#pragma once

#include "Student.h"
#include "Tutor.h"
#include <string>
#include <vector>

using namespace std;

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
    static vector<MatchResult> matchStudentsToTutors(Student& student, vector<Tutor*>& tutors);
    static double calculateExperienceScore(int yearsOfExperience);
    static double calculateLocationScore(string studentArea, vector<string>& tutorAreas);
    static double calculatePriceScore(long long tutorRate, long long studentBudget);
    static double calculateScheduleScore(string studentSchedule, string tutorAvailability);
};
