#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <thread>
#include <mutex>
#include <algorithm>

using Clause = std::set<int>;
using ClauseSet = std::set<Clause>;
using Clock = std::chrono::high_resolution_clock;
std::mutex resolvent_mutex;


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


bool unit_propagate(ClauseSet &clauses) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto &clause : clauses) {
            if (clause.size() == 1) {
                int unit = *clause.begin();
                ClauseSet new_clauses;

                for (const auto &c : clauses) {
                    if (c.count(unit)) continue;
                    if (c.count(-unit)) {
                        Clause new_c = c;
                        new_c.erase(-unit);
                        if (new_c.empty()) return false;
                        new_clauses.insert(new_c);
                    } else {
                        new_clauses.insert(c);
                    }
                }

                clauses = std::move(new_clauses);
                changed = true;
                break;
            }
        }
    }
    return true;
}


void resolve_worker(const std::vector<Clause> &pos, const std::vector<Clause> &neg,
                    size_t start, size_t end, ClauseSet &local_resolvents, bool &found_empty) {
    for (size_t i = start; i < end && !found_empty; ++i) {
        for (size_t j = 0; j < neg.size(); ++j) {
            Clause resolvent;
            for (int l : pos[i]) if (l != *pos[i].begin()) resolvent.insert(l);
            for (int l : neg[j]) if (l != -*pos[i].begin()) resolvent.insert(l);
            if (resolvent.empty()) {
                found_empty = true;
                return;
            }
            std::lock_guard<std::mutex> lock(resolvent_mutex);
            local_resolvents.insert(resolvent);
        }
    }
}


bool dp_solver_mt(ClauseSet clauses, int num_threads, double &solving_time_ms) {
    auto start = Clock::now();

    if (!unit_propagate(clauses)) {
        solving_time_ms = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
        return false;
    }

    while (!clauses.empty()) {
        int chosen_var = *clauses.begin()->begin();

        ClauseSet pos_set, neg_set, rest;
        for (const auto &clause : clauses) {
            if (clause.count(chosen_var)) pos_set.insert(clause);
            else if (clause.count(-chosen_var)) neg_set.insert(clause);
            else rest.insert(clause);
        }

        std::vector<Clause> pos(pos_set.begin(), pos_set.end());
        std::vector<Clause> neg(neg_set.begin(), neg_set.end());

        ClauseSet resolvents;
        std::vector<std::thread> threads;
        bool found_empty = false;

        size_t chunk_size = pos.size() / num_threads + 1;

        for (int t = 0; t < num_threads; ++t) {
            size_t start_i = t * chunk_size;
            size_t end_i = std::min(start_i + chunk_size, pos.size());

            threads.emplace_back(resolve_worker, std::cref(pos), std::cref(neg),
                                 start_i, end_i, std::ref(resolvents), std::ref(found_empty));
        }

        for (auto &th : threads) th.join();

        if (found_empty) {
            solving_time_ms = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
            return false;
        }

        clauses = rest;
        clauses.insert(resolvents.begin(), resolvents.end());

        if (!unit_propagate(clauses)) {
            solving_time_ms = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
            return false;
        }
    }

    solving_time_ms = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
    return true;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./dp_sat_solver <file.cnf> [threads]\n";
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
    bool result = dp_solver_mt(clauses, num_threads, solving_time);

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
