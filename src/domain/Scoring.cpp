#include "domain/Scoring.hpp"

#include <algorithm>
#include <initializer_list>

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
    const int operational = project.softwareIntensive
        ? ((project.hasReadme ? 35 : 0) +
           (project.hasCi ? 35 : 0) +
           (project.hasTests ? 30 : 0))
        : ((project.hasReadme ? 40 : 0) +
           (project.hasWorkPlan ? 30 : 0) +
           (project.hasTimeline ? 30 : 0));

    const int maturity =
        (project.hasAdr ? 25 : 0) +
        (project.hasDdd ? 25 : 0) +
        (project.hasDai ? 25 : 0) +
        (project.governanceItems > 0 ? 25 : 0);

    const bool reliabilityApplicable = project.softwareIntensive;
    int reliability = 100;
    if (reliabilityApplicable) {
        const int reliabilityChecks = countTrue({
            project.hasAsanUbsan,
            project.hasLeakChecks,
            project.hasStaticAnalysis,
            project.hasStrictWarnings,
            project.hasComplexityGuard,
            project.hasCycleGuard,
            project.hasFormatLint,
        });
        reliability = (reliabilityChecks * 100) / 7;
    }

    int execution = 0;
    execution += project.hasMethodology ? 10 : 0;
    execution += project.hasWorkPlan ? 15 : 0;
    execution += project.hasTimeline ? 15 : 0;
    execution += project.hasBudgetPlan ? 10 : 0;
    execution += project.hasTerritorialNetwork ? 10 : 0;
    execution += project.hasDataGovernance ? 10 : 0;
    execution += project.hasValidationPlan ? 15 : 0;
    execution += project.hasPublicPolicyAlignment ? 5 : 0;
    execution += (project.reviewMeetings > 0) ? 3 : 0;
    if (project.plannedDeliverables > 0) {
        const int clampedDelivered = std::max(0, std::min(project.deliveredDeliverables, project.plannedDeliverables));
        execution += static_cast<int>((5.0 * clampedDelivered) / project.plannedDeliverables);
    }
    execution += (project.openImpediments == 0) ? 2 : 0;
    execution = std::max(0, std::min(execution, 100));

    const int total =
        (operational * 30 + maturity * 25 + reliability * 20 + execution * 25) / 100;

    return ScoreBreakdown{operational, maturity, reliability, execution, total, reliabilityApplicable};
}

} // namespace labgp::domain
