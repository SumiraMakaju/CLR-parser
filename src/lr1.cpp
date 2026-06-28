#include "lr1.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include "grammar.h"

std::string LR1Item::toString(const Grammar& g) const {
    const Production& prod = g.productions[static_cast<size_t>(productionId)];
    std::string s = prod.lhs.name + " ->";
    for (int i = 0; i <= static_cast<int>(prod.rhs.size()); i++) {
        if (i == dotPos) s += " .";
        if (i < static_cast<int>(prod.rhs.size()))
            s += " " + prod.rhs[static_cast<size_t>(i)].name;
    }
    s += ",  " + lookahead.name;
    return s;
}

LR1Builder::LR1Builder(Grammar& g) : grammar_(g) {}

ItemSet LR1Builder::closure(const ItemSet& items) {
    ItemSet result = items;
    bool changed = true;
    while (changed) {
        changed = false;
        ItemSet toAdd;

        for (const auto& item : result) {
            const Production& prod = grammar_.productions[static_cast<size_t>(item.productionId)];

            if (item.dotPos >= static_cast<int>(prod.rhs.size())) continue;

            const Symbol& B = prod.rhs[static_cast<size_t>(item.dotPos)];
            if (B.isTerminal) continue;
            std::vector<Symbol> beta(
                prod.rhs.begin() + item.dotPos + 1,
                prod.rhs.end()
            );
            for (const auto& p : grammar_.productions) {
                if (!(p.lhs == B)) continue;

                for (const auto& la : grammar_.first(beta, item.lookahead)) {
                    if (la == EPSILON) continue;
                    LR1Item newItem{p.id, 0, la};
                    if (result.find(newItem) == result.end() &&
                        toAdd.find(newItem)  == toAdd.end()) {
                        toAdd.insert(newItem);
                        changed = true;
                    }
                }
            }
        }
        result.insert(toAdd.begin(), toAdd.end());
    }

    return result;
}

ItemSet LR1Builder::gotoSet(const ItemSet& items, const Symbol& sym) {
    ItemSet kernel;
    for (const auto& item : items) {
        const Production& prod = grammar_.productions[static_cast<size_t>(item.productionId)];
        if (item.dotPos >= static_cast<int>(prod.rhs.size())) continue;
        if (prod.rhs[static_cast<size_t>(item.dotPos)] == sym) {
            kernel.insert({item.productionId, item.dotPos + 1, item.lookahead});
        }
    }
    if (kernel.empty()) return {};
    return closure(kernel);
}

int LR1Builder::findOrAddState(const ItemSet& items) {
    for (const auto& s : states_) {
        if (s.items == items) return s.id;
    }
    int id = static_cast<int>(states_.size());
    states_.push_back({id, items});
    return id;
}

void LR1Builder::build() {
    LR1Item start{0, 0, END_OF_INPUT};
    findOrAddState(closure({start}));

    for (int i = 0; i < static_cast<int>(states_.size()); i++) {
        // Collect every symbol that appears after a dot in this state
        std::set<Symbol> symsAfterDot;
        for (const auto& item : states_[static_cast<size_t>(i)].items) {
            const Production& prod = grammar_.productions[static_cast<size_t>(item.productionId)];
            if (item.dotPos < static_cast<int>(prod.rhs.size())) {
                symsAfterDot.insert(prod.rhs[static_cast<size_t>(item.dotPos)]);
            }
        }

        for (const auto& sym : symsAfterDot) {
            ItemSet next = gotoSet(states_[static_cast<size_t>(i)].items, sym);
            if (next.empty()) continue;
            int nextId = findOrAddState(next);
            transitions_[states_[static_cast<size_t>(i)].id][sym.name] = nextId;
        }
    }

    buildTable();
}

