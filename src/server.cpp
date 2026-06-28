#include "grammar.h"
#include "lr1.h"
#include "parser.h"
#include "server.h"
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <cstdint>
  #pragma comment(lib, "ws2_32.lib")
  using sock_t = SOCKET;
  #define SOCK_INVALID INVALID_SOCKET
  #define SOCK_ERR     SOCKET_ERROR
  #define closeSock(s) closesocket(s)
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <unistd.h>
  #include <cstdint>
  using sock_t = int;
  #define SOCK_INVALID (-1)
  #define SOCK_ERR     (-1)
  #define closeSock(s) close(s)
#endif

static std::vector<std::string> splitTokens(const std::string& s) {
    std::vector<std::string> out;
    std::istringstream ss(s);
    std::string t;
    while (ss >> t) out.push_back(t);
    return out;
}

static std::string urlDecode(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '+') { out += ' '; }
        else if (s[i] == '%' && i+2 < s.size()) {
            int val = 0;
            std::istringstream ss(s.substr(i+1,2));
            ss >> std::hex >> val;
            out += static_cast<char>(val);
            i += 2;
        } else { out += s[i]; }
    }
    return out;
}

static std::string queryParam(const std::string& query, const std::string& key) {
    std::string search = key + "=";
    size_t pos = query.find(search);
    if (pos == std::string::npos) return "";
    size_t start = pos + search.size();
    size_t end   = query.find('&', start);
    return urlDecode(end == std::string::npos ? query.substr(start) : query.substr(start, end-start));
}

// Read any file under visualization/ and serve it
static std::string readFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return "";
    return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

static std::string mimeType(const std::string& path) {
    if (path.size() >= 5 && path.substr(path.size()-5) == ".html") return "text/html; charset=utf-8";
    if (path.size() >= 4 && path.substr(path.size()-4) == ".css")  return "text/css";
    if (path.size() >= 3 && path.substr(path.size()-3) == ".js")   return "application/javascript";
    return "text/plain";
}

static std::string httpResponse(int code, const std::string& ct, const std::string& body) {
    std::string status = code == 200 ? "200 OK" : code == 404 ? "404 Not Found" : "400 Bad Request";
    std::ostringstream r;
    r << "HTTP/1.1 " << status << "\r\n"
      << "Content-Type: " << ct << "\r\n"
      << "Content-Length: " << body.size() << "\r\n"
      << "Access-Control-Allow-Origin: *\r\n"
      << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
      << "Connection: close\r\n\r\n"
      << body;
    return r.str();
}

static bool handleRequest(sock_t client,
                           Grammar& g,
                           ParseTable& table,
                           std::vector<State>& states,
                           std::map<int,std::map<std::string,int>>& transitions) {
    char buf[8192] = {};
    int  received  = recv(client, buf, sizeof(buf)-1, 0);
    if (received <= 0) return false;

    std::string req(buf, static_cast<size_t>(received));
    std::istringstream ss(req);
    std::string method, path, version;
    ss >> method >> path >> version;

    if (method == "OPTIONS") {
        std::string r = httpResponse(200, "text/plain", "");
        send(client, r.c_str(), static_cast<int>(r.size()), 0);
        return false;
    }

    std::string query;
    size_t qpos = path.find('?');
    if (qpos != std::string::npos) { query = path.substr(qpos+1); path = path.substr(0,qpos); }

    std::string response;

    if (path == "/" || path == "/index.html") {
        // Serve the visualizer entry point
        std::string body = readFile("visualization/index.html");
        if (body.empty()) body = readFile("../visualization/index.html");
        response = httpResponse(body.empty() ? 404 : 200,
                                "text/html; charset=utf-8",
                                body.empty() ? "visualization/index.html not found" : body);

    } else if (path.substr(0,4) == "/css" || path.substr(0,3) == "/js") {
        // Serve static files: /css/base.css -> visualization/css/base.css
        std::string filePath = "visualization" + path;
        std::string body     = readFile(filePath);
        if (body.empty()) body = readFile("../" + filePath);
        response = httpResponse(body.empty() ? 404 : 200, mimeType(path),
                                body.empty() ? "File not found" : body);

    } else if (path == "/parse") {
        // Parse a token string and return JSON
        std::string inputStr;
        if (method == "GET") {
            inputStr = queryParam(query, "q");
        } else {
            size_t bodyStart = req.find("\r\n\r\n");
            if (bodyStart != std::string::npos) inputStr = req.substr(bodyStart+4);
        }
        while (!inputStr.empty() && (inputStr.back()=='\r'||inputStr.back()=='\n'||inputStr.back()==' '))
            inputStr.pop_back();

        if (inputStr.empty()) {
            response = httpResponse(400, "application/json", "{\"error\":\"empty input\"}");
        } else {
            auto        tokens = splitTokens(inputStr);
            Parser      parser(g, table);
            ParseResult result = parser.parse(tokens);
            std::string json   = serializeToJSON(g, states, transitions, table, result);
            response = httpResponse(200, "application/json", json);
        }

    } else {
        response = httpResponse(404, "text/plain", "Not found");
    }

    send(client, response.c_str(), static_cast<int>(response.size()), 0);
    return false;
}

void startServer(Grammar& g, ParseTable& table,
                 std::vector<State>& states,
                 std::map<int,std::map<std::string,int>>& transitions,
                 int port) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif
    sock_t server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == SOCK_INVALID) { std::cerr << "Cannot create socket\n"; return; }

    int opt = 1;
#ifdef _WIN32
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(static_cast<uint16_t>(port));

    if (bind(server, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCK_ERR) {
        std::cerr << "Cannot bind to port " << port << "\n"; closeSock(server); return;
    }
    listen(server, 8);
    std::cout << "Server  : http://localhost:" << port << "\n";
    std::cout << "Open that URL in your browser. Ctrl+C to stop.\n";

    while (true) {
        sockaddr_in clientAddr{};
#ifdef _WIN32
        int clientLen = sizeof(clientAddr);
#else
        socklen_t clientLen = sizeof(clientAddr);
#endif
        sock_t client = accept(server, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
        if (client == SOCK_INVALID) continue;
        handleRequest(client, g, table, states, transitions);
        closeSock(client);
    }
#ifdef _WIN32
    WSACleanup();
#endif
}