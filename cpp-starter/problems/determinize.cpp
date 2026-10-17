#include "determinize.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

void DeterminizeProblem::initialize_parser(cxxopts::Options &options) {
    options.add_options()
        ("det", "Determinizalja az automatát");
}

bool DeterminizeProblem::is_chosen_problem(
    const cxxopts::ParseResult &args
) {
    return args.count("det") > 0
        && args.count("input") > 0
        && args.count("output") > 0;
}

bool DeterminizeProblem::read_automaton(
    const std::string &filename,
    Automaton &automaton
) {
    std::ifstream input(filename);

    if (!input.is_open()) {
        std::cerr << "Error: Could not open input file: "
                  << filename << std::endl;
        return false;
    }

    std::string line;

    // 1. sor: állapotok
    if (!std::getline(input, line)) {
        return false;
    }

    {
        std::stringstream ss(line);
        std::string state;

        while (ss >> state) {
            automaton.states.push_back(state);
        }
    }

    // 2. sor: ábécé
    if (!std::getline(input, line)) {
        return false;
    }

    {
        std::stringstream ss(line);
        std::string symbol;

        while (ss >> symbol) {
            automaton.alphabet.push_back(symbol);
        }
    }

    // 3. sor: kezdőállapot
    if (!std::getline(input, line)) {
        return false;
    }

    {
        std::stringstream ss(line);
        ss >> automaton.start_state;
    }

    // 4. sor: végállapotok
    if (!std::getline(input, line)) {
        return false;
    }

    {
        std::stringstream ss(line);
        std::string state;

        while (ss >> state) {
            automaton.accept_states.insert(state);
        }
    }

    // További sorok: átmenetek
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);

        std::string from;
        std::string symbol;
        std::string to;

        if (!(ss >> from >> symbol >> to)) {
            std::cerr << "Error: Invalid transition: "
                      << line << std::endl;
            return false;
        }

        automaton.transitions[
            {from, symbol}
        ].insert(to);
    }

    return is_valid_automaton(automaton);
}

bool DeterminizeProblem::is_valid_automaton(
    const Automaton &automaton
) {
    std::set<std::string> valid_states(
        automaton.states.begin(),
        automaton.states.end()
    );

    std::set<std::string> valid_symbols(
        automaton.alphabet.begin(),
        automaton.alphabet.end()
    );

    // Kezdőállapot ellenőrzése
    if (!valid_states.count(automaton.start_state)) {
        std::cerr << "Error: Invalid start state: "
                  << automaton.start_state << std::endl;
        return false;
    }

    // Végállapotok ellenőrzése
    for (const std::string &state : automaton.accept_states) {
        if (!valid_states.count(state)) {
            std::cerr << "Error: Invalid accepting state: "
                      << state << std::endl;
            return false;
        }
    }

    // Átmenetek állapotainak és jeleinek ellenőrzése
    for (const auto &entry : automaton.transitions) {
        const std::string &from = entry.first.first;
        const std::string &symbol = entry.first.second;

        if (!valid_states.count(from)) {
            std::cerr << "Error: Invalid transition state: "
                      << from << std::endl;
            return false;
        }

        if (!valid_symbols.count(symbol)) {
            std::cerr << "Error: Invalid transition symbol: "
                      << symbol << std::endl;
            return false;
        }

        for (const std::string &to : entry.second) {
            if (!valid_states.count(to)) {
                std::cerr << "Error: Invalid transition state: "
                          << to << std::endl;
                return false;
            }
        }
    }

    return true;
}

void DeterminizeProblem::determinize(
    const Automaton &automaton,
    std::vector<StateSet> &dfa_states,
    std::map<
        std::pair<int, std::string>,
        int
    > &dfa_transitions
) {
    StateSet start_set;
    start_set.insert(automaton.start_state);

    dfa_states.push_back(start_set);

    for (size_t current_index = 0;
         current_index < dfa_states.size();
         ++current_index) {

        StateSet current = dfa_states[current_index];
        // Az ábécét rendezett sorrendben dolgozzuk fel.
        std::vector<std::string> sorted_alphabet =
            automaton.alphabet;

        std::sort(
            sorted_alphabet.begin(),
            sorted_alphabet.end()
        );

        for (const std::string &symbol : sorted_alphabet) {
            StateSet destination;

            for (const std::string &state : current) {
                auto transition_it =
                    automaton.transitions.find(
                        {state, symbol}
                    );

                if (transition_it == automaton.transitions.end()) {
                    continue;
                }

                destination.insert(
                    transition_it->second.begin(),
                    transition_it->second.end()
                );
            }

            // Ha nincs átmenet, nem írjuk ki.
            if (destination.empty()) {
                continue;
            }

            int destination_index = -1;

            for (size_t i = 0; i < dfa_states.size(); ++i) {
                if (dfa_states[i] == destination) {
                    destination_index =
                        static_cast<int>(i);
                    break;
                }
            }

            // Új részhalmaz létrehozása.
            if (destination_index == -1) {
                dfa_states.push_back(destination);

                destination_index =
                    static_cast<int>(dfa_states.size() - 1);
            }

            dfa_transitions[
                {static_cast<int>(current_index), symbol}
            ] = destination_index;
        }
    }
}

bool DeterminizeProblem::is_accepting(
    const StateSet &state_set,
    const std::set<std::string> &accept_states
) const {
    for (const std::string &state : state_set) {
        if (accept_states.count(state)) {
            return true;
        }
    }

    return false;
}

void DeterminizeProblem::write_result(
    const std::string &filename,
    const Automaton &automaton,
    const std::vector<StateSet> &dfa_states,
    const std::map<
        std::pair<int, std::string>,
        int
    > &dfa_transitions
) {
    std::ofstream output(filename);

    if (!output.is_open()) {
        std::cerr << "Error: Could not open output file: "
                  << filename << std::endl;
        return;
    }

    // Állapotok
    for (size_t i = 0; i < dfa_states.size(); ++i) {
        if (i > 0) {
            output << ' ';
        }

        output << 's' << i;
    }

    output << '\n';

    // Ábécé
    for (size_t i = 0; i < automaton.alphabet.size(); ++i) {
        if (i > 0) {
            output << ' ';
        }

        output << automaton.alphabet[i];
    }

    output << '\n';

    // Kezdőállapot
    output << "s0\n";

    // Végállapotok
    bool first_accepting = true;

    for (size_t i = 0; i < dfa_states.size(); ++i) {
        if (is_accepting(
                dfa_states[i],
                automaton.accept_states
            )) {

            if (!first_accepting) {
                output << ' ';
            }

            output << 's' << i;
            first_accepting = false;
        }
    }

    output << '\n';

    // Átmenetek:
    // először csomópont,
    // utána élcímke,
    // végül célállapot.
    for (const auto &entry : dfa_transitions) {
        int from = entry.first.first;
        const std::string &symbol = entry.first.second;
        int to = entry.second;

        output << 's' << from
               << ' ' << symbol
               << ' ' << 's' << to
               << '\n';
    }
}

int DeterminizeProblem::run(
    const cxxopts::ParseResult &args
) {
    const std::string input_file =
        args["input"].as<std::string>();

    const std::string output_file =
        args["output"].as<std::string>();

    Automaton automaton;

    if (!read_automaton(input_file, automaton)) {
        return 1;
    }

    std::vector<StateSet> dfa_states;

    std::map<
        std::pair<int, std::string>,
        int
    > dfa_transitions;

    determinize(
        automaton,
        dfa_states,
        dfa_transitions
    );

    write_result(
        output_file,
        automaton,
        dfa_states,
        dfa_transitions
    );

    return 0;
}