#include "parser.h"
#include <sstream>
#include <algorithm>
#include <iomanip>
#include "lr1.h"
#include "grammar.h"

Parser::Parser(Grammar& g, ParseTable& t) : grammar_(g), table_(t) {}

ParseResult Parser::parse(const std::vector<std::string>& tokens) {
    ParseResult result;

    std::vector<std::string> input = tokens;
    input.push_back("$");

    std::vector<int>         stateStack  = {0};
    std::vector<std::string> symbolStack = {};
    int pos = 0;

    while (true) {
        int               curState = stateStack.back();
        const std::string& curTok  = input[static_cast<size_t>(pos)];

        ParseStep step;
        step.stateStack       = stateStack;
        step.symbolStack      = symbolStack;
        step.remainingInput   = std::vector<std::string>(input.begin() + pos, input.end());
        step.reduceProduction = -1;

        // Look up action
        auto rowIt = table_.action.find(curState);
        if (rowIt == table_.action.end() ||
            rowIt->second.find(curTok) == rowIt->second.end() ||
            rowIt->second.at(curTok).type == ActionType::ERROR) {
            step.action     = "Error: no action for state " +
                              std::to_string(curState) + " on '" + curTok + "'";
            step.actionType = "error";
            result.steps.push_back(step);
            result.errorMessage = step.action;
            return result;
        }

        const Action& act = rowIt->second.at(curTok);

        if (act.type == ActionType::SHIFT) {
            step.action     = "Shift " + curTok + ", go to state " + std::to_string(act.value);
            step.actionType = "shift";
            result.steps.push_back(step);
            stateStack.push_back(act.value);
            symbolStack.push_back(curTok);
            pos++;

        } else if (act.type == ActionType::REDUCE) {
            const Production& prod   = grammar_.productions[static_cast<size_t>(act.value)];
            int               rhsLen = static_cast<int>(prod.rhs.size());

            std::string prodStr = prod.lhs.name + " ->";
            for (const auto& s : prod.rhs) prodStr += " " + s.name;

            step.action            = "Reduce by: " + prodStr;
            step.actionType        = "reduce";
            step.reduceProduction  = act.value;
            result.steps.push_back(step);

            for (int i = 0; i < rhsLen; i++) {
                stateStack.pop_back();
                symbolStack.pop_back();
            }

            symbolStack.push_back(prod.lhs.name);
            int top = stateStack.back();

            auto gotoRow = table_.gotoTable.find(top);
            if (gotoRow == table_.gotoTable.end() ||
                gotoRow->second.find(prod.lhs.name) == gotoRow->second.end()) {
                result.errorMessage = "GOTO error: no entry for state " +
                                      std::to_string(top) + " on " + prod.lhs.name;
                return result;
            }
            stateStack.push_back(gotoRow->second.at(prod.lhs.name));

        } else if (act.type == ActionType::ACCEPT) {
            step.action     = "Accept! Input parsed successfully.";
            step.actionType = "accept";
            result.steps.push_back(step);
            result.accepted = true;
            return result;
        }
    }
}


static std::string jStr(const std::string& s) {
    std::string o = "\"";
    for (char c : s) {
        if      (c == '"')  o += "\\\"";
        else if (c == '\\') o += "\\\\";
        else if (c == '\n') o += "\\n";
        else                o += c;
    }
    return o + "\"";
}

