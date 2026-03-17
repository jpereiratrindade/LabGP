#include "ui/GuiDashboard.hpp"

#include "domain/Scoring.hpp"

#include "imgui.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

namespace labgp::ui {

namespace {

const char* bandColorLabel(int total) {
    if (total >= 80) return "Verde";
    if (total >= 60) return "Amarelo";
    if (total >= 40) return "Laranja";
    return "Vermelho";
}

void renderProjectsTab(const std::vector<domain::ResearchProject>& projects) {
    if (ImGui::BeginTable("projects_table", 9, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Titulo");
        ImGui::TableSetupColumn("Status");
        ImGui::TableSetupColumn("Tipo");
        ImGui::TableSetupColumn("Total");
        ImGui::TableSetupColumn("Oper");
        ImGui::TableSetupColumn("Matur");
        ImGui::TableSetupColumn("Exec");
        ImGui::TableSetupColumn("Conf");
        ImGui::TableHeadersRow();

        std::vector<const domain::ResearchProject*> rows;
        rows.reserve(projects.size());
        for (const auto& p : projects) rows.push_back(&p);

        std::sort(rows.begin(), rows.end(), [](const auto* a, const auto* b) {
            const auto as = domain::computeScore(*a);
            const auto bs = domain::computeScore(*b);
            if (as.total != bs.total) return as.total > bs.total;
            return a->id < b->id;
        });

        for (const auto* p : rows) {
            const auto score = domain::computeScore(*p);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(p->id.c_str());
            ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(p->title.c_str());
            ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(domain::toString(p->status).c_str());
            ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(p->softwareIntensive ? "Software" : "Pesquisa");
            ImGui::TableSetColumnIndex(4); ImGui::Text("%d", score.total);
            ImGui::TableSetColumnIndex(5); ImGui::Text("%d", score.operational);
            ImGui::TableSetColumnIndex(6); ImGui::Text("%d", score.maturity);
            ImGui::TableSetColumnIndex(7); ImGui::Text("%d", score.execution);
            ImGui::TableSetColumnIndex(8);
            if (score.reliabilityApplicable) {
                ImGui::Text("%d", score.reliability);
            } else {
                ImGui::TextUnformatted("N/A");
            }
        }

        ImGui::EndTable();
    }
}

void renderKanbanTab(const std::vector<domain::ResearchProject>& projects) {
    const std::array<domain::ResearchStatus, 7> orderedStatus = {
        domain::ResearchStatus::Proposal,
        domain::ResearchStatus::InReview,
        domain::ResearchStatus::Approved,
        domain::ResearchStatus::Execution,
        domain::ResearchStatus::Analysis,
        domain::ResearchStatus::Publication,
        domain::ResearchStatus::Closed,
    };

    const float contentWidth = ImGui::GetContentRegionAvail().x;
    const float colWidth = contentWidth / static_cast<float>(orderedStatus.size()) - 6.0f;

    for (size_t i = 0; i < orderedStatus.size(); ++i) {
        const auto status = orderedStatus[i];
        ImGui::BeginChild((std::string("kanban_") + std::to_string(static_cast<int>(i))).c_str(), ImVec2(colWidth, 0), true);
        ImGui::TextUnformatted(domain::toString(status).c_str());
        ImGui::Separator();

        bool any = false;
        for (const auto& p : projects) {
            if (p.status != status) continue;
            const auto score = domain::computeScore(p);
            any = true;
            ImGui::BulletText("%s", p.id.c_str());
            ImGui::TextWrapped("%s", p.title.c_str());
            ImGui::Text("Score: %d (%s) | Exec: %d", score.total, bandColorLabel(score.total), score.execution);
            ImGui::Spacing();
        }
        if (!any) {
            ImGui::TextDisabled("(vazio)");
        }
        ImGui::EndChild();
        if (i + 1 < orderedStatus.size()) ImGui::SameLine();
    }
}

void renderInventoryTab(const std::vector<domain::InventoryEntry>& inventory) {
    ImGui::Text("Repositorios detectados: %d", static_cast<int>(inventory.size()));
    ImGui::Spacing();

    if (ImGui::BeginTable("inventory_table", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Repo");
        ImGui::TableSetupColumn("Total");
        ImGui::TableSetupColumn("Oper");
        ImGui::TableSetupColumn("Matur");
        ImGui::TableSetupColumn("Conf");
        ImGui::TableSetupColumn("Integrado");
        ImGui::TableHeadersRow();

        for (const auto& it : inventory) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(it.repoName.c_str());
            ImGui::TableSetColumnIndex(1); ImGui::Text("%d", it.score.total);
            ImGui::TableSetColumnIndex(2); ImGui::Text("%d", it.score.operational);
            ImGui::TableSetColumnIndex(3); ImGui::Text("%d", it.score.maturity);
            ImGui::TableSetColumnIndex(4); ImGui::Text("%d", it.score.reliability);
            ImGui::TableSetColumnIndex(5); ImGui::TextUnformatted(it.integrated ? "Sim" : "Nao");
        }

        ImGui::EndTable();
    }
}

} // namespace

void GuiDashboard::render(
    const std::vector<domain::ResearchProject>& projects,
    const std::vector<domain::InventoryEntry>& inventory,
    const std::string& workspaceRoot,
    bool* requestRescan,
    std::string* requestApplyWorkspacePath,
    const std::string& workspaceFeedback
) const {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("LabGP", nullptr, flags | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Arquivo")) {
            if (ImGui::MenuItem("Selecionar pasta de projetos...")) {
                m_showWorkspaceModal = true;
                std::snprintf(m_workspacePathBuf.data(), m_workspacePathBuf.size(), "%s", workspaceRoot.c_str());
                m_workspaceNavPath = workspaceRoot;
                ImGui::OpenPopup("workspace_modal");
            }
            if (ImGui::MenuItem("Reescanear agora")) {
                if (requestRescan) {
                    *requestRescan = true;
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::TextUnformatted("LabGP - Gestao de Projetos de Pesquisa");
    ImGui::Text("Workspace: %s", workspaceRoot.c_str());
    if (ImGui::Button("Selecionar Pasta")) {
        m_showWorkspaceModal = true;
        std::snprintf(m_workspacePathBuf.data(), m_workspacePathBuf.size(), "%s", workspaceRoot.c_str());
        m_workspaceNavPath = workspaceRoot;
        ImGui::OpenPopup("workspace_modal");
    }
    ImGui::SameLine();
    if (ImGui::Button("Reescanear")) {
        if (requestRescan) {
            *requestRescan = true;
        }
    }
    if (!workspaceFeedback.empty()) {
        ImGui::TextColored(ImVec4(0.55f, 0.82f, 0.55f, 1.0f), "%s", workspaceFeedback.c_str());
    }
    ImGui::Separator();

    if (m_showWorkspaceModal) {
        namespace fs = std::filesystem;

        ImGui::SetNextWindowSize(ImVec2(860, 540), ImGuiCond_FirstUseEver);
        if (ImGui::BeginPopupModal("workspace_modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            if (m_workspaceNavPath.empty()) m_workspaceNavPath = workspaceRoot;

            ImGui::TextWrapped("Selecione a pasta que contem os repositorios de projetos:");
            ImGui::InputText("Caminho", m_workspacePathBuf.data(), m_workspacePathBuf.size());

            if (ImGui::Button("Ir para caminho digitado")) {
                const fs::path typed(m_workspacePathBuf.data());
                std::error_code ec;
                if (fs::exists(typed, ec) && fs::is_directory(typed, ec)) {
                    m_workspaceNavPath = typed.string();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Subir um nivel")) {
                fs::path p(m_workspaceNavPath);
                p = p.parent_path();
                if (!p.empty()) {
                    m_workspaceNavPath = p.string();
                    std::snprintf(m_workspacePathBuf.data(), m_workspacePathBuf.size(), "%s", m_workspaceNavPath.c_str());
                }
            }

            ImGui::Separator();
            ImGui::Text("Navegando: %s", m_workspaceNavPath.c_str());
            ImGui::BeginChild("dirs_list", ImVec2(820, 360), true);

            std::vector<std::string> children;
            std::error_code ec;
            for (const auto& entry : fs::directory_iterator(fs::path(m_workspaceNavPath), fs::directory_options::skip_permission_denied, ec)) {
                if (entry.is_directory(ec)) {
                    children.push_back(entry.path().filename().string());
                }
            }
            std::sort(children.begin(), children.end());

            for (const auto& name : children) {
                if (ImGui::Selectable(name.c_str())) {
                    fs::path next = fs::path(m_workspaceNavPath) / name;
                    m_workspaceNavPath = next.string();
                    std::snprintf(m_workspacePathBuf.data(), m_workspacePathBuf.size(), "%s", m_workspaceNavPath.c_str());
                }
            }

            ImGui::EndChild();
            ImGui::Separator();

            if (ImGui::Button("Usar esta pasta")) {
                if (requestApplyWorkspacePath) {
                    *requestApplyWorkspacePath = m_workspaceNavPath;
                }
                m_showWorkspaceModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancelar")) {
                m_showWorkspaceModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    if (ImGui::BeginTabBar("labgp_tabs")) {
        if (ImGui::BeginTabItem("Projetos")) {
            renderProjectsTab(projects);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Kanban")) {
            renderKanbanTab(projects);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Inventario")) {
            renderInventoryTab(inventory);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

} // namespace labgp::ui
