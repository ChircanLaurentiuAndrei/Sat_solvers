#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

using Clause = std::vector<int>;
using CNF = std::vector<Clause>;
using Assignment = std::map<int, bool>;
using Clock = std::chrono::high_resolution_clock;
using Trail = std::vector<std::pair<int, bool>>;

CNF parse_cnf(const std::string& filename, int &num_vars) {
    std::ifstream file(filename);
    std::string line;
    CNF formula;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == 'c') continue;
        if (line[0] == 'p') {
            std::istringstream iss(line);
            std::string tmp;
            int num_clauses;
            iss >> tmp >> tmp >> num_vars >> num_clauses;
        } else {
            std::istringstream iss(line);
            int lit;
            Clause clause;
            while (iss >> lit && lit != 0) {
                clause.push_back(lit);
            }
            formula.push_back(clause);
        }
    }

    return formula;
}

CNF simplify(const CNF &cnf, int var, bool value) {
    CNF new_cnf;
    for (const auto &clause : cnf) {
        Clause new_clause;
        bool satisfied = false;

        for (int lit : clause) {
            if ((lit > 0 && var == lit && value) || (lit < 0 && var == -lit && !value)) {
                satisfied = true;
                break;
            } else if ((lit > 0 && var == lit && !value) || (lit < 0 && var == -lit && value)) {
                continue;
            } else {
                new_clause.push_back(lit);
            }
        }

        if (!satisfied) {
            if (new_clause.empty()) return {{}};
            new_cnf.push_back(new_clause);
        }
    }
    return new_cnf;
}

bool unit_propagate(CNF &cnf, Assignment &assignment, Trail &trail) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto it = cnf.begin(); it != cnf.end(); ) {
            if (it->size() == 1) {
                int unit = (*it)[0];
                int var = abs(unit);
                bool value = unit > 0;

                if (assignment.count(var) && assignment[var] != value) return false;

                assignment[var] = value;
                trail.push_back({var, value});
                cnf = simplify(cnf, var, value);
                changed = true;
                break;
            } else {
                ++it;
            }
        }
    }
    return true;
}

void analyze_conflict(const CNF &cnf, Trail &trail, CNF &learned_clauses) {
    if (!trail.empty()) {
        Clause conflict_clause;
        for (const auto &[var, value] : trail) {
            conflict_clause.push_back(value ? -var : var);
        }
        learned_clauses.push_back(conflict_clause);
        trail.clear();
    }
}

bool cdcl(CNF cnf, CNF &learned_clauses, Assignment &assignment, Trail &trail) {
    if (!unit_propagate(cnf, assignment, trail)) return false;

    if (cnf.empty()) return true;
    for (const auto &clause : cnf) {
        if (clause.empty()) return false;
    }

    int var = 0;
    for (const auto &clause : cnf) {
        for (int lit : clause) {
            var = abs(lit);
            if (!assignment.count(var)) break;
        }
        if (var != 0) break;
    }

    trail.push_back({var, true});
    CNF cnf_copy = simplify(cnf, var, true);
    assignment[var] = true;
    if (cdcl(cnf_copy, learned_clauses, assignment, trail)) return true;

    analyze_conflict(cnf, trail, learned_clauses);

    trail.push_back({var, false});
    cnf_copy = simplify(cnf, var, false);
    assignment[var] = false;
    if (cdcl(cnf_copy, learned_clauses, assignment, trail)) return true;

    return false;
}

int main() {
    std::string folder = "../../cnf_files/cnfs/";
    std::ofstream out("results.txt");
    out << std::fixed << std::setprecision(3);

    for (const auto &entry : fs::directory_iterator(folder)) {
        if (entry.path().extension() == ".cnf") {
            std::string file = entry.path().string();
            std::string name = entry.path().filename().string();

            int num_vars;
            CNF cnf = parse_cnf(file, num_vars);
            Assignment assignment;
            CNF learned_clauses;
            Trail trail;

            auto start = Clock::now();
            bool sat = cdcl(cnf, learned_clauses, assignment, trail);
            auto end = Clock::now();

            double ms = std::chrono::duration<double, std::milli>(end - start).count();
            out << name << ": " << (sat ? "SAT" : "UNSAT") << " in " << ms << " ms\n";
            std::cout << name << ": " << (sat ? "SAT" : "UNSAT") << " in " << ms << " ms\n";
        }
    }

    out.close();
    std::cout << "Results written to results.txt\n";
    return 0;
}