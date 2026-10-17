#ifndef DETERMINIZE_HPP
#define DETERMINIZE_HPP

#include "../problem.hpp"

#include <map>
#include <set>
#include <string>
#include <vector>

class DeterminizeProblem : public Problem {
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
            std::set<std::string>
        > transitions;
    };

    using StateSet = std::set<std::string>;

    bool read_automaton(
        const std::string &filename,
        Automaton &automaton
    );

    bool is_valid_automaton(const Automaton &automaton);

    void determinize(
        const Automaton &automaton,
        std::vector<StateSet> &dfa_states,
        std::map<
            std::pair<int, std::string>,
            int
        > &dfa_transitions
    );

    bool is_accepting(
        const StateSet &state_set,
        const std::set<std::string> &accept_states
    ) const;

    void write_result(
        const std::string &filename,
        const Automaton &automaton,
        const std::vector<StateSet> &dfa_states,
        const std::map<
            std::pair<int, std::string>,
            int
        > &dfa_transitions
    );
};

#endif // DETERMINIZE_HPP