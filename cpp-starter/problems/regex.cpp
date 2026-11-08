#include "regex.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <map>
#include <set>
#include <vector>
#include <string>

using namespace std;

namespace {
    // Android logcat fejléc:
    // dátum idő PID TID szint tag: üzenet
    const regex logPattern(
        R"(^([0-9]{2}-[0-9]{2})\s+([0-9]{2}:[0-9]{2}:[0-9]{2})\.[0-9]+\s+([0-9]+)\s+([0-9]+)\s+([IDVEFAW])\s+(.+?):(?:\s*(.*))?$)"
    );

    bool parseLog(const string &line, smatch &match) {
        return regex_search(line, match, logPattern);
    }

    bool contains(const string &value, const string &pattern) {
        return regex_search(value, regex(pattern));
    }
}

void RegexProblem::initialize_parser(cxxopts::Options &options) {
    options.add_options()
        ("rgx", "Regex feladat szama",
         cxxopts::value<int>());
}

bool RegexProblem::is_chosen_problem(
    const cxxopts::ParseResult &args
) {
    return args.count("rgx") > 0;
}

int RegexProblem::run(const cxxopts::ParseResult &args) {
    if (!args.count("input") || !args.count("output")) {
        cerr << "Error: Missing input or output file\n";
        return 1;
    }

    string inputFile = args["input"].as<string>();
    string outputFile = args["output"].as<string>();
    int task = args["rgx"].as<int>();

    ifstream input(inputFile);

    if (!input) {
        cerr << "Error: Cannot open input file\n";
        return 1;
    }

    vector<string> lines;
    string line;

    while (getline(input, line)) {
        lines.push_back(line);
    }

    ofstream output(outputFile);

    if (!output) {
        cerr << "Error: Cannot open output file\n";
        return 1;
    }

    switch (task) {
        case 1: solve1(lines, output); break;
        case 2: solve2(lines, output); break;
        case 3: solve3(lines, output); break;
        case 4: solve4(lines, output); break;
        case 5: solve5(lines, output); break;
        default:
            cerr << "Error: Invalid regex task\n";
            return 1;
    }

    return 0;
}

// 1. feladat: 10 feltetel megszamlalasa



void RegexProblem::solve1(
    const vector<string> &lines, ostream &out
) {
    vector<int> count(10, 0);

    const vector<regex> patterns = {
        regex(R"(^10-15\s)"),
        regex(R"(^10-15\s+10:18:51(?:\.[0-9]+)?\s)"),
        regex(R"(^[0-9]{2}-[0-9]{2}\s+[0-9]{2}:[0-9]{2}:51(?:\.[0-9]+)?\s)"),
        regex(R"(\s+lowmemorykiller\s*:)"),
        regex(R"(^[0-9]{2}-[0-9]{2}\s+10:18:51(?:\.[0-9]+)?\s+221\s)"),
        regex(R"(\sE\s+[^:]+:)"),
        regex(R"(\sW\s+PackageManager\s*:)"),
        regex(R"(\sD\s+ExoPlayer\s*:)"),
        regex(R"(\s[EW]\s+.*\.java:)"),
        regex(R"(\s[IDVEFAW]\s+[^:]+:\s*.*Thread)")
    };

    for (const string &line : lines) {
        for (size_t i = 0; i < patterns.size(); ++i) {
            if (regex_search(line, patterns[i]))
                count[i]++;
        }
    }

    for (int value : count)
        out << value << '\n';
}




// 2. feladat: Java fajlnevek a hibakbol
void RegexProblem::solve2(
    const vector<string> &lines, ostream &out
) {
    const regex javaPattern(R"(([A-Za-z0-9_$]+\.java):[0-9]+)");
    bool errorContext = false;

    for (const string &line : lines) {
        smatch m;

        if (parseLog(line, m)) {
            errorContext = (m[5].str() == "E");
        }

        if (!errorContext)
            continue;

        auto begin = sregex_iterator(
            line.begin(), line.end(), javaPattern
        );
        auto end = sregex_iterator();

        for (auto it = begin; it != end; ++it)
            out << (*it)[1].str() << '\n';
    }
}

// 3. feladat: egyedi PID-k elofordulasi sorrendben
void RegexProblem::solve3(
    const vector<string> &lines, ostream &out
) {
    set<string> seen;

    for (const string &line : lines) {
        smatch m;

        if (!parseLog(line, m))
            continue;

        string pid = m[3].str();

        if (seen.insert(pid).second)
            out << pid << '\n';
    }
}

// 4. feladat: legtobb hibat okozo process
void RegexProblem::solve4(
    const vector<string> &lines, ostream &out
) {
    map<string, int> errors;
    vector<string> order;

    for (const string &line : lines) {
        smatch m;

        if (!parseLog(line, m))
            continue;

        if (m[5].str() != "E")
            continue;

        string pid = m[3].str();

        if (!errors.count(pid))
            order.push_back(pid);

        errors[pid]++;
    }

    string bestPid;
    int maximum = 0;

    for (const string &pid : order) {
        if (errors[pid] > maximum) {
            maximum = errors[pid];
            bestPid = pid;
        }
    }

    if (!bestPid.empty())
        out << "PID=" << bestPid
            << " (" << maximum << " errors)\n";
}

// 5. feladat: hibak a fo szalon
void RegexProblem::solve5(
    const vector<string> &lines, ostream &out
) {
    int count = 0;

    for (const string &line : lines) {
        smatch m;

        if (!parseLog(line, m))
            continue;

        if (m[5].str() == "E" &&
            m[3].str() == m[4].str()) {
            count++;
        }
    }

    out << count << '\n';
}