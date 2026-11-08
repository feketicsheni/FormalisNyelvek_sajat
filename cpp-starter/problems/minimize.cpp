#include "minimize.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <tuple>
#include <vector>

void MinimizeProblem::initialize_parser(cxxopts::Options &options) {
    options.add_options()
        ("mini", "Minimalizalja az automatat");
}

bool MinimizeProblem::is_chosen_problem(
    const cxxopts::ParseResult &args
) {
    return args.count("mini") > 0 &&
           args.count("input") > 0 &&
           args.count("output") > 0;
}

bool MinimizeProblem::read_automaton(
    const std::string &filename,
    Automaton &a
) {
    std::ifstream input(filename);

    if (!input) {
        std::cerr << "Error: Could not open input file: "
                  << filename << '\n';
        return false;
    }

    std::string line, value;

    if (!std::getline(input, line)) return false;
    {
        std::stringstream ss(line);
        while (ss >> value) a.states.push_back(value);
    }

    if (!std::getline(input, line)) return false;
    {
        std::stringstream ss(line);
        while (ss >> value) a.alphabet.push_back(value);
    }

    if (!std::getline(input, line)) return false;
    {
        std::stringstream ss(line);
        ss >> a.start_state;
    }

    if (!std::getline(input, line)) return false;
    {
        std::stringstream ss(line);
        while (ss >> value) a.accept_states.insert(value);
    }

    std::set<std::string> states(a.states.begin(), a.states.end());
    std::set<std::string> alphabet(a.alphabet.begin(), a.alphabet.end());

    if (states.empty() || alphabet.empty()) {
        std::cerr << "Error: Empty states or alphabet\n";
        return false;
    }

    if (!states.count(a.start_state)) {
        std::cerr << "Error: Invalid start state: "
                  << a.start_state << '\n';
        return false;
    }

    for (const auto &s : a.accept_states) {
        if (!states.count(s)) {
            std::cerr << "Error: Invalid accepting state: "
                      << s << '\n';
            return false;
        }
    }

    while (std::getline(input, line)) {
        if (line.find_first_not_of(" \t\r") == std::string::npos)
            continue;

        std::stringstream ss(line);
        std::string from, symbol, to, extra;

        if (!(ss >> from >> symbol >> to) || (ss >> extra)) {
            std::cerr << "Error: Invalid transition: "
                      << line << '\n';
            return false;
        }

        if (!states.count(from) || !states.count(to)) {
            std::cerr << "Error: Invalid transition state\n";
            return false;
        }

        if (!alphabet.count(symbol)) {
            std::cerr << "Error: Invalid transition symbol: "
                      << symbol << '\n';
            return false;
        }

        auto key = std::make_pair(from, symbol);
        auto it = a.transitions.find(key);

        if (it != a.transitions.end() && it->second != to) {
            std::cerr << "Error: Nondeterministic transition\n";
            return false;
        }

        a.transitions[key] = to;
    }

    return true;
}

bool MinimizeProblem::minimize(
    const Automaton &a,
    const std::string &filename
) {
    // Az elerheto allapotok meghatarozasa.
    std::set<std::string> reachable;
    std::queue<std::string> queue;

    reachable.insert(a.start_state);
    queue.push(a.start_state);

    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();

        for (const auto &symbol : a.alphabet) {
            auto it = a.transitions.find({current, symbol});

            if (it != a.transitions.end()) {
                if (reachable.insert(it->second).second)
                    queue.push(it->second);
            }
        }
    }

    std::vector<std::string> states(
        reachable.begin(), reachable.end()
    );

    int n = static_cast<int>(states.size());

    std::map<std::string, int> index;
    for (int i = 0; i < n; ++i)
        index[states[i]] = i;

    // Kezdeti particio: elfogado es nem elfogado allapotok.
    std::vector<int> group(n);

    for (int i = 0; i < n; ++i)
        group[i] = a.accept_states.count(states[i]) ? 1 : 0;

    // Particiok finomitasa, amig stabilak nem lesznek.
    while (true) {
        std::map<std::vector<int>, int> signature_groups;
        std::vector<int> next_group(n);
        int group_count = 0;

        for (int i = 0; i < n; ++i) {
            std::vector<int> signature;

            signature.push_back(
                a.accept_states.count(states[i]) ? 1 : 0
            );

            for (const auto &symbol : a.alphabet) {
                auto it = a.transitions.find({states[i], symbol});

                if (it == a.transitions.end()) {
                    signature.push_back(-1);
                } else {
                    signature.push_back(group[index.at(it->second)]);
                }
            }

            auto result = signature_groups.emplace(
                signature, group_count
            );

            if (result.second)
                ++group_count;

            next_group[i] = result.first->second;
        }

        // A csoportok szamozasa valtozhat,
        // ezert az ekvivalencia-relaciot hasonlitjuk ossze.
        bool same = true;

        for (int i = 0; i < n && same; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if ((group[i] == group[j]) !=
                    (next_group[i] == next_group[j])) {
                    same = false;
                    break;
                }
            }
        }

        group = next_group;

        if (same)
            break;
    }

    // Minden csoporthoz a legkisebb eredeti allapotot rendeljük.
    std::map<int, std::string> representative;

    for (int i = 0; i < n; ++i) {
        int g = group[i];

        if (!representative.count(g) ||
            states[i] < representative[g]) {
            representative[g] = states[i];
        }
    }

    // q0 -> s0, q1 -> s1 stb.
    auto rename = [](const std::string &name) {
        if (!name.empty() && name[0] == 'q')
            return "s" + name.substr(1);

        return "s" + name;
    };

    std::map<int, std::string> new_name;

    for (const auto &entry : representative)
        new_name[entry.first] = rename(entry.second);

    std::set<std::string> output_states;
    std::set<std::string> output_accepting;

    for (const auto &entry : new_name)
        output_states.insert(entry.second);

    for (int i = 0; i < n; ++i) {
        if (a.accept_states.count(states[i]))
            output_accepting.insert(new_name[group[i]]);
    }

    std::set<std::tuple<std::string, std::string, std::string>>
        output_transitions;

    for (int i = 0; i < n; ++i) {
        for (const auto &symbol : a.alphabet) {
            auto it = a.transitions.find({states[i], symbol});

            if (it == a.transitions.end())
                continue;

            std::string from = new_name[group[i]];
            std::string to = new_name[group[index.at(it->second)]];

            output_transitions.insert({from, symbol, to});
        }
    }

    std::ofstream output(filename);

    if (!output) {
        std::cerr << "Error: Could not open output file: "
                  << filename << '\n';
        return false;
    }

    auto write_set = [&](const std::set<std::string> &items) {
        bool first = true;

        for (const auto &item : items) {
            if (!first) output << ' ';
            output << item;
            first = false;
        }

        output << '\n';
    };

    write_set(output_states);

    std::set<std::string> sorted_alphabet(
        a.alphabet.begin(), a.alphabet.end()
    );
    write_set(sorted_alphabet);

    output << new_name[group[index.at(a.start_state)]] << '\n';

    write_set(output_accepting);

    for (const auto &t : output_transitions) {
        output << std::get<0>(t) << ' '
               << std::get<1>(t) << ' '
               << std::get<2>(t) << '\n';
    }

    return static_cast<bool>(output);
}

int MinimizeProblem::run(const cxxopts::ParseResult &args) {
    Automaton automaton;

    std::string input = args["input"].as<std::string>();
    std::string output = args["output"].as<std::string>();

    if (!read_automaton(input, automaton))
        return 1;

    if (!minimize(automaton, output))
        return 1;

    return 0;
}