static std::string actionCode(const Action& a) {
    switch (a.type) {
        case ActionType::SHIFT:  return "s" + std::to_string(a.value);
        case ActionType::REDUCE: return "r" + std::to_string(a.value);
        case ActionType::ACCEPT: return "acc";
        default:                 return "";
    }
}
std::string serializeToJSON(
    const Grammar&                                   g,
    const std::vector<State>&                        states,
    const std::map<int, std::map<std::string, int>>& transitions,
    const ParseTable&                                table,
    const ParseResult&                               result
) {
    std::ostringstream o;
    o << "{\n";

    // Productions
    o << "\"productions\": [\n";
    for (int i = 0; i < static_cast<int>(g.productions.size()); i++) {
        const auto& p = g.productions[static_cast<size_t>(i)];
        o << "  {\"id\":" << p.id << ", \"lhs\":" << jStr(p.lhs.name) << ", \"rhs\":[";
        for (int j = 0; j < static_cast<int>(p.rhs.size()); j++) {
            if (j > 0) o << ",";
            o << jStr(p.rhs[static_cast<size_t>(j)].name);
        }
        o << "]}";
        if (i + 1 < static_cast<int>(g.productions.size())) o << ",";
        o << "\n";
    }
    o << "],\n";

    // States with their LR(1) items
    o << "\"states\": [\n";
    for (int i = 0; i < static_cast<int>(states.size()); i++) {
        const auto& s = states[static_cast<size_t>(i)];
        o << "  {\"id\":" << s.id << ", \"items\":[\n";
        bool first = true;
        for (const auto& item : s.items) {
            if (!first) o << ",\n";
            first = false;
            const auto& prod = g.productions[static_cast<size_t>(item.productionId)];
            std::string str  = prod.lhs.name + " ->";
            for (int j = 0; j <= static_cast<int>(prod.rhs.size()); j++) {
                if (j == item.dotPos) str += " .";
                if (j < static_cast<int>(prod.rhs.size()))
                    str += " " + prod.rhs[static_cast<size_t>(j)].name;
            }
            str += ", " + item.lookahead.name;
            o << "    " << jStr(str);
        }
        o << "\n  ]}";
        if (i + 1 < static_cast<int>(states.size())) o << ",";
        o << "\n";
    }
    o << "],\n";

    // Transitions (graph edges)
    o << "\"transitions\": [\n";
    bool firstTrans = true;
    for (const auto& kv1 : transitions) {
        const auto& from = kv1.first;
        const auto& edges = kv1.second;
        for (const auto& kv2 : edges) {
            const auto& sym = kv2.first;
            const auto& to = kv2.second;
            if (!firstTrans) o << ",\n";
            firstTrans = false;
            o << "  {\"from\":" << from << ", \"to\":" << to
              << ", \"label\":" << jStr(sym) << "}";
        }
    }
    o << "\n],\n";

    // ACTION table
    o << "\"actionTable\": {\n";
    bool firstRow = true;
    for (const auto& kv1 : table.action) {
        const auto& stId = kv1.first;
        const auto& row = kv1.second;
        if (!firstRow) o << ",\n";
        firstRow = false;
        o << "  \"" << stId << "\": {";
        bool firstCell = true;
        for (const auto& kv2 : row) {
            const auto& tok = kv2.first;
            const auto& act = kv2.second;
            if (!firstCell) o << ", ";
            firstCell = false;
            o << jStr(tok) << ":" << jStr(actionCode(act));
        }
        o << "}";
    }
    o << "\n},\n";

    // GOTO table
    o << "\"gotoTable\": {\n";
    firstRow = true;
    for (const auto& kv1 : table.gotoTable) {
        const auto& stId = kv1.first;
        const auto& row = kv1.second;
        if (!firstRow) o << ",\n";
        firstRow = false;
        o << "  \"" << stId << "\": {";
        bool firstCell = true;
        for (const auto& kv2 : row) {
            const auto& nt = kv2.first;
            const auto& nextSt = kv2.second;
            if (!firstCell) o << ", ";
            firstCell = false;
            o << jStr(nt) << ":" << nextSt;
        }
        o << "}";
    }
    o << "\n},\n";

    // Symbol lists (for visualizer column headers)
    o << "\"terminals\": [";
    bool ft = true;
    for (const auto& t : g.terminals) {
        if (!ft) o << ",";
        ft = false;
        o << jStr(t.name);
    }
    o << "],\n";

    o << "\"nonTerminals\": [";
    ft = true;
    for (const auto& nt : g.nonTerminals) {
        if (!ft) o << ",";
        ft = false;
        o << jStr(nt.name);
    }
    o << "],\n";

    // Conflict info
    o << "\"hasConflict\":" << (table.hasConflict ? "true" : "false") << ",\n";
    o << "\"conflicts\": [";
    for (int i = 0; i < static_cast<int>(table.conflicts.size()); i++) {
        if (i > 0) o << ",";
        o << jStr(table.conflicts[static_cast<size_t>(i)]);
    }
    o << "],\n";

    // Parse trace
    o << "\"parseResult\": {\n";
    o << "  \"accepted\":"     << (result.accepted ? "true" : "false") << ",\n";
    o << "  \"errorMessage\":" << jStr(result.errorMessage)             << ",\n";
    o << "  \"steps\": [\n";
    for (int i = 0; i < static_cast<int>(result.steps.size()); i++) {
        const auto& step = result.steps[static_cast<size_t>(i)];
        o << "    {\n";
        o << "      \"stateStack\":[";
        for (int j = 0; j < static_cast<int>(step.stateStack.size()); j++) {
            if (j > 0) o << ",";
            o << step.stateStack[static_cast<size_t>(j)];
        }
        o << "],\n";
        o << "      \"symbolStack\":[";
        for (int j = 0; j < static_cast<int>(step.symbolStack.size()); j++) {
            if (j > 0) o << ",";
            o << jStr(step.symbolStack[static_cast<size_t>(j)]);
        }
        o << "],\n";
        o << "      \"input\":[";
        for (int j = 0; j < static_cast<int>(step.remainingInput.size()); j++) {
            if (j > 0) o << ",";
            o << jStr(step.remainingInput[static_cast<size_t>(j)]);
        }
        o << "],\n";
        o << "      \"action\":"           << jStr(step.action)     << ",\n";
        o << "      \"actionType\":"       << jStr(step.actionType) << ",\n";
        o << "      \"reduceProduction\":" << step.reduceProduction << "\n";
        o << "    }";
        if (i + 1 < static_cast<int>(result.steps.size())) o << ",";
        o << "\n";
    }
    o << "  ]\n}\n}\n";
    return o.str();
}

