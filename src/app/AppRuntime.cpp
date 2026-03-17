#include "app/AppRuntime.hpp"

#include <iostream>
#include <string>

#include "domain/InventoryScanner.hpp"
#include "domain/ResearchProjectStore.hpp"
#include "ui/AppUI.hpp"

namespace labgp::app {

int AppRuntime::run() {
    using namespace labgp::domain;

    constexpr const char* kWorkspaceRoot = "/run/media/jpereiratrindade/labeco10T/dev/cpp";
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
    store.add(ResearchProject{
        .id = "LGP-002",
        .title = "Pipeline de Dados de Solo",
        .coordinator = "Nucleo Geo",
        .line = "Ciencia de Dados",
        .status = ResearchStatus::Proposal,
        .openImpediments = 0,
        .hasReadme = true,
        .hasCi = false,
        .hasTests = false,
        .hasAdr = false,
        .hasDdd = false,
        .hasDai = false,
        .governanceItems = 0,
    });
    store.add(ResearchProject{
        .id = "LGP-003",
        .title = "Estudo de Reprodutibilidade em C++",
        .coordinator = "Equipe Engenharia",
        .line = "Engenharia de Software",
        .status = ResearchStatus::Analysis,
        .openImpediments = 0,
        .hasReadme = true,
        .hasCi = true,
        .hasTests = true,
        .hasAdr = true,
        .hasDdd = true,
        .hasDai = true,
        .governanceItems = 3,
        .hasAsanUbsan = true,
        .hasLeakChecks = true,
        .hasStaticAnalysis = true,
        .hasStrictWarnings = true,
        .hasComplexityGuard = true,
        .hasCycleGuard = false,
        .hasFormatLint = true,
    });

    InventoryScanner scanner;
    const auto entries = scanner.scan(kWorkspaceRoot);
    for (size_t i = 0; i < entries.size() && i < 6; ++i) {
        const auto& entry = entries[i];
        store.add(ResearchProject{
            .id = "INV-" + std::to_string(i + 1),
            .title = "Integracao: " + entry.repoName,
            .coordinator = "Auto Scanner",
            .line = "Inventario Workspace",
            .status = entry.score.total >= 70 ? ResearchStatus::Approved : ResearchStatus::Proposal,
            .openImpediments = entry.score.total >= 60 ? 0 : 1,
            .hasReadme = entry.score.operational >= 35,
            .hasCi = entry.score.operational >= 70,
            .hasTests = entry.score.operational >= 100,
            .hasAdr = entry.score.maturity >= 25,
            .hasDdd = entry.score.maturity >= 50,
            .hasDai = entry.score.maturity >= 75,
            .governanceItems = entry.score.maturity >= 100 ? 1 : 0,
            .hasAsanUbsan = entry.score.reliability > 0,
            .hasLeakChecks = entry.score.reliability > 14,
            .hasStaticAnalysis = entry.score.reliability > 28,
            .hasStrictWarnings = entry.score.reliability > 42,
            .hasComplexityGuard = entry.score.reliability > 57,
            .hasCycleGuard = entry.score.reliability > 71,
            .hasFormatLint = entry.score.reliability > 85,
        });
    }

    const ui::AppUI appUi(store);
    std::cout << "Workspace: " << kWorkspaceRoot << "\n";
    std::cout << "Repositorios Git detectados: " << entries.size() << "\n";
    std::cout << "Projetos em tela (demo + inventario): " << store.all().size() << "\n\n";
    std::cout << appUi.render();

    return 0;
}

} // namespace labgp::app
