#pragma once

#include "Student.h"
#include "Tutor.h"
#include "Vector.h"
#include <string>

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
    static Vector<MatchResult> matchStudentsToTutors(Student& student, Vector<Tutor*>& tutors);
    static double calculateExperienceScore(int yearsOfExperience);
    static double calculateLocationScore(string studentArea, Vector<string>& tutorAreas);
    static double calculatePriceScore(long long tutorRate, long long studentBudget);
    static double calculateScheduleScore(string studentSchedule, string tutorAvailability);
};
