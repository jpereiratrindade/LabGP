#pragma once

#include <string>
#include <vector>

#include "domain/ResearchProject.hpp"
#include "domain/Scoring.hpp"

namespace labgp::domain {

struct InventoryEntry {
    std::string repoName;
    std::string repoPath;
    std::string source{"Git"}; // Git | Dossie
    int innovationSignals{0};
    int activitySignals{0};
    int plannedResultsSignals{0};
    ResearchStatus inferredStatus{ResearchStatus::Proposal};
    ScoreBreakdown score;
};

class InventoryScanner {
public:
    std::vector<InventoryEntry> scan(const std::string& workspaceRoot) const;
};

} // namespace labgp::domain
