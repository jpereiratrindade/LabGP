#pragma once

#include <string>
#include <vector>

#include "domain/ResearchProject.hpp"
#include "domain/Scoring.hpp"

namespace labgp::domain {

struct InterpretedDocument {
    std::string fileName;
    std::string filePath;
    std::string curationTag;
    std::string sha256;
    std::string cachePath;
    bool usedCache{false};
    bool includedInCorpus{false};
    int relevanceScore{0};
    int textBytes{0};
};

struct InventoryEntry {
    std::string repoName;
    std::string repoPath;
    std::string source{"Git"}; // Git | Dossie
    std::string summary;
    std::string objectives;
    std::string innovationContributions;
    std::string researchActivities;
    std::string expectedResults;
    std::vector<std::string> teamMembers;
    std::vector<InterpretedDocument> interpretedDocuments;
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
