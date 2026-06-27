#include "grammar.h"
#include <algorithm>
#include <iomanip>

Symbol EPSILON     = {"eps", true};
Symbol END_OF_INPUT = {"$",  true};

Symbol terminal(const std::string& name)    { return {name, true};  }
Symbol nonTerminal(const std::string& name) { return {name, false}; }

// Iterative fixed-point computation of FIRST sets for all grammar symbols
void Grammar::computeFirstSets() {

    // Terminals: FIRST(t) = {t}

    for (const auto& t : terminals) {
        firstSets[t] = {t};
    }
    firstSets[EPSILON]      = {EPSILON};
    firstSets[END_OF_INPUT] = {END_OF_INPUT};

    // Non-terminals start empty

    for (const auto& nt : nonTerminals) {
        firstSets[nt] = {};
    }

    bool changed = true;
    while (changed) {
        changed = false;

        for (const auto& prod : productions) {
            const Symbol& lhs = prod.lhs;

            // Epsilon production
            if (prod.rhs.empty() || prod.rhs[0] == EPSILON) {
                if (firstSets[lhs].insert(EPSILON).second) changed = true;
                continue;
            }

            // Walk rhs left to right; add non-epsilon FIRST entries
            bool allDeriveEpsilon = true;
            for (const auto& sym : prod.rhs) {
                for (const auto& f : firstSets[sym]) {
                    if (f == EPSILON) continue;
                    if (firstSets[lhs].insert(f).second) changed = true;
                }
                if (firstSets[sym].find(EPSILON) == firstSets[sym].end()) {
                    allDeriveEpsilon = false;
                    break;
                }
            }
            if (allDeriveEpsilon) {
                if (firstSets[lhs].insert(EPSILON).second) changed = true;
            }
        }
    }
}

// FIRST(seq + lookahead): used during LR(1) closure to compute new lookaheads
std::set<Symbol> Grammar::first(const std::vector<Symbol>& seq, const Symbol& lookahead) {
    std::set<Symbol> result;

    // Empty sequence: FIRST = {lookahead}
    if (seq.empty()) {
        result.insert(lookahead);
        return result;
    }

    bool allDeriveEpsilon = true;
    for (const auto& sym : seq) {
        for (const auto& f : firstSets[sym]) {
            if (f != EPSILON) result.insert(f);
        }
        if (firstSets[sym].find(EPSILON) == firstSets[sym].end()) {
            allDeriveEpsilon = false;
            break;
        }
    }

    if (allDeriveEpsilon) result.insert(lookahead);
    return result;
}

void Grammar::print(std::ostream& out) const {
    out << "\n=== Grammar ===\n";
    for (const auto& p : productions) {
        out << "  [" << p.id << "] " << p.lhs.name << " ->";
        for (const auto& s : p.rhs) out << " " << s.name;
        out << "\n";
    }
 
    out << "\nTerminals:";
    for (const auto& t : terminals) out << " " << t.name;
    out << "\n";
 
    out << "Non-terminals:";
    for (const auto& nt : nonTerminals) out << " " << nt.name;
    out << "\n";
 
    out << "\nFIRST sets:\n";
    for (const auto& kv : firstSets) {
        const auto& sym = kv.first;
        const auto& fset = kv.second;
        if (sym == EPSILON || sym == END_OF_INPUT) continue;
        out << "  FIRST(" << sym.name << ") = {";
        bool first = true;
        for (const auto& f : fset) {
            if (!first) out << ", ";
            first = false;
            out << f.name;
        }
        out << "}\n";
    }
}

