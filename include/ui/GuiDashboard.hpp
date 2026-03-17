#pragma once

#include <array>
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
        bool* requestRescan,
        std::string* requestApplyWorkspacePath,
        const std::string& workspaceFeedback
    ) const;

private:
    mutable bool m_showWorkspaceModal{false};
    mutable std::array<char, 1024> m_workspacePathBuf{};
    mutable std::string m_workspaceNavPath;
};

} // namespace labgp::ui
