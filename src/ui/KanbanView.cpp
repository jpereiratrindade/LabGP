#include "ui/KanbanView.hpp"

#include <array>
#include <sstream>

#include "domain/Scoring.hpp"

namespace labgp::ui {

std::string KanbanView::render(const std::vector<domain::ResearchProject>& projects) const {
    const std::array<domain::ResearchStatus, 7> orderedStatus = {
        domain::ResearchStatus::Proposal,
        domain::ResearchStatus::InReview,
        domain::ResearchStatus::Approved,
        domain::ResearchStatus::Execution,
        domain::ResearchStatus::Analysis,
        domain::ResearchStatus::Publication,
        domain::ResearchStatus::Closed,
    };

    std::ostringstream out;
    out << "=== Visao 2: Kanban por Status ===\n";

    for (const auto status : orderedStatus) {
        out << "\n[" << domain::toString(status) << "]\n";
        int count = 0;
        for (const auto& project : projects) {
            if (project.status != status) {
                continue;
            }
            const auto score = domain::computeScore(project);
            out << " - " << project.id << " | " << project.title
                << " | Tipo: " << (project.softwareIntensive ? "Software" : "Pesquisa")
                << " | Total: " << score.total
                << " | Inst: " << score.institutional
                << " | Pesq: " << score.researcher
                << " | Exec: " << score.execution << "\n";
            ++count;
        }
        if (count == 0) {
            out << " - (vazio)\n";
        }
    }
    out << "\n";
    return out.str();
}

} // namespace labgp::ui
