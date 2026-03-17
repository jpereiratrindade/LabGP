#include "ui/GuiDashboard.hpp"

#include "domain/Scoring.hpp"

#include "imgui.h"

#include <algorithm>
#include <array>
#include <string>

namespace labgp::ui {

namespace {

const char* bandColorLabel(int total) {
    if (total >= 80) return "Verde";
    if (total >= 60) return "Amarelo";
    if (total >= 40) return "Laranja";
    return "Vermelho";
}

void renderProjectsTab(const std::vector<domain::ResearchProject>& projects) {
    if (ImGui::BeginTable("projects_table", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Titulo");
        ImGui::TableSetupColumn("Status");
        ImGui::TableSetupColumn("Total");
        ImGui::TableSetupColumn("Oper");
        ImGui::TableSetupColumn("Matur");
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
            ImGui::TableSetColumnIndex(3); ImGui::Text("%d", score.total);
            ImGui::TableSetColumnIndex(4); ImGui::Text("%d", score.operational);
            ImGui::TableSetColumnIndex(5); ImGui::Text("%d", score.maturity);
            ImGui::TableSetColumnIndex(6); ImGui::Text("%d", score.reliability);
        }

        ImGui::EndTable();
    }
}

void renderKanbanTab(const std::vector<domain::ResearchProject>& projects) {
    const std::array<domain::ResearchStatus, 6> orderedStatus = {
        domain::ResearchStatus::Proposal,
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
            ImGui::Text("Score: %d (%s)", score.total, bandColorLabel(score.total));
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
    const std::string& workspaceRoot
) const {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("LabGP", nullptr, flags);

    ImGui::TextUnformatted("LabGP - Gestao de Projetos de Pesquisa");
    ImGui::Text("Workspace: %s", workspaceRoot.c_str());
    ImGui::Separator();

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
