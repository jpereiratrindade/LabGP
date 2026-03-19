#include "app/AppRuntime.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <fstream>
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
constexpr const char* kProjectsFileName = "labgp_projects.tsv";

struct AppData {
    std::string workspaceRoot{};
    bool includeDemoProjects{false};
    domain::ResearchProjectStore store{};
    std::vector<domain::InventoryEntry> inventory{};
};

bool loadProjectsFromDisk(AppData* data, std::string* workspaceFeedback);
bool saveProjectsToDisk(const AppData& data, std::string* workspaceFeedback);
int applyInventoryMetadataToExistingProjects(AppData* data);
bool processInnovationMiningForSource(AppData* data, const std::string& sourceRepoPath, std::string* workspaceFeedback);

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

    AppData data{};
    data.workspaceRoot = workspaceRoot;
    data.includeDemoProjects = includeDemoProjects;
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
    data.inventory = scanner.scan(workspaceRoot, false);
    loadProjectsFromDisk(&data, nullptr);
    applyInventoryMetadataToExistingProjects(&data);

    return data;
}

std::string escapeTsv(const std::string& raw) {
    std::string out;
    out.reserve(raw.size());
    for (char c : raw) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '\t': out += "\\t"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            default: out.push_back(c); break;
        }
    }
    return out;
}

std::string unescapeTsv(const std::string& escaped) {
    std::string out;
    out.reserve(escaped.size());
    for (std::size_t i = 0; i < escaped.size(); ++i) {
        const char c = escaped[i];
        if (c == '\\' && i + 1 < escaped.size()) {
            const char n = escaped[i + 1];
            switch (n) {
                case '\\': out.push_back('\\'); ++i; continue;
                case 't': out.push_back('\t'); ++i; continue;
                case 'n': out.push_back('\n'); ++i; continue;
                case 'r': out.push_back('\r'); ++i; continue;
                default: break;
            }
        }
        out.push_back(c);
    }
    return out;
}

