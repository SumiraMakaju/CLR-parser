#pragma once
#include "grammar.h"
#include <vector>
#include <map>
#include <set>
#include <string>
#include <iostream>

struct LR1Item {
    int    productionId;
    int    dotPos;
    Symbol lookahead;

    bool operator==(const LR1Item& o) const {
        return productionId == o.productionId &&
               dotPos       == o.dotPos       &&
               lookahead    == o.lookahead;
    }
    bool operator<(const LR1Item& o) const {
        if (productionId != o.productionId) return productionId < o.productionId;
        if (dotPos       != o.dotPos)       return dotPos       < o.dotPos;
        return lookahead < o.lookahead;
    }

    std::string toString(const Grammar& g) const;
};

using ItemSet = std::set<LR1Item>;

// One state in the LR(1) DFA
struct State {
    int     id;
    ItemSet items;
    bool operator==(const State& o) const { return items == o.items; }
};

// Action types in the parse table
enum class ActionType { ERROR, SHIFT, REDUCE, ACCEPT };

struct Action {
    ActionType type  = ActionType::ERROR;
    int        value = 0; // next state for SHIFT, production id for REDUCE
};

// The complete LR(1) parse table
struct ParseTable {
    // action[stateId][terminal] = Action
    std::map<int, std::map<std::string, Action>> action;
    // gotoTable[stateId][nonTerminal] = nextStateId
    std::map<int, std::map<std::string, int>>    gotoTable;
    bool                     hasConflict = false;
    std::vector<std::string> conflicts;
};

// Builds the canonical LR(1) item collection and parse table
class LR1Builder {
public:
    explicit LR1Builder(Grammar& g);

    // Run full construction: builds states, transitions, and parse table
    void build();

    ParseTable&         getTable()      { return table_; }
    std::vector<State>& getStates()     { return states_; }
    const ParseTable&   getTable()const { return table_; }

    // transitions_[fromState][symbolName] = toState
    std::map<int, std::map<std::string, int>> transitions_;

    // Print all states and items to a stream
    void printStates(std::ostream& out = std::cout) const;

    // Print the parse table to a stream
    void printTable(std::ostream& out = std::cout) const;

private:
    Grammar&           grammar_;
    std::vector<State> states_;
    ParseTable         table_;

    ItemSet closure(const ItemSet& items);
    ItemSet gotoSet(const ItemSet& items, const Symbol& sym);
    int     findOrAddState(const ItemSet& items);
    void    buildTable();
};