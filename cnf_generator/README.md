# CNF Generator

A Python script to generate random CNF (Conjunctive Normal Form) formulas in DIMACS format, commonly used for SAT solvers.

## Features

- Generates CNF formulas with customizable variables, clauses, and literals per clause.
- Outputs files in standard DIMACS format.

## Usage

Run the script with customizable parameters:

```bash
python3 generate_random_cnf.py --vars 30 --clauses 30 --lits 30 --out random.cnf
```

### Arguments:
- `--vars`: Number of variables (default: 30)
- `--clauses`: Number of clauses (default: 30)
- `--lits`: Literals per clause (default: 3)
- `--out`: Output CNF file (default: `random.cnf`)

## Example Output

Sample CNF file generated:

```dimacs
p cnf 30 30
1 -3 7 0
-2 4 -9 0
...
```

## License

[MIT License](LICENSE)