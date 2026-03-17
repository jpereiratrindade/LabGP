#pragma once

#include <string>
#include <vector>

#include "domain/InventoryScanner.hpp"
#include "domain/ResearchProject.hpp"

namespace labgp::ui {

class GuiDashboard {
public:
    void render(
        const std::vector<domain::ResearchProject>& projects,
        const std::vector<domain::InventoryEntry>& inventory,
        const std::string& workspaceRoot,
        bool* requestPickWorkspace
    ) const;
};

} // namespace labgp::ui
