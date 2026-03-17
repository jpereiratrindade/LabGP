#include <iostream>

#include "domain/ResearchProjectStore.hpp"
#include "domain/Scoring.hpp"

namespace {
void require(bool cond, const char* message) {
    if (!cond) {
        std::cerr << "FAIL: " << message << "\n";
        std::exit(1);
    }
}
} // namespace

int main() {
    using namespace labgp::domain;

    ResearchProjectStore store;
    store.add(ResearchProject{
        .id = "T-1",
        .title = "Teste",
        .coordinator = "Equipe",
        .line = "Linha",
        .status = ResearchStatus::Proposal,
        .hasReadme = true,
        .hasCi = true,
        .hasTests = false,
        .hasAdr = true,
        .hasDdd = false,
        .hasDai = true,
        .governanceItems = 1,
        .hasAsanUbsan = true,
        .hasLeakChecks = false,
        .hasStaticAnalysis = true,
        .hasStrictWarnings = false,
        .hasComplexityGuard = false,
        .hasCycleGuard = false,
        .hasFormatLint = true,
    });

    require(store.moveStatus("T-1", ResearchStatus::Execution), "deve mover status");
    require(store.all().front().status == ResearchStatus::Execution, "status atualizado");

    const auto score = computeScore(store.all().front());
    require(score.operational == 70, "score operacional esperado");
    require(score.maturity == 75, "score maturidade esperado");
    require(score.reliability == 42, "score confiabilidade esperado");
    require(score.total == 64, "score total esperado");

    std::cout << "OK\n";
    return 0;
}
