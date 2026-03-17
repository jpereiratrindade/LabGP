#include "domain/Scoring.hpp"

namespace labgp::domain {

namespace {
int countTrue(std::initializer_list<bool> values) {
    int count = 0;
    for (bool value : values) {
        if (value) {
            ++count;
        }
    }
    return count;
}
} // namespace

ScoreBreakdown computeScore(const ResearchProject& project) {
    const int operational =
        (project.hasReadme ? 35 : 0) +
        (project.hasCi ? 35 : 0) +
        (project.hasTests ? 30 : 0);

    const int maturity =
        (project.hasAdr ? 25 : 0) +
        (project.hasDdd ? 25 : 0) +
        (project.hasDai ? 25 : 0) +
        (project.governanceItems > 0 ? 25 : 0);

    const int reliabilityChecks = countTrue({
        project.hasAsanUbsan,
        project.hasLeakChecks,
        project.hasStaticAnalysis,
        project.hasStrictWarnings,
        project.hasComplexityGuard,
        project.hasCycleGuard,
        project.hasFormatLint,
    });
    const int reliability = (reliabilityChecks * 100) / 7;

    const int total =
        (operational * 45 + maturity * 30 + reliability * 25) / 100;

    return ScoreBreakdown{operational, maturity, reliability, total};
}

} // namespace labgp::domain
