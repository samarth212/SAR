#include <deque>
#include <algorithm>
#include <cctype>
#include <condition_variable>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "anomaly_detector.h"
#include "api.h"
#include "data_parser.h"
#include "socket.h"

std::unordered_map<std::string, SymbolState> bySymbol;
std::deque<Anomaly> recentAnomalies;
std::mutex stateMutex;
std::unordered_set<std::string> trackedSymbols;
std::mutex subscriptionMutex;
std::condition_variable subscriptionCv;

static std::string trim(std::string value) {
    auto is_space = [](unsigned char ch) { return std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [&](char ch) {
                    return !is_space(static_cast<unsigned char>(ch));
                }));
    value.erase(std::find_if(value.rbegin(), value.rend(),
                             [&](char ch) { return !is_space(static_cast<unsigned char>(ch)); })
                    .base(),
                value.end());
    return value;
}

static void load_env_file(const std::filesystem::path &path) {
    std::ifstream file(path);
    if (!file)
        return;

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        constexpr std::string_view export_prefix = "export ";
        if (line.starts_with(export_prefix))
            line = trim(line.substr(export_prefix.size()));

        const auto equal_pos = line.find('=');
        if (equal_pos == std::string::npos)
            continue;

        std::string key = trim(line.substr(0, equal_pos));
        std::string value = trim(line.substr(equal_pos + 1));
        if (key.empty())
            continue;

        if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                                  (value.front() == '\'' && value.back() == '\''))) {
            value = value.substr(1, value.size() - 2);
        }

        if (std::getenv(key.c_str()) == nullptr)
            setenv(key.c_str(), value.c_str(), 0);
    }
}

static void load_backend_env(const char *executable_path) {
    std::vector<std::filesystem::path> candidates{
        ".env",
        "backend/.env",
        "../.env",
    };

    if (executable_path && *executable_path) {
        std::filesystem::path executable{executable_path};
        if (executable.has_parent_path()) {
            candidates.push_back(executable.parent_path() / ".env");
            candidates.push_back(executable.parent_path().parent_path() / ".env");
        }
    }

    for (const auto &candidate : candidates)
        load_env_file(candidate);
}

int main(int argc, char *argv[]) {
    load_backend_env(argc > 0 ? argv[0] : nullptr);

    std::thread apiThread([] { run_http_server(8080); });
    apiThread.detach();

    run_socket();

    return 0;
}
