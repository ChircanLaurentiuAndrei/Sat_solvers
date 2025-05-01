#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <string>
#include <chrono>

using Clause = std::set<int>;
using ClauseSet = std::set<Clause>;

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

// Try to resolve two clauses. Returns true and sets `resolvent` if resolvable.
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

// Resolution algorithm (single-threaded) with timing
bool resolution_solver(ClauseSet clauses) {
    ClauseSet new_clauses;
    Clause resolvent;

    auto start_time = std::chrono::high_resolution_clock::now();

    while (true) {
        std::vector<Clause> clause_list(clauses.begin(), clauses.end());
        for (size_t i = 0; i < clause_list.size(); ++i) {
            for (size_t j = i + 1; j < clause_list.size(); ++j) {
                if (resolve(clause_list[i], clause_list[j], resolvent)) {
                    if (resolvent.empty()) {
                        auto end_time = std::chrono::high_resolution_clock::now();
                        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                        std::cout << "Derived empty clause. UNSAT.\n";
                        std::cout << "Time: " << ms << " ms\n";
                        return false;
                    }
                    if (clauses.count(resolvent) == 0 && new_clauses.count(resolvent) == 0) {
                        new_clauses.insert(resolvent);
                    }
                }
            }
        }

        if (new_clauses.empty()) {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            std::cout << "No new clauses. SAT.\n";
            std::cout << "Time: " << ms << " ms\n";
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
        std::cerr << "Usage: ./sat_solver_single <file.cnf>\n";
        return 1;
    }

    ClauseSet clauses = parse_dimacs(argv[1]);
    bool result = resolution_solver(clauses);
    std::cout << "Result: " << (result ? "SAT" : "UNSAT") << "\n";
    return 0;
}
