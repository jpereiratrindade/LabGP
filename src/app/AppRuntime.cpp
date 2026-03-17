#include "app/AppRuntime.hpp"

#include <iostream>

#include "domain/ResearchProjectStore.hpp"
#include "domain/Scoring.hpp"

namespace labgp::app {

int AppRuntime::run() {
    using namespace labgp::domain;

    ResearchProjectStore store;
    store.add(ResearchProject{
        .id = "LGP-001",
        .title = "Mapa de Integracao dos Projetos LabEco",
        .coordinator = "Equipe LabEco",
        .line = "Engenharia de Software",
        .status = ResearchStatus::Execution,
        .openImpediments = 1,
        .hasReadme = true,
        .hasCi = true,
        .hasTests = true,
        .hasAdr = true,
        .hasDdd = true,
        .hasDai = false,
        .governanceItems = 1,
        .hasAsanUbsan = true,
        .hasLeakChecks = false,
        .hasStaticAnalysis = true,
        .hasStrictWarnings = true,
        .hasComplexityGuard = false,
        .hasCycleGuard = false,
        .hasFormatLint = true,
    });

    std::cout << "LabGP - Gestao de Projetos de Pesquisa\n";
    std::cout << "=====================================\n\n";

    for (const auto& project : store.all()) {
        const auto score = computeScore(project);
        std::cout << "[" << project.id << "] " << project.title << "\n";
        std::cout << "Status: " << toString(project.status) << " | Impedimentos: " << project.openImpediments << "\n";
        std::cout << "Score -> Oper: " << score.operational
                  << " | Matur: " << score.maturity
                  << " | Confiab: " << score.reliability
                  << " | Total: " << score.total << "\n\n";
    }

    return 0;
}

} // namespace labgp::app
