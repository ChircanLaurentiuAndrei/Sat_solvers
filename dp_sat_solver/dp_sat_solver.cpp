#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <algorithm>

using Clause = std::set<int>;
using ClauseSet = std::set<Clause>;
using Clock = std::chrono::high_resolution_clock;

// Parse DIMACS CNF file
ClauseSet parse_dimacs(const std::string &filename) {
    std::ifstream infile(filename);
    std::string line;
    ClauseSet clauses;

    while (getline(infile, line)) {
        if (line.empty() || line[0] == 'c' || line[0] == 'p') continue;

        std::istringstream iss(line);
        int lit;
        Clause clause;
        while (iss >> lit && lit != 0) {
            clause.insert(lit);
        }
        if (!clause.empty())
            clauses.insert(clause);
    }
    return clauses;
}

// Apply unit propagation
bool unit_propagate(ClauseSet &clauses) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto &clause : clauses) {
            if (clause.size() == 1) {
                int unit = *clause.begin();
                ClauseSet new_clauses;

                for (const auto &c : clauses) {
                    if (c.count(unit)) continue;
                    if (c.count(-unit)) {
                        Clause new_c = c;
                        new_c.erase(-unit);
                        if (new_c.empty()) return false;
                        new_clauses.insert(new_c);
                    } else {
                        new_clauses.insert(c);
                    }
                }

                clauses = std::move(new_clauses);
                changed = true;
                break;
            }
        }
    }
    return true;
}

// DP solver (eliminates variables)
bool dp_solver(ClauseSet clauses, double &solving_time_ms) {
    auto start = Clock::now();

    if (!unit_propagate(clauses)) {
        solving_time_ms = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
        return false;
    }

    while (!clauses.empty()) {
        // Select the first literal from the first clause
        int chosen_var = *clauses.begin()->begin();

        ClauseSet pos, neg, rest;
        for (const auto &clause : clauses) {
            if (clause.count(chosen_var)) pos.insert(clause);
            else if (clause.count(-chosen_var)) neg.insert(clause);
            else rest.insert(clause);
        }

        ClauseSet resolvents;
        for (const auto &c1 : pos) {
            for (const auto &c2 : neg) {
                Clause resolvent;
                for (int l : c1) if (l != chosen_var) resolvent.insert(l);
                for (int l : c2) if (l != -chosen_var) resolvent.insert(l);
                if (resolvent.empty()) {
                    solving_time_ms = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
                    return false;
                }
                resolvents.insert(resolvent);
            }
        }

        clauses = rest;
        clauses.insert(resolvents.begin(), resolvents.end());

        if (!unit_propagate(clauses)) {
            solving_time_ms = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
            return false;
        }
    }

    solving_time_ms = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
    return true;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./dp_sat_solver <file.cnf>\n";
        return 1;
    }

    auto total_start = Clock::now();

    auto parse_start = Clock::now();
    ClauseSet clauses = parse_dimacs(argv[1]);
    auto parse_end = Clock::now();
    double parsing_time = std::chrono::duration<double, std::milli>(parse_end - parse_start).count();

    double solving_time = 0;
    bool result = dp_solver(clauses, solving_time);

    auto total_end = Clock::now();
    double total_time = std::chrono::duration<double, std::milli>(total_end - total_start).count();

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Result: " << (result ? "SAT" : "UNSAT") << "\n";
    std::cout << "Timing Breakdown:\n";
    std::cout << "  Parsing Time: " << parsing_time << " ms\n";
    std::cout << "  Solving Time: " << solving_time << " ms\n";
    std::cout << "  Total Time:   " << total_time << " ms\n";

    return 0;
}
