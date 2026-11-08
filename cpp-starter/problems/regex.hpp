#ifndef REGEX_HPP
#define REGEX_HPP

#include "../problem.hpp"
#include <string>
#include <vector>

class RegexProblem : public Problem {
public:
    void initialize_parser(cxxopts::Options &options) override;
    bool is_chosen_problem(const cxxopts::ParseResult &args) override;
    int run(const cxxopts::ParseResult &args) override;

private:
    void solve1(const std::vector<std::string> &lines,
                std::ostream &out);
    void solve2(const std::vector<std::string> &lines,
                std::ostream &out);
    void solve3(const std::vector<std::string> &lines,
                std::ostream &out);
    void solve4(const std::vector<std::string> &lines,
                std::ostream &out);
    void solve5(const std::vector<std::string> &lines,
                std::ostream &out);
};

#endif