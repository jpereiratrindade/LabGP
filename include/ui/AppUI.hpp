#pragma once

#include <string>

#include "domain/ResearchProjectStore.hpp"
#include "ui/KanbanView.hpp"
#include "ui/ListView.hpp"

namespace labgp::ui {

class AppUI {
public:
    explicit AppUI(const domain::ResearchProjectStore& store);
    std::string render() const;

private:
    const domain::ResearchProjectStore& store_;
    ListView listView_;
    KanbanView kanbanView_;
};

} // namespace labgp::ui
