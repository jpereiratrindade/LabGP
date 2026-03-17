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

int clampPercent(int value) {
    return std::max(0, std::min(value, 100));
}

int computeInstitutionalScore(const ResearchProject& project, int reliability, bool reliabilityApplicable) {
    int lifecycle = 0;
    lifecycle += project.hasMethodology ? 5 : 0;
    lifecycle += project.hasWorkPlan ? 5 : 0;
    lifecycle += project.hasTimeline ? 5 : 0;
    lifecycle += project.hasValidationPlan ? 5 : 0;

    int governance = 0;
    governance += project.hasAdr ? 5 : 0;
    governance += project.hasDdd ? 5 : 0;
    governance += project.hasDai ? 5 : 0;
    governance += project.governanceItems > 0 ? 5 : 0;

    int traceability = 0;
    traceability += project.hasReadme ? 5 : 0;
    traceability += project.hasCi ? 5 : 0;
    traceability += project.hasTests ? 5 : 0;
    traceability += project.reviewMeetings > 0 ? 5 : 0;

    int financeAndCompliance = 0;
    financeAndCompliance += project.hasBudgetPlan ? 8 : 0;
    financeAndCompliance += project.hasDataGovernance ? 6 : 0;
    financeAndCompliance += project.hasPublicPolicyAlignment ? 6 : 0;

    int monitoring = 0;
    monitoring += project.openImpediments == 0 ? 5 : 0;
    monitoring += project.plannedDeliverables > 0 ? 5 : 0;
    monitoring += project.deliveredDeliverables > 0 ? 5 : 0;
    monitoring += project.reviewMeetings > 0 ? 5 : 0;

    int softwareQuality = 20;
    if (project.softwareIntensive && reliabilityApplicable) {
        softwareQuality = reliability / 5;
    }

    const int score = lifecycle + governance + traceability + financeAndCompliance + monitoring + softwareQuality;
    return clampPercent(score);
}

int computeResearcherScore(const ResearchProject& project) {
    int planning = 0;
    planning += project.hasMethodology ? 10 : 0;
    planning += project.hasWorkPlan ? 10 : 0;
    planning += project.hasTimeline ? 10 : 0;

    int executionProgress = 0;
    if (project.plannedDeliverables > 0) {
        const int clampedDelivered = std::max(0, std::min(project.deliveredDeliverables, project.plannedDeliverables));
        executionProgress += static_cast<int>((15.0 * clampedDelivered) / project.plannedDeliverables);
    }
    executionProgress += project.hasValidationPlan ? 10 : 0;
    executionProgress += project.reviewMeetings > 0 ? 5 : 0;

    int collaboration = 0;
    collaboration += project.hasTerritorialNetwork ? 10 : 0;
    collaboration += project.hasDataGovernance ? 5 : 0;
    collaboration += project.hasPublicPolicyAlignment ? 5 : 0;

    int infrastructure = 0;
    if (project.softwareIntensive) {
        infrastructure += project.hasReadme ? 5 : 0;
        infrastructure += project.hasCi ? 5 : 0;
        infrastructure += project.hasTests ? 5 : 0;
        infrastructure += project.hasStaticAnalysis || project.hasFormatLint ? 5 : 0;
    } else {
        infrastructure += project.hasReadme ? 8 : 0;
        infrastructure += project.hasBudgetPlan ? 6 : 0;
        infrastructure += project.hasValidationPlan ? 6 : 0;
    }

    return clampPercent(planning + executionProgress + collaboration + infrastructure);
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

    const int institutional = computeInstitutionalScore(project, reliability, reliabilityApplicable);
    const int researcher = computeResearcherScore(project);
    const int total =
        (operational * 30 + maturity * 25 + reliability * 20 + execution * 25) / 100;

    return ScoreBreakdown{
        operational,
        maturity,
        reliability,
        execution,
        institutional,
        researcher,
        total,
        reliabilityApplicable
    };
}

} // namespace labgp::domain
