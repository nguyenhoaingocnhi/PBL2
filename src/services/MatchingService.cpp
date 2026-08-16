#include "MatchingService.h"

#include <algorithm>
#include <sstream>
#include <string>

namespace {
std::vector<std::string> splitByComma(const std::string& text) {
    std::vector<std::string> result;
    std::stringstream ss(text);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    return result;
}

int countMatches(const std::string& studentSchedule, const std::string& tutorAvailability) {
    std::vector<std::string> s = splitByComma(studentSchedule);
    std::vector<std::string> t = splitByComma(tutorAvailability);
    int matches = 0;
    for (const auto& day : s) {
        for (const auto& tutorDay : t) {
            if (day == tutorDay) {
                ++matches;
                break;
            }
        }
    }
    return matches;
}
}

std::vector<MatchResult> MatchingService::matchStudentsToTutors(const Student& student, const std::vector<Tutor*>& tutors) {
    std::vector<MatchResult> results;
    for (Tutor* tutor : tutors) {
        if (tutor == nullptr || !tutor->getIsAvailable()) {
            continue;
        }

        bool hasSubject = false;
        for (const auto& subject : tutor->getSubjects()) {
            if (student.getNeedSubjects().empty()) {
                hasSubject = true;
                break;
            }
            for (const auto& need : student.getNeedSubjects()) {
                if (subject == need) {
                    hasSubject = true;
                    break;
                }
            }
            if (hasSubject) break;
        }
        if (!hasSubject) continue;

        MatchResult result;
        result.tutor = tutor;
        result.experienceScore = calculateExperienceScore(tutor->getYearsOfExperience());
        result.locationScore = calculateLocationScore(student.getAddress(), tutor->getTeachingAreas());

        long long tutorRate = std::stoll(tutor->getRatePerHour());
        long long studentBudget = std::stoll(student.getBudgetPerHour());
        result.priceScore = calculatePriceScore(tutorRate, studentBudget);
        result.scheduleScore = calculateScheduleScore(student.getAvailableSchedule(), tutor->getAvailability());
        result.totalScore = result.experienceScore + result.locationScore + result.priceScore + result.scheduleScore;

        results.push_back(result);
    }

    std::sort(results.begin(), results.end(), [](const MatchResult& a, const MatchResult& b) {
        return a.totalScore > b.totalScore;
    });

    return results;
}

double MatchingService::calculateExperienceScore(int yearsOfExperience) {
    if (yearsOfExperience >= 10) return 25.0;
    if (yearsOfExperience >= 5) return 20.0;
    if (yearsOfExperience >= 3) return 15.0;
    if (yearsOfExperience >= 1) return 10.0;
    return 5.0;
}

double MatchingService::calculateLocationScore(const std::string& studentArea, const std::vector<std::string>& tutorAreas) {
    for (const auto& area : tutorAreas) {
        if (area == studentArea) {
            return 10.0;
        }
    }
    return tutorAreas.empty() ? 0.0 : 5.0;
}

double MatchingService::calculatePriceScore(long long tutorRate, long long studentBudget) {
    double r = (studentBudget == 0) ? 0.0 : static_cast<double>(tutorRate) / static_cast<double>(studentBudget);
    if (r <= 0.8) return 45.0;
    if (r <= 1.0) return 40.0;
    if (r <= 1.2) return 30.0;
    if (r <= 1.5) return 20.0;
    return 10.0;
}

double MatchingService::calculateScheduleScore(const std::string& studentSchedule, const std::string& tutorAvailability) {
    int matched = countMatches(studentSchedule, tutorAvailability);
    if (matched >= 4) return 20.0;
    if (matched == 3) return 15.0;
    if (matched == 2) return 10.0;
    if (matched == 1) return 5.0;
    return 0.0;
}
