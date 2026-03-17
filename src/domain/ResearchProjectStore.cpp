#include "domain/ResearchProjectStore.hpp"

#include <algorithm>

namespace labgp::domain {

namespace {

bool isAllowedTransition(ResearchStatus current, ResearchStatus next) {
    if (current == next) return true;
    switch (current) {
        case ResearchStatus::Proposal:
            return next == ResearchStatus::InReview;
        case ResearchStatus::InReview:
            return next == ResearchStatus::Proposal || next == ResearchStatus::Approved;
        case ResearchStatus::Approved:
            return next == ResearchStatus::Execution;
        case ResearchStatus::Execution:
            return next == ResearchStatus::Analysis || next == ResearchStatus::Closed;
        case ResearchStatus::Analysis:
            return next == ResearchStatus::Execution || next == ResearchStatus::Publication;
        case ResearchStatus::Publication:
            return next == ResearchStatus::Closed;
        case ResearchStatus::Closed:
            return false;
    }
    return false;
}

} // namespace

std::string toString(ResearchStatus status) {
    switch (status) {
        case ResearchStatus::Proposal: return "Proposta";
        case ResearchStatus::InReview: return "Em avaliacao";
        case ResearchStatus::Approved: return "Aprovado";
        case ResearchStatus::Execution: return "Execucao";
        case ResearchStatus::Analysis: return "Analise";
        case ResearchStatus::Publication: return "Publicacao";
        case ResearchStatus::Closed: return "Encerrado";
    }
    return "Desconhecido";
}

void ResearchProjectStore::add(ResearchProject project) {
    projects_.push_back(std::move(project));
}

bool ResearchProjectStore::moveStatus(const std::string& id, ResearchStatus nextStatus) {
    auto it = std::find_if(projects_.begin(), projects_.end(), [&](const ResearchProject& p) {
        return p.id == id;
    });
    if (it == projects_.end()) {
        return false;
    }
    if (!isAllowedTransition(it->status, nextStatus)) {
        return false;
    }
    it->status = nextStatus;
    return true;
}

const std::vector<ResearchProject>& ResearchProjectStore::all() const {
    return projects_;
}

} // namespace labgp::domain
