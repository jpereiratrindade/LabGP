#include "domain/InventoryScanner.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <string>
#include <unordered_set>

#include "domain/ResearchProject.hpp"

namespace labgp::domain {

namespace fs = std::filesystem;

namespace {

bool hasAnyFile(const fs::path& root, std::initializer_list<const char*> names) {
    for (const auto* name : names) {
        if (fs::exists(root / name)) {
            return true;
        }
    }
    return false;
}

std::string toLowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string shellEscapeSingleQuoted(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    out.push_back('\'');
    for (char c : s) {
        if (c == '\'') out += "'\\''";
        else out.push_back(c);
    }
    out.push_back('\'');
    return out;
}

void replaceAll(std::string* value, const std::string& from, const std::string& to) {
    if (!value || from.empty()) return;
    std::size_t pos = 0;
    while ((pos = value->find(from, pos)) != std::string::npos) {
        value->replace(pos, from.size(), to);
        pos += to.size();
    }
}

std::string normalizeSearchText(std::string value) {
    replaceAll(&value, "\xC3\xA1", "a"); // á
    replaceAll(&value, "\xC3\xA0", "a"); // à
    replaceAll(&value, "\xC3\xA3", "a"); // ã
    replaceAll(&value, "\xC3\xA2", "a"); // â
    replaceAll(&value, "\xC3\xA4", "a"); // ä
    replaceAll(&value, "\xC3\x81", "a"); // Á
    replaceAll(&value, "\xC3\x80", "a"); // À
    replaceAll(&value, "\xC3\x83", "a"); // Ã
    replaceAll(&value, "\xC3\x82", "a"); // Â
    replaceAll(&value, "\xC3\x84", "a"); // Ä
    replaceAll(&value, "\xC3\xA9", "e"); // é
    replaceAll(&value, "\xC3\xAA", "e"); // ê
    replaceAll(&value, "\xC3\xA8", "e"); // è
    replaceAll(&value, "\xC3\xAB", "e"); // ë
    replaceAll(&value, "\xC3\x89", "e"); // É
    replaceAll(&value, "\xC3\x8A", "e"); // Ê
    replaceAll(&value, "\xC3\x88", "e"); // È
    replaceAll(&value, "\xC3\x8B", "e"); // Ë
    replaceAll(&value, "\xC3\xAD", "i"); // í
    replaceAll(&value, "\xC3\xAE", "i"); // î
    replaceAll(&value, "\xC3\xAC", "i"); // ì
    replaceAll(&value, "\xC3\xAF", "i"); // ï
    replaceAll(&value, "\xC3\x8D", "i"); // Í
    replaceAll(&value, "\xC3\x8E", "i"); // Î
    replaceAll(&value, "\xC3\x8C", "i"); // Ì
    replaceAll(&value, "\xC3\x8F", "i"); // Ï
    replaceAll(&value, "\xC3\xB3", "o"); // ó
    replaceAll(&value, "\xC3\xB4", "o"); // ô
    replaceAll(&value, "\xC3\xB5", "o"); // õ
    replaceAll(&value, "\xC3\xB2", "o"); // ò
    replaceAll(&value, "\xC3\xB6", "o"); // ö
    replaceAll(&value, "\xC3\x93", "o"); // Ó
    replaceAll(&value, "\xC3\x94", "o"); // Ô
    replaceAll(&value, "\xC3\x95", "o"); // Õ
    replaceAll(&value, "\xC3\x92", "o"); // Ò
    replaceAll(&value, "\xC3\x96", "o"); // Ö
    replaceAll(&value, "\xC3\xBA", "u"); // ú
    replaceAll(&value, "\xC3\xBB", "u"); // û
    replaceAll(&value, "\xC3\xB9", "u"); // ù
    replaceAll(&value, "\xC3\xBC", "u"); // ü
    replaceAll(&value, "\xC3\x9A", "u"); // Ú
    replaceAll(&value, "\xC3\x9B", "u"); // Û
    replaceAll(&value, "\xC3\x99", "u"); // Ù
    replaceAll(&value, "\xC3\x9C", "u"); // Ü
    replaceAll(&value, "\xC3\xA7", "c"); // ç
    replaceAll(&value, "\xC3\x87", "c"); // Ç
    return toLowerAscii(std::move(value));
}

bool commandExists(const char* command) {
    std::string cmd = "command -v ";
    cmd += command;
    cmd += " >/dev/null 2>&1";
    return std::system(cmd.c_str()) == 0;
}

bool runCommandCollectText(const std::string& command, std::string* out) {
    if (!out) return false;
    out->clear();
    std::array<char, 1024> buffer{};
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return false;
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        out->append(buffer.data());
    }
    const int rc = pclose(pipe);
    return rc == 0;
}

int pdfRelevanceScoreFromName(const std::string& fileName) {
    const std::string n = normalizeSearchText(fileName);
    int score = 0;
    if (n.find("proposta") != std::string::npos) score += 70;
    if (n.find("submetido") != std::string::npos || n.find("submissao") != std::string::npos) score += 60;
    if (n.find("projeto") != std::string::npos) score += 30;
    if (n.find("resumo") != std::string::npos) score += 15;
    if (n.find("edital") != std::string::npos || n.find("chamada") != std::string::npos) score -= 35;
    if (n.find("orcamento") != std::string::npos) score -= 25;
    if (n.find("elaboracao") != std::string::npos || n.find("consulta") != std::string::npos) score -= 30;
    return score;
}

std::string classifyPdfCurationTag(const std::string& fileName) {
    const std::string n = normalizeSearchText(fileName);
    if (n.find("proposta") != std::string::npos ||
        n.find("projeto") != std::string::npos ||
        n.find("submetido") != std::string::npos ||
        n.find("submissao") != std::string::npos ||
        n.find("resumo") != std::string::npos ||
        n.find("dossie") != std::string::npos) {
        return "nucleo_projeto";
    }
    if (n.find("metod") != std::string::npos ||
        n.find("cronograma") != std::string::npos ||
        n.find("atividade") != std::string::npos ||
        n.find("resultado") != std::string::npos ||
        n.find("relatorio") != std::string::npos ||
        n.find("parecer") != std::string::npos) {
        return "evidencia_execucao";
    }
    if (n.find("edital") != std::string::npos ||
        n.find("chamada") != std::string::npos ||
        n.find("anexo") != std::string::npos ||
        n.find("orcamento") != std::string::npos ||
        n.find("termo") != std::string::npos ||
        n.find("contrato") != std::string::npos) {
        return "suporte_admin";
    }
    return "complementar";
}

std::string trimCopy(std::string value) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(line);
    }
    return lines;
}

