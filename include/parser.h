#pragma once
#include "lr1.h"
#include <string>
#include <vector>
#include <iostream>

// Snapshot of the parser state at one step
struct ParseStep {
    std::vector<int>         stateStack;
    std::vector<std::string> symbolStack;
    std::vector<std::string> remainingInput; 
    std::string              action;         
    std::string              actionType;     
    int                      reduceProduction = -1; 
};

// Complete result returned by Parser::parse()
struct ParseResult {
    bool                   accepted     = false;
    std::string            errorMessage;
    std::vector<ParseStep> steps;
};

// LR(1) stack-driven parser
class Parser {
public:
    Parser(Grammar& g, ParseTable& t);

    ParseResult parse(const std::vector<std::string>& tokens);

private:
    Grammar&    grammar_;
    ParseTable& table_;
};

std::string serializeToJSON(
    const Grammar&                                   g,
    const std::vector<State>&                        states,
    const std::map<int, std::map<std::string, int>>& transitions,
    const ParseTable&                                table,
    const ParseResult&                               result
);

// Print a human-readable parse trace table to a stream
void printParseTrace(const ParseResult& result, std::ostream& out = std::cout);

// Print the grammar, states, table, and result summary to a stream
void printFullReport(
    const Grammar&     g,
    const LR1Builder&  builder,
    const ParseResult& result,
    std::ostream&      out = std::cout
);