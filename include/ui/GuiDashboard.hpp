#pragma once

#include <array>
#include <string>
#include <vector>

#include "domain/InventoryScanner.hpp"
#include "domain/ResearchProject.hpp"

namespace labgp::ui {

struct CreateProjectRequest {
    std::string sourceRepoPath;
    std::string projectId;
    std::string projectTitle;
    std::string coordinator;
    std::string institution;
};

struct UpdateProjectRequest {
    domain::ResearchProject project;
};

class GuiDashboard {
public:
    void render(
        const std::vector<domain::ResearchProject>& projects,
        const std::vector<domain::InventoryEntry>& inventory,
        const std::string& workspaceRoot,
        bool* requestRescan,
        bool* requestExportInventory,
        bool* requestSaveProjects,
        bool* requestExit,
        std::string* requestApplyWorkspacePath,
        std::string* requestProcessInnovationSourceRepoPath,
        CreateProjectRequest* requestCreateProject,
        UpdateProjectRequest* requestUpdateProject,
        const std::string& workspaceFeedback
    ) const;

private:
    mutable bool m_showWorkspaceModal{false};
    mutable bool m_showHelpModal{false};
    mutable bool m_showCreateProjectModal{false};
    mutable std::array<char, 1024> m_workspacePathBuf{};
    mutable std::array<char, 128> m_projectIdBuf{};
    mutable std::array<char, 256> m_projectTitleBuf{};
    mutable std::array<char, 256> m_projectCoordinatorBuf{};
    mutable std::array<char, 256> m_projectInstitutionBuf{};
    mutable std::string m_workspaceNavPath;
    mutable std::string m_createProjectSourceRepoPath;
};

} // namespace labgp::ui
