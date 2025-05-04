#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>

using Clause = std::set<int>;
using ClauseSet = std::set<Clause>;
using Clock = std::chrono::high_resolution_clock;

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

bool resolve(const Clause &c1, const Clause &c2, Clause &resolvent) {
    for (int lit : c1) {
        if (c2.count(-lit)) {
            resolvent.clear();
            for (int l : c1) if (l != lit) resolvent.insert(l);
            for (int l : c2) if (l != -lit) resolvent.insert(l);
            return true;
        }
    }
    return false;
}


bool resolution_solver(ClauseSet clauses, double &solving_time_ms) {
    ClauseSet new_clauses;
    auto start_time = Clock::now();

    while (true) {
        std::vector<Clause> clause_list(clauses.begin(), clauses.end());
        bool found_new = false;

        for (size_t i = 0; i < clause_list.size(); ++i) {
            for (size_t j = i + 1; j < clause_list.size(); ++j) {
                Clause resolvent;
                if (resolve(clause_list[i], clause_list[j], resolvent)) {
                    if (resolvent.empty()) {
                        solving_time_ms = std::chrono::duration<double, std::milli>(Clock::now() - start_time).count();
                        std::cout << "Derived empty clause. UNSAT.\n";
                        return false;
                    }
                    if (clauses.count(resolvent) == 0 && new_clauses.count(resolvent) == 0) {
                        new_clauses.insert(resolvent);
                        found_new = true;
                    }
                }
            }
        }

        if (!found_new) {
            solving_time_ms = std::chrono::duration<double, std::milli>(Clock::now() - start_time).count();
            std::cout << "No new clauses. SAT.\n";
            return true;
        }

        for (const Clause &c : new_clauses) {
            clauses.insert(c);
        }
        new_clauses.clear();
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./sat_solver <file.cnf>\n";
        return 1;
    }

    auto total_start = Clock::now();

    auto parse_start = Clock::now();
    ClauseSet clauses = parse_dimacs(argv[1]);
    auto parse_end = Clock::now();
    double parsing_time = std::chrono::duration<double, std::milli>(parse_end - parse_start).count();

    double solving_time = 0;
    bool result = resolution_solver(clauses, solving_time);

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
