 #include <algorithm>
 #include <cmath>
 #include <iostream>
 #include <random>
 #include <vector>
 #include <fstream>
 #include <chrono>
 #include <iomanip>
 #include <filesystem>
 
 using namespace std;
 namespace fs = std::filesystem;
 
 enum RetVal {
   r_satisfied,
   r_unsatisfied,
   r_normal
 };
 
 class SATSolverCDCL {
 private:
   vector<int> literals;
   vector<vector<int>> literal_list_per_clause;
   vector<int> literal_frequency;
   vector<int> literal_polarity;
   vector<int> original_literal_frequency;
   int literal_count;
   int clause_count;
   int kappa_antecedent;
   vector<int> literal_decision_level;
   vector<int> literal_antecedent;
   int assigned_literal_count;
   bool already_unsatisfied;
   int pick_counter;
   random_device random_generator;
   mt19937 generator;
 
   int unit_propagate(int);
   void assign_literal(int, int, int);
   void unassign_literal(int);
   int literal_to_variable_index(int);
   int conflict_analysis_and_backtrack(int);
   vector<int>& resolve(vector<int>&, int);
   int pick_branching_variable();
   bool all_variables_assigned();
 
 public:
   SATSolverCDCL() : generator(random_generator()) {}
   void initialize();
   int CDCL();
   int solve();
 };
 
 void SATSolverCDCL::initialize() {
   char c;
   string s;
   while (true) {
     cin >> c;
     if (c == 'c') {
       getline(cin, s);
     } else {
       cin >> s;
       break;
     }
   }
   cin >> literal_count >> clause_count;
   assigned_literal_count = 0;
   kappa_antecedent = -1;
   pick_counter = 0;
   already_unsatisfied = false;
   literals.clear(); literals.resize(literal_count, -1);
   literal_frequency.clear(); literal_frequency.resize(literal_count, 0);
   literal_polarity.clear(); literal_polarity.resize(literal_count, 0);
   literal_list_per_clause.clear(); literal_list_per_clause.resize(clause_count);
   literal_antecedent.clear(); literal_antecedent.resize(literal_count, -1);
   literal_decision_level.clear(); literal_decision_level.resize(literal_count, -1);
 
   int literal, literal_count_in_clause = 0;
   for (int i = 0; i < clause_count; i++) {
     literal_count_in_clause = 0;
     while (true) {
       cin >> literal;
       if (literal > 0) {
         literal_list_per_clause[i].push_back(literal);
         literal_frequency[literal - 1]++;
         literal_polarity[literal - 1]++;
       } else if (literal < 0) {
         literal_list_per_clause[i].push_back(literal);
         literal_frequency[-1 - literal]++;
         literal_polarity[-1 - literal]--;
       } else {
         if (literal_count_in_clause == 0) already_unsatisfied = true;
         break;
       }
       literal_count_in_clause++;
     }
   }
   original_literal_frequency = literal_frequency;
 }
 
 int SATSolverCDCL::unit_propagate(int decision_level) {
   bool unit_clause_found = false;
   int false_count = 0, unset_count = 0, literal_index, last_unset_literal = -1;
   bool satisfied_flag = false;
   do {
     unit_clause_found = false;
     for (int i = 0; i < literal_list_per_clause.size() && !unit_clause_found; i++) {
       false_count = 0; unset_count = 0; satisfied_flag = false;
       for (int j = 0; j < literal_list_per_clause[i].size(); j++) {
         literal_index = literal_to_variable_index(literal_list_per_clause[i][j]);
         if (literals[literal_index] == -1) {
           unset_count++;
           last_unset_literal = j;
         } else if ((literals[literal_index] == 0 && literal_list_per_clause[i][j] > 0) ||
                    (literals[literal_index] == 1 && literal_list_per_clause[i][j] < 0)) {
           false_count++;
         } else {
           satisfied_flag = true;
           break;
         }
       }
       if (satisfied_flag) continue;
       if (unset_count == 1) {
         assign_literal(literal_list_per_clause[i][last_unset_literal], decision_level, i);
         unit_clause_found = true;
         break;
       } else if (false_count == literal_list_per_clause[i].size()) {
         kappa_antecedent = i;
         return RetVal::r_unsatisfied;
       }
     }
   } while (unit_clause_found);
   kappa_antecedent = -1;
   return RetVal::r_normal;
 }
 
 void SATSolverCDCL::assign_literal(int variable, int decision_level, int antecedent) {
   int literal = literal_to_variable_index(variable);
   int value = (variable > 0) ? 1 : 0;
   literals[literal] = value;
   literal_decision_level[literal] = decision_level;
   literal_antecedent[literal] = antecedent;
   literal_frequency[literal] = -1;
   assigned_literal_count++;
 }
 
 void SATSolverCDCL::unassign_literal(int literal_index) {
   literals[literal_index] = -1;
   literal_decision_level[literal_index] = -1;
   literal_antecedent[literal_index] = -1;
   literal_frequency[literal_index] = original_literal_frequency[literal_index];
   assigned_literal_count--;
 }
 
 int SATSolverCDCL::literal_to_variable_index(int variable) {
   return (variable > 0) ? variable - 1 : -variable - 1;
 }
 
 int SATSolverCDCL::conflict_analysis_and_backtrack(int decision_level) {
   vector<int> learnt_clause = literal_list_per_clause[kappa_antecedent];
   int conflict_decision_level = decision_level, this_level_count = 0, resolver_literal, literal;
   do {
     this_level_count = 0;
     for (int i = 0; i < learnt_clause.size(); i++) {
       literal = literal_to_variable_index(learnt_clause[i]);
       if (literal_decision_level[literal] == conflict_decision_level) this_level_count++;
       if (literal_decision_level[literal] == conflict_decision_level &&
           literal_antecedent[literal] != -1) resolver_literal = literal;
     }
     if (this_level_count == 1) break;
     learnt_clause = resolve(learnt_clause, resolver_literal);
   } while (true);
   literal_list_per_clause.push_back(learnt_clause);
   for (int i = 0; i < learnt_clause.size(); i++) {
     int literal_index = literal_to_variable_index(learnt_clause[i]);
     int update = (learnt_clause[i] > 0) ? 1 : -1;
     literal_polarity[literal_index] += update;
     if (literal_frequency[literal_index] != -1) literal_frequency[literal_index]++;
     original_literal_frequency[literal_index]++;
   }
   clause_count++;
   int backtracked_decision_level = 0;
   for (int i = 0; i < learnt_clause.size(); i++) {
     int literal_index = literal_to_variable_index(learnt_clause[i]);
     int decision_level_here = literal_decision_level[literal_index];
     if (decision_level_here != conflict_decision_level &&
         decision_level_here > backtracked_decision_level) {
       backtracked_decision_level = decision_level_here;
     }
   }
   for (int i = 0; i < literals.size(); i++) {
     if (literal_decision_level[i] > backtracked_decision_level) {
       unassign_literal(i);
     }
   }
   return backtracked_decision_level;
 }
 
 vector<int>& SATSolverCDCL::resolve(vector<int>& input_clause, int literal) {
   vector<int> second_input = literal_list_per_clause[literal_antecedent[literal]];
   input_clause.insert(input_clause.end(), second_input.begin(), second_input.end());
   for (int i = 0; i < input_clause.size(); i++) {
     if (input_clause[i] == literal + 1 || input_clause[i] == -literal - 1) {
       input_clause.erase(input_clause.begin() + i);
       i--;
     }
   }
   sort(input_clause.begin(), input_clause.end());
   input_clause.erase(unique(input_clause.begin(), input_clause.end()), input_clause.end());
   return input_clause;
 }
 
 int SATSolverCDCL::pick_branching_variable() {
   uniform_int_distribution<int> choose_branch(1, 10);
   uniform_int_distribution<int> choose_literal(0, literal_count - 1);
   int random_value = choose_branch(generator);
   bool too_many_attempts = false;
   int attempt_counter = 0;
   do {
     if (random_value > 4 || assigned_literal_count < literal_count / 2 || too_many_attempts) {
       pick_counter++;
       if (pick_counter == 20 * literal_count) {
         for (int i = 0; i < literals.size(); i++) {
           original_literal_frequency[i] /= 2;
           if (literal_frequency[i] != -1) literal_frequency[i] /= 2;
         }
         pick_counter = 0;
       }
       int variable = distance(literal_frequency.begin(),
                               max_element(literal_frequency.begin(), literal_frequency.end()));
       return (literal_polarity[variable] >= 0) ? variable + 1 : -variable - 1;
     } else {
       while (attempt_counter < 10 * literal_count) {
         int variable = choose_literal(generator);
         if (literal_frequency[variable] != -1) {
           return (literal_polarity[variable] >= 0) ? variable + 1 : -variable - 1;
         }
         attempt_counter++;
       }
       too_many_attempts = true;
     }
   } while (too_many_attempts);
   return 1;
 }
 
 bool SATSolverCDCL::all_variables_assigned() {
   return literal_count == assigned_literal_count;
 }
 
 int SATSolverCDCL::CDCL() {
   int decision_level = 0;
   if (already_unsatisfied) return RetVal::r_unsatisfied;
   int unit_propagate_result = unit_propagate(decision_level);
   if (unit_propagate_result == RetVal::r_unsatisfied) return unit_propagate_result;
   while (!all_variables_assigned()) {
     int picked_variable = pick_branching_variable();
     decision_level++;
     assign_literal(picked_variable, decision_level, -1);
     while (true) {
       unit_propagate_result = unit_propagate(decision_level);
       if (unit_propagate_result == RetVal::r_unsatisfied) {
         if (decision_level == 0) return unit_propagate_result;
         decision_level = conflict_analysis_and_backtrack(decision_level);
       } else break;
     }
   }
   return RetVal::r_satisfied;
 }
 
 int SATSolverCDCL::solve() {
   return CDCL();
 }
 

 int main() {
    std::string folder = "../../cnf_files/2sat_cnf/";
    std::ofstream out("results.txt");
    out << std::fixed << std::setprecision(3);

    for (const auto &entry : fs::directory_iterator(folder)) {
        if (entry.path().extension() == ".cnf") {
            std::string file = entry.path().string();
            std::string name = entry.path().filename().string();

            std::ifstream cnf_input(file);
            if (!cnf_input.is_open()) {
                std::cerr << "Failed to open " << file << "\n";
                continue;
            }

            std::streambuf *orig_cin = std::cin.rdbuf();
            std::cin.rdbuf(cnf_input.rdbuf());

            SATSolverCDCL solver;
            solver.initialize();

            auto start = std::chrono::high_resolution_clock::now();
            int result = solver.solve();
            auto end = std::chrono::high_resolution_clock::now();

            double ms = std::chrono::duration<double, std::milli>(end - start).count();

            std::string outcome = (result == RetVal::r_satisfied ? "SAT" : "UNSAT");
            out << name << ": " << outcome << " in " << ms << " ms\n";
            std::cout << name << ": " << outcome << " in " << ms << " ms\n";
            std::cin.rdbuf(orig_cin);
        }
    }

    out.close();
    std::cout << "Results written to results.txt\n";
    return 0;
}
