#include <iostream>
#include <filesystem>
#include <fstream>

#include "domain/InventoryScanner.hpp"
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
    namespace fs = std::filesystem;

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

    const fs::path tempRoot = fs::temp_directory_path() / "labgp_domain_tests_inventory";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "DossieOCRFraco");
    fs::create_directories(tempRoot / "DossieAprovado");

    {
        std::ofstream(tempRoot / "DossieOCRFraco" / "Edital_Chamada.pdf") << "";
        std::ofstream(tempRoot / "DossieOCRFraco" / "Proposta_Preliminar.pdf") << "";
        std::ofstream(tempRoot / "DossieAprovado" / "Projeto_Aprovado.pdf") << "";
    }

    InventoryScanner scanner;
    const auto inventory = scanner.scan(tempRoot.string());
    require(inventory.size() == 2, "deve detectar dois dossies");

    auto findEntry = [&](const std::string& name) -> const InventoryEntry* {
        for (const auto& entry : inventory) {
            if (entry.repoName == name) return &entry;
        }
        return nullptr;
    };

    const InventoryEntry* weakOcr = findEntry("DossieOCRFraco");
    require(weakOcr != nullptr, "deve encontrar dossier com OCR fraco");
    require(weakOcr->inferredStatus == ResearchStatus::Proposal,
            "OCR fraco com edital/proposta deve ficar conservador em Proposta");
    require(computeScore(weakOcr->projectSnapshot).total == weakOcr->score.total,
            "snapshot do projeto deve preservar score calculado do inventario");

    const InventoryEntry* approved = findEntry("DossieAprovado");
    require(approved != nullptr, "deve encontrar dossier aprovado");
    require(approved->inferredStatus == ResearchStatus::Approved,
            "sinal forte de aprovado no nome deve ser preservado mesmo sem OCR");
    require(computeScore(approved->projectSnapshot).institutional == approved->score.institutional,
            "snapshot deve manter score institucional do inventario");

    const auto singleSourceInventory = scanner.scan((tempRoot / "DossieAprovado").string());
    require(singleSourceInventory.size() == 1, "deve detectar dossie quando a propria pasta e a fonte");
    require(singleSourceInventory.front().repoName == "DossieAprovado",
            "deve usar o nome da pasta selecionada como identificador da fonte");

    fs::remove_all(tempRoot);

    std::cout << "OK\n";
    return 0;
}