// Fill ACTION and GOTO parse table entries from the automaton
void LR1Builder::buildTable() {
    for (const auto& state : states_) {
        for (const auto& item : state.items) {
            const Production& prod = grammar_.productions[static_cast<size_t>(item.productionId)];

            if (item.dotPos < static_cast<int>(prod.rhs.size())) {
                const Symbol& next = prod.rhs[static_cast<size_t>(item.dotPos)];
                auto it = transitions_[state.id].find(next.name);
                if (it == transitions_[state.id].end()) continue;

                if (next.isTerminal) {
                    Action act{ActionType::SHIFT, it->second};
                    auto& cell = table_.action[state.id][next.name];
                    if (cell.type != ActionType::ERROR &&
                        !(cell.type == ActionType::SHIFT && cell.value == act.value)) {
                        table_.hasConflict = true;
                        table_.conflicts.push_back(
                            "Shift-reduce conflict: state " +
                            std::to_string(state.id) + " on '" + next.name + "'");
                    }
                    cell = act;
                } else {
                    table_.gotoTable[state.id][next.name] = it->second;
                }

            } else {
                if (prod.lhs == grammar_.augmentedStart && item.lookahead == END_OF_INPUT) {
                    table_.action[state.id]["$"] = {ActionType::ACCEPT, 0};
                } else {
                    Action act{ActionType::REDUCE, prod.id};
                    auto& cell = table_.action[state.id][item.lookahead.name];
                    if (cell.type != ActionType::ERROR) {
                        if (cell.type == ActionType::SHIFT) {
                            table_.hasConflict = true;
                            table_.conflicts.push_back(
                                "Shift-reduce conflict: state " +
                                std::to_string(state.id) + " on '" + item.lookahead.name + "'");
                        } else if (cell.type == ActionType::REDUCE && cell.value != act.value) {
                            table_.hasConflict = true;
                            table_.conflicts.push_back(
                                "Reduce-reduce conflict: state " +
                                std::to_string(state.id) + " on '" + item.lookahead.name + "'");
                        }
                    } else {
                        cell = act;
                    }
                }
            }
        }
    }
}

// Print all LR(1) states and their items
void LR1Builder::printStates(std::ostream& out) const {
    out << "\n=== LR(1) States (" << states_.size() << " total) ===\n";
    for (const auto& s : states_) {
        out << "\nState I" << s.id << ":\n";
        for (const auto& item : s.items) {
            out << "  [ " << item.toString(grammar_) << " ]\n";
        }
    }
}

// Print the ACTION/GOTO parse table
void LR1Builder::printTable(std::ostream& out) const {
    auto actionStr = [](const Action& a) -> std::string {
        switch (a.type) {
            case ActionType::SHIFT:  return "s" + std::to_string(a.value);
            case ActionType::REDUCE: return "r" + std::to_string(a.value);
            case ActionType::ACCEPT: return "acc";
            default:                 return ".";
        }
    };

    // Collect ordered terminal and non-terminal columns
    std::vector<std::string> terms, nts;
    for (const auto& t : grammar_.terminals) {
        if (t != END_OF_INPUT) terms.push_back(t.name);
    }
    terms.push_back("$");
    for (const auto& nt : grammar_.nonTerminals) {
        if (nt != grammar_.augmentedStart) nts.push_back(nt.name);
    }
    std::sort(terms.begin(), terms.end());
    std::sort(nts.begin(), nts.end());

    const int W = 7;
    out << "\n=== Parse Table ===\n";
    out << std::setw(6) << "State";
    out << " | ";
    for (const auto& t : terms) out << std::setw(W) << t;
    out << " | ";
    for (const auto& n : nts)  out << std::setw(W) << n;
    out << "\n" << std::string(6 + 3 + terms.size() * W + 3 + nts.size() * W, '-') << "\n";

    for (const auto& state : states_) {
        out << std::setw(6) << state.id << " | ";
        for (const auto& t : terms) {
            auto rowIt = table_.action.find(state.id);
            if (rowIt != table_.action.end()) {
                auto colIt = rowIt->second.find(t);
                if (colIt != rowIt->second.end()) {
                    out << std::setw(W) << actionStr(colIt->second);
                    continue;
                }
            }
            out << std::setw(W) << ".";
        }
        out << " | ";
        for (const auto& n : nts) {
            auto rowIt = table_.gotoTable.find(state.id);
            if (rowIt != table_.gotoTable.end()) {
                auto colIt = rowIt->second.find(n);
                if (colIt != rowIt->second.end()) {
                    out << std::setw(W) << colIt->second;
                    continue;
                }
            }
            out << std::setw(W) << ".";
        }
        out << "\n";
    }

    out << "\nConflicts: " << (table_.hasConflict ? "YES" : "NONE") << "\n";
    for (const auto& c : table_.conflicts) out << "  " << c << "\n";
}