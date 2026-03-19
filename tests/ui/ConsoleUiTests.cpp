#include <cstdlib>
#include <iostream>
#include <string>

#include "domain/ResearchProjectStore.hpp"
#include "ui/AppUI.hpp"

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

    {
        ResearchProjectStore emptyStore;
        const labgp::ui::AppUI emptyUi(emptyStore);
        const std::string emptyOutput = emptyUi.render();
        require(emptyOutput.find("(nenhum projeto cadastrado)") != std::string::npos,
                "deve orientar quando nao houver projeto cadastrado");
        require(emptyOutput.find("inventario de fontes") != std::string::npos,
                "deve explicar que o workspace pode ser usado como inventario de fontes");
    }

    ResearchProjectStore store;
    store.add(ResearchProject{
        .id = "LGP-T1",
        .title = "Projeto A",
        .status = ResearchStatus::Execution,
        .hasReadme = true,
        .hasCi = true,
        .hasTests = true,
    });
    store.add(ResearchProject{
        .id = "LGP-T2",
        .title = "Projeto B",
        .status = ResearchStatus::Proposal,
        .hasReadme = true,
    });

    const labgp::ui::AppUI ui(store);
    const std::string output = ui.render();

    require(output.find("Visao 1: Lista de Projetos") != std::string::npos, "deve renderizar lista");
    require(output.find("Visao 2: Kanban por Status") != std::string::npos, "deve renderizar kanban");
    require(output.find("[Execucao]") != std::string::npos, "deve conter coluna Execucao");
    require(output.find("LGP-T1") != std::string::npos, "deve conter projeto T1");
    require(output.find("LGP-T2") != std::string::npos, "deve conter projeto T2");

    std::cout << "OK\n";
    return 0;
}
