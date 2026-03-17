#pragma once

#include <string>

namespace labgp::domain {

enum class ResearchStatus {
    Proposal,
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
    std::string line;
    ResearchStatus status{ResearchStatus::Proposal};
    int openImpediments{0};

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
