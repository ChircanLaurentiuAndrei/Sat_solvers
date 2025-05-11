# Naive SAT Solvers
This repository includes the implementation of __Resolution method, DP, DPLL, CDCL__ , four of the most well-known [SAT solving methods](http://en.wikipedia.org/wiki/Boolean_satisfiability_problem#Algorithms_for_solving_SAT)  in their **basic** form. Each implementation has its own code base, but they share the same fundamental ideas. A console executable reads all the [CNF](http://en.wikipedia.org/wiki/Conjunctive_normal_form) files in [DIMACS](https://people.sc.fsu.edu/~jburkardt/data/cnf/cnf.html) format from a folder, processes them, and outputs the result in a `.txt` file with the format `<flename>: SAT/UNSAT in x.xxx ms`.
## Motivation

This repository aims to benchmark popular SAT solving methods/algorithms for the purpose of ***science*** ~~(but also to complete an assigment for university)~~.
The main algorithms I will be tackling are Resolution, DP, DPLL, and CDCL, _and maybe some more if i have the time_.
All the algorithms are implemented without heuristics or other complex optimizations to prioritize simplicity and ease of use. This journey has led me down a path of knowledge and made me appreciate all the work that has been dedicated to the problem of satisfiability and state-of-the-art solvers. I have great respect for the people who made all this possible. I hope my work inspires others to look at this problem with the same enthusiasm as I do.

## C++11?

The main algorithms have been implemented entirely in C11 for efficiency and simplicity. C11 provides all the necessary libraries to make this as straightforward as possible.
My motivation for choosing C++ over Python is its speed and compiled nature.
**And everybody knows that _speed_ is key.**

## Usage

We take this simple CNF file as example:
```
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

The first line, `p cnf 5 10` is ignored as it is treated as a comment.
Running one of the included solvers produces the output file `result_<method_used>.txt`, containing `<filename>.cnf: SAT in 0.069 ms`.

The repository contains the [CNF_files](cnf_files/) directory, where all the test cases are stored, as well as the [samples](cnf_files/samples/) directory, where I left some trivial CNF files. The sample files can be removed, and the programs will accept any CNF file placed in the `samples` folder. Simply drag and drop the files there and enjoy the results!

Additionally, there is a short [Python script](cnf_generator/) that can generate CNF files in DIMACS format.

## ToDO list

~~1. Actually implement the algorithims.~~
~~2. Investigate the use of multithreading and other optimizations.~~ _Multithreading usually slows things down_
~~3. Find a way to store benchmarks.~~
4. Maybe write a script to streamline everything and make testing easier. _This is still a valid goal, but not right now._
5. Allow users to choose the folder from which CNF files are read.
6. Unify all the code into one program with an option to select a preferred method.

## License

[MIT License](LICENSE)