std::string joinAndClamp(const std::vector<std::string>& lines, std::size_t maxChars = 650) {
    std::string out;
    for (const auto& raw : lines) {
        std::string line = trimCopy(raw);
        if (line.empty()) continue;
        if (!out.empty()) out += " ";
        out += line;
        if (out.size() >= maxChars) break;
    }
    if (out.size() > maxChars) {
        out.resize(maxChars);
        out += "...";
    }
    return out;
}

std::vector<std::string> listDocFileNames(const fs::path& root) {
    std::vector<std::string> names;
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(root, fs::directory_options::skip_permission_denied, ec)) {
        if (!entry.is_regular_file(ec)) continue;
        const std::string ext = toLowerAscii(entry.path().extension().string());
        if (ext == ".pdf") {
            names.push_back(toLowerAscii(entry.path().filename().string()));
        }
    }
    return names;
}

std::string extractPdfText(const fs::path& pdfPath) {
    if (!commandExists("pdftotext")) return {};
    std::string text;
    const std::string cmd =
        "pdftotext -layout -enc UTF-8 " + shellEscapeSingleQuoted(pdfPath.string()) + " - 2>/dev/null";
    if (!runCommandCollectText(cmd, &text)) return {};
    return text;
}

std::string sha256OfFile(const fs::path& filePath) {
    if (commandExists("sha256sum")) {
        std::string out;
        const std::string cmd = "sha256sum " + shellEscapeSingleQuoted(filePath.string()) + " 2>/dev/null";
        if (runCommandCollectText(cmd, &out)) {
            std::istringstream in(out);
            std::string hash;
            in >> hash;
            if (!hash.empty()) return hash;
        }
    }
    if (commandExists("shasum")) {
        std::string out;
        const std::string cmd = "shasum -a 256 " + shellEscapeSingleQuoted(filePath.string()) + " 2>/dev/null";
        if (runCommandCollectText(cmd, &out)) {
            std::istringstream in(out);
            std::string hash;
            in >> hash;
            if (!hash.empty()) return hash;
        }
    }
    return {};
}

