#!/usr/bin/env python3
import sys
import argparse
import copy
import logging

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO, format="[%(levelname)s]: %(message)s")

# Global variables for the backtracking search.
rows = 0
cols = 0
row_counts = []
col_counts = []
grid = []  # 2D list: "tree" or None
trees = []  # list of (i, j) positions where there is a tree
candidates = {}  # maps tree position -> list of ((i, j), symbol) candidate placements

# Variables to track best solution and stopping criteria.
best_solution = None       # best assignment found so far: dict mapping (i, j) -> symbol
best_penalty = float("inf")
iterations_since_improvement = 0
MAX_NO_IMPROVEMENT = 2000000  # maximum iterations without an improvement

def in_bounds(i, j):
    return 0 <= i < rows and 0 <= j < cols

def conflict(assignment, i, j):
    """Return True if placing a tent at (i, j) would be adjacent (including diagonally) to an existing tent."""
    for di in (-1, 0, 1):
        for dj in (-1, 0, 1):
            if di == 0 and dj == 0:
                continue
            if (i + di, j + dj) in assignment:
                return True
    return False

def compute_penalty(assignment):
    """
    Computes the total violation penalty of a given assignment.
    
    Violations:
      - For each row/column: 1 penalty per tent too many or missing.
      - For each tree: if not exactly one tent is correctly associated with the tree (based on the tent's symbol),
        add abs(associated_count - 1).
      - Each "X" (tent with no associated tree) adds 1 penalty.
    """
    penalty = 0
    # Count tents in each row and column.
    row_usage = [0] * rows
    col_usage = [0] * cols
    for (i, j) in assignment:
        row_usage[i] += 1
        col_usage[j] += 1
    for i in range(rows):
        penalty += abs(row_usage[i] - row_counts[i])
    for j in range(cols):
        penalty += abs(col_usage[j] - col_counts[j])
        
    # Build a mapping from tree cell to number of tents associated with it.
    tree_associations = {}
    for (i, j), sym in assignment.items():
        if sym == "X":
            continue  # No associated tree.
        # Determine the tree cell based on the tent's symbol.
        if sym == "U":
            tree_cell = (i - 1, j)
        elif sym == "D":
            tree_cell = (i + 1, j)
        elif sym == "L":
            tree_cell = (i, j - 1)
        elif sym == "R":
            tree_cell = (i, j + 1)
        else:
            continue
        tree_associations[tree_cell] = tree_associations.get(tree_cell, 0) + 1

    # For each tree in the grid, add a penalty if it doesn't have exactly one associated tent.
    for tree in trees:
        associated_count = tree_associations.get(tree, 0)
        penalty += abs(associated_count - 1)
        
    # Add penalty for each "X" tent.
    for sym in assignment.values():
        if sym == "X":
            penalty += 1
            
    return penalty

def fill_deficits(assignment, row_usage, col_usage):
    """
    Greedily add "X" tents (with no associated tree) to help meet row and column counts.
    A cell is chosen if it is empty, not a tree, and its addition would not create an adjacent conflict.
    """
    new_assignment = copy.deepcopy(assignment)
    new_row_usage = row_usage[:]
    new_col_usage = col_usage[:]
    changed = True
    while changed:
        changed = False
        for i in range(rows):
            for j in range(cols):
                if grid[i][j] == "tree":
                    continue
                if (i, j) in new_assignment:
                    continue
                # If the current row or column is still below the target.
                if new_row_usage[i] < row_counts[i] or new_col_usage[j] < col_counts[j]:
                    if not conflict(new_assignment, i, j):
                        new_assignment[(i, j)] = "X"
                        new_row_usage[i] += 1
                        new_col_usage[j] += 1
                        changed = True
        # Continue until no more changes can be made.
    return new_assignment

def backtrack_tree(index, assignment, row_usage, col_usage):
    """
    Recursive backtracking over the list of trees.
    For each tree, try placing a tent in one of its candidate positions (if available)
    and also allow leaving the tree unsatisfied.
    Uses a global counter to terminate if no improvement is seen after many iterations.
    """
    global best_solution, best_penalty, iterations_since_improvement

    # Check stopping criterion: if too many iterations without improvement, return early.
    if iterations_since_improvement >= MAX_NO_IMPROVEMENT:
        return

    if index == len(trees):
        # All trees processed: fill deficits using "X" tents.
        completed_assignment = fill_deficits(assignment, row_usage, col_usage)
        penalty = compute_penalty(completed_assignment)
        if penalty < best_penalty:
            best_penalty = penalty
            best_solution = copy.deepcopy(completed_assignment)
            iterations_since_improvement = 0  # reset counter on improvement
            logger.debug(f"New best penalty: {best_penalty}")
        else:
            iterations_since_improvement += 1
        return

    tree = trees[index]
    cand_list = candidates.get(tree, [])
    # Try each candidate placement for this tree.
    for (pos, sym) in cand_list:
        i, j = pos
        # Skip if already used or if placing here would conflict with other tents.
        if pos in assignment or conflict(assignment, i, j):
            continue
        # Place the candidate tent.
        assignment[pos] = sym
        row_usage[i] += 1
        col_usage[j] += 1
        backtrack_tree(index + 1, assignment, row_usage, col_usage)
        # Backtrack.
        del assignment[pos]
        row_usage[i] -= 1
        col_usage[j] -= 1

    # Also try the option of not assigning any candidate for this tree.
    backtrack_tree(index + 1, assignment, row_usage, col_usage)

