#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>

using Clause = std::set<int>;
using ClauseSet = std::set<Clause>;

std::mutex resolvent_mutex;

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

// Threaded clause resolver
void resolve_worker(const std::vector<Clause> &clauses, size_t start, size_t end, ClauseSet &local_new_clauses, const ClauseSet &global_clauses) {
    Clause resolvent;

    for (size_t i = start; i < end; ++i) {
        for (size_t j = i + 1; j < clauses.size(); ++j) {
            if (resolve(clauses[i], clauses[j], resolvent)) {
                if (resolvent.empty()) {
                    std::lock_guard<std::mutex> lock(resolvent_mutex);
                    local_new_clauses.insert(resolvent);
                    return; // Empty clause means UNSAT
                }
                if (global_clauses.count(resolvent) == 0 && local_new_clauses.count(resolvent) == 0) {
                    std::lock_guard<std::mutex> lock(resolvent_mutex);
                    local_new_clauses.insert(resolvent);
                }
            }
        }
    }
}

// Resolution algorithm with multithreading
bool resolution_solver(ClauseSet clauses, int num_threads) {
    ClauseSet new_clauses;
    auto start_time = std::chrono::high_resolution_clock::now();

    while (true) {
        std::vector<Clause> clause_list(clauses.begin(), clauses.end());
        std::vector<std::thread> threads;
        std::vector<ClauseSet> local_new_clauses(num_threads);

        size_t chunk_size = clause_list.size() / num_threads + 1;
        bool empty_clause_found = false;

        for (int t = 0; t < num_threads; ++t) {
            size_t start = t * chunk_size;
            size_t end = std::min(start + chunk_size, clause_list.size());

            threads.emplace_back([&, start, end, t]() {
                resolve_worker(clause_list, start, end, local_new_clauses[t], clauses);
            });
        }

        for (auto &thread : threads) {
            thread.join();
        }

        for (const auto &local : local_new_clauses) {
            for (const auto &cl : local) {
                if (cl.empty()) {
                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                    std::cout << "Derived empty clause. UNSAT.\n";
                    std::cout << "Time: " << ms << " ms\n";
                    return false;
                }
                if (clauses.count(cl) == 0 && new_clauses.count(cl) == 0) {
                    new_clauses.insert(cl);
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
        std::cerr << "Usage: ./sat_solver <file.cnf> [threads]\n";
        return 1;
    }

    int num_threads = std::thread::hardware_concurrency();
    if (argc >= 3) {
        num_threads = std::stoi(argv[2]);
    }

    ClauseSet clauses = parse_dimacs(argv[1]);
    bool result = resolution_solver(clauses, num_threads);
    std::cout << "Result: " << (result ? "SAT" : "UNSAT") << "\n";
    return 0;
}