struct PdfTextResult {
    std::string text;
    InterpretedDocument doc;
};

PdfTextResult readCachedPdfTextOrExtract(const fs::path& root, const fs::path& pdfPath) {
    PdfTextResult result;
    result.doc.fileName = pdfPath.filename().string();
    result.doc.filePath = pdfPath.string();
    result.doc.relevanceScore = pdfRelevanceScoreFromName(result.doc.fileName);
    result.doc.curationTag = classifyPdfCurationTag(result.doc.fileName);
    const std::string hash = sha256OfFile(pdfPath);
    result.doc.sha256 = hash;
    if (hash.empty()) {
        result.text = extractPdfText(pdfPath);
        result.doc.usedCache = false;
        result.doc.cachePath = "";
        result.doc.textBytes = static_cast<int>(result.text.size());
        return result;
    }

    const fs::path cacheDir = root / ".labgp_cache" / "pdf_text";
    std::error_code ec;
    fs::create_directories(cacheDir, ec);
    const fs::path cachedTextPath = cacheDir / (hash + ".txt");
    result.doc.cachePath = cachedTextPath.string();
    if (fs::exists(cachedTextPath, ec) && fs::is_regular_file(cachedTextPath, ec)) {
        std::ifstream in(cachedTextPath);
        if (in.is_open()) {
            std::ostringstream ss;
            ss << in.rdbuf();
            result.text = ss.str();
            result.doc.usedCache = true;
            result.doc.textBytes = static_cast<int>(result.text.size());
            return result;
        }
    }

    result.text = extractPdfText(pdfPath);
    if (!result.text.empty()) {
        std::ofstream out(cachedTextPath);
        if (out.is_open()) out << result.text;
    }
    result.doc.usedCache = false;
    result.doc.textBytes = static_cast<int>(result.text.size());
    return result;
}

std::string loadDossierTextCorpusRaw(const fs::path& root, std::vector<InterpretedDocument>* interpretedDocs) {
    if (interpretedDocs) interpretedDocs->clear();
    std::ostringstream corpus;
    const fs::path cacheDir = root / ".labgp_cache" / "pdf_text";

    std::vector<fs::path> pdfFiles;
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(root, fs::directory_options::skip_permission_denied, ec)) {
        if (!entry.is_regular_file(ec)) continue;
        const std::string ext = toLowerAscii(entry.path().extension().string());
        if (ext != ".pdf") continue;
        pdfFiles.push_back(entry.path());
    }

    std::sort(pdfFiles.begin(), pdfFiles.end(), [](const fs::path& a, const fs::path& b) {
        const int sa = pdfRelevanceScoreFromName(a.filename().string());
        const int sb = pdfRelevanceScoreFromName(b.filename().string());
        if (sa != sb) return sa > sb;
        return a.filename().string() < b.filename().string();
    });

    int includedCount = 0;
    for (const auto& pdfPath : pdfFiles) {
        PdfTextResult textResult = readCachedPdfTextOrExtract(root, pdfPath);
        const bool preferByTag = (textResult.doc.curationTag == "nucleo_projeto") ||
                                 (textResult.doc.curationTag == "evidencia_execucao");
        const bool includeThis = preferByTag || (textResult.doc.relevanceScore >= 20) || (includedCount < 2);
        textResult.doc.includedInCorpus = includeThis;
        if (interpretedDocs) interpretedDocs->push_back(textResult.doc);
        if (!includeThis || textResult.text.empty()) continue;
        corpus << textResult.text << "\n";
        ++includedCount;
    }

    if (interpretedDocs) {
        fs::create_directories(cacheDir, ec);
        const fs::path manifestPath = cacheDir / "manifest.tsv";
        std::ofstream mf(manifestPath);
        if (mf.is_open()) {
            mf << "file_name\tfile_path\tcuration_tag\trelevance_score\tincluded_in_corpus\tsha256\tcache_path\tused_cache\ttext_bytes\n";
            for (const auto& d : *interpretedDocs) {
                mf << d.fileName << '\t'
                   << d.filePath << '\t'
                   << d.curationTag << '\t'
                   << d.relevanceScore << '\t'
                   << (d.includedInCorpus ? "1" : "0") << '\t'
                   << d.sha256 << '\t'
                   << d.cachePath << '\t'
                   << (d.usedCache ? "1" : "0") << '\t'
                   << d.textBytes << '\n';
            }
        }
    }
    return corpus.str();
}