def parse_input_file(file_name):
    """
    Parses the input file.
    Expected format:
       First line: <rows> <cols>
       Second line: row tent counts (space-separated)
       Third line: column tent counts (space-separated)
       Subsequent lines: grid rows; 'T' indicates a tree, other characters are empty.
    """
    try:
        with open(file_name, "r") as f:
            lines = f.read().strip().splitlines()
    except Exception as e:
        logger.error(f"Error reading input file {file_name}: {e}")
        sys.exit(1)

    if len(lines) < 4:
        logger.error("Input file format error: Not enough lines.")
        sys.exit(1)

    parts = lines[0].split()
    if len(parts) < 2:
        logger.error("Input file format error: First line must contain two numbers.")
        sys.exit(1)
    try:
        r = int(parts[0])
        c = int(parts[1])
    except ValueError:
        logger.error("Input file format error: Invalid numbers in first line.")
        sys.exit(1)

    try:
        r_counts = list(map(int, lines[1].split()))
        c_counts = list(map(int, lines[2].split()))
    except ValueError:
        logger.error("Input file format error: Row/column counts must be integers.")
        sys.exit(1)

    if len(r_counts) != r or len(c_counts) != c:
        logger.error("Row/column counts do not match grid dimensions.")
        sys.exit(1)

    grid_local = []
    for line in lines[3:]:
        if len(line) != c:
            logger.error("A grid row does not match the specified column count.")
            sys.exit(1)
        row_list = []
        for char in line:
            if char == "T":
                row_list.append("tree")
            else:
                row_list.append(None)
        grid_local.append(row_list)
    return r, c, r_counts, c_counts, grid_local

def output_solution(assignment):
    """
    Formats and prints the solution.
    Output format:
      First line: total computed violation penalty
      Second line: total number of tents
      Subsequent lines: each tent placement as: <row> <col> <symbol>
      (Coordinates are 1-indexed.)
    """
    penalty = compute_penalty(assignment)
    tents = list(assignment.items())
    print(penalty)
    print(len(tents))
    for (i, j), sym in tents:
        print(i + 1, j + 1, sym)

def build_candidates():
    """
    For each tree, build candidate positions for a tent.
    A candidate is valid if it is orthogonally adjacent, in bounds, and the cell is empty.
    The associated symbol is based on the relative direction from the tent to its tree.
    """
    global candidates
    for tree in trees:
        i, j = tree
        cand = []
        # Adjusted directions:
        directions = {
            "U": (1, 0),    # Place tent below the tree so tree is above (U) the tent.
            "D": (-1, 0),   # Place tent above the tree so tree is below (D) the tent.
            "L": (0, 1),    # Place tent to the right of the tree so tree is left (L) of the tent.
            "R": (0, -1)    # Place tent to the left of the tree so tree is right (R) of the tent.
        }
        for sym, (di, dj) in directions.items():
            ni, nj = i + di, j + dj
            if in_bounds(ni, nj) and grid[ni][nj] is None:
                cand.append(((ni, nj), sym))
        candidates[tree] = cand

def solve():
    """
    Solve the Tents and Trees puzzle using backtracking with a stopping criterion based on lack of improvement.
    Returns the best found assignment (a dict mapping (i, j) -> symbol).
    """
    global best_solution, best_penalty, iterations_since_improvement
    best_solution = {}
    best_penalty = float("inf")
    iterations_since_improvement = 0
    assignment = {}
    row_usage = [0] * rows
    col_usage = [0] * cols
    backtrack_tree(0, assignment, row_usage, col_usage)
    return best_solution

def setup_parser():
    parser = argparse.ArgumentParser(description="Solver for the Tents and Trees puzzle.")
    parser.add_argument("input", help="Input file name")
    parser.add_argument("--debug", action="store_true", help="Enable debug logging")
    return parser

def main():
    global rows, cols, row_counts, col_counts, grid, trees
    parser = setup_parser()
    args = parser.parse_args()
    if args.debug:
        logger.setLevel(logging.DEBUG)
    else:
        logger.setLevel(logging.INFO)

    rows, cols, row_counts, col_counts, grid = parse_input_file(args.input)
    logger.debug(f"Parsed grid with {rows} rows and {cols} cols")
    trees = [(i, j) for i in range(rows) for j in range(cols) if grid[i][j] == "tree"]
    logger.debug(f"Found {len(trees)} trees")
    build_candidates()
    solution = solve()
    if solution is None:
        logger.info("No solution found.")
        sys.exit(1)
    output_solution(solution)

if __name__ == "__main__":
    main()
