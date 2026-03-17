#pragma once

#include <string>

namespace labgp::domain {

enum class ResearchStatus {
    Proposal,
    InReview,
    Approved,
    Execution,
    Analysis,
    Publication,
    Closed
};

struct ResearchProject {
    std::string id;
    std::string title;
    std::string coordinator;
    std::string institution{"Nao informado"};
    std::string program;
    std::string thematicAxis;
    std::string callNotice;
    std::string projectType{"PD&I"};
    std::string startDate;
    std::string endDate;
    int durationMonths{0};
    std::string line;
    ResearchStatus status{ResearchStatus::Proposal};
    int openImpediments{0};
    bool softwareIntensive{true};

    bool hasMethodology{false};
    bool hasWorkPlan{false};
    bool hasTimeline{false};
    bool hasBudgetPlan{false};
    int plannedDeliverables{0};
    int deliveredDeliverables{0};
    int reviewMeetings{0};
    bool hasTerritorialNetwork{false};   // ex.: UO/URT/UAC
    bool hasDataGovernance{false};       // ex.: DDC
    bool hasValidationPlan{false};       // validacao com dados/series
    bool hasPublicPolicyAlignment{false};

    bool hasReadme{false};
    bool hasCi{false};
    bool hasTests{false};

    bool hasAdr{false};
    bool hasDdd{false};
    bool hasDai{false};
    int governanceItems{0};

    bool hasAsanUbsan{false};
    bool hasLeakChecks{false};
    bool hasStaticAnalysis{false};
    bool hasStrictWarnings{false};
    bool hasComplexityGuard{false};
    bool hasCycleGuard{false};
    bool hasFormatLint{false};
};

std::string toString(ResearchStatus status);

} // namespace labgp::domain