bool hasDossierSignals(const fs::path& root) {
    return !listDocFileNames(root).empty();
}

bool containsToken(const std::vector<std::string>& names, const std::string& token) {
    for (const auto& n : names) {
        if (n.find(token) != std::string::npos) return true;
    }
    return false;
}

bool containsToken(const std::string& text, const std::string& token) {
    return text.find(token) != std::string::npos;
}

bool containsAnyToken(
    const std::vector<std::string>& names,
    const std::string& corpus,
    std::initializer_list<const char*> tokens
) {
    for (const char* token : tokens) {
        const std::string t = normalizeSearchText(token);
        if (containsToken(names, t) || containsToken(corpus, t)) return true;
    }
    return false;
}

bool lineHasKeyword(const std::string& lowerLine, const std::vector<std::string>& keywords) {
    for (const auto& k : keywords) {
        if (lowerLine.find(normalizeSearchText(k)) != std::string::npos) return true;
    }
    return false;
}

bool looksLikeAnySectionHeader(const std::string& lowerLine) {
    static const std::vector<std::string> headers = {
        "resumo",
        "objetivos",
        "contribuicoes para inovacao",
        "contribuicoes para inovacao",
        "atividades de pesquisa",
        "metodologia",
        "resultados esperados",
        "resultados",
        "equipe",
        "orcamento",
        "item do dispendio",
        "cronograma",
    };
    std::string line = trimCopy(lowerLine);
    if (line.empty()) return false;
    if (line.size() > 120) return false;
    for (const auto& h : headers) {
        const std::string hn = normalizeSearchText(h);
        if (line == hn) return true;
        if (line.rfind(hn + ":", 0) == 0) return true;
        if (line.rfind("# " + hn, 0) == 0) return true;
    }
    return false;
}

std::string extractSectionByKeywords(const std::string& rawText, const std::vector<std::string>& keywords) {
    const auto lines = splitLines(rawText);
    const int n = static_cast<int>(lines.size());
    for (int i = 0; i < n; ++i) {
        const std::string line = trimCopy(lines[i]);
        if (line.empty()) continue;
        const std::string lower = normalizeSearchText(line);
        const bool isHeading = (!line.empty() && line[0] == '#') && lineHasKeyword(lower, keywords);
        const bool isLabeled = (lower.find(':') != std::string::npos) && lineHasKeyword(lower, keywords);
        if (!isHeading && !isLabeled) continue;

        std::vector<std::string> out;
        if (isLabeled) {
            const std::size_t pos = line.find(':');
            if (pos != std::string::npos && pos + 1 < line.size()) {
                const std::string tail = trimCopy(line.substr(pos + 1));
                if (!tail.empty()) out.push_back(tail);
            }
        }
        for (int j = i + 1; j < n; ++j) {
            std::string next = trimCopy(lines[j]);
            if (next.empty()) {
                if (!out.empty()) break;
                continue;
            }
            if (!next.empty() && next[0] == '#') break;
            if (looksLikeAnySectionHeader(normalizeSearchText(next))) break;
            if (next.size() > 2 && next[0] == '-' && next[1] == ' ') next = trimCopy(next.substr(2));
            out.push_back(next);
            if (out.size() >= 8) break;
        }
        const std::string merged = joinAndClamp(out);
        if (!merged.empty()) return merged;
    }
    return {};
}

