#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <algorithm>

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
        if (!clause.empty())
            formula.push_back(clause);
    }
    return formula;
}

bool contains_empty_clause(const CNF &cnf) {
    for (const auto &clause : cnf)
        if (clause.empty()) return true;
    return false;
}

void remove_clauses_with_literal(CNF &cnf, int lit) {
    cnf.erase(remove_if(cnf.begin(), cnf.end(),
        [lit](const Clause &c) { return c.count(lit); }), cnf.end());
}

void remove_literal_from_clauses(CNF &cnf, int lit) {
    for (auto &clause : cnf)
        clause.erase(lit);
}

bool unit_propagate(CNF &cnf, unordered_set<int> &assignments) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto it = cnf.begin(); it != cnf.end(); ) {
            if (it->size() == 1) {
                int unit = *it->begin();
                assignments.insert(unit);
                remove_clauses_with_literal(cnf, unit);
                remove_literal_from_clauses(cnf, -unit);
                changed = true;
                it = cnf.begin(); // start over
            } else {
                ++it;
            }
        }
    }
    return !contains_empty_clause(cnf);
}

void eliminate_pure_literals(CNF &cnf, unordered_set<int> &assignments) {
    unordered_map<int, int> literal_count;
    for (const auto &clause : cnf)
        for (int lit : clause)
            literal_count[lit]++;
    for (const auto &[lit, _] : literal_count) {
        if (!literal_count.count(-lit)) {
            assignments.insert(lit);
            remove_clauses_with_literal(cnf, lit);
        }
    }
}

CNF resolve_on_variable(const CNF &cnf, int var) {
    CNF pos_clauses, neg_clauses, rest;
    for (const auto &clause : cnf) {
        if (clause.count(var))
            pos_clauses.push_back(clause);
        else if (clause.count(-var))
            neg_clauses.push_back(clause);
        else
            rest.push_back(clause);
    }

    CNF resolvents = rest;
    for (const auto &c1 : pos_clauses) {
        for (const auto &c2 : neg_clauses) {
            Clause resolvent;
            for (int l : c1) if (l != var) resolvent.insert(l);
            for (int l : c2) if (l != -var) resolvent.insert(l);
            resolvents.push_back(resolvent);
        }
    }
    return resolvents;
}

bool davis_putnam(CNF cnf, unordered_set<int> &assignments) {
    if (contains_empty_clause(cnf)) return false;
    if (cnf.empty()) return true;

    if (!unit_propagate(cnf, assignments)) return false;
    eliminate_pure_literals(cnf, assignments);
    if (cnf.empty()) return true;

    unordered_set<int> vars;
    for (const auto &clause : cnf)
        for (int lit : clause)
            vars.insert(abs(lit));

    int var = *vars.begin();  // pick arbitrary variable
    CNF reduced = resolve_on_variable(cnf, var);
    return davis_putnam(reduced, assignments);
}

int main() {
    string folder = "../../cnf_files/samples/";
    ofstream out("results_davis_putnam.txt");
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
            unordered_set<int> assignments;

            auto start = chrono::high_resolution_clock::now();
            bool sat = davis_putnam(cnf, assignments);
            auto end = chrono::high_resolution_clock::now();

            double ms = chrono::duration<double, milli>(end - start).count();
            out << name << ": " << (sat ? "SAT" : "UNSAT") << " in " << ms << " ms\n";
            cout << name << ": " << (sat ? "SAT" : "UNSAT") << " in " << ms << " ms\n";
        }
    }

    out.close();
    cout << "Results written to results_davis_putnam.txt\n";
    return 0;
}
