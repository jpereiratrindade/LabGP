#pragma once

#include "domain/ResearchProject.hpp"

namespace labgp::domain {

struct ScoreBreakdown {
    int operational{0};
    int maturity{0};
    int reliability{0};
    int total{0};
};

ScoreBreakdown computeScore(const ResearchProject& project);

} // namespace labgp::domain
