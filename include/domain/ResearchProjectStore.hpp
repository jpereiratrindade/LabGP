#pragma once

#include <string>
#include <vector>

#include "domain/ResearchProject.hpp"

namespace labgp::domain {

class ResearchProjectStore {
public:
    void add(ResearchProject project);
    bool moveStatus(const std::string& id, ResearchStatus nextStatus);

    const std::vector<ResearchProject>& all() const;

private:
    std::vector<ResearchProject> projects_;
};

} // namespace labgp::domain
