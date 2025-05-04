#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <iomanip>

using Clause = std::set<int>;
using ClauseSet = std::set<Clause>;
std::mutex resolvent_mutex;
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


void resolve_worker(const std::vector<Clause> &clauses, size_t start, size_t end, ClauseSet &local_new_clauses, const ClauseSet &global_clauses) {
    Clause resolvent;
    for (size_t i = start; i < end; ++i) {
        for (size_t j = i + 1; j < clauses.size(); ++j) {
            if (resolve(clauses[i], clauses[j], resolvent)) {
                if (resolvent.empty()) {
                    std::lock_guard<std::mutex> lock(resolvent_mutex);
                    local_new_clauses.insert(resolvent);
                    return; // 
                }
                if (global_clauses.count(resolvent) == 0 && local_new_clauses.count(resolvent) == 0) {
                    std::lock_guard<std::mutex> lock(resolvent_mutex);
                    local_new_clauses.insert(resolvent);
                }
            }
        }
    }
}


bool resolution_solver(ClauseSet clauses, int num_threads, double &solving_time_ms) {
    ClauseSet new_clauses;
    auto start_time = Clock::now();

    while (true) {
        std::vector<Clause> clause_list(clauses.begin(), clauses.end());
        std::vector<std::thread> threads;
        std::vector<ClauseSet> local_new_clauses(num_threads);

        size_t chunk_size = clause_list.size() / num_threads + 1;

        for (int t = 0; t < num_threads; ++t) {
            size_t start = t * chunk_size;
            size_t end = std::min(start + chunk_size, clause_list.size());

            threads.emplace_back([&, start, end, t]() {
                resolve_worker(clause_list, start, end, local_new_clauses[t], clauses);
            });
        }

        for (auto &thread : threads) thread.join();

        for (const auto &local : local_new_clauses) {
            for (const auto &cl : local) {
                if (cl.empty()) {
                    solving_time_ms = std::chrono::duration<double, std::milli>(Clock::now() - start_time).count();
                    std::cout << "Derived empty clause. UNSAT.\n";
                    return false;
                }
                if (clauses.count(cl) == 0 && new_clauses.count(cl) == 0) {
                    new_clauses.insert(cl);
                }
            }
        }

        if (new_clauses.empty()) {
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
        std::cerr << "Usage: ./sat_solver_mt <file.cnf> [threads]\n";
        return 1;
    }

    int num_threads = std::thread::hardware_concurrency();
    if (argc >= 3) {
        num_threads = std::stoi(argv[2]);
    }

    auto total_start = Clock::now();

    auto parse_start = Clock::now();
    ClauseSet clauses = parse_dimacs(argv[1]);
    auto parse_end = Clock::now();
    double parsing_time = std::chrono::duration<double, std::milli>(parse_end - parse_start).count();

    double solving_time = 0;
    bool result = resolution_solver(clauses, num_threads, solving_time);

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