std::vector<std::string> extractTeamMembers(const std::string& rawText) {
    const auto lines = splitLines(rawText);
    const int n = static_cast<int>(lines.size());
    const std::vector<std::string> keys = {
        "equipe", "equipe executora", "pesquisadores", "coordenador", "colaboradores", "time"
    };
    std::vector<std::string> members;
    std::unordered_set<std::string> seen;
    auto seemsAdministrativeLine = [](const std::string& lower) {
        return lower.find("orcamento") != std::string::npos ||
               lower.find("dispendio") != std::string::npos ||
               lower.find("item") != std::string::npos ||
               lower.find("total") != std::string::npos ||
               lower.find("ano previsto") != std::string::npos ||
               lower.find("status do resultado") != std::string::npos ||
               lower.find("trl") != std::string::npos ||
               lower.find("r$") != std::string::npos ||
               lower.find("%") != std::string::npos;
    };
    auto hasTooManyDigits = [](const std::string& s) {
        int digits = 0;
        for (unsigned char c : s) {
            if (std::isdigit(c)) ++digits;
        }
        return digits >= 4;
    };

    for (int i = 0; i < n; ++i) {
        const std::string line = trimCopy(lines[i]);
        if (line.empty()) continue;
        const std::string lower = normalizeSearchText(line);
        bool startsSection = ((!line.empty() && line[0] == '#') || lower.find(':') != std::string::npos) &&
                             lineHasKeyword(lower, keys);
        if (!startsSection) continue;

        for (int j = i + 1; j < n; ++j) {
            std::string next = trimCopy(lines[j]);
            if (next.empty()) {
                if (!members.empty()) break;
                continue;
            }
            if (!next.empty() && next[0] == '#') break;
            if (looksLikeAnySectionHeader(normalizeSearchText(next))) break;
            if (next.size() > 2 && next[0] == '-' && next[1] == ' ') next = trimCopy(next.substr(2));
            if (next.empty()) continue;
            const std::string normalized = normalizeSearchText(next);
            if (seemsAdministrativeLine(normalized)) continue;
            if (hasTooManyDigits(next)) continue;
            if (seen.insert(normalized).second) {
                members.push_back(next);
            }
            if (members.size() >= 12) break;
        }
        if (!members.empty()) break;
    }
    return members;
}

int countGroups(const std::vector<std::vector<std::string>>& groups, const std::vector<std::string>& names, const std::string& corpus) {
    int total = 0;
    for (const auto& group : groups) {
        bool hit = false;
        for (const auto& token : group) {
            if (containsToken(names, token) || containsToken(corpus, token)) {
                hit = true;
                break;
            }
        }
        if (hit) ++total;
    }
    return total;
}

ResearchStatus inferStatusFromSignals(const std::vector<std::string>& names, const std::string& corpus) {
    auto hasAny = [&](std::initializer_list<const char*> tokens) {
        return containsAnyToken(names, corpus, tokens);
    };

    if (hasAny({"termo_de_encerramento", "projeto_encerrado", "prestacao_final_aprovada", "status: encerrado"})) {
        return ResearchStatus::Closed;
    }
    if (hasAny({"aprovado", "homologado", "deferido", "projeto_aprovado"})) return ResearchStatus::Approved;
    if (hasAny({"submetido", "submissao", "proposta_enviada", "em_avaliacao", "edital"})) return ResearchStatus::InReview;
    if (hasAny({"execucao", "andamento", "em_execucao", "workplan"})) return ResearchStatus::Execution;
    if (hasAny({"analise", "avaliacao_tecnica", "parecer", "em_analise"})) return ResearchStatus::Analysis;
    if (hasAny({"artigo_publicado", "publicacao_aceita", "paper_aceito", "patente_concedida"})) {
        return ResearchStatus::Publication;
    }
    return ResearchStatus::Proposal;
}

