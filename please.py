import sys
import random, math, copy

class TentsSolver:
    def __init__(self, board, row_counts, col_counts, extra_sample_size=20):
        """
        board: list of strings (each string is a row).
               'T' indicates a tree; '.' indicates an empty cell.
        row_counts: list of integers; target number of tents per row.
        col_counts: list of integers; target number of tents per column.
        extra_sample_size: number of extra (non-adjacent) candidate cells to sample per tree.
        """
        self.board = board
        self.row_counts = row_counts
        self.col_counts = col_counts
        self.rows = len(board)
        self.cols = len(board[0])
        self.extra_sample_size = extra_sample_size
        
        # Compute list of all empty cells.
        self.empty_cells = []
        for r in range(self.rows):
            for c in range(self.cols):
                if board[r][c] == '.':
                    self.empty_cells.append((r, c))
                    
        # Identify tree positions.
        self.trees = []
        for r in range(self.rows):
            for c in range(self.cols):
                if board[r][c] == 'T':
                    self.trees.append((r, c))
        self.n_trees = len(self.trees)
        print(f"[DEBUG] Found {self.n_trees} trees.", file=sys.stderr)
        
        # For each tree, compute candidate cells.
        # Candidates include cells adjacent horizontally or vertically,
        # plus a small random sample of other empty cells.
        self.candidates = []
        for tree in self.trees:
            r, c = tree
            adjacent = []
            for dr, dc in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
                nr, nc = r + dr, c + dc
                if 0 <= nr < self.rows and 0 <= nc < self.cols:
                    if board[nr][nc] == '.':
                        adjacent.append((nr, nc))
            extra = []
            if self.empty_cells:
                sample = random.sample(self.empty_cells, min(self.extra_sample_size, len(self.empty_cells)))
                # Exclude any that are already in the adjacent list.
                extra = [pos for pos in sample if pos not in adjacent]
            # Use the union of adjacent and extra candidates.
            cand_set = set(adjacent + extra)
            self.candidates.append(list(cand_set))
        print("[DEBUG] Candidate positions computed for each tree (including extra non-adjacent candidates).", file=sys.stderr)
    
    def initial_state(self):
        """Randomly assign each tree one candidate (or None if no candidate exists)."""
        state = []
        for cand in self.candidates:
            if cand:
                state.append(random.choice(cand))
            else:
                state.append(None)
        return state

    def state_to_grid(self, state):
        """Build a mapping from cell positions to list of trees using that cell."""
        grid = {}
        for i, pos in enumerate(state):
            if pos is not None:
                grid.setdefault(pos, []).append(i)
        return grid

    def compute_cost(self, state):
        """
        Revised cost function that mirrors the autograder’s violation counts:
        - Duplicate usage of a candidate cell gets a heavy penalty.
        - Row/column counts are penalized by the absolute difference.
        - Each tent that touches at least one other tent (in any direction) gets 1 penalty.
        - Each tree that either has no assigned candidate or has a candidate
            not immediately (orthogonally) adjacent gets 1 penalty.
        """
        INF = 10**6
        cost = 0
        grid = self.state_to_grid(state)

        # 1. Duplicate usage (illegal if two trees use the same candidate).
        for pos, tree_ids in grid.items():
            if len(tree_ids) > 1:
                cost += INF

        # 2. Row/column violations.
        row_actual = [0] * self.rows
        col_actual = [0] * self.cols
        for (r, c) in grid.keys():
            row_actual[r] += 1
            col_actual[c] += 1
        for r in range(self.rows):
            cost += abs(row_actual[r] - self.row_counts[r])
        for c in range(self.cols):
            cost += abs(col_actual[c] - self.col_counts[c])

        # 3. Tent adjacency violations.
        # (For each tent, if any neighbor (including diagonally) is also a tent, count 1 penalty.)
        tent_positions = set(grid.keys())
        for (r, c) in tent_positions:
            for dr, dc in [(-1, -1), (-1, 0), (-1, 1),
                        (0, -1),           (0, 1),
                        (1, -1),  (1, 0),  (1, 1)]:
                if (r + dr, c + dc) in tent_positions:
                    cost += 1
                    break  # Only one penalty per tent.
                    
        # 4. Tree association violations.
        # Each tree must have a candidate that is immediately (horizontally or vertically) adjacent.
        for i, assign in enumerate(state):
            tree_r, tree_c = self.trees[i]
            if assign is None:
                # No candidate assigned.
                cost += 1
            else:
                # Check for orthogonal adjacency.
                if abs(tree_r - assign[0]) + abs(tree_c - assign[1]) != 1:
                    cost += 1

        return cost

    def get_direction(self, tree, tent):
        """
        Given a tree and its assigned tent, return the direction letter:
          U, D, L, R if adjacent (determined by relative position), or X if not.
        """
        tr, tc = tree
        r, c = tent
        # Check for immediate adjacency.
        if abs(tr - r) + abs(tc - c) == 1:
            if tr < r:
                return 'U'
            elif tr > r:
                return 'D'
            elif tc < c:
                return 'L'
            elif tc > c:
                return 'R'
        return 'X'
     
    def simulated_annealing(self, max_iter=400000, initial_temp=1.0, cooling_rate=0.9999953):
        """
        Perform simulated annealing to minimize the cost.
        Returns (best_state, best_cost, best_iteration).
        """
        current_state = self.initial_state()
        current_cost = self.compute_cost(current_state)
        best_state = current_state[:]
        best_cost = current_cost
        best_iter = 0
        temp = initial_temp
        
        for i in range(max_iter):
            if i % 1000 == 0:
                print(f"[DEBUG] Iteration {i}: current_cost={current_cost}, best_cost={best_cost}, temp={temp}", file=sys.stderr)
            new_state = current_state[:]
            idx = random.randint(0, self.n_trees - 1)
            cand = self.candidates[idx][:]
            cand.append(None)
            new_state[idx] = random.choice(cand)
            new_cost = self.compute_cost(new_state)
            delta = new_cost - current_cost
            
            if delta < 0 or random.random() < math.exp(-delta / temp):
                current_state = new_state
                current_cost = new_cost
                if current_cost < best_cost:
                    best_state = current_state[:]
                    best_cost = current_cost
                    best_iter = i
            temp *= cooling_rate
            if temp < 1e-6:
                break
        print(f"[DEBUG] Simulated annealing complete. Best cost: {best_cost} at iteration {best_iter}", file=sys.stderr)
        return best_state, best_cost, best_iter

    def repair_solution(self, state):
        """
        For each tree, if its assigned candidate is not adjacent (Manhattan distance ≠ 1),
        try to reassign an adjacent candidate (from immediate neighbors) that is not already used.
        If no valid adjacent candidate is available, clear the assignment.
        """
        repaired_state = state[:]  # Copy the state.
        used = {}  # Keep track of cells already assigned as valid.
        # First, mark assignments that are valid.
        for i, pos in enumerate(state):
            if pos is not None and abs(self.trees[i][0] - pos[0]) + abs(self.trees[i][1] - pos[1]) == 1:
                used[pos] = i
        # Now, for each tree with an invalid candidate, try to repair.
        for i, pos in enumerate(state):
            if pos is not None and abs(self.trees[i][0] - pos[0]) + abs(self.trees[i][1] - pos[1]) != 1:
                tree = self.trees[i]
                adjacent = []
                for dr, dc in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
                    nr, nc = tree[0] + dr, tree[1] + dc
                    if 0 <= nr < self.rows and 0 <= nc < self.cols:
                        if self.board[nr][nc] == '.':
                            adjacent.append((nr, nc))
                # Try to pick an adjacent candidate that's not already used.
                found = False
                for cand in adjacent:
                    if cand not in used:
                        repaired_state[i] = cand
                        used[cand] = i
                        found = True
                        break
                if not found:
                    # Clear the assignment if no valid adjacent candidate exists.
                    repaired_state[i] = None
        return repaired_state

    def solve(self):
        """
        Runs simulated annealing, then repairs the solution so that each tent is adjacent to its tree.
        Returns:
          - A solution grid where trees remain 'T', empty cells remain '.', and tent cells
            contain the direction letter.
          - The final computed cost.
        """
        best_state, _, best_iter = self.simulated_annealing()
        print(f"[DEBUG] Best solution found at iteration {best_iter}", file=sys.stderr)
        repaired_state = self.repair_solution(best_state)
        final_cost = self.compute_cost(repaired_state)
        sol = [list(row) for row in self.board]
        for i, assign in enumerate(repaired_state):
            if assign is not None:
                tree = self.trees[i]
                direction = self.get_direction(tree, assign)
                sol[assign[0]][assign[1]] = direction
        return sol, final_cost

