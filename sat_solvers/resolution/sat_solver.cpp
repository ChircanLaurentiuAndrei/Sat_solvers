#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <set>
#include <unordered_set>
#include <chrono>
#include <filesystem>
#include <iomanip>

namespace fs = std::filesystem;
using namespace std;

using Clause = set<int>;
using CNF = vector<Clause>;

CNF parse_cnf(const string &filename) {
    ifstream file(filename);
    string line;
    CNF formula;

    while (getline(file, line)) {
        if (line.empty() || line[0] == 'c') continue;
        if (line[0] == 'p') continue;

        istringstream iss(line);
        int lit;
        Clause clause;
        while (iss >> lit && lit != 0) clause.insert(lit);
        formula.push_back(clause);
    }
    return formula;
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

bool resolution_algorithm(CNF formula) {
    set<Clause> clauses(formula.begin(), formula.end());
    bool added = true;

    while (added) {
        added = false;
        vector<Clause> new_clauses;
        for (auto it1 = clauses.begin(); it1 != clauses.end(); ++it1) {
            for (auto it2 = next(it1); it2 != clauses.end(); ++it2) {
                Clause resolvent;
                if (resolve(*it1, *it2, resolvent)) {
                    if (resolvent.empty()) return false;
                    if (!clauses.count(resolvent)) {
                        new_clauses.push_back(resolvent);
                    }
                }
            }
        }
        for (auto &c : new_clauses) {
            clauses.insert(c);
            added = true;
        }
    }
    return true;
}

int main() {
    string folder = "../../cnf_files/resolution_cnfs/";
    ofstream out("results_resolution.txt");
    out << fixed << setprecision(3);

    if (!fs::exists(folder) || !fs::is_directory(folder)) {
        cerr << "Error: directory not found: " << folder << "\n";
        return 1;
    }

    for (const auto &entry : fs::directory_iterator(folder)) {
        if (entry.path().extension() == ".cnf") {
            string file = entry.path().string();
            string name = entry.path().filename().string();

            CNF cnf = parse_cnf(file);

            auto start = chrono::high_resolution_clock::now();
            bool sat = resolution_algorithm(cnf);
            auto end = chrono::high_resolution_clock::now();

            double ms = chrono::duration<double, milli>(end - start).count();
            out << name << ": " << (sat ? "SAT" : "UNSAT") << " in " << ms << " ms\n";
            cout << name << ": " << (sat ? "SAT" : "UNSAT") << " in " << ms << " ms\n";
        }
    }

    out.close();
    cout << "Results written to results_resolution.txt\n";
    return 0;
}
