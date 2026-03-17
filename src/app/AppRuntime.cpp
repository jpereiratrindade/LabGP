#include "app/AppRuntime.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "domain/InventoryScanner.hpp"
#include "domain/ResearchProjectStore.hpp"
#include "ui/AppUI.hpp"

#if LABGP_HAS_GUI_UI
#include "ui/GuiDashboard.hpp"

#include "backends/imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "imgui.h"

#include <SDL2/SDL.h>
#endif

namespace labgp::app {
namespace {

constexpr const char* kDefaultWorkspaceRoot = "/run/media/jpereiratrindade/labeco10T/dev/cpp";

struct AppData {
    std::string workspaceRoot;
    bool includeDemoProjects{false};
    domain::ResearchProjectStore store;
    std::vector<domain::InventoryEntry> inventory;
};

bool hasArg(int argc, char** argv, const char* option) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == option) return true;
    }
    return false;
}

std::string getArgValue(int argc, char** argv, const char* option) {
    const std::string prefix = std::string(option) + "=";
    for (int i = 1; i < argc; ++i) {
        const std::string current = argv[i];
        if (current.rfind(prefix, 0) == 0) {
            return current.substr(prefix.size());
        }
        if (current == option && i + 1 < argc) {
            return argv[i + 1];
        }
    }
    return {};
}

bool commandExists(const char* command) {
#if defined(_WIN32)
    (void) command;
    return false;
#else
    std::string cmd = "command -v ";
    cmd += command;
    cmd += " >/dev/null 2>&1";
    return std::system(cmd.c_str()) == 0;
#endif
}

bool runPickerCommand(const std::string& command, std::string* selectedPath) {
    if (!selectedPath) return false;
    selectedPath->clear();

    std::array<char, 512> buffer{};
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return false;

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        *selectedPath += buffer.data();
    }

    const int rc = pclose(pipe);
    if (rc != 0) return false;

    while (!selectedPath->empty() && (selectedPath->back() == '\n' || selectedPath->back() == '\r')) {
        selectedPath->pop_back();
    }
    return !selectedPath->empty();
}

std::string shellEscapeSingleQuoted(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    out.push_back('\'');
    for (char c : s) {
        if (c == '\'') out += "'\\''";
        else out.push_back(c);
    }
    out.push_back('\'');
    return out;
}

std::string selectDirectoryWithSystemDialog(const std::string& initialDir) {
#if defined(_WIN32)
    (void) initialDir;
    return {};
#elif defined(__APPLE__)
    std::string selected;
    const std::string cmd =
        "osascript -e 'set p to POSIX path of (choose folder with prompt \"Selecionar Workspace\")' 2>/dev/null";
    if (runPickerCommand(cmd, &selected)) return selected;
    return {};
#else
    (void) initialDir;
    std::string selected;
    if (commandExists("kdialog")) {
        const std::string cmd = "kdialog --getexistingdirectory " + shellEscapeSingleQuoted(initialDir) + " 2>/dev/null";
        if (runPickerCommand(cmd, &selected)) return selected;
    }
    if (commandExists("zenity")) {
        const std::string cmd = "zenity --file-selection --directory --title=\"Selecionar Workspace\" 2>/dev/null";
        if (runPickerCommand(cmd, &selected)) return selected;
    }
    return {};
#endif
}

std::string resolveWorkspaceRoot(int argc, char** argv) {
    namespace fs = std::filesystem;

    if (hasArg(argc, argv, "--pick-workspace")) {
        const std::string picked = selectDirectoryWithSystemDialog(kDefaultWorkspaceRoot);
        if (!picked.empty() && fs::exists(picked) && fs::is_directory(picked)) return picked;
    }

    const std::string cliWorkspace = getArgValue(argc, argv, "--workspace");
    if (!cliWorkspace.empty() && fs::exists(cliWorkspace) && fs::is_directory(cliWorkspace)) return cliWorkspace;

    const char* envWorkspace = std::getenv("LABGP_WORKSPACE");
    if (envWorkspace && *envWorkspace && fs::exists(envWorkspace) && fs::is_directory(envWorkspace)) {
        return envWorkspace;
    }

    return kDefaultWorkspaceRoot;
}