# --- Hyperparameter Tuning Functions ---

def tune_parameters(board, row_counts, col_counts):
    """
    Run a grid search over hyperparameters and return the best combination (and iteration)
    along with the best solution state.
    """
    best_combo = None
    best_cost = float('inf')
    best_iter_overall = None
    best_state_overall = None
    
    solver = TentsSolver(board, row_counts, col_counts)
    
    for max_iter in [100000, 200000, 300000]:
        for initial_temp in [1.0, 10.0, 100.0, 50.0, 25.0]:
            for cooling_rate in [0.9999, 0.99995, 0.99999, 0.999, 0.99, 0.9994534]:
                print(f"[TUNE] Trying: max_iter={max_iter}, initial_temp={initial_temp}, cooling_rate={cooling_rate}", file=sys.stderr)
                state, cost, best_iter = solver.simulated_annealing(max_iter=max_iter, initial_temp=initial_temp, cooling_rate=cooling_rate)
                print(f"[TUNE] Cost: {cost}, Best iteration: {best_iter}", file=sys.stderr)
                if cost < best_cost:
                    best_cost = cost
                    best_combo = (max_iter, initial_temp, cooling_rate)
                    best_iter_overall = best_iter
                    best_state_overall = state
    print(f"[TUNE] Best parameters: max_iter={best_combo[0]}, initial_temp={best_combo[1]}, cooling_rate={best_combo[2]} with cost {best_cost} at iteration {best_iter_overall}", file=sys.stderr)
    return best_combo, best_cost, best_iter_overall, best_state_overall

