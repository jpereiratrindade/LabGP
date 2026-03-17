#pragma once

#include <string>
#include <vector>

#include "domain/ResearchProject.hpp"

namespace labgp::ui {

class KanbanView {
public:
    std::string render(const std::vector<domain::ResearchProject>& projects) const;
};

} // namespace labgp::ui
