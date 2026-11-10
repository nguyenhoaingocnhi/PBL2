#pragma once

#include "Student.h"
#include "Tutor.h"
#include "TutoringClass.h"
#include "Vector.h"

#include <string>

// Single business-logic gateway for ALL class mutations.
// The class list is authoritative for assignment and tutor workload.
class ClassService {
public:
    ClassService(Vector<Student*>& students, Vector<Tutor*>& tutors,
                 Vector<TutoringClass*>& classes);

    // Reconstruct derived flags, live class IDs, and distinct assigned students.
    void synchronize();

    // Active means an assigned tutor and a non-final, non-unassigned status.
    static bool isAssigned(const TutoringClass& item);
    static bool canReceiveStudents(Tutor& tutor);

    bool create(const TutoringClass& candidate, std::string& error);
    bool update(const std::string& id, const TutoringClass& candidate, std::string& error);
    bool remove(const std::string& id, std::string& error);

    // Profile edits are transactions too: reject edits invalidating existing
    // assignments; restore the original object if validation/persistence fails.
    bool updateStudent(Student& student, Student proposed, std::string& error);
    bool updateTutor(Tutor& tutor, Tutor proposed, std::string& error);

    // Candidate dates for a NEW assignment, excluding all other bookings of
    // the student AND the tutor; based on available days, not raw overlap.
    Vector<std::string> availableSlots(Student& student, Tutor& tutor) const;

    bool canDeleteStudent(const std::string& id) const;
    bool canDeleteTutor(const std::string& id) const;
    bool isAssignedTo(const std::string& studentId, const std::string& tutorId) const;
    bool hasAssignedSubject(const std::string& studentId, const std::string& subject) const;
    int countBrokenReferences() const;
    int countInvalidClasses() const;
    bool validateExisting(const TutoringClass& item, std::string& error) const;
    int countDuplicateAssignments() const;

private:
    Vector<Student*>& students_;
    Vector<Tutor*>& tutors_;
    Vector<TutoringClass*>& classes_;

    Student* studentById(const std::string& id) const;
    Tutor* tutorById(const std::string& id) const;
    TutoringClass* classById(const std::string& id) const;
    bool validate(const TutoringClass& candidate, const std::string& replacingId,
                  std::string& error) const;
    bool persist(std::string& error);
};