std::vector<std::string> splitTabLine(const std::string& line) {
    std::vector<std::string> cols;
    std::string current;
    for (char c : line) {
        if (c == '\t') {
            cols.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    cols.push_back(current);
    return cols;
}

int parseIntField(const std::vector<std::string>& cols, std::size_t idx, int fallback = 0) {
    if (idx >= cols.size()) return fallback;
    try {
        return std::stoi(cols[idx]);
    } catch (...) {
        return fallback;
    }
}

bool parseBoolField(const std::vector<std::string>& cols, std::size_t idx, bool fallback = false) {
    return parseIntField(cols, idx, fallback ? 1 : 0) != 0;
}

domain::ResearchStatus statusFromPersistedInt(int value) {
    switch (value) {
        case 0: return domain::ResearchStatus::Proposal;
        case 1: return domain::ResearchStatus::InReview;
        case 2: return domain::ResearchStatus::Approved;
        case 3: return domain::ResearchStatus::Execution;
        case 4: return domain::ResearchStatus::Analysis;
        case 5: return domain::ResearchStatus::Publication;
        case 6: return domain::ResearchStatus::Closed;
        default: return domain::ResearchStatus::Proposal;
    }
}

int statusToPersistedInt(domain::ResearchStatus status) {
    switch (status) {
        case domain::ResearchStatus::Proposal: return 0;
        case domain::ResearchStatus::InReview: return 1;
        case domain::ResearchStatus::Approved: return 2;
        case domain::ResearchStatus::Execution: return 3;
        case domain::ResearchStatus::Analysis: return 4;
        case domain::ResearchStatus::Publication: return 5;
        case domain::ResearchStatus::Closed: return 6;
    }
    return 0;
}

void upsertProject(domain::ResearchProjectStore* store, const domain::ResearchProject& project) {
    if (!store) return;
    if (!store->update(project)) {
        store->add(project);
    }
}

bool loadProjectsFromDisk(AppData* data, std::string* workspaceFeedback) {
    namespace fs = std::filesystem;
    if (!data) return false;
    const fs::path inPath = fs::path(data->workspaceRoot) / kProjectsFileName;
    if (!fs::exists(inPath)) {
        return false;
    }

    std::ifstream in(inPath);
    if (!in.is_open()) {
        if (workspaceFeedback) {
            *workspaceFeedback = "Falha ao carregar projetos salvos: arquivo inacessivel.";
        }
        return false;
    }

    std::string header;
    std::getline(in, header);

    int loaded = 0;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        const auto cols = splitTabLine(line);
        if (cols.size() < 41) continue;

        domain::ResearchProject p;
        p.id = unescapeTsv(cols[0]);
        p.title = unescapeTsv(cols[1]);
        p.sourceRepoPath = unescapeTsv(cols[2]);
        p.coordinator = unescapeTsv(cols[3]);
        p.institution = unescapeTsv(cols[4]);
        p.program = unescapeTsv(cols[5]);
        p.thematicAxis = unescapeTsv(cols[6]);
        p.callNotice = unescapeTsv(cols[7]);
        p.projectType = unescapeTsv(cols[8]);
        p.startDate = unescapeTsv(cols[9]);
        p.endDate = unescapeTsv(cols[10]);
        p.durationMonths = parseIntField(cols, 11);
        p.line = unescapeTsv(cols[12]);
        p.status = statusFromPersistedInt(parseIntField(cols, 13));
        p.openImpediments = parseIntField(cols, 14);
        p.softwareIntensive = parseBoolField(cols, 15);
        p.hasMethodology = parseBoolField(cols, 16);
        p.hasWorkPlan = parseBoolField(cols, 17);
        p.hasTimeline = parseBoolField(cols, 18);
        p.hasBudgetPlan = parseBoolField(cols, 19);
        p.plannedDeliverables = parseIntField(cols, 20);
        p.deliveredDeliverables = parseIntField(cols, 21);
        p.reviewMeetings = parseIntField(cols, 22);
        p.hasTerritorialNetwork = parseBoolField(cols, 23);
        p.hasDataGovernance = parseBoolField(cols, 24);
        p.hasValidationPlan = parseBoolField(cols, 25);
        p.hasPublicPolicyAlignment = parseBoolField(cols, 26);
        p.hasReadme = parseBoolField(cols, 27);
        p.hasCi = parseBoolField(cols, 28);
        p.hasTests = parseBoolField(cols, 29);
        p.hasAdr = parseBoolField(cols, 30);
        p.hasDdd = parseBoolField(cols, 31);
        p.hasDai = parseBoolField(cols, 32);
        p.governanceItems = parseIntField(cols, 33);
        p.hasAsanUbsan = parseBoolField(cols, 34);
        p.hasLeakChecks = parseBoolField(cols, 35);
        p.hasStaticAnalysis = parseBoolField(cols, 36);
        p.hasStrictWarnings = parseBoolField(cols, 37);
        p.hasComplexityGuard = parseBoolField(cols, 38);
        p.hasCycleGuard = parseBoolField(cols, 39);
        p.hasFormatLint = parseBoolField(cols, 40);

        if (p.id.empty() || p.title.empty()) continue;
        upsertProject(&data->store, p);
        ++loaded;
    }

    if (workspaceFeedback) {
        *workspaceFeedback = "Projetos carregados de: " + inPath.string() + " (" + std::to_string(loaded) + ")";
    }
    return loaded > 0;
}

bool saveProjectsToDisk(const AppData& data, std::string* workspaceFeedback) {
    namespace fs = std::filesystem;
    const fs::path outPath = fs::path(data.workspaceRoot) / kProjectsFileName;
    std::ofstream out(outPath);
    if (!out.is_open()) {
        if (workspaceFeedback) {
            *workspaceFeedback = "Falha ao salvar projetos: nao foi possivel abrir arquivo de saida.";
        }
        return false;
    }

    out << "id\ttitle\tsource_repo_path\tcoordinator\tinstitution\tprogram\tthematic_axis\tcall_notice\tproject_type\tstart_date\tend_date\tduration_months\tline\tstatus\topen_impediments\tsoftware_intensive\thas_methodology\thas_work_plan\thas_timeline\thas_budget_plan\tplanned_deliverables\tdelivered_deliverables\treview_meetings\thas_territorial_network\thas_data_governance\thas_validation_plan\thas_public_policy_alignment\thas_readme\thas_ci\thas_tests\thas_adr\thas_ddd\thas_dai\tgovernance_items\thas_asan_ubsan\thas_leak_checks\thas_static_analysis\thas_strict_warnings\thas_complexity_guard\thas_cycle_guard\thas_format_lint\n";

    for (const auto& p : data.store.all()) {
        out << escapeTsv(p.id) << '\t'
            << escapeTsv(p.title) << '\t'
            << escapeTsv(p.sourceRepoPath) << '\t'
            << escapeTsv(p.coordinator) << '\t'
            << escapeTsv(p.institution) << '\t'
            << escapeTsv(p.program) << '\t'
            << escapeTsv(p.thematicAxis) << '\t'
            << escapeTsv(p.callNotice) << '\t'
            << escapeTsv(p.projectType) << '\t'
            << escapeTsv(p.startDate) << '\t'
            << escapeTsv(p.endDate) << '\t'
            << p.durationMonths << '\t'
            << escapeTsv(p.line) << '\t'
            << statusToPersistedInt(p.status) << '\t'
            << p.openImpediments << '\t'
            << (p.softwareIntensive ? 1 : 0) << '\t'
            << (p.hasMethodology ? 1 : 0) << '\t'
            << (p.hasWorkPlan ? 1 : 0) << '\t'
            << (p.hasTimeline ? 1 : 0) << '\t'
            << (p.hasBudgetPlan ? 1 : 0) << '\t'
            << p.plannedDeliverables << '\t'
            << p.deliveredDeliverables << '\t'
            << p.reviewMeetings << '\t'
            << (p.hasTerritorialNetwork ? 1 : 0) << '\t'
            << (p.hasDataGovernance ? 1 : 0) << '\t'
            << (p.hasValidationPlan ? 1 : 0) << '\t'
            << (p.hasPublicPolicyAlignment ? 1 : 0) << '\t'
            << (p.hasReadme ? 1 : 0) << '\t'
            << (p.hasCi ? 1 : 0) << '\t'
            << (p.hasTests ? 1 : 0) << '\t'
            << (p.hasAdr ? 1 : 0) << '\t'
            << (p.hasDdd ? 1 : 0) << '\t'
            << (p.hasDai ? 1 : 0) << '\t'
            << p.governanceItems << '\t'
            << (p.hasAsanUbsan ? 1 : 0) << '\t'
            << (p.hasLeakChecks ? 1 : 0) << '\t'
            << (p.hasStaticAnalysis ? 1 : 0) << '\t'
            << (p.hasStrictWarnings ? 1 : 0) << '\t'
            << (p.hasComplexityGuard ? 1 : 0) << '\t'
            << (p.hasCycleGuard ? 1 : 0) << '\t'
            << (p.hasFormatLint ? 1 : 0) << '\n';
    }

    if (workspaceFeedback) {
        *workspaceFeedback = "Projetos salvos em: " + outPath.string();
    }
    return true;
}

bool createProjectFromInventorySource(
    AppData* data,
    const ui::CreateProjectRequest& request,
    std::string* workspaceFeedback
) {
    if (!data) return false;
    const auto it = std::find_if(data->inventory.begin(), data->inventory.end(), [&](const domain::InventoryEntry& entry) {
        return entry.repoPath == request.sourceRepoPath;
    });
    if (it == data->inventory.end()) {
        if (workspaceFeedback) *workspaceFeedback = "Fonte selecionada nao encontrada no inventario atual.";
        return false;
    }
    if (request.projectId.empty() || request.projectTitle.empty()) {
        if (workspaceFeedback) *workspaceFeedback = "Projeto nao criado: ID e titulo sao obrigatorios.";
        return false;
    }

    domain::ResearchProject project = it->projectSnapshot;
    project.id = request.projectId;
    project.title = request.projectTitle;
    project.sourceRepoPath = it->repoPath;
    project.coordinator = request.coordinator.empty() ? "Coordenacao a definir" : request.coordinator;
    project.institution = request.institution.empty() ? "Nao informado" : request.institution;
    project.program = "Curadoria Manual";
    project.thematicAxis = "Projeto de Pesquisa";
    if (project.projectType.empty()) {
        project.projectType = project.softwareIntensive ? "PD&I Software" : "PD&I";
    }
    project.line = it->repoName;
    project.status = it->inferredStatus;
    data->store.add(std::move(project));
    if (workspaceFeedback) *workspaceFeedback = "Projeto criado manualmente a partir da fonte selecionada.";
    return true;
}

std::string joinWithPipe(const std::vector<std::string>& values) {
    std::string out;
    for (const auto& v : values) {
        if (v.empty()) continue;
        if (!out.empty()) out += '|';
        out += v;
    }
    return out;
}

bool updateProjectInStore(
    AppData* data,
    const ui::UpdateProjectRequest& request,
    std::string* workspaceFeedback
) {
    if (!data) return false;
    if (request.project.id.empty()) {
        if (workspaceFeedback) *workspaceFeedback = "Projeto nao atualizado: ID invalido.";
        return false;
    }
    if (request.project.title.empty()) {
        if (workspaceFeedback) *workspaceFeedback = "Projeto nao atualizado: titulo obrigatorio.";
        return false;
    }
    if (!data->store.update(request.project)) {
        if (workspaceFeedback) *workspaceFeedback = "Projeto nao encontrado para atualizacao.";
        return false;
    }
    if (workspaceFeedback) *workspaceFeedback = "Projeto atualizado com sucesso.";
    return true;
}

bool processInnovationMiningForSource(
    AppData* data,
    const std::string& sourceRepoPath,
    std::string* workspaceFeedback
) {
    if (!data || sourceRepoPath.empty()) return false;

    domain::InventoryScanner scanner;
    const auto rescanned = scanner.scan(sourceRepoPath, true);
    const domain::InventoryEntry* enriched = nullptr;
    for (const auto& item : rescanned) {
        if (item.repoPath == sourceRepoPath) {
            enriched = &item;
            break;
        }
    }
    if (!enriched) {
        if (workspaceFeedback) *workspaceFeedback = "Nao foi possivel processar contribuicoes/atividades para esta fonte.";
        return false;
    }

    for (auto& item : data->inventory) {
        if (item.repoPath != sourceRepoPath) continue;
        item.innovationContributions = enriched->innovationContributions;
        item.researchActivities = enriched->researchActivities;
        item.innovationSolutionName = enriched->innovationSolutionName;
        item.innovationSolutionDescription = enriched->innovationSolutionDescription;
        item.innovationSolutionResponsible = enriched->innovationSolutionResponsible;
        item.innovationSolutionStartDate = enriched->innovationSolutionStartDate;
        item.innovationSolutionDurationMonths = enriched->innovationSolutionDurationMonths;
        item.innovationSolutionEndDate = enriched->innovationSolutionEndDate;
        if (workspaceFeedback) {
            *workspaceFeedback = "Varredura de inovacao/atividades concluida para a fonte selecionada.";
        }
        return true;
    }

    if (workspaceFeedback) *workspaceFeedback = "Fonte selecionada nao encontrada no inventario atual.";
    return false;
}

int applyInventoryMetadataToExistingProjects(AppData* data) {
    if (!data) return 0;
    int updated = 0;
    for (const auto& inv : data->inventory) {
        if (inv.repoPath.empty()) continue;
        for (const auto& existing : data->store.all()) {
            if (existing.sourceRepoPath != inv.repoPath) continue;
            domain::ResearchProject merged = existing;
            if (!inv.projectSnapshot.title.empty()) merged.title = inv.projectSnapshot.title;
            if (!inv.projectSnapshot.coordinator.empty()) merged.coordinator = inv.projectSnapshot.coordinator;
            if (!inv.projectSnapshot.institution.empty()) merged.institution = inv.projectSnapshot.institution;
            if (!inv.projectSnapshot.callNotice.empty()) merged.callNotice = inv.projectSnapshot.callNotice;
            if (!inv.projectSnapshot.projectType.empty()) merged.projectType = inv.projectSnapshot.projectType;
            if (!inv.projectSnapshot.startDate.empty()) merged.startDate = inv.projectSnapshot.startDate;
            if (!inv.projectSnapshot.endDate.empty()) merged.endDate = inv.projectSnapshot.endDate;
            if (inv.projectSnapshot.durationMonths > 0) merged.durationMonths = inv.projectSnapshot.durationMonths;
            if (data->store.update(merged)) {
                ++updated;
            }
        }
    }
    return updated;
}

bool exportInventoryTsv(const AppData& data, std::string* workspaceFeedback) {
    namespace fs = std::filesystem;
    const fs::path outPath = fs::path(data.workspaceRoot) / "labgp_inventory_export.tsv";
    std::ofstream out(outPath);
    if (!out.is_open()) {
        if (workspaceFeedback) *workspaceFeedback = "Falha ao exportar inventario: nao foi possivel abrir arquivo de saida.";
        return false;
    }

    out << "repo_name\trepo_path\tsource\ttitle\tcoordinator\tinstitution\tleader_role\tsubmission_state\tsubmission_print_date\tcode_seg\tlinked_contract\tcall_notice\tproject_type\tstart_date\tduration_months\tend_date\tinnovation_solution_name\tinnovation_solution_description\tinnovation_solution_responsible\tinnovation_solution_start_date\tinnovation_solution_duration_months\tinnovation_solution_end_date\tteam_members_count\tteam_members\tfinancial_institutions_count\tfinancial_institutions\tpartner_institutions_count\tpartner_institutions\tsupport_foundations_count\tsupport_foundations\ttotal\tinstitutional\tresearcher\toperational\tmaturity\treliability\tinnovation\tactivity\tplanned_results\tcurated\tstatus_suggested\n";
    for (const auto& it : data.inventory) {
        out << it.repoName << '\t'
            << it.repoPath << '\t'
            << it.source << '\t'
            << it.projectSnapshot.title << '\t'
            << it.projectSnapshot.coordinator << '\t'
            << it.projectSnapshot.institution << '\t'
            << it.leaderRole << '\t'
            << it.submissionState << '\t'
            << it.submissionPrintDate << '\t'
            << it.codeSeg << '\t'
            << it.linkedContract << '\t'
            << it.projectSnapshot.callNotice << '\t'
            << it.projectSnapshot.projectType << '\t'
            << it.projectSnapshot.startDate << '\t'
            << it.projectSnapshot.durationMonths << '\t'
            << it.projectSnapshot.endDate << '\t'
            << escapeTsv(it.innovationSolutionName) << '\t'
            << escapeTsv(it.innovationSolutionDescription) << '\t'
            << escapeTsv(it.innovationSolutionResponsible) << '\t'
            << it.innovationSolutionStartDate << '\t'
            << it.innovationSolutionDurationMonths << '\t'
            << it.innovationSolutionEndDate << '\t'
            << static_cast<int>(it.teamMembers.size()) << '\t'
            << joinWithPipe(it.teamMembers) << '\t'
            << static_cast<int>(it.financialInstitutions.size()) << '\t'
            << joinWithPipe(it.financialInstitutions) << '\t'
            << static_cast<int>(it.partnerInstitutions.size()) << '\t'
            << joinWithPipe(it.partnerInstitutions) << '\t'
            << static_cast<int>(it.supportFoundations.size()) << '\t'
            << joinWithPipe(it.supportFoundations) << '\t'
            << it.score.total << '\t'
            << it.score.institutional << '\t'
            << it.score.researcher << '\t'
            << it.score.operational << '\t'
            << it.score.maturity << '\t'
            << it.score.reliability << '\t'
            << it.innovationSignals << '\t'
            << it.activitySignals << '\t'
            << it.plannedResultsSignals << '\t'
            << it.interpretedDocsIncluded << "/" << it.interpretedDocsTotal << '\t'
            << domain::toString(it.inferredStatus) << '\n';
    }
    if (workspaceFeedback) {
        *workspaceFeedback = "Inventario exportado para: " + outPath.string();
    }
    return true;
}

int runConsole(const AppData& data) {
    const ui::AppUI appUi(data.store);
    std::cout << "Workspace: " << data.workspaceRoot << "\n";
    std::cout << "Fontes detectadas no inventario: " << data.inventory.size() << "\n";
    std::cout << "Projetos cadastrados em tela: " << data.store.all().size() << "\n\n";
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
        bool requestExportInventory = false;
        bool requestSaveProjects = false;
        bool requestExit = false;
        std::string requestApplyWorkspacePath;
        std::string requestProcessInnovationSourceRepoPath;
        ui::CreateProjectRequest requestCreateProject;
        ui::UpdateProjectRequest requestUpdateProject;
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
            &requestExportInventory,
            &requestSaveProjects,
            &requestExit,
            &requestApplyWorkspacePath,
            &requestProcessInnovationSourceRepoPath,
            &requestCreateProject,
            &requestUpdateProject,
            workspaceFeedback
        );

        if (requestRescan) {
            *data = buildAppData(data->workspaceRoot, data->includeDemoProjects);
            workspaceFeedback = "Inventario reescanado (modo leve). Use o botao de processamento para inovacao/atividades por fonte.";
        }

        if (!requestApplyWorkspacePath.empty()) {
            if (std::filesystem::exists(requestApplyWorkspacePath) && std::filesystem::is_directory(requestApplyWorkspacePath)) {
                *data = buildAppData(requestApplyWorkspacePath, data->includeDemoProjects);
                workspaceFeedback = "Workspace atualizado manualmente.";
            } else {
                workspaceFeedback = "Pasta invalida. Verifique o caminho informado.";
            }
        }
        if (!requestProcessInnovationSourceRepoPath.empty()) {
            processInnovationMiningForSource(data, requestProcessInnovationSourceRepoPath, &workspaceFeedback);
        }

        if (!requestCreateProject.sourceRepoPath.empty()) {
            createProjectFromInventorySource(data, requestCreateProject, &workspaceFeedback);
        }
        if (!requestUpdateProject.project.id.empty()) {
            updateProjectInStore(data, requestUpdateProject, &workspaceFeedback);
        }
        if (requestExportInventory) {
            exportInventoryTsv(*data, &workspaceFeedback);
        }
        if (requestSaveProjects) {
            saveProjectsToDisk(*data, &workspaceFeedback);
        }
        if (requestExit) {
            done = true;
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
