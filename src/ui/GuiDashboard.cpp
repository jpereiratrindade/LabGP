#include "ui/GuiDashboard.hpp"

#include "domain/Scoring.hpp"

#include "imgui.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace labgp::ui {

namespace {

enum class ScorePerspective {
    Consolidated = 0,
    Institutional = 1,
    Researcher = 2,
};

const char* bandColorLabel(int total) {
    if (total >= 80) return "Verde";
    if (total >= 60) return "Amarelo";
    if (total >= 40) return "Laranja";
    return "Vermelho";
}

ImU32 bandColorU32(int total) {
    if (total >= 80) return IM_COL32(64, 196, 99, 255);
    if (total >= 60) return IM_COL32(225, 193, 67, 255);
    if (total >= 40) return IM_COL32(221, 141, 67, 255);
    return IM_COL32(214, 75, 75, 255);
}

const char* perspectiveLabel(ScorePerspective perspective) {
    switch (perspective) {
        case ScorePerspective::Institutional: return "Institucional";
        case ScorePerspective::Researcher: return "Pesquisador";
        case ScorePerspective::Consolidated:
        default: return "Consolidado";
    }
}

const char* perspectiveShortLabel(ScorePerspective perspective) {
    switch (perspective) {
        case ScorePerspective::Institutional: return "Inst";
        case ScorePerspective::Researcher: return "Pesq";
        case ScorePerspective::Consolidated:
        default: return "Total";
    }
}

int scoreByPerspective(const domain::ScoreBreakdown& score, ScorePerspective perspective) {
    switch (perspective) {
        case ScorePerspective::Institutional: return score.institutional;
        case ScorePerspective::Researcher: return score.researcher;
        case ScorePerspective::Consolidated:
        default: return score.total;
    }
}

void renderPerspectiveSelector(ScorePerspective* perspective) {
    if (!perspective) return;
    int current = static_cast<int>(*perspective);
    ImGui::TextUnformatted("Visao de score:");
    ImGui::SameLine();
    ImGui::RadioButton("Consolidado", &current, static_cast<int>(ScorePerspective::Consolidated));
    ImGui::SameLine();
    ImGui::RadioButton("Institucional", &current, static_cast<int>(ScorePerspective::Institutional));
    ImGui::SameLine();
    ImGui::RadioButton("Pesquisador", &current, static_cast<int>(ScorePerspective::Researcher));
    *perspective = static_cast<ScorePerspective>(current);
}

void renderInterpretedDocuments(const std::vector<domain::InterpretedDocument>& docs) {
    ImGui::TextDisabled("Documentos PDF interpretados");
    if (docs.empty()) {
        ImGui::TextUnformatted("-");
        return;
    }
    for (const auto& d : docs) {
        const std::string shortHash = d.sha256.empty() ? "-" : d.sha256.substr(0, std::min<std::size_t>(12, d.sha256.size()));
        ImGui::BulletText("%s", d.fileName.c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("[%s] %d bytes %s", shortHash.c_str(), d.textBytes, d.usedCache ? "(cache)" : "(extraido)");
    }
}

void renderProjectsTab(
    const std::vector<domain::ResearchProject>& projects,
    const std::vector<domain::InventoryEntry>& inventory,
    ScorePerspective perspective
) {
    static std::string selectedProjectId;
    static float projectDetailWidth = 430.f;
    const float splitterWidth = 6.f;

    ImGui::TextColored(ImVec4(0.85f, 0.90f, 1.0f, 1.0f), "Fluxo do Projeto: coluna 'Fluxo (Status)'");
    ImGui::TextDisabled("Score de Qualidade: %s/Inst/Pesq/Total", perspectiveShortLabel(perspective));
    ImGui::Spacing();

    const bool hasDetail = !selectedProjectId.empty();
    const float detailWidth = hasDetail ? projectDetailWidth : 0.f;
    const float availableHeight = ImGui::GetContentRegionAvail().y;
    float tableWidth = ImGui::GetContentRegionAvail().x - detailWidth - (hasDetail ? (splitterWidth + 4.f) : 0.f);
    if (tableWidth < 300.f) tableWidth = ImGui::GetContentRegionAvail().x;

    ImGui::BeginChild("##projects_table_region", ImVec2(tableWidth, availableHeight), false);
    if (ImGui::BeginTable("projects_table", 12, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Titulo");
        ImGui::TableSetupColumn("Fluxo (Status)");
        ImGui::TableSetupColumn("Tipo");
        std::string qualityCol = std::string("Qualid(") + perspectiveShortLabel(perspective) + ")";
        ImGui::TableSetupColumn(qualityCol.c_str());
        ImGui::TableSetupColumn("Total");
        ImGui::TableSetupColumn("Inst");
        ImGui::TableSetupColumn("Pesq");
        ImGui::TableSetupColumn("Oper");
        ImGui::TableSetupColumn("Matur");
        ImGui::TableSetupColumn("Exec");
        ImGui::TableSetupColumn("Conf");
        ImGui::TableHeadersRow();

        std::vector<const domain::ResearchProject*> rows;
        rows.reserve(projects.size());
        for (const auto& p : projects) rows.push_back(&p);

        std::sort(rows.begin(), rows.end(), [perspective](const auto* a, const auto* b) {
            const auto as = domain::computeScore(*a);
            const auto bs = domain::computeScore(*b);
            const int asPerspective = scoreByPerspective(as, perspective);
            const int bsPerspective = scoreByPerspective(bs, perspective);
            if (asPerspective != bsPerspective) return asPerspective > bsPerspective;
            return a->id < b->id;
        });

        for (const auto* p : rows) {
            const auto score = domain::computeScore(*p);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            bool selected = (selectedProjectId == p->id);
            if (ImGui::Selectable(("##proj_row_" + p->id).c_str(), selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
                selectedProjectId = selected ? "" : p->id;
            }
            ImGui::SameLine();
            ImGui::TextUnformatted(p->id.c_str());
            ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(p->title.c_str());
            ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(domain::toString(p->status).c_str());
            ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(p->softwareIntensive ? "Software" : "Pesquisa");
            ImGui::TableSetColumnIndex(4); ImGui::Text("%d", scoreByPerspective(score, perspective));
            ImGui::TableSetColumnIndex(5); ImGui::Text("%d", score.total);
            ImGui::TableSetColumnIndex(6); ImGui::Text("%d", score.institutional);
            ImGui::TableSetColumnIndex(7); ImGui::Text("%d", score.researcher);
            ImGui::TableSetColumnIndex(8); ImGui::Text("%d", score.operational);
            ImGui::TableSetColumnIndex(9); ImGui::Text("%d", score.maturity);
            ImGui::TableSetColumnIndex(10); ImGui::Text("%d", score.execution);
            ImGui::TableSetColumnIndex(11);
            if (score.reliabilityApplicable) {
                ImGui::Text("%d", score.reliability);
            } else {
                ImGui::TextUnformatted("N/A");
            }
        }

        ImGui::EndTable();
    }
    ImGui::EndChild();

    if (!selectedProjectId.empty()) {
        const domain::ResearchProject* selected = nullptr;
        for (const auto& p : projects) {
            if (p.id == selectedProjectId) {
                selected = &p;
                break;
            }
        }
        if (!selected) {
            selectedProjectId.clear();
            return;
        }
        const auto score = domain::computeScore(*selected);
        auto findInventoryForProject = [&](const domain::ResearchProject& p) -> const domain::InventoryEntry* {
            const std::string prefix = "Integracao: ";
            if (p.title.rfind(prefix, 0) != 0) return nullptr;
            const std::string repoName = p.title.substr(prefix.size());
            for (const auto& it : inventory) {
                if (it.repoName == repoName) return &it;
            }
            return nullptr;
        };
        const domain::InventoryEntry* inv = findInventoryForProject(*selected);

        auto renderField = [](const char* label, const std::string& value) {
            ImGui::TextDisabled("%s", label);
            if (value.empty()) ImGui::TextUnformatted("-");
            else ImGui::TextWrapped("%s", value.c_str());
        };

        ImGui::SameLine(0.f, 2.f);
        ImGui::InvisibleButton("##projects_splitter", ImVec2(splitterWidth, availableHeight));
        if (ImGui::IsItemActive()) {
            projectDetailWidth -= ImGui::GetIO().MouseDelta.x;
            projectDetailWidth = std::clamp(projectDetailWidth, 300.0f, 760.0f);
        }
        ImGui::SameLine(0.f, 2.f);
        ImGui::BeginChild("##projects_detail_region", ImVec2(projectDetailWidth, availableHeight), true);
        if (ImGui::SmallButton("X")) selectedProjectId.clear();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.55f, 0.75f, 1.f, 1.f), "%s", selected->id.c_str());
        ImGui::TextWrapped("%s", selected->title.c_str());
        ImGui::Separator();
        ImGui::Text("Fluxo: %s", domain::toString(selected->status).c_str());
        ImGui::Text("Tipo: %s", selected->softwareIntensive ? "Software" : "Pesquisa");
        ImGui::Text("Qualidade (%s): %d", perspectiveShortLabel(perspective), scoreByPerspective(score, perspective));
        ImGui::Text("Total: %d | Inst: %d | Pesq: %d", score.total, score.institutional, score.researcher);
        ImGui::Text("Oper: %d | Matur: %d | Exec: %d | Conf: %s",
            score.operational, score.maturity, score.execution,
            score.reliabilityApplicable ? std::to_string(score.reliability).c_str() : "N/A");
        ImGui::Separator();
        renderField("Programa", selected->program);
        renderField("Eixo tematico", selected->thematicAxis);
        renderField("Linha", selected->line);
        renderField("Coordenacao", selected->coordinator);
        renderField("Instituicao", selected->institution);
        if (inv) {
            ImGui::Separator();
            renderField("Resumo", inv->summary);
            renderField("Objetivos", inv->objectives);
            renderField("Contribuicoes para Inovacao", inv->innovationContributions);
            renderField("Atividades de Pesquisa", inv->researchActivities);
            renderField("Resultados Esperados", inv->expectedResults);
            ImGui::TextDisabled("Equipe");
            if (inv->teamMembers.empty()) {
                ImGui::TextUnformatted("-");
            } else {
                for (const auto& m : inv->teamMembers) ImGui::BulletText("%s", m.c_str());
            }
            ImGui::Separator();
            renderInterpretedDocuments(inv->interpretedDocuments);
        }
        ImGui::EndChild();
    }
}

void renderKanbanTab(const std::vector<domain::ResearchProject>& projects, ScorePerspective perspective) {
    ImGui::TextColored(ImVec4(0.85f, 0.90f, 1.0f, 1.0f), "Kanban = Fluxo do Projeto (etapas).");
    ImGui::TextDisabled("Score mostrado no card = qualidade (%s/Inst/Pesq/Total).", perspectiveShortLabel(perspective));
    ImGui::Spacing();

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
            const int perspectiveScore = scoreByPerspective(score, perspective);
            any = true;
            ImGui::BulletText("%s", p.id.c_str());
            ImGui::TextWrapped("%s", p.title.c_str());
            ImGui::Text("Fluxo: %s", domain::toString(p.status).c_str());
            ImGui::Text(
                "Qualidade %s: %d (%s) | Total: %d | Exec: %d",
                perspectiveShortLabel(perspective),
                perspectiveScore,
                bandColorLabel(perspectiveScore),
                score.total,
                score.execution
            );
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
    static std::string selectedRepoPath;
    static float inventoryDetailWidth = 420.f;
    const float splitterWidth = 6.f;

    ImGui::Text("Repositorios detectados: %d", static_cast<int>(inventory.size()));
    ImGui::TextColored(ImVec4(0.85f, 0.90f, 1.0f, 1.0f), "Fluxo do Projeto: coluna 'Fluxo (Status inferido)'.");
    ImGui::TextDisabled("Score de Qualidade: Total/Inst/Pesq/Oper/Matur/Conf.");
    ImGui::Spacing();

    const bool hasDetail = !selectedRepoPath.empty();
    const float detailWidth = hasDetail ? inventoryDetailWidth : 0.f;
    const float availableHeight = ImGui::GetContentRegionAvail().y;
    float tableWidth = ImGui::GetContentRegionAvail().x - detailWidth - (hasDetail ? (splitterWidth + 4.f) : 0.f);
    if (tableWidth < 300.f) tableWidth = ImGui::GetContentRegionAvail().x;

    ImGui::BeginChild("##inventory_table_region", ImVec2(tableWidth, availableHeight), false);
    if (ImGui::BeginTable("inventory_table", 12, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Repo");
        ImGui::TableSetupColumn("Origem");
        ImGui::TableSetupColumn("Total");
        ImGui::TableSetupColumn("Inst");
        ImGui::TableSetupColumn("Pesq");
        ImGui::TableSetupColumn("Oper");
        ImGui::TableSetupColumn("Matur");
        ImGui::TableSetupColumn("Conf");
        ImGui::TableSetupColumn("Inov");
        ImGui::TableSetupColumn("Ativ");
        ImGui::TableSetupColumn("ResPrev");
        ImGui::TableSetupColumn("Fluxo (Status inferido)");
        ImGui::TableHeadersRow();

        for (const auto& it : inventory) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            bool selected = (selectedRepoPath == it.repoPath);
            if (ImGui::Selectable(("##inv_row_" + it.repoPath).c_str(), selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
                selectedRepoPath = selected ? "" : it.repoPath;
            }
            ImGui::SameLine();
            ImGui::TextUnformatted(it.repoName.c_str());
            ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(it.source.c_str());
            ImGui::TableSetColumnIndex(2); ImGui::Text("%d", it.score.total);
            ImGui::TableSetColumnIndex(3); ImGui::Text("%d", it.score.institutional);
            ImGui::TableSetColumnIndex(4); ImGui::Text("%d", it.score.researcher);
            ImGui::TableSetColumnIndex(5); ImGui::Text("%d", it.score.operational);
            ImGui::TableSetColumnIndex(6); ImGui::Text("%d", it.score.maturity);
            ImGui::TableSetColumnIndex(7); ImGui::Text("%d", it.score.reliability);
            ImGui::TableSetColumnIndex(8); ImGui::Text("%d", it.innovationSignals);
            ImGui::TableSetColumnIndex(9); ImGui::Text("%d", it.activitySignals);
            ImGui::TableSetColumnIndex(10); ImGui::Text("%d", it.plannedResultsSignals);
            ImGui::TableSetColumnIndex(11); ImGui::TextUnformatted(domain::toString(it.inferredStatus).c_str());
        }

        ImGui::EndTable();
    }
    ImGui::EndChild();

    if (!selectedRepoPath.empty()) {
        const domain::InventoryEntry* selected = nullptr;
        for (const auto& it : inventory) {
            if (it.repoPath == selectedRepoPath) {
                selected = &it;
                break;
            }
        }
        if (!selected) {
            selectedRepoPath.clear();
            return;
        }

        auto renderField = [](const char* label, const std::string& value) {
            ImGui::TextDisabled("%s", label);
            if (value.empty()) {
                ImGui::TextUnformatted("-");
            } else {
                ImGui::TextWrapped("%s", value.c_str());
            }
        };

        ImGui::SameLine(0.f, 2.f);
        ImGui::InvisibleButton("##inventory_splitter", ImVec2(splitterWidth, availableHeight));
        if (ImGui::IsItemActive()) {
            inventoryDetailWidth -= ImGui::GetIO().MouseDelta.x;
            inventoryDetailWidth = std::clamp(inventoryDetailWidth, 300.0f, 760.0f);
        }
        ImGui::SameLine(0.f, 2.f);
        ImGui::BeginChild("##inventory_detail_region", ImVec2(inventoryDetailWidth, availableHeight), true);
        if (ImGui::SmallButton("X")) selectedRepoPath.clear();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.55f, 0.75f, 1.f, 1.f), "%s", selected->repoName.c_str());
        ImGui::TextDisabled("%s", selected->repoPath.c_str());
        ImGui::Separator();
        renderField("Resumo", selected->summary);
        ImGui::Separator();
        renderField("Objetivos", selected->objectives);
        ImGui::Separator();
        renderField("Contribuicoes para Inovacao", selected->innovationContributions);
        ImGui::Separator();
        renderField("Atividades de Pesquisa", selected->researchActivities);
        ImGui::Separator();
        renderField("Resultados Esperados", selected->expectedResults);
        ImGui::Separator();
        ImGui::TextDisabled("Equipe");
        if (selected->teamMembers.empty()) {
            ImGui::TextUnformatted("-");
        } else {
            for (const auto& member : selected->teamMembers) {
                ImGui::BulletText("%s", member.c_str());
            }
        }
        ImGui::Separator();
        renderInterpretedDocuments(selected->interpretedDocuments);
        ImGui::EndChild();
    }
}

void renderGraphTab(const std::vector<domain::ResearchProject>& projects, ScorePerspective perspective) {
    struct GraphState {
        float panX{0.f};
        float panY{0.f};
        float scale{1.f};
        int minQuality{0};
        bool onlyCritical{false};
        int criticalThreshold{40};
        bool layoutInitialized{false};
        bool layoutRunning{true};
        std::string selectedId;
        std::string hoveredId;
        std::unordered_map<std::string, ImVec2> pos;
        std::unordered_map<std::string, ImVec2> vel;
    };
    static GraphState state;

    auto statusColor = [](domain::ResearchStatus status) -> ImU32 {
        switch (status) {
            case domain::ResearchStatus::Proposal: return IM_COL32(95, 67, 180, 235);
            case domain::ResearchStatus::InReview: return IM_COL32(60, 140, 228, 235);
            case domain::ResearchStatus::Approved: return IM_COL32(225, 170, 60, 235);
            case domain::ResearchStatus::Execution: return IM_COL32(44, 192, 100, 235);
            case domain::ResearchStatus::Analysis: return IM_COL32(163, 124, 220, 235);
            case domain::ResearchStatus::Publication: return IM_COL32(87, 206, 225, 235);
            case domain::ResearchStatus::Closed: return IM_COL32(160, 160, 160, 235);
        }
        return IM_COL32(120, 120, 120, 235);
    };

    auto sharesAutoConnection = [](const domain::ResearchProject& a, const domain::ResearchProject& b) {
        if (a.status == b.status) return true;
        if (!a.program.empty() && a.program == b.program) return true;
        if (!a.thematicAxis.empty() && a.thematicAxis == b.thematicAxis) return true;
        if (!a.line.empty() && a.line == b.line) return true;
        return false;
    };

    ImGui::Checkbox("Layout automatico", &state.layoutRunning);
    ImGui::SameLine();
    if (ImGui::Button("Reorganizar")) {
        state.layoutInitialized = false;
        state.selectedId.clear();
        state.hoveredId.clear();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("| Scroll = zoom  |  Arrastar fundo = pan  |  Clique = detalhes");

    ImGui::SetNextItemWidth(140.f);
    ImGui::SliderInt("Qualidade Min", &state.minQuality, 0, 100);
    ImGui::SameLine();
    ImGui::Checkbox("So criticos", &state.onlyCritical);
    ImGui::TextDisabled("Borda por score da visao ativa: verde>=80, amarelo>=60, laranja>=40, vermelho<40.");
    ImGui::TextDisabled("Nos criticos (<%d) aparecem maiores.", state.criticalThreshold);

    std::vector<const domain::ResearchProject*> filtered;
    filtered.reserve(projects.size());
    for (const auto& p : projects) {
        const auto score = domain::computeScore(p);
        const int quality = scoreByPerspective(score, perspective);
        if (quality < state.minQuality) continue;
        if (state.onlyCritical && quality >= state.criticalThreshold) continue;
        filtered.push_back(&p);
    }

    if (!state.layoutInitialized) {
        std::mt19937 rng{42};
        std::uniform_real_distribution<float> distX(120.f, 880.f);
        std::uniform_real_distribution<float> distY(100.f, 600.f);
        state.pos.clear();
        state.vel.clear();
        for (const auto* p : filtered) {
            if (!p) continue;
            state.pos[p->id] = ImVec2(distX(rng), distY(rng));
            state.vel[p->id] = ImVec2(0.f, 0.f);
        }
        state.layoutInitialized = true;
    } else {
        for (const auto* p : filtered) {
            if (!p) continue;
            if (state.pos.find(p->id) == state.pos.end()) {
                state.pos[p->id] = ImVec2(180.f, 180.f);
                state.vel[p->id] = ImVec2(0.f, 0.f);
            }
        }
    }

    if (state.layoutRunning && filtered.size() > 1) {
        const float dt = ImGui::GetIO().DeltaTime > 0.f ? ImGui::GetIO().DeltaTime : 1.0f / 60.0f;
        const float repulsion = 7000.f;
        const float attraction = 0.035f;
        const float damping = 0.86f;
        const float idealLen = 170.f;

        for (const auto* p : filtered) state.vel[p->id] = ImVec2(0.f, 0.f);

        for (size_t i = 0; i < filtered.size(); ++i) {
            for (size_t j = i + 1; j < filtered.size(); ++j) {
                const auto* a = filtered[i];
                const auto* b = filtered[j];
                ImVec2& pa = state.pos[a->id];
                ImVec2& pb = state.pos[b->id];
                float dx = pb.x - pa.x;
                float dy = pb.y - pa.y;
                float d2 = dx * dx + dy * dy + 1.f;
                float invDist = 1.0f / std::sqrt(d2);
                float force = repulsion / d2;
                float fx = force * dx * invDist;
                float fy = force * dy * invDist;
                state.vel[a->id].x -= fx;
                state.vel[a->id].y -= fy;
                state.vel[b->id].x += fx;
                state.vel[b->id].y += fy;
            }
        }

        for (size_t i = 0; i < filtered.size(); ++i) {
            for (size_t j = i + 1; j < filtered.size(); ++j) {
                const auto* a = filtered[i];
                const auto* b = filtered[j];
                if (!sharesAutoConnection(*a, *b)) continue;
                ImVec2& pa = state.pos[a->id];
                ImVec2& pb = state.pos[b->id];
                float dx = pb.x - pa.x;
                float dy = pb.y - pa.y;
                float dist = std::sqrt(dx * dx + dy * dy) + 0.01f;
                float force = attraction * (dist - idealLen);
                float nx = dx / dist;
                float ny = dy / dist;
                state.vel[a->id].x += force * nx;
                state.vel[a->id].y += force * ny;
                state.vel[b->id].x -= force * nx;
                state.vel[b->id].y -= force * ny;
            }
        }

        for (const auto* p : filtered) {
            ImVec2& v = state.vel[p->id];
            ImVec2& pos = state.pos[p->id];
            pos.x += v.x * dt * damping;
            pos.y += v.y * dt * damping;
        }
    }

    float detailWidth = state.selectedId.empty() ? 0.f : 360.f;
    float graphWidth = ImGui::GetContentRegionAvail().x - detailWidth - (detailWidth > 0 ? 8.f : 0.f);
    if (graphWidth < 240.f) graphWidth = ImGui::GetContentRegionAvail().x;

    ImGui::BeginChild("##graph_canvas", ImVec2(graphWidth, 0.f), false, ImGuiWindowFlags_NoScrollWithMouse);
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    if (canvasSize.x < 60.f || canvasSize.y < 60.f) {
        ImGui::EndChild();
        return;
    }

    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(10, 12, 18, 255));
    draw->AddRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(50, 60, 80, 200));

