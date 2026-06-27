#include "grammar.h"
#include "lr1.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>

// Forward declarations
Grammar                  buildGrammar();
std::vector<std::string> tokenize(const std::string& line);
void                     printUsage(const char* prog);

Grammar buildGrammar() {
    Grammar g;

    g.augmentedStart = nonTerminal("S'");
    g.startSymbol    = nonTerminal("S");

    g.terminals.insert(terminal("="));
    g.terminals.insert(terminal("*"));
    g.terminals.insert(terminal("id"));
    g.terminals.insert(END_OF_INPUT);

    g.nonTerminals.insert(nonTerminal("S'"));
    g.nonTerminals.insert(nonTerminal("S"));
    g.nonTerminals.insert(nonTerminal("L"));
    g.nonTerminals.insert(nonTerminal("R"));

    // Order must match production ids
    g.productions.push_back({nonTerminal("S'"), {nonTerminal("S")},                                 0});
    g.productions.push_back({nonTerminal("S"),  {nonTerminal("L"), terminal("="), nonTerminal("R")}, 1});
    g.productions.push_back({nonTerminal("S"),  {nonTerminal("R")},                                 2});
    g.productions.push_back({nonTerminal("L"),  {terminal("*"),    nonTerminal("R")},                3});
    g.productions.push_back({nonTerminal("L"),  {terminal("id")},                                   4});
    g.productions.push_back({nonTerminal("R"),  {nonTerminal("L")},                                 5});

    g.computeFirstSets();
    return g;
}

// Split a line into whitespace-separated tokens
std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream ss(line);
    std::string tok;
    while (ss >> tok) tokens.push_back(tok);
    return tokens;
}

void printUsage(const char* prog) {
    std::cout
        << "\nUsage: " << prog << " [OPTIONS] \"<token1 token2 ...>\"\n"
        << "\nOptions:\n"
        << "  -o, --output <file>   Write JSON data to <file> (default: lr1_data.json)\n"
        << "  -r, --report          Print full text report to stdout\n"
        << "  -t, --trace           Print parse trace table to stdout\n"
        << "  -q, --quiet           Suppress summary output\n"
        << "  -j, --json            Print JSON to stdout instead of a file\n"
        << "  -h, --help            Show this help message\n"
        << "\nExamples:\n"
        << "  " << prog << " \"id = * id\"\n"
        << "  " << prog << " --report \"* * id\"\n"
        << "  " << prog << " --output out.json \"id = id\"\n"
        << "  " << prog << " --json --quiet \"id\"\n"
        << "\nValid token set:  id   =   *\n"
        << "* is a unary prefix dereference (pointer), not multiplication.\n\n";
}

int main(int argc, char* argv[]) {
    std::string inputStr   = "id = * id";
    std::string outFile    = "lr1_data.json";
    bool        doReport   = false;
    bool        doTrace    = false;
    bool        quiet      = false;
    bool        jsonStdout = false;

    for (int i = 1; i < argc; i++) {
        if      (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help")   == 0) {
            printUsage(argv[0]); return 0;
        } else if (std::strcmp(argv[i], "-r") == 0 || std::strcmp(argv[i], "--report") == 0) {
            doReport = true;
        } else if (std::strcmp(argv[i], "-t") == 0 || std::strcmp(argv[i], "--trace")  == 0) {
            doTrace = true;
        } else if (std::strcmp(argv[i], "-q") == 0 || std::strcmp(argv[i], "--quiet")  == 0) {
            quiet = true;
        } else if (std::strcmp(argv[i], "-j") == 0 || std::strcmp(argv[i], "--json")   == 0) {
            jsonStdout = true;
        } else if ((std::strcmp(argv[i], "-o") == 0 || std::strcmp(argv[i], "--output") == 0) && i + 1 < argc) {
            outFile = argv[++i];
        } else {
            inputStr = argv[i];
        }
    }

    Grammar    g = buildGrammar();
    LR1Builder builder(g);
    builder.build();

    std::vector<std::string> tokens = tokenize(inputStr);
    Parser      parser(g, builder.getTable());
    ParseResult result = parser.parse(tokens);

    if (!quiet) {
        std::cout << "Input      : " << inputStr                                          << "\n";
        std::cout << "States     : " << builder.getStates().size()                        << "\n";
        std::cout << "Conflicts  : " << (builder.getTable().hasConflict ? "YES" : "NONE") << "\n";
        std::cout << "Result     : " << (result.accepted ? "ACCEPTED" : "REJECTED")       << "\n";
        std::cout << "Steps      : " << result.steps.size()                               << "\n";
        if (!result.errorMessage.empty())
            std::cerr << "Error      : " << result.errorMessage << "\n";
    }

    if (doReport) {
        printFullReport(g, builder, result, std::cout);
    } else if (doTrace) {
        printParseTrace(result, std::cout);
    }

    std::string json = serializeToJSON(
        g,
        builder.getStates(),
        builder.transitions_,
        builder.getTable(),
        result
    );

    if (jsonStdout) {
        std::cout << json;
    } else {
        std::ofstream out(outFile);
        if (!out) {
            std::cerr << "Error: cannot open " << outFile << " for writing\n";
            return 1;
        }
        out << json;
        if (!quiet) std::cout << "JSON       : " << outFile << "\n";
    }

    return result.accepted ? 0 : 1;
}