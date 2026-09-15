#include "MatchingService.h"

#include <algorithm>
#include <functional>
#include <sstream>
#include <string>

using namespace std;

namespace {

vector<string> splitByComma(const string& text) {
    vector<string> result;
    stringstream ss(text);
    string item;
    while (getline(ss, item, ',')) {
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    return result;
}

bool hasMatchingSubject(const Student& student, const Tutor& tutor) {
    if (student.getNeedSubjects().empty()) {
        return true;
    }

    for (const auto& tutorSubject : tutor.getSubjects()) {
        for (const auto& requestedSubject : student.getNeedSubjects()) {
            if (tutorSubject == requestedSubject) {
                return true;
            }
        }
    }
    return false;
}

long long parseRate(const string& rate) {
    try {
        return stoll(rate);
    } catch (const exception&) {
        return 0;
    }
}

int countMatchingScheduleItems(const string& studentSchedule,
                               const string& tutorAvailability) {
    vector<string> s = splitByComma(studentSchedule);
    vector<string> t = splitByComma(tutorAvailability);
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

MatchResult createMatchResult(const Student& student, Tutor& tutor) {
    MatchResult result;
    result.tutor = &tutor;
    result.experienceScore = MatchingService::calculateExperienceScore(tutor.getYearsOfExperience());
    result.locationScore = MatchingService::calculateLocationScore(
        student.getAddress(), tutor.getTeachingAreas());
    result.priceScore = MatchingService::calculatePriceScore(
        parseRate(tutor.getRatePerHour()), parseRate(student.getBudgetPerHour()));
    result.scheduleScore = MatchingService::calculateScheduleScore(
        student.getAvailableSchedule(), tutor.getAvailability());
    result.totalScore = result.experienceScore + result.locationScore
                      + result.priceScore + result.scheduleScore;
    return result;
}

void sortMatches(vector<MatchResult>& results) {
    sort(results.begin(), results.end(), [](const MatchResult& left, const MatchResult& right) {
        return left.totalScore > right.totalScore;
    });
}
}

vector<MatchResult> MatchingService::matchStudentsToTutors(
    const Student& student, const vector<Tutor*>& tutors) {
    vector<reference_wrapper<Tutor>> safeTutors;
    safeTutors.reserve(tutors.size());
    for (Tutor* tutor : tutors) {
        if (tutor != nullptr) {
            safeTutors.emplace_back(*tutor);
        }
    }
    return matchStudentsToTutors(student, safeTutors);
}

vector<MatchResult> MatchingService::matchStudentsToTutors(
    const Student& student, const vector<reference_wrapper<Tutor>>& tutors) {
    vector<MatchResult> results;
    for (Tutor& tutor : tutors) {
        if (!tutor.getIsAvailable()) {
            continue;
        }

        if (hasMatchingSubject(student, tutor)) {
            results.push_back(createMatchResult(student, tutor));
        }
    }

    sortMatches(results);
    return results;
}

double MatchingService::calculateExperienceScore(int yearsOfExperience) {
    if (yearsOfExperience >= 10) return 25.0;
    if (yearsOfExperience >= 5) return 20.0;
    if (yearsOfExperience >= 3) return 15.0;
    if (yearsOfExperience >= 1) return 10.0;
    return 5.0;
}

double MatchingService::calculateLocationScore(const string& studentArea, const vector<string>& tutorAreas) {
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

double MatchingService::calculateScheduleScore(const string& studentSchedule, const string& tutorAvailability) {
    int matched = countMatchingScheduleItems(studentSchedule, tutorAvailability);
    if (matched >= 4) return 20.0;
    if (matched == 3) return 15.0;
    if (matched == 2) return 10.0;
    if (matched == 1) return 5.0;
    return 0.0;
}
