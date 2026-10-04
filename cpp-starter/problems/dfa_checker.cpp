#include "dfa_checker.hpp"
#include <fstream>
#include <sstream>

void DFAChecker::initialize_parser(cxxopts::Options &options) {
    try {
        options.add_options()
            ("check", "Ellenorizendo szo/szavak", cxxopts::value<std::string>());
    } catch (const cxxopts::exceptions::specification &) {
        // Ha egy masik feladat (pl. dfa.cpp) mar hozzaadta a "check" opciot, elkapjuk a hibat
    }
}

bool DFAChecker::is_chosen_problem(const cxxopts::ParseResult &args) {
    return args.count("input") > 0 && args.count("output") > 0 && args.count("check") > 0;
}

std::vector<std::string> DFAChecker::split(const std::string &str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(str);
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void DFAChecker::parseInputFile(const std::string &filePath) {
    std::ifstream infile(filePath);
    if (!infile.is_open()) return;

    std::string line;

    // 1. sor: Állapotok
    if (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string state;
        while (ss >> state) states.insert(state);
    }

    // 2. sor: Ábécé
    if (std::getline(infile, line)) {
        std::stringstream ss(line);
        char ch;
        while (ss >> ch) alphabet.insert(ch);
    }

    // 3. sor: Kezdőállapot
    if (std::getline(infile, line)) {
        std::stringstream ss(line);
        ss >> start_state;
    }

    // 4. sor: Végállapotok
    if (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string state;
        while (ss >> state) accept_states.insert(state);
    }

    // Átmenetek: src char dst
    std::string src, dst;
    char symbol;
    while (infile >> src >> symbol >> dst) {
        transitions[{src, symbol}] = dst;
    }
}

std::string DFAChecker::simulate(const std::string &word) {
    std::string current_state = start_state;

    for (char ch : word) {
        auto it = transitions.find({current_state, ch});
        if (it == transitions.end()) {
            return "NEM";
        }
        current_state = it->second;
    }

    if (accept_states.count(current_state) > 0) {
        return "IGEN";
    }
    return "NEM";
}

int DFAChecker::run(const cxxopts::ParseResult &args) {
    std::string input_file = args["input"].as<std::string>();
    std::string output_file = args["output"].as<std::string>();
    std::string check_words = args["check"].as<std::string>();

    parseInputFile(input_file);

    std::vector<std::string> words = split(check_words, ',');
    std::ofstream outfile(output_file);

    for (size_t i = 0; i < words.size(); ++i) {
        outfile << simulate(words[i]);
        if (i + 1 < words.size()) {
            outfile << "\n";
        }
    }
    outfile << "\n";

    return 0;
}