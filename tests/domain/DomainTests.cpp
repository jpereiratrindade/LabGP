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
        .softwareIntensive = true,
        .hasMethodology = true,
        .hasWorkPlan = true,
        .hasTimeline = true,
        .hasBudgetPlan = true,
        .plannedDeliverables = 4,
        .deliveredDeliverables = 2,
        .reviewMeetings = 2,
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

    require(!store.moveStatus("T-1", ResearchStatus::Execution), "nao deve pular fluxo Proposta->Execucao");
    require(store.moveStatus("T-1", ResearchStatus::InReview), "deve mover Proposta->Em avaliacao");
    require(store.moveStatus("T-1", ResearchStatus::Approved), "deve mover Em avaliacao->Aprovado");
    require(store.moveStatus("T-1", ResearchStatus::Execution), "deve mover Aprovado->Execucao");
    require(store.all().front().status == ResearchStatus::Execution, "status atualizado");

    const auto score = computeScore(store.all().front());
    require(score.operational == 70, "score operacional esperado");
    require(score.maturity == 75, "score maturidade esperado");
    require(score.reliability == 42, "score confiabilidade esperado");
    require(score.execution == 57, "score execucao esperado");
    require(score.institutional == 81, "score institucional esperado");
    require(score.researcher == 57, "score pesquisador esperado");
    require(score.total == 62, "score total esperado");

    std::cout << "OK\n";
    return 0;
}
