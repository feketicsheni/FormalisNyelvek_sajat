#include "dfa.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

void DFAProblem::initialize_parser(cxxopts::Options& options) {
    options.add_options()
        ("check", "Check word(s) separated by commas", cxxopts::value<std::string>());
}

bool DFAProblem::is_chosen_problem(const cxxopts::ParseResult& args) {
    return args.count("check") > 0 && args.count("input") > 0 && args.count("output") > 0;
}

std::vector<std::string> DFAProblem::split_words(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

DFAProblem::DFA DFAProblem::parse_input_file(const std::string& filepath) {
    DFA dfa;
    std::ifstream infile(filepath);
    if (!infile.is_open()) {
        std::cerr << "Error: Could not open input file: " << filepath << std::endl;
        return dfa;
    }

    std::string line;

    // 1. sor: Állapotok (nem feltétlenül kell eltárolni)
    std::getline(infile, line);

    // 2. sor: Ábécé (nem feltétlenül kell eltárolni)
    std::getline(infile, line);

    // 3. sor: Kezdőállapot
    if (std::getline(infile, line)) {
        std::stringstream ss(line);
        ss >> dfa.start_state;
    }

    // 4. sor: Elfogadó állapotok
    if (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string state;
        while (ss >> state) {
            dfa.accept_states.insert(state);
        }
    }

    // További sorok: Átmenetek (from symbol to)
    while (std::getline(infile, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string from_state, to_state;
        char symbol;
        if (ss >> from_state >> symbol >> to_state) {
            dfa.transitions[from_state][symbol] = to_state;
        }
    }

    return dfa;
}

bool DFAProblem::DFA::simulate(const std::string& word) const {
    std::string current_state = start_state;

    for (char symbol : word) {
        auto state_it = transitions.find(current_state);
        if (state_it == transitions.end()) {
            return false;
        }

        auto trans_it = state_it->second.find(symbol);
        if (trans_it == state_it->second.end()) {
            return false;
        }

        current_state = trans_it->second;
    }

    return accept_states.count(current_state) > 0;
}

int DFAProblem::run(const cxxopts::ParseResult& args) {
    std::string input_file = args["input"].as<std::string>();
    std::string output_file = args["output"].as<std::string>();
    std::string check_arg = args["check"].as<std::string>();

    DFA dfa = parse_input_file(input_file);
    std::vector<std::string> words = split_words(check_arg, ',');

    std::ofstream outfile(output_file);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open output file: " << output_file << std::endl;
        return 1;
    }

    for (size_t i = 0; i < words.size(); ++i) {
        bool accepted = dfa.simulate(words[i]);
        outfile << (accepted ? "IGEN" : "NEM");
        if (i + 1 < words.size()) {
            outfile << "\n";
        }
    }
    return 0;
}