    const float gridStep = 60.f;
    for (float gx = std::fmod(state.panX * state.scale, gridStep); gx < canvasSize.x; gx += gridStep) {
        draw->AddLine(ImVec2(canvasPos.x + gx, canvasPos.y), ImVec2(canvasPos.x + gx, canvasPos.y + canvasSize.y), IM_COL32(30, 35, 48, 180));
    }
    for (float gy = std::fmod(state.panY * state.scale, gridStep); gy < canvasSize.y; gy += gridStep) {
        draw->AddLine(ImVec2(canvasPos.x, canvasPos.y + gy), ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + gy), IM_COL32(30, 35, 48, 180));
    }

    draw->PushClipRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), true);

    auto toScreen = [&](const std::string& id) {
        const ImVec2 p = state.pos[id];
        return ImVec2(canvasPos.x + state.panX + p.x * state.scale, canvasPos.y + state.panY + p.y * state.scale);
    };

    for (size_t i = 0; i < filtered.size(); ++i) {
        for (size_t j = i + 1; j < filtered.size(); ++j) {
            const auto* a = filtered[i];
            const auto* b = filtered[j];
            if (!sharesAutoConnection(*a, *b)) continue;
            ImVec2 sa = toScreen(a->id);
            ImVec2 sb = toScreen(b->id);
            draw->AddLine(sa, sb, IM_COL32(100, 130, 180, 140), 1.4f);
        }
    }

    const ImVec2 mouse = ImGui::GetIO().MousePos;
    state.hoveredId.clear();
    for (const auto* p : filtered) {
        if (!p) continue;
        const auto score = domain::computeScore(*p);
        const int quality = scoreByPerspective(score, perspective);
        ImVec2 center = toScreen(p->id);
        float radius = 22.f * state.scale;
        if (quality < state.criticalThreshold) radius *= 1.22f;
        radius = std::clamp(radius, 8.0f, 34.0f);

        const ImU32 fill = statusColor(p->status);
        const ImU32 border = (p->id == state.selectedId) ? IM_COL32(255, 255, 120, 255) : bandColorU32(quality);
        draw->AddCircleFilled(center, radius, fill, 28);
        draw->AddCircle(center, radius, border, 28, 2.f);
        draw->AddText(ImVec2(center.x - radius, center.y + radius + 3.f), IM_COL32(220, 220, 240, 220), p->id.c_str());

        const float dx = mouse.x - center.x;
        const float dy = mouse.y - center.y;
        if (dx * dx + dy * dy < radius * radius) {
            state.hoveredId = p->id;
        }
    }
    draw->PopClipRect();

    ImGui::SetCursorScreenPos(canvasPos);
    ImGui::InvisibleButton("##graph_canvas_btn", canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    const bool hovered = ImGui::IsItemHovered();
    const bool active = ImGui::IsItemActive();
    if (active && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && state.hoveredId.empty()) {
        const ImVec2 delta = ImGui::GetIO().MouseDelta;
        state.panX += delta.x;
        state.panY += delta.y;
    }
    if (hovered && ImGui::GetIO().MouseWheel != 0.f) {
        const float factor = ImGui::GetIO().MouseWheel > 0.f ? 1.1f : 0.9f;
        state.scale = std::clamp(state.scale * factor, 0.20f, 3.5f);
    }
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        state.selectedId = state.hoveredId;
    }

    if (!state.hoveredId.empty()) {
        for (const auto* p : filtered) {
            if (!p || p->id != state.hoveredId) continue;
            const auto score = domain::computeScore(*p);
            ImGui::BeginTooltip();
            ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.f, 1.f), "%s", p->title.c_str());
            ImGui::TextDisabled("Fluxo: %s", domain::toString(p->status).c_str());
            ImGui::Separator();
            ImGui::Text("Qualidade(%s): %d", perspectiveShortLabel(perspective), scoreByPerspective(score, perspective));
            ImGui::Text("Total: %d | Inst: %d | Pesq: %d", score.total, score.institutional, score.researcher);
            ImGui::Text("Oper: %d | Matur: %d | Exec: %d", score.operational, score.maturity, score.execution);
            ImGui::Text("Conf: %s", score.reliabilityApplicable ? std::to_string(score.reliability).c_str() : "N/A");
            ImGui::EndTooltip();
            break;
        }
    }

    ImGui::EndChild();

    if (!state.selectedId.empty()) {
        const domain::ResearchProject* selected = nullptr;
        for (const auto* p : filtered) {
            if (p && p->id == state.selectedId) {
                selected = p;
                break;
            }
        }
        if (!selected) {
            state.selectedId.clear();
        } else {
            const auto score = domain::computeScore(*selected);
            ImGui::SameLine();
            ImGui::BeginChild("##graph_detail", ImVec2(detailWidth, 0.f), true);
            if (ImGui::SmallButton("X")) state.selectedId.clear();
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.55f, 0.75f, 1.0f, 1.0f), "%s", selected->id.c_str());
            ImGui::TextWrapped("%s", selected->title.c_str());
            ImGui::Separator();
            ImGui::Text("Fluxo: %s", domain::toString(selected->status).c_str());
            ImGui::Text("Qualidade (%s): %d (%s)", perspectiveShortLabel(perspective), scoreByPerspective(score, perspective), bandColorLabel(scoreByPerspective(score, perspective)));
            ImGui::Text("Total: %d", score.total);
            ImGui::Text("Institucional: %d", score.institutional);
            ImGui::Text("Pesquisador: %d", score.researcher);
            ImGui::Text("Operacional: %d", score.operational);
            ImGui::Text("Maturidade: %d", score.maturity);
            ImGui::Text("Execucao: %d", score.execution);
            ImGui::Text("Confiabilidade: %s", score.reliabilityApplicable ? std::to_string(score.reliability).c_str() : "N/A");
            ImGui::EndChild();
        }
    }
}