def run_tuning_from_input(input_file):
    """
    Read the puzzle from input_file and run hyperparameter tuning on it.
    """
    print(f"[DEBUG] Reading input from {input_file} for tuning.", file=sys.stderr)
    with open(input_file, "r") as fin:
        data = fin.read().strip().split()
    
    if not data:
        print("[DEBUG] No data found in input file.", file=sys.stderr)
        sys.exit(1)
    
    R = int(data[0])
    C = int(data[1])
    row_counts = list(map(int, data[2:2+R]))
    col_counts = list(map(int, data[2+R:2+R+C]))
    
    board = []
    index = 2 + R + C
    for i in range(R):
        board.append(data[index])
        index += 1

    print(f"[DEBUG] Puzzle for tuning: R={R}, C={C}", file=sys.stderr)
    for row in board:
        print(row, file=sys.stderr)
    
    best_combo, best_cost, best_iter, best_state = tune_parameters(board, row_counts, col_counts)
    print(f"[TUNE RESULT] Best parameters: max_iter={best_combo[0]}, initial_temp={best_combo[1]}, cooling_rate={best_combo[2]} with cost {best_cost} at iteration {best_iter}", file=sys.stderr)
    return board, row_counts, col_counts, best_state, best_cost

def run_batch_tuning(input_file, output_file):
    """
    Read the puzzle from input_file, run hyperparameter tuning (i.e. loop over many
    different hyperparameter values), then write the best solution to output_file.
    """
    board, row_counts, col_counts, best_state, best_cost = run_tuning_from_input(input_file)
    # Build the solution grid from best_state, using repair to ensure valid adjacency.
    solver = TentsSolver(board, row_counts, col_counts)
    repaired_state = solver.repair_solution(best_state)
    sol = [list(row) for row in board]
    for i, assign in enumerate(repaired_state):
        if assign is not None:
            tree = solver.trees[i]
            direction = solver.get_direction(tree, assign)
            sol[assign[0]][assign[1]] = direction
    
    # Count tents.
    tents = []
    for i in range(len(board)):
        for j in range(len(board[0])):
            if sol[i][j] != '.' and sol[i][j] != 'T':
                tents.append((i+1, j+1, sol[i][j]))
    
    output_lines = []
    output_lines.append(str(best_cost))
    output_lines.append(str(len(tents)))
    for r, c, letter in tents:
        output_lines.append(f"{r} {c} {letter}")
    
    with open(output_file, "w") as fout:
        fout.write("\n".join(output_lines))
    print(f"[DEBUG] Best solution saved to {output_file}", file=sys.stderr)

