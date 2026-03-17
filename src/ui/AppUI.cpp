#include "ui/AppUI.hpp"

#include <sstream>

namespace labgp::ui {

AppUI::AppUI(const domain::ResearchProjectStore& store)
    : store_(store) {}

std::string AppUI::render() const {
    std::ostringstream out;
    out << "LabGP - Gestao de Projetos de Pesquisa (Console)\n";
    out << "================================================\n\n";
    out << listView_.render(store_.all());
    out << kanbanView_.render(store_.all());
    return out.str();
}

} // namespace labgp::ui
