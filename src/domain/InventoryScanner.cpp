#include "domain/InventoryScanner.hpp"

#include <algorithm>
#include <filesystem>
#include <initializer_list>

#include "domain/ResearchProject.hpp"

namespace labgp::domain {

namespace fs = std::filesystem;

namespace {

bool hasAnyFile(const fs::path& root, std::initializer_list<const char*> names) {
    for (const auto* name : names) {
        if (fs::exists(root / name)) {
            return true;
        }
    }
    return false;
}

bool hasCi(const fs::path& root) {
    return fs::exists(root / ".github" / "workflows") ||
           fs::exists(root / ".gitlab-ci.yml") ||
           fs::exists(root / "azure-pipelines.yml");
}

bool hasTests(const fs::path& root) {
    return fs::exists(root / "tests") || hasAnyFile(root, {"CTestTestfile.cmake"});
}

bool hasAdr(const fs::path& root) {
    return fs::exists(root / "docs" / "adr") || fs::exists(root / "adr");
}

bool hasDdd(const fs::path& root) {
    return fs::exists(root / "docs" / "architecture") || fs::exists(root / "architecture");
}

bool hasDai(const fs::path& root) {
    return fs::exists(root / "docs" / "dai") || fs::exists(root / "dai");
}

bool hasGovernance(const fs::path& root) {
    return hasAnyFile(root, {"CONTRIBUTING.md", "CODEOWNERS", "GOVERNANCE.md", "SECURITY.md"});
}

bool hasSanitizersConfig(const fs::path& root) {
    return hasAnyFile(root, {".sanitizers", "asan.options", "ubsan.options"});
}

bool hasStaticAnalysisConfig(const fs::path& root) {
    return hasAnyFile(root, {".clang-tidy", ".clang-format", "cppcheck.suppress"});
}

bool hasStrictWarningsConfig(const fs::path& root) {
    // Heuristica fase 1: se existir CMake, assume possibilidade de configurar warnings estritos.
    return fs::exists(root / "CMakeLists.txt");
}

bool hasCycleGuardConfig(const fs::path& root) {
    return hasAnyFile(root, {"depgraph.yml", "dependency-cruiser.js"});
}

bool hasComplexityGuardConfig(const fs::path& root) {
    return hasAnyFile(root, {"lizard.cfg", ".lizard", "oclint.json"});
}

bool hasWorkPlanDoc(const fs::path& root) {
    return hasAnyFile(root, {"ROADMAP.md", "docs/roadmap.md", "docs/plan.md", "PROJECT_PLAN.md"});
}

bool hasBudgetDoc(const fs::path& root) {
    return hasAnyFile(root, {"BUDGET.md", "docs/budget.md", "funding.md", "docs/funding.md"});
}

} // namespace

std::vector<InventoryEntry> InventoryScanner::scan(const std::string& workspaceRoot) const {
    std::vector<InventoryEntry> entries;

    const fs::path root(workspaceRoot);
    if (!fs::exists(root) || !fs::is_directory(root)) {
        return entries;
    }

    for (const auto& dirEntry : fs::directory_iterator(root)) {
        if (!dirEntry.is_directory()) {
            continue;
        }

        const fs::path repoPath = dirEntry.path();
        if (!fs::exists(repoPath / ".git")) {
            continue;
        }

        ResearchProject probe;
        probe.id = repoPath.filename().string();
        probe.title = probe.id;
        probe.hasReadme = hasAnyFile(repoPath, {"README.md", "README.txt", "README"});
        probe.hasCi = hasCi(repoPath);
        probe.hasTests = hasTests(repoPath);
        probe.softwareIntensive = true;
        probe.hasAdr = hasAdr(repoPath);
        probe.hasDdd = hasDdd(repoPath);
        probe.hasDai = hasDai(repoPath);
        probe.governanceItems = hasGovernance(repoPath) ? 1 : 0;
        probe.hasMethodology = probe.hasAdr || probe.hasDdd;
        probe.hasWorkPlan = hasWorkPlanDoc(repoPath) || probe.hasDai;
        probe.hasTimeline = probe.hasWorkPlan;
        probe.hasBudgetPlan = hasBudgetDoc(repoPath);
        probe.plannedDeliverables = probe.hasWorkPlan ? 4 : 0;
        probe.deliveredDeliverables = probe.hasReadme ? 1 : 0;
        probe.reviewMeetings = probe.governanceItems;

        probe.hasAsanUbsan = hasSanitizersConfig(repoPath);
        probe.hasLeakChecks = probe.hasAsanUbsan;
        probe.hasStaticAnalysis = hasStaticAnalysisConfig(repoPath);
        probe.hasStrictWarnings = hasStrictWarningsConfig(repoPath);
        probe.hasComplexityGuard = hasComplexityGuardConfig(repoPath);
        probe.hasCycleGuard = hasCycleGuardConfig(repoPath);
        probe.hasFormatLint = fs::exists(repoPath / ".clang-format");

        InventoryEntry inv;
        inv.repoName = probe.id;
        inv.repoPath = repoPath.string();
        inv.integrated = true;
        inv.score = computeScore(probe);

        entries.push_back(std::move(inv));
    }

    std::sort(entries.begin(), entries.end(), [](const InventoryEntry& a, const InventoryEntry& b) {
        if (a.score.total != b.score.total) {
            return a.score.total > b.score.total;
        }
        return a.repoName < b.repoName;
    });

    return entries;
}

} // namespace labgp::domain