bool hasCi(const fs::path& root) {
    return fs::exists(root / ".github" / "workflows") ||
           fs::exists(root / ".gitlab-ci.yml") ||
           fs::exists(root / "azure-pipelines.yml");
}

bool hasTests(const fs::path& root) {
    return fs::exists(root / "tests") || hasAnyFile(root, {"CTestTestfile.cmake"});
}

bool hasAdr(const fs::path& root) {
    return fs::exists(root / "docs" / "adr") || fs::exists(root / "adr");
}

bool hasDdd(const fs::path& root) {
    return fs::exists(root / "docs" / "architecture") || fs::exists(root / "architecture");
}

bool hasDai(const fs::path& root) {
    return fs::exists(root / "docs" / "dai") || fs::exists(root / "dai");
}

bool hasGovernance(const fs::path& root) {
    return hasAnyFile(root, {"CONTRIBUTING.md", "CODEOWNERS", "GOVERNANCE.md", "SECURITY.md"});
}

bool hasSanitizersConfig(const fs::path& root) {
    return hasAnyFile(root, {".sanitizers", "asan.options", "ubsan.options"});
}

bool hasStaticAnalysisConfig(const fs::path& root) {
    return hasAnyFile(root, {".clang-tidy", ".clang-format", "cppcheck.suppress"});
}

bool hasStrictWarningsConfig(const fs::path& root) {
    // Heuristica fase 1: se existir CMake, assume possibilidade de configurar warnings estritos.
    return fs::exists(root / "CMakeLists.txt");
}

bool hasCycleGuardConfig(const fs::path& root) {
    return hasAnyFile(root, {"depgraph.yml", "dependency-cruiser.js"});
}

bool hasComplexityGuardConfig(const fs::path& root) {
    return hasAnyFile(root, {"lizard.cfg", ".lizard", "oclint.json"});
}

bool hasWorkPlanDoc(const fs::path& root) {
    return hasAnyFile(root, {"ROADMAP.md", "docs/roadmap.md", "docs/plan.md", "PROJECT_PLAN.md"});
}

bool hasBudgetDoc(const fs::path& root) {
    return hasAnyFile(root, {"BUDGET.md", "docs/budget.md", "funding.md", "docs/funding.md"});
}

} // namespace

