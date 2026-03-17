#include "ui/ListView.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "domain/Scoring.hpp"

namespace labgp::ui {

std::string ListView::render(const std::vector<domain::ResearchProject>& projects) const {
    using ProjectWithScore = std::pair<const domain::ResearchProject*, domain::ScoreBreakdown>;

    std::vector<ProjectWithScore> rows;
    rows.reserve(projects.size());
    for (const auto& project : projects) {
        rows.emplace_back(&project, domain::computeScore(project));
    }

    std::sort(rows.begin(), rows.end(), [](const ProjectWithScore& lhs, const ProjectWithScore& rhs) {
        if (lhs.second.total != rhs.second.total) {
            return lhs.second.total > rhs.second.total;
        }
        return lhs.first->id < rhs.first->id;
    });

    std::ostringstream out;
    out << "=== Visao 1: Lista de Projetos ===\n";
    out << std::left << std::setw(10) << "ID"
        << std::setw(30) << "Titulo"
        << std::setw(14) << "Status"
        << std::setw(8) << "Tipo"
        << std::setw(8) << "Total"
        << std::setw(8) << "Inst"
        << std::setw(8) << "Pesq"
        << std::setw(8) << "Oper"
        << std::setw(8) << "Matur"
        << std::setw(8) << "Exec"
        << std::setw(8) << "Conf"
        << "\n";
    out << std::string(120, '-') << "\n";

    for (const auto& row : rows) {
        const auto& project = *row.first;
        const auto& score = row.second;
        out << std::left << std::setw(10) << project.id
            << std::setw(30) << project.title.substr(0, 29)
            << std::setw(14) << domain::toString(project.status)
            << std::setw(8) << (project.softwareIntensive ? "SW" : "Pesq")
            << std::setw(8) << score.total
            << std::setw(8) << score.institutional
            << std::setw(8) << score.researcher
            << std::setw(8) << score.operational
            << std::setw(8) << score.maturity
            << std::setw(8) << score.execution
            << std::setw(8) << (score.reliabilityApplicable ? std::to_string(score.reliability) : "N/A")
            << "\n";
    }
    out << "\n";
    return out.str();
}

} // namespace labgp::ui
