import random
import argparse

def generate_clause(num_vars, literals_per_clause):
    clause = set()
    while len(clause) < literals_per_clause:
        lit = random.randint(1, num_vars)
        lit *= random.choice([-1, 1])
        clause.add(lit)
    return list(clause)

def generate_cnf(num_vars, num_clauses, literals_per_clause, filename):
    with open(filename, 'w') as f:
        f.write(f"p cnf {num_vars} {num_clauses}\n")
        for _ in range(num_clauses):
            clause = generate_clause(num_vars, literals_per_clause)
            f.write(" ".join(map(str, clause)) + " 0\n")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate random CNF file in DIMACS format.")
    parser.add_argument("--vars", type=int, default=30, help="Number of variables")
    parser.add_argument("--clauses", type=int, default=30, help="Number of clauses")
    parser.add_argument("--lits", type=int, default=3, help="Literals per clause")
    parser.add_argument("--out", type=str, default="random.cnf", help="Output CNF file")

    args = parser.parse_args()
    generate_cnf(args.vars, args.clauses, args.lits, args.out)
    print(f"CNF file written to {args.out}")