void renderHelpModal(bool* showHelpModal) {
    if (!showHelpModal) return;
    if (*showHelpModal) {
        ImGui::OpenPopup("help_modal");
    }

    ImGui::SetNextWindowSize(ImVec2(860, 620), ImGuiCond_FirstUseEver);
    if (ImGui::BeginPopupModal("help_modal", nullptr, ImGuiWindowFlags_NoResize)) {
        ImGui::TextColored(ImVec4(0.55f, 0.75f, 1.0f, 1.0f), "LabGP - Manual Rapido");
        ImGui::Separator();
        ImGui::TextWrapped("Guia pratico para interpretar o sistema sem confundir Fluxo do projeto com Score de Qualidade.");
        ImGui::Spacing();

        if (ImGui::BeginTabBar("help_tabs")) {
            if (ImGui::BeginTabItem("Visao Geral")) {
                ImGui::BulletText("Fluxo do Projeto = etapa atual (Proposta, Em avaliacao, Aprovado, Execucao, Analise, Publicacao, Encerrado).");
                ImGui::BulletText("Score de Qualidade = robustez do projeto (Total, Inst, Pesq, Oper, Matur, Exec, Conf).");
                ImGui::BulletText("Um projeto pode estar Aprovado no fluxo e ainda com qualidade baixa.");
                ImGui::Spacing();
                ImGui::TextDisabled("Atalho mental:");
                ImGui::BulletText("Fluxo responde: 'Em que fase estamos?'");
                ImGui::BulletText("Qualidade responde: 'Quao robusto estamos nesta fase?'");
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Abas")) {
                ImGui::TextUnformatted("Projetos");
                ImGui::BulletText("Tabela consolidada por projeto.");
                ImGui::BulletText("Mostra Fluxo (Status) e Scores de Qualidade.");
                ImGui::Separator();
                ImGui::TextUnformatted("Kanban");
                ImGui::BulletText("Organizacao por Fluxo do Projeto.");
                ImGui::BulletText("Cards exibem score para leitura de risco.");
                ImGui::Separator();
                ImGui::TextUnformatted("Grafo");
                ImGui::BulletText("Mapa visual de qualidade e relacoes.");
                ImGui::BulletText("Scroll=zoom, arrastar fundo=pan, clique=detalhes.");
                ImGui::Separator();
                ImGui::TextUnformatted("Inventario");
                ImGui::BulletText("Sinais detectados por repositorio/dossie.");
                ImGui::BulletText("Fluxo (Status inferido) e colunas de qualidade.");
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Scores")) {
                ImGui::BulletText("Total: visao consolidada.");
                ImGui::BulletText("Inst: governanca/compliance/controle.");
                ImGui::BulletText("Pesq: execucao cientifica/equipe.");
                ImGui::BulletText("Oper: base operacional.");
                ImGui::BulletText("Matur: ADR/DDD/DAI/governanca.");
                ImGui::BulletText("Exec: plano, cronograma, validacao e entregas.");
                ImGui::BulletText("Conf: confiabilidade tecnica (N/A quando nao aplicavel).");
                ImGui::Spacing();
                ImGui::TextDisabled("Faixas de leitura:");
                ImGui::BulletText(">= 80: Verde (robusto)");
                ImGui::BulletText(">= 60: Amarelo (atencao)");
                ImGui::BulletText(">= 40: Laranja (risco)");
                ImGui::BulletText("< 40: Vermelho (critico)");
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Inventario")) {
                ImGui::BulletText("Origem: Git ou Dossie.");
                ImGui::BulletText("Inov: sinais de contribuicao para inovacao.");
                ImGui::BulletText("Ativ: sinais de atividades de pesquisa.");
                ImGui::BulletText("ResPrev: sinais de resultados previstos.");
                ImGui::BulletText("Fluxo (Status inferido): etapa inferida automaticamente.");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::Separator();
        ImGui::TextDisabled("Documentacao tecnica: docs/architecture/UI_PARAMETERIZATION.md");
        if (ImGui::Button("Fechar", ImVec2(100.f, 0.f))) {
            *showHelpModal = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
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
    static ScorePerspective scorePerspective = ScorePerspective::Consolidated;
    bool requestOpenWorkspaceModal = false;
    bool requestOpenHelpModal = false;

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("LabGP", nullptr, flags | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Arquivo")) {
            if (ImGui::MenuItem("Selecionar pasta de projetos...")) {
                requestOpenWorkspaceModal = true;
            }
            if (ImGui::MenuItem("Reescanear agora")) {
                if (requestRescan) {
                    *requestRescan = true;
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Ajuda")) {
            if (ImGui::MenuItem("Manual rapido")) {
                requestOpenHelpModal = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::TextUnformatted("LabGP - Gestao de Projetos de Pesquisa");
    ImGui::Text("Workspace: %s", workspaceRoot.c_str());
    if (ImGui::Button("Selecionar Pasta")) {
        requestOpenWorkspaceModal = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reescanear")) {
        if (requestRescan) {
            *requestRescan = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Ajuda")) {
        requestOpenHelpModal = true;
    }
    if (!workspaceFeedback.empty()) {
        ImGui::TextColored(ImVec4(0.55f, 0.82f, 0.55f, 1.0f), "%s", workspaceFeedback.c_str());
    }
    ImGui::Separator();
    renderPerspectiveSelector(&scorePerspective);
    ImGui::Separator();

    if (requestOpenWorkspaceModal) {
        m_showWorkspaceModal = true;
        std::snprintf(m_workspacePathBuf.data(), m_workspacePathBuf.size(), "%s", workspaceRoot.c_str());
        m_workspaceNavPath = workspaceRoot;
        ImGui::OpenPopup("workspace_modal");
    }
    if (requestOpenHelpModal) {
        m_showHelpModal = true;
    }

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

    renderHelpModal(&m_showHelpModal);

    if (ImGui::BeginTabBar("labgp_tabs")) {
        if (ImGui::BeginTabItem("Projetos")) {
            renderProjectsTab(projects, inventory, scorePerspective);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Kanban")) {
            renderKanbanTab(projects, scorePerspective);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Grafo")) {
            renderGraphTab(projects, scorePerspective);
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
