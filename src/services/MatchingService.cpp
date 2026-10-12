#include "MatchingService.h"

#include <sstream>
#include <string>
#include <utility>

using namespace std;

namespace {

Vector<string> splitByComma(string text) {
    Vector<string> result;
    stringstream ss(text);
    string item;
    while (getline(ss, item, ',')) {
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    return result;
}

bool hasMatchingSubject(Student& student, Tutor& tutor) {
    if (student.getNeedSubjects().empty()) {
        return true;
    }

    for (size_t i = 0; i < tutor.getSubjects().size(); ++i) {
        for (size_t j = 0; j < student.getNeedSubjects().size(); ++j) {
            if (tutor.getSubjects()[i] == student.getNeedSubjects()[j]) {
                return true;
            }
        }
    }
    return false;
}

long long parseRate(string rate) {
    try {
        return stoll(rate);
    } catch (const exception&) {
        return 0;
    }
}

int countMatchingScheduleItems(string studentSchedule,
                               string tutorAvailability) {
    Vector<string> s = splitByComma(studentSchedule);
    Vector<string> t = splitByComma(tutorAvailability);
    int matches = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        for (size_t j = 0; j < t.size(); ++j) {
            if (s[i] == t[j]) {
                ++matches;
                break;
            }
        }
    }
    return matches;
}

MatchResult createMatchResult(Student& student, Tutor& tutor) {
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

void sortMatches(Vector<MatchResult>& results) {
    for (size_t i = 0; i < results.size(); ++i) {
        for (size_t j = i + 1; j < results.size(); ++j) {
            if (results[j].totalScore > results[i].totalScore) {
                swap(results[i], results[j]);
            }
        }
    }
}
}

Vector<MatchResult> MatchingService::matchStudentsToTutors(
    Student& student, Vector<Tutor*>& tutors) {
    Vector<MatchResult> results;
    for (size_t i = 0; i < tutors.size(); ++i) {
        Tutor* tutor = tutors[i];
        if (tutor == nullptr || !tutor->getIsAvailable()) {
            continue;
        }

        if (hasMatchingSubject(student, *tutor)) {
            results.push_back(createMatchResult(student, *tutor));
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

double MatchingService::calculateLocationScore(string studentArea, Vector<string>& tutorAreas) {
    for (size_t i = 0; i < tutorAreas.size(); ++i) {
        if (tutorAreas[i] == studentArea) {
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

double MatchingService::calculateScheduleScore(string studentSchedule, string tutorAvailability) {
    int matched = countMatchingScheduleItems(studentSchedule, tutorAvailability);
    if (matched >= 4) return 20.0;
    if (matched == 3) return 15.0;
    if (matched == 2) return 10.0;
    if (matched == 1) return 5.0;
    return 0.0;
}
