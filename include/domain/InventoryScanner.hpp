#pragma once

#include <string>
#include <vector>

#include "domain/Scoring.hpp"

namespace labgp::domain {

struct InventoryEntry {
    std::string repoName;
    std::string repoPath;
    bool integrated{false};
    ScoreBreakdown score;
};

class InventoryScanner {
public:
    std::vector<InventoryEntry> scan(const std::string& workspaceRoot) const;
};

} // namespace labgp::domain
