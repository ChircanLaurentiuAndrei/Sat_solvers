#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>

namespace fs = std::filesystem;

using Clause = std::vector<int>;
using CNF = std::vector<Clause>;
using Clock = std::chrono::high_resolution_clock;

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

bool contains_empty_clause(const CNF &cnf) {
    for (const auto &clause : cnf) {
        if (clause.empty()) return true;
    }
    return false;
}

bool contains_var(const CNF &cnf, int var) {
    for (const auto &clause : cnf) {
        for (int lit : clause) {
            if (abs(lit) == var) return true;
        }
    }
    return false;
}

CNF resolve(const CNF &cnf, int var) {
    CNF resolvents;
    std::vector<Clause> pos, neg;

    for (const auto &clause : cnf) {
        bool has_pos = false, has_neg = false;
        for (int lit : clause) {
            if (lit == var) has_pos = true;
            if (lit == -var) has_neg = true;
        }
        if (has_pos && !has_neg) pos.push_back(clause);
        else if (has_neg && !has_pos) neg.push_back(clause);
        else if (!has_pos && !has_neg) resolvents.push_back(clause);
    }

    for (const auto &c1 : pos) {
        for (const auto &c2 : neg) {
            Clause res;
            std::set<int> lits;
            for (int lit : c1) if (lit != var) lits.insert(lit);
            for (int lit : c2) if (lit != -var) lits.insert(lit);
            res.assign(lits.begin(), lits.end());
            resolvents.push_back(res);
        }
    }

    return resolvents;
}

bool dp(const CNF &original_cnf, int num_vars) {
    CNF cnf = original_cnf;

    for (int var = 1; var <= num_vars; ++var) {
        if (!contains_var(cnf, var)) continue;

        cnf = resolve(cnf, var);

        if (contains_empty_clause(cnf)) {
            return false;
        }
    }

    return true;
}

int main() {
    std::string folder = "../../cnf_files/resolution_cnfs/";
    std::ofstream out("results.txt");
    out << std::fixed << std::setprecision(3);

    for (const auto &entry : fs::directory_iterator(folder)) {
        if (entry.path().extension() == ".cnf") {
            std::string file = entry.path().string();
            std::string name = entry.path().filename().string();

            int num_vars;
            CNF cnf = parse_cnf(file, num_vars);

            auto start = Clock::now();
            bool sat = dp(cnf, num_vars);
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