# --- Main Function ---

def main():
    # Batch tuning mode: run through a grid of hyperparameters and save best solution.
    if len(sys.argv) >= 2 and sys.argv[1] == "--batch":
        if len(sys.argv) >= 4:
            input_file = sys.argv[2]
            output_file = sys.argv[3]
            run_batch_tuning(input_file, output_file)
        else:
            print("Usage: python solver.py --batch input_file output_file", file=sys.stderr)
        sys.exit(0)
        
    # Tuning-only mode: run hyperparameter tuning and print results (no output file).
    if len(sys.argv) >= 2 and sys.argv[1] == "--tune":
        if len(sys.argv) >= 3:
            input_file = sys.argv[2]
            run_tuning_from_input(input_file)
        else:
            print("Usage: python solver.py --tune input_file", file=sys.stderr)
        sys.exit(0)
        
    # Normal mode: file I/O.
    if len(sys.argv) < 3:
        print("Usage: python solver.py input_file output_file", file=sys.stderr)
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    print(f"[DEBUG] Reading input from {input_file}", file=sys.stderr)
    with open(input_file, "r") as fin:
        data = fin.read().strip().split()
    
    if not data:
        print("[DEBUG] No data found in input file.", file=sys.stderr)
        sys.exit(1)
    
    R = int(data[0])
    C = int(data[1])
    print(f"[DEBUG] Board dimensions: R = {R}, C = {C}", file=sys.stderr)
    
    row_counts = list(map(int, data[2:2+R]))
    col_counts = list(map(int, data[2+R:2+R+C]))
    
    print(f"[DEBUG] Row tent counts: {row_counts}", file=sys.stderr)
    print(f"[DEBUG] Column tent counts: {col_counts}", file=sys.stderr)
    
    board = []
    index = 2 + R + C
    for i in range(R):
        board.append(data[index])
        index += 1
    
    print("[DEBUG] Board read:", file=sys.stderr)
    for row in board:
        print(row, file=sys.stderr)
    
    solver = TentsSolver(board, row_counts, col_counts)
    solution, cost = solver.solve()
    print(f"[DEBUG] Final computed cost: {cost}", file=sys.stderr)
    
    tents = []
    for i in range(R):
        for j in range(C):
            if solution[i][j] != '.' and solution[i][j] != 'T':
                tents.append((i+1, j+1, solution[i][j]))
    
    print(f"[DEBUG] Number of tents placed: {len(tents)}", file=sys.stderr)
    
    output_lines = []
    output_lines.append(str(cost))
    output_lines.append(str(len(tents)))
    for r, c, letter in tents:
        output_lines.append(f"{r} {c} {letter}")
    
    with open(output_file, "w") as fout:
        fout.write("\n".join(output_lines))
    
    print(f"[DEBUG] Output written to {output_file}", file=sys.stderr)
        
if __name__ == "__main__":
    main()