std::vector<InventoryEntry> InventoryScanner::scan(const std::string& workspaceRoot) const {
    std::vector<InventoryEntry> entries;

    const fs::path root(workspaceRoot);
    if (!fs::exists(root) || !fs::is_directory(root)) {
        return entries;
    }

    for (const auto& dirEntry : fs::directory_iterator(root)) {
        if (!dirEntry.is_directory()) {
            continue;
        }

        const fs::path repoPath = dirEntry.path();
        const bool isGitRepo = fs::exists(repoPath / ".git");
        const bool isDossier = hasDossierSignals(repoPath);
        if (!isGitRepo && !isDossier) continue;

        std::vector<std::string> dossierNames;
        std::string dossierRawCorpus;
        std::string dossierSearchCorpus;
        std::vector<InterpretedDocument> dossierInterpretedDocs;
        if (isDossier) {
            dossierNames = listDocFileNames(repoPath);
            dossierRawCorpus = loadDossierTextCorpusRaw(repoPath, &dossierInterpretedDocs);
            dossierSearchCorpus = normalizeSearchText(dossierRawCorpus);
        }

        ResearchProject probe;
        probe.id = repoPath.filename().string();
        probe.title = probe.id;
        if (isGitRepo) {
            probe.hasReadme = hasAnyFile(repoPath, {"README.md", "README.txt", "README"});
            probe.hasCi = hasCi(repoPath);
            probe.hasTests = hasTests(repoPath);
            probe.softwareIntensive = true;
            probe.hasAdr = hasAdr(repoPath);
            probe.hasDdd = hasDdd(repoPath);
            probe.hasDai = hasDai(repoPath);
            probe.governanceItems = hasGovernance(repoPath) ? 1 : 0;
            probe.hasMethodology = probe.hasAdr || probe.hasDdd;
            probe.hasWorkPlan = hasWorkPlanDoc(repoPath) || probe.hasDai;
            probe.hasTimeline = probe.hasWorkPlan;
            probe.hasBudgetPlan = hasBudgetDoc(repoPath);
            probe.plannedDeliverables = probe.hasWorkPlan ? 4 : 0;
            probe.deliveredDeliverables = probe.hasReadme ? 1 : 0;
            probe.reviewMeetings = probe.governanceItems;

            probe.hasAsanUbsan = hasSanitizersConfig(repoPath);
            probe.hasLeakChecks = probe.hasAsanUbsan;
            probe.hasStaticAnalysis = hasStaticAnalysisConfig(repoPath);
            probe.hasStrictWarnings = hasStrictWarningsConfig(repoPath);
            probe.hasComplexityGuard = hasComplexityGuardConfig(repoPath);
            probe.hasCycleGuard = hasCycleGuardConfig(repoPath);
            probe.hasFormatLint = fs::exists(repoPath / ".clang-format");
        } else {
            const auto& names = dossierNames;
            const std::string& corpus = dossierSearchCorpus;
            const bool hasCorpusText = !trimCopy(corpus).empty();
            auto hasToken = [&](const std::string& token) {
                return containsAnyToken(names, corpus, {token.c_str()});
            };
            probe.softwareIntensive = false;
            probe.hasReadme = hasToken("readme") || hasToken("resumo") || (!names.empty() && !hasCorpusText);
            probe.hasMethodology = hasToken("metod") || (!hasCorpusText && (hasToken("proposta") || hasToken("projeto")));
            probe.hasWorkPlan = hasToken("plano de trabalho") || hasToken("plano") || (!hasCorpusText && hasToken("proposta"));
            probe.hasTimeline = hasToken("cronograma") || (!hasCorpusText && (hasToken("submetido") || hasToken("submissao")));
            probe.hasBudgetPlan = hasToken("orc") || hasToken("budget");
            probe.hasValidationPlan = hasToken("avali") || hasToken("valid");
            probe.hasDataGovernance = hasToken("dados") || hasToken("govern");
            probe.hasTerritorialNetwork = hasToken("territorial") || hasToken("sait");
            probe.hasPublicPolicyAlignment = hasToken("politica") || hasToken("publica");
            probe.plannedDeliverables = 0;
            probe.deliveredDeliverables = 0;
            probe.reviewMeetings = 0;
        }

        InventoryEntry inv;
        inv.repoName = probe.id;
        inv.repoPath = repoPath.string();
        inv.source = isGitRepo ? "Git" : "Dossie";
        if (isDossier) {
            const auto& names = dossierNames;
            const std::string& rawCorpus = dossierRawCorpus;
            const std::string& corpus = dossierSearchCorpus;
            const bool hasExtractedText = !trimCopy(rawCorpus).empty();
            inv.interpretedDocuments = std::move(dossierInterpretedDocs);
            inv.inferredStatus = inferStatusFromSignals(names, corpus);

            inv.summary = extractSectionByKeywords(rawCorpus, {"resumo", "summary"});
            inv.objectives = extractSectionByKeywords(rawCorpus, {"objetivos", "objetivo geral", "objetivo"});
            inv.innovationContributions = extractSectionByKeywords(rawCorpus, {"inovacao", "contribuicoes para inovacao", "contribuicao"});
            inv.researchActivities = extractSectionByKeywords(rawCorpus, {"atividades de pesquisa", "atividades", "metodologia", "plano de trabalho"});
            inv.expectedResults = extractSectionByKeywords(rawCorpus, {"resultados esperados", "resultados", "entregaveis", "outcomes"});
            inv.teamMembers = extractTeamMembers(rawCorpus);
            if (!hasExtractedText && !inv.interpretedDocuments.empty()) {
                inv.summary = "Sem texto extraivel via pdftotext nos PDFs atuais. Curadoria mantida por nome/hash; revisar qualidade do PDF (OCR).";
            }
            inv.interpretedDocsTotal = static_cast<int>(inv.interpretedDocuments.size());
            inv.interpretedDocsIncluded = 0;
            for (const auto& d : inv.interpretedDocuments) {
                if (d.includedInCorpus) ++inv.interpretedDocsIncluded;
                if (d.curationTag == "nucleo_projeto") ++inv.curatedNucleoProjeto;
                else if (d.curationTag == "evidencia_execucao") ++inv.curatedEvidenciaExecucao;
                else if (d.curationTag == "suporte_admin") ++inv.curatedSuporteAdmin;
                else ++inv.curatedComplementar;
            }

            const std::vector<std::vector<std::string>> innovationGroups = {
                {"inov", "sociotecnica", "tecnolog"},
                {"protocolo", "metodo", "modelo"},
                {"dashboard", "georrefer", "indice"},
            };
            const std::vector<std::vector<std::string>> activityGroups = {
                {"levantamento", "coleta", "campo"},
                {"analise", "modelagem", "multicriterio"},
                {"oficina", "rede", "urt", "uac", "uo"},
            };
            const std::vector<std::vector<std::string>> resultGroups = {
                {"resultado", "entreg", "validado"},
                {"politica_publica", "subsidi", "impacto"},
                {"banco_de_dados", "publicacao", "software"},
            };
            inv.innovationSignals = countGroups(innovationGroups, names, corpus);
            inv.activitySignals = countGroups(activityGroups, names, corpus);
            inv.plannedResultsSignals = countGroups(resultGroups, names, corpus);

            probe.hasReadme = probe.hasReadme || !inv.summary.empty();
            probe.hasMethodology = probe.hasMethodology || !inv.researchActivities.empty();
            probe.hasWorkPlan = probe.hasWorkPlan || (inv.activitySignals > 0);
            probe.hasTimeline = probe.hasTimeline || containsAnyToken(names, corpus, {"cronograma", "trimestre", "prazo"});
            probe.hasValidationPlan = probe.hasValidationPlan || (inv.plannedResultsSignals > 0);
            probe.hasDataGovernance = probe.hasDataGovernance || containsAnyToken(names, corpus, {"base de dados", "governanca de dados", "ddc"});
            probe.hasPublicPolicyAlignment = probe.hasPublicPolicyAlignment || containsAnyToken(names, corpus, {"politica publica", "subsidiar", "instrumento"});

            const int planned = std::clamp(2 + inv.activitySignals + inv.plannedResultsSignals, 2, 8);
            int delivered = countGroups(
                {
                    {"entreg", "concluid", "finalizado"},
                    {"validado", "aprovado", "homologado"},
                    {"publicado", "divulgado", "implantado"},
                },
                names,
                corpus
            );
            if (inv.inferredStatus == ResearchStatus::Approved ||
                inv.inferredStatus == ResearchStatus::Execution ||
                inv.inferredStatus == ResearchStatus::Analysis ||
                inv.inferredStatus == ResearchStatus::Publication ||
                inv.inferredStatus == ResearchStatus::Closed) {
                delivered = std::max(delivered, 1);
            }
            if (inv.inferredStatus == ResearchStatus::Publication || inv.inferredStatus == ResearchStatus::Closed) {
                delivered = std::max(delivered, planned - 1);
            }
            probe.plannedDeliverables = planned;
            probe.deliveredDeliverables = std::clamp(delivered, 0, planned);
            probe.reviewMeetings = countGroups({{"reuniao", "oficina", "workshop", "monitoramento"}}, names, corpus);
        } else {
            inv.inferredStatus = ResearchStatus::Proposal;
        }
        inv.score = computeScore(probe);

        entries.push_back(std::move(inv));
    }

    std::sort(entries.begin(), entries.end(), [](const InventoryEntry& a, const InventoryEntry& b) {
        if (a.score.total != b.score.total) {
            return a.score.total > b.score.total;
        }
        return a.repoName < b.repoName;
    });

    return entries;
}

} // namespace labgp::domain
