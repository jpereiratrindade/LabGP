#pragma once

#include "domain/ResearchProject.hpp"

namespace labgp::domain {

struct ScoreBreakdown {
    int operational{0};
    int maturity{0};
    int reliability{0};
    int execution{0};
    int institutional{0};
    int researcher{0};
    int total{0};
    bool reliabilityApplicable{true};
};

ScoreBreakdown computeScore(const ResearchProject& project);

} // namespace labgp::domain
