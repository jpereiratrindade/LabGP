#include "app/AppRuntime.hpp"

#include <iostream>
#include <string>
#include <vector>

#include "domain/InventoryScanner.hpp"
#include "domain/ResearchProjectStore.hpp"
#include "ui/AppUI.hpp"

#if LABGP_HAS_GUI_UI
#include "ui/GuiDashboard.hpp"

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#endif

namespace labgp::app {
namespace {

constexpr const char* kWorkspaceRoot = "/run/media/jpereiratrindade/labeco10T/dev/cpp";

struct AppData {
    domain::ResearchProjectStore store;
    std::vector<domain::InventoryEntry> inventory;
};

bool hasArg(int argc, char** argv, const char* option) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == option) return true;
    }
    return false;
}

AppData buildAppData() {
    using namespace labgp::domain;

    AppData data;
    data.store.add(ResearchProject{
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
    data.store.add(ResearchProject{
        .id = "LGP-002",
        .title = "Pipeline de Dados de Solo",
        .coordinator = "Nucleo Geo",
        .line = "Ciencia de Dados",
        .status = ResearchStatus::Proposal,
        .openImpediments = 0,
        .hasReadme = true,
        .hasCi = false,
        .hasTests = false,
    });
    data.store.add(ResearchProject{
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

    domain::InventoryScanner scanner;
    data.inventory = scanner.scan(kWorkspaceRoot);

    for (size_t i = 0; i < data.inventory.size() && i < 6; ++i) {
        const auto& entry = data.inventory[i];
        data.store.add(ResearchProject{
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

    return data;
}

int runConsole(const AppData& data) {
    const ui::AppUI appUi(data.store);
    std::cout << "Workspace: " << kWorkspaceRoot << "\n";
    std::cout << "Repositorios Git detectados: " << data.inventory.size() << "\n";
    std::cout << "Projetos em tela (demo + inventario): " << data.store.all().size() << "\n\n";
    std::cout << appUi.render();
    return 0;
}

#if LABGP_HAS_GUI_UI
bool runGui(const AppData& data) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "SDL2 init falhou: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window = SDL_CreateWindow(
        "LabGP - Gestao de Projetos de Pesquisa",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1440,
        900,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!window) {
        std::cerr << "Falha ao criar janela SDL2: " << SDL_GetError() << "\n";
        SDL_Quit();
        return false;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 150");

    ui::GuiDashboard dashboard;
    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
                done = true;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        dashboard.render(data.store.all(), data.inventory, kWorkspaceRoot);

        ImGui::Render();
        glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
        glClearColor(0.06f, 0.08f, 0.11f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return true;
}
#endif

} // namespace

int AppRuntime::run(int argc, char** argv) {
    const AppData data = buildAppData();

    const bool forceConsole = hasArg(argc, argv, "--console");
    const bool forceGui = hasArg(argc, argv, "--gui");

#if LABGP_HAS_GUI_UI
    if (!forceConsole && (forceGui || !forceConsole)) {
        if (runGui(data)) {
            return 0;
        }
        std::cerr << "Fallback para modo console.\n";
    }
#else
    (void) forceGui;
#endif

    return runConsole(data);
}

} // namespace labgp::app
