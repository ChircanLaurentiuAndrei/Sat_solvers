#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <set>
#include <unordered_map>
#include <chrono>
#include <filesystem>
#include <iomanip>

namespace fs = std::filesystem;
using namespace std;

using Clause = vector<int>;
using CNF = vector<Clause>;

CNF parse_cnf(const string &filename, int &num_vars) {
    ifstream file(filename);
    string line;
    CNF formula;

    while (getline(file, line)) {
        if (line.empty() || line[0] == 'c') continue;
        if (line[0] == 'p') {
            istringstream iss(line);
            string tmp;
            int num_clauses;
            iss >> tmp >> tmp >> num_vars >> num_clauses;
        } else {
            istringstream iss(line);
            int lit;
            Clause clause;
            while (iss >> lit && lit != 0) clause.push_back(lit);
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

bool dp_algorithm(CNF cnf, int num_vars) {
    for (int var = 1; var <= num_vars; ++var) {
        CNF pos_clauses, neg_clauses, others;

        for (auto &clause : cnf) {
            bool has_pos = false, has_neg = false;
            Clause reduced;
            for (int lit : clause) {
                if (lit == var) has_pos = true;
                else if (lit == -var) has_neg = true;
                else reduced.push_back(lit);
            }
            if (has_pos && has_neg) continue;
            if (has_pos) pos_clauses.push_back(reduced);
            else if (has_neg) neg_clauses.push_back(reduced);
            else others.push_back(clause);
        }

        CNF resolvents;
        for (auto &c1 : pos_clauses) {
            for (auto &c2 : neg_clauses) {
                set<int> lits(c1.begin(), c1.end());
                lits.insert(c2.begin(), c2.end());
                Clause res(lits.begin(), lits.end());
                resolvents.push_back(res);
            }
        }

        cnf = others;
        cnf.insert(cnf.end(), resolvents.begin(), resolvents.end());
        if (contains_empty_clause(cnf)) return false;
    }
    return true;
}

int main() {
    string folder = "../../cnf_files/resolution_cnfs/";
    ofstream out("results_dp.txt");
    out << fixed << setprecision(3);

    if (!fs::exists(folder) || !fs::is_directory(folder)) {
        cerr << "Error: directory not found: " << folder << "\n";
        return 1;
    }

    for (const auto &entry : fs::directory_iterator(folder)) {
        if (entry.path().extension() == ".cnf") {
            string file = entry.path().string();
            string name = entry.path().filename().string();

            int num_vars;
            CNF cnf = parse_cnf(file, num_vars);

            auto start = chrono::high_resolution_clock::now();
            bool sat = dp_algorithm(cnf, num_vars);
            auto end = chrono::high_resolution_clock::now();

            double ms = chrono::duration<double, milli>(end - start).count();
            out << name << ": " << (sat ? "SAT" : "UNSAT") << " in " << ms << " ms\n";
            cout << name << ": " << (sat ? "SAT" : "UNSAT") << " in " << ms << " ms\n";
        }
    }

    out.close();
    cout << "Results written to results_dp.txt\n";
    return 0;
}
