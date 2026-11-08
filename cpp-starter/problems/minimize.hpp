#ifndef MINIMIZE_HPP
#define MINIMIZE_HPP

#include "../problem.hpp"

#include <map>
#include <set>
#include <string>
#include <vector>

class MinimizeProblem : public Problem {
public:
    void initialize_parser(cxxopts::Options &options) override;
    bool is_chosen_problem(const cxxopts::ParseResult &args) override;
    int run(const cxxopts::ParseResult &args) override;

private:
    struct Automaton {
        std::vector<std::string> states;
        std::vector<std::string> alphabet;
        std::string start_state;
        std::set<std::string> accept_states;

        std::map<
            std::pair<std::string, std::string>,
            std::string
        > transitions;
    };

    bool read_automaton(
        const std::string &filename,
        Automaton &automaton
    );

    bool minimize(
        const Automaton &automaton,
        const std::string &filename
    );
};

#endif