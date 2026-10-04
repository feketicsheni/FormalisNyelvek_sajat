#ifndef DFA_CHECKER_HPP
#define DFA_CHECKER_HPP

#include "../problem.hpp"
#include <string>
#include <vector>
#include <set>
#include <map>

class DFAChecker : public Problem {
public:
    void initialize_parser(cxxopts::Options &options) override;
    bool is_chosen_problem(const cxxopts::ParseResult &args) override;
    int run(const cxxopts::ParseResult &args) override;

private:
    std::set<std::string> states;
    std::set<char> alphabet;
    std::string start_state;
    std::set<std::string> accept_states;
    std::map<std::pair<std::string, char>, std::string> transitions;

    void parseInputFile(const std::string &filePath);
    std::string simulate(const std::string &word);
    std::vector<std::string> split(const std::string &str, char delimiter);
};

#endif // DFA_CHECKER_HPP