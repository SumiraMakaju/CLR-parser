#pragma once
#include "grammar.h"
#include "lr1.h"
#include "parser.h"
#include <string>
#include <vector>
#include <map>

// Start a blocking HTTP server on the given port.
// Routes:
//   GET  /              serves visualization/index.html
//   GET  /parse?q=...   parses the token string, returns JSON
//   POST /parse         body = token string, returns JSON
void startServer(Grammar&                                         g,
                 ParseTable&                                      table,
                 std::vector<State>&                              states,
                 std::map<int, std::map<std::string, int>>&       transitions,
                 int                                              port = 7373);