// Human-readable parse trace 

void printParseTrace(const ParseResult& result, std::ostream& out) {
    const int W1 = 22, W2 = 18, W3 = 18, W4 = 8;
    out << "\n=== Parse Trace ===\n";
    out << std::left
        << std::setw(5)  << "Step"
        << std::setw(W1) << "State Stack"
        << std::setw(W2) << "Symbol Stack"
        << std::setw(W3) << "Input"
        << std::setw(W4) << "Action"
        << "Description\n";
    out << std::string(5 + W1 + W2 + W3 + W4 + 30, '-') << "\n";

    for (int i = 0; i < static_cast<int>(result.steps.size()); i++) {
        const auto& s = result.steps[static_cast<size_t>(i)];

        // State stack string
        std::string ss;
        for (int st : s.stateStack) ss += std::to_string(st) + " ";

        // Symbol stack string
        std::string sym = "$ ";
        for (const auto& sy : s.symbolStack) sym += sy + " ";

        // Remaining input string
        std::string inp;
        for (const auto& t : s.remainingInput) inp += t + " ";

        // Short action code
        std::string code;
        if (s.actionType == "shift") {
            size_t p = s.action.rfind("state ");
            code = "s" + (p != std::string::npos ? s.action.substr(p + 6) : "?");
        } else if (s.actionType == "reduce") {
            code = "r" + std::to_string(s.reduceProduction);
        } else if (s.actionType == "accept") {
            code = "acc";
        } else {
            code = "err";
        }

        // Truncate long strings for table alignment
        auto trunc = [](const std::string& str, int w) {
            return str.size() > static_cast<size_t>(w) ?
                   str.substr(0, static_cast<size_t>(w - 2)) + ".." : str;
        };

        out << std::left
            << std::setw(5)  << (i + 1)
            << std::setw(W1) << trunc(ss,  W1)
            << std::setw(W2) << trunc(sym, W2)
            << std::setw(W3) << trunc(inp, W3)
            << std::setw(W4) << code
            << s.action << "\n";
    }

    out << "\nResult: " << (result.accepted ? "ACCEPTED" : "REJECTED") << "\n";
    if (!result.errorMessage.empty()) out << "Error:  " << result.errorMessage << "\n";
}

//  Full deployment report 

void printFullReport(
    const Grammar&    g,
    const LR1Builder& builder,
    const ParseResult& result,
    std::ostream&     out
) {
    g.print(out);
    builder.printStates(out);
    builder.printTable(out);
    printParseTrace(result, out);
}