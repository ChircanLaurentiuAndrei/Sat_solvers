#include <iostream>
#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iomanip>

using namespace std;
namespace fs = std::filesystem;

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

struct Assignment {
    int value = 0;
    int decision_level = -1;
    Clause antecedent;
};

bool unit_propagation(const CNF &clauses, vector<Assignment> &assignments, int decision_level, Clause &conflict_clause) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (const Clause &clause : clauses) {
            int num_unassigned = 0, last_unassigned = 0, num_true = 0;
            for (int lit : clause) {
                int var = abs(lit);
                int val = assignments[var].value;
                if (val == 0) {
                    num_unassigned++;
                    last_unassigned = lit;
                } else if ((val == 1 && lit > 0) || (val == -1 && lit < 0)) {
                    num_true++;
                }
            }
            if (num_true == 0 && num_unassigned == 0) {
                conflict_clause = clause;
                return false;
            }
            if (num_true == 0 && num_unassigned == 1) {
                int var = abs(last_unassigned);
                assignments[var].value = (last_unassigned > 0) ? 1 : -1;
                assignments[var].decision_level = decision_level;
                assignments[var].antecedent = clause;
                changed = true;
            }
        }
    }
    return true;
}

int pick_branching_variable(const vector<Assignment> &assignments) {
    for (int i = 1; i < assignments.size(); ++i) {
        if (assignments[i].value == 0)
            return i;
    }
    return -1;
}

int backtrack_level(const Clause &conflict, const vector<Assignment> &assignments, int &second_level) {
    unordered_set<int> levels;
    for (int lit : conflict) {
        int var = abs(lit);
        levels.insert(assignments[var].decision_level);
    }
    if (levels.empty()) return -1;
    vector<int> level_list(levels.begin(), levels.end());
    sort(level_list.rbegin(), level_list.rend());
    second_level = (level_list.size() > 1) ? level_list[1] : 0;
    return level_list[0];
}

void backtrack(vector<Assignment> &assignments, int level) {
    for (auto &assign : assignments) {
        if (assign.decision_level > level) {
            assign.value = 0;
            assign.decision_level = -1;
            assign.antecedent.clear();
        }
    }
}

bool cdcl(CNF &clauses, int num_vars) {
    vector<Assignment> assignments(num_vars + 1);
    int decision_level = 0;

    Clause conflict_clause;
    if (!unit_propagation(clauses, assignments, decision_level, conflict_clause))
        return false;

    while (true) {
        int var = pick_branching_variable(assignments);
        if (var == -1) return true;

        decision_level++;
        assignments[var].value = 1;
        assignments[var].decision_level = decision_level;

        if (!unit_propagation(clauses, assignments, decision_level, conflict_clause)) {
            int second_level;
            int backtrack_lvl = backtrack_level(conflict_clause, assignments, second_level);
            if (backtrack_lvl < 0) return false;

            backtrack(assignments, second_level);
            decision_level = second_level;

            clauses.push_back(conflict_clause);
        }
    }
}

int main() {
    string folder = "../../cnf_files/";
    ofstream out("results_cdcl.txt");
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
            bool sat = cdcl(cnf, num_vars);
            auto end = chrono::high_resolution_clock::now();

            double ms = chrono::duration<double, milli>(end - start).count();
            out << name << ": " << (sat ? "SAT" : "UNSAT") << " in " << ms << " ms\n";
            cout << name << ": " << (sat ? "SAT" : "UNSAT") << " in " << ms << " ms\n";
        }
    }

    out.close();
    cout << "Results written to results_cdcl.txt\n";
    return 0;
}
