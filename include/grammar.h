#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>

//grammar symbol
struct Symbol {
    std::string name;
    bool isTerminal;

    bool operator==(const Symbol& o) const { return name == o.name && isTerminal == o.isTerminal; }
    bool operator!=(const Symbol& o) const { return !(*this == o); }
    bool operator<(const Symbol& o) const {
        if (isTerminal != o.isTerminal) return isTerminal < o.isTerminal;
        return name < o.name;
    }
};

struct Production {
    Symbol lhs;
    std::vector<Symbol> rhs;
    int id;
};


struct Grammar {
    Symbol startSymbol;
    Symbol augmentedStart;
    std::vector<Production> productions;
    std::set<Symbol> terminals;
    std::set<Symbol> nonTerminals;
    std::map<Symbol, std::set<Symbol>> firstSets;

    // Compute FIRST sets for all symbols (iterative fixed-point)
    void computeFirstSets();

    // Compute FIRST of a sequence of symbols followed by a lookahead terminal
    std::set<Symbol> first(const std::vector<Symbol>& seq, const Symbol& lookahead);

     void print(std::ostream& out = std::cout) const;
};

// Factory helpers
Symbol terminal(const std::string& name);
Symbol nonTerminal(const std::string& name);

// Special symbols
extern Symbol EPSILON;
extern Symbol END_OF_INPUT;
