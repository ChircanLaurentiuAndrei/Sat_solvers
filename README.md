# Naive SAT Solvers
This repository includes implementations of the __Resolution Method, Davis–Putnam (DP), DPLL, and CDCL__— four of the most well-known [SAT solving methods](http://en.wikipedia.org/wiki/Boolean_satisfiability_problem#Algorithms_for_solving_SAT)—in their **basic** form.  
Each implementation has its own codebase but shares the same fundamental ideas. A console executable reads all [CNF](http://en.wikipedia.org/wiki/Conjunctive_normal_form) files in [DIMACS](https://people.sc.fsu.edu/~jburkardt/data/cnf/cnf.html) format from a folder, processes them, and outputs the results in a `.txt` file with the format `<flename>: SAT/UNSAT in x.xxx ms`.
## Motivation

This repository aims to benchmark popular SAT solving methods/algorithms for the purpose of ***science*** ~~(but also to complete an assignment for university)~~.  
The main algorithms tackled are Resolution, DP, DPLL, and CDCL—_and maybe more if I have the time_.  
All algorithms are implemented without heuristics or complex optimizations to prioritize **simplicity and ease of use**.  

This journey has led me down a path of discovery and made me appreciate the immense work behind the problem of satisfiability and modern SAT solvers. I have great respect for the people who made all this possible, and I hope this project inspires others to approach the problem with the same enthusiasm.

## C++11?

All algorithms are implemented entirely in **C++11** for both efficiency and simplicity.  
C++ provides all the necessary libraries to make this as straightforward as possible.  
My motivation for choosing C++ over Python is its **speed** and **compiled nature**.  
**And everybody knows that _speed_ is key.**

## Usage

We take this simple CNF file as example:
``` dimacs
p cnf 5 10
1 4 -2 0
3 -1 -2 0
-5 4 -1 0
-5 -3 -2 0
2 3 -3 0
-5 -3 -2 0
-5 -4 -3 0
1 2 5 0
2 3 -4 0
4 -3 -2 0
```

The first line, `p cnf 5 10` is treated as metadata and ignored.
Running any of the included solvers produces an output file named `result_<method_used>.txt`, containing lines like: `<filename>.cnf: SAT in 0.069 ms`.  
The repository contains the [CNF_files](cnf_files/) directory, where all test cases are stored, and a [samples](cnf_files/samples/) directory with a few trivial CNF examples.
You can remove or replace the sample files—just drag and drop any CNF files you want to test into the `samples` folder, and enjoy the results!  
Additionally, there's a simple [Python script](cnf_generator/) that can generate CNF files in DIMACS format.

## Test results

In the [results](results/) directory, you can find all test case outputs, split into **2SAT** and **3SAT**.  
You'll notice the **lack of results** for the Resolution method and Davis–Putnam. That's because those methods are **way too slow** for anything non-trivial in the 3SAT category.  
All results follow the format described in the first section of this [README.md](README.md).

## ToDO list

1. ~~Actually implement the algorithims.~~
2. ~~Investigate the use of multithreading and other optimizations.~~ _Multithreading usually slows things down_
3. ~~Find a way to store benchmarks.~~
4. Maybe write a script to streamline everything and make testing easier. _This is still a valid goal, but not right now._
5. Allow users to choose the folder from which CNF files are read.
6. Unify all the code into one program with an option to select a preferred method.

## License

[MIT License](LICENSE)