AppData buildAppData(const std::string& workspaceRoot, bool includeDemoProjects) {
    using namespace labgp::domain;

    AppData data{.workspaceRoot = workspaceRoot, .includeDemoProjects = includeDemoProjects};
    if (includeDemoProjects) {
        data.store.add(ResearchProject{
        .id = "LGP-001",
        .title = "Mapa de Integracao dos Projetos LabEco",
        .coordinator = "Equipe LabEco",
        .institution = "LabEco",
        .program = "Integracao de Portfolio",
        .thematicAxis = "Governanca Digital",
        .projectType = "Gestao de Portfolio",
        .startDate = "2026-01-10",
        .endDate = "2027-12-20",
        .durationMonths = 24,
        .line = "Engenharia de Software",
        .status = ResearchStatus::Execution,
        .openImpediments = 1,
        .softwareIntensive = true,
        .hasMethodology = true,
        .hasWorkPlan = true,
        .hasTimeline = true,
        .hasBudgetPlan = true,
        .plannedDeliverables = 8,
        .deliveredDeliverables = 3,
        .reviewMeetings = 4,
        .hasTerritorialNetwork = false,
        .hasDataGovernance = true,
        .hasValidationPlan = true,
        .hasPublicPolicyAlignment = false,
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
        .title = "Modelo Integrado de Risco e Resiliencia Territorial",
        .coordinator = "Nucleo Agro",
        .institution = "Rede Institucional de Pesquisa",
        .program = "Resiliencia Territorial",
        .thematicAxis = "Socioecologia",
        .callNotice = "Chamada de Pesquisa Aplicada",
        .projectType = "PD&I",
        .startDate = "2026-05-01",
        .endDate = "2029-04-30",
        .durationMonths = 36,
        .line = "Pesquisa de Campo",
        .status = ResearchStatus::InReview,
        .openImpediments = 0,
        .softwareIntensive = false,
        .hasMethodology = true,
        .hasWorkPlan = true,
        .hasTimeline = true,
        .hasBudgetPlan = true,
        .plannedDeliverables = 6,
        .deliveredDeliverables = 1,
        .reviewMeetings = 2,
        .hasTerritorialNetwork = true,
        .hasDataGovernance = true,
        .hasValidationPlan = true,
        .hasPublicPolicyAlignment = true,
        .hasReadme = true,
        });
        data.store.add(ResearchProject{
        .id = "LGP-003",
        .title = "Estudo de Reprodutibilidade Computacional Multilinguagem",
        .coordinator = "Equipe Engenharia",
        .institution = "LabEco",
        .program = "Confiabilidade Cientifica",
        .thematicAxis = "Metodologia",
        .projectType = "Pesquisa Aplicada",
        .startDate = "2025-09-01",
        .endDate = "2027-02-28",
        .durationMonths = 18,
        .line = "Engenharia de Software",
        .status = ResearchStatus::Analysis,
        .openImpediments = 0,
        .softwareIntensive = true,
        .hasMethodology = true,
        .hasWorkPlan = true,
        .hasTimeline = true,
        .hasBudgetPlan = true,
        .plannedDeliverables = 5,
        .deliveredDeliverables = 4,
        .reviewMeetings = 6,
        .hasTerritorialNetwork = false,
        .hasDataGovernance = true,
        .hasValidationPlan = true,
        .hasPublicPolicyAlignment = true,
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
    }

    domain::InventoryScanner scanner;
    data.inventory = scanner.scan(workspaceRoot);

    for (size_t i = 0; i < data.inventory.size() && i < 6; ++i) {
        const auto& entry = data.inventory[i];
        data.store.add(ResearchProject{
            .id = "INV-" + std::to_string(i + 1),
            .title = "Integracao: " + entry.repoName,
            .coordinator = "Auto Scanner",
            .institution = "Workspace",
            .program = "Inventario Tecnico",
            .thematicAxis = "Integracao",
            .projectType = "Diagnostico",
            .line = "Inventario Workspace",
            .status = entry.score.total >= 70 ? ResearchStatus::Approved : ResearchStatus::Proposal,
            .openImpediments = entry.score.total >= 60 ? 0 : 1,
            .softwareIntensive = entry.score.reliabilityApplicable,
            .hasMethodology = entry.score.maturity >= 25,
            .hasWorkPlan = entry.score.maturity >= 75,
            .hasTimeline = entry.score.maturity >= 50,
            .hasBudgetPlan = entry.score.maturity >= 100,
            .plannedDeliverables = 4,
            .deliveredDeliverables = std::min(4, entry.score.total / 25),
            .reviewMeetings = entry.score.maturity >= 50 ? 1 : 0,
            .hasTerritorialNetwork = false,
            .hasDataGovernance = entry.score.maturity >= 100,
            .hasValidationPlan = entry.score.maturity >= 50,
            .hasPublicPolicyAlignment = false,
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
    std::cout << "Workspace: " << data.workspaceRoot << "\n";
    std::cout << "Repositorios Git detectados: " << data.inventory.size() << "\n";
    std::cout << "Projetos em tela (demo + inventario): " << data.store.all().size() << "\n\n";
    std::cout << appUi.render();
    return 0;
}

#if LABGP_HAS_GUI_UI
bool runGui(AppData* data) {
    if (!data) return false;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "SDL2 init falhou: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_Window* window = SDL_CreateWindow(
        "LabGP - Gestao de Projetos de Pesquisa",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1440,
        900,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!window) {
        std::cerr << "Falha ao criar janela SDL2: " << SDL_GetError() << "\n";
        SDL_Quit();
        return false;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Falha ao criar renderer SDL2: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    ui::GuiDashboard dashboard;
    std::string workspaceFeedback;
    bool done = false;
    while (!done) {
        bool requestRescan = false;
        std::string requestApplyWorkspacePath;
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
                done = true;
            }
        }

        ImGui_ImplSDL2_NewFrame();
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui::NewFrame();

        dashboard.render(
            data->store.all(),
            data->inventory,
            data->workspaceRoot,
            &requestRescan,
            &requestApplyWorkspacePath,
            workspaceFeedback
        );

        if (requestRescan) {
            *data = buildAppData(data->workspaceRoot, data->includeDemoProjects);
            workspaceFeedback = "Inventario reescanado com sucesso.";
        }

        if (!requestApplyWorkspacePath.empty()) {
            if (std::filesystem::exists(requestApplyWorkspacePath) && std::filesystem::is_directory(requestApplyWorkspacePath)) {
                *data = buildAppData(requestApplyWorkspacePath, data->includeDemoProjects);
                workspaceFeedback = "Workspace atualizado manualmente.";
            } else {
                workspaceFeedback = "Pasta invalida. Verifique o caminho informado.";
            }
        }

        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 15, 20, 28, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return true;
}
#endif

} // namespace

int AppRuntime::run(int argc, char** argv) {
    const std::string workspaceRoot = resolveWorkspaceRoot(argc, argv);
    const bool includeDemoProjects = hasArg(argc, argv, "--demo");
    AppData data = buildAppData(workspaceRoot, includeDemoProjects);

    const bool forceConsole = hasArg(argc, argv, "--console");
    const bool forceGui = hasArg(argc, argv, "--gui");

#if LABGP_HAS_GUI_UI
    if (!forceConsole && (forceGui || !forceConsole)) {
        if (runGui(&data)) {
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
