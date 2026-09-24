#ifndef DFA_HPP
#define DFA_HPP

#include "../problem.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class DFAProblem : public Problem {
public:
    void initialize_parser(cxxopts::Options& options) override;
    bool is_chosen_problem(const cxxopts::ParseResult& args) override;
    int run(const cxxopts::ParseResult& args) override;

private:
    struct DFA {
        std::string start_state;
        std::unordered_set<std::string> accept_states;
        // transitions[state][symbol] = next_state
        std::unordered_map<std::string, std::unordered_map<char, std::string>> transitions;

        bool simulate(const std::string& word) const;
    };

    DFA parse_input_file(const std::string& filepath);
    std::vector<std::string> split_words(const std::string& str, char delimiter);
};

#endif // DFA_HPP