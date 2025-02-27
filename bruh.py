import sys
import torch
import torch.nn.functional as F


def tent_adjacency_loss(tent_probs, alpha=1.0):
    """
    Penalize any tent with one or more adjacent tents (including diagonally).
    tent_probs: Tensor of shape (6, R, C) (output after softmax)
    alpha: scaling factor for the exponential penalty.
    """
    # Compute probability of a tent being placed (states 0 to 4)
    tent_placed = tent_probs[:5].sum(dim=0)  # shape (R, C)
    
    # Reshape to 4D for convolution: (batch=1, channel=1, height, width)
    tent_placed_4d = tent_placed.unsqueeze(0).unsqueeze(0)
    
    # Define 3x3 kernel with ones everywhere except center (neighbors only)
    kernel = torch.tensor([[1.0, 1.0, 1.0],
                           [1.0, 0.0, 1.0],
                           [1.0, 1.0, 1.0]], device=tent_probs.device).unsqueeze(0).unsqueeze(0)
    
    # Convolve with padding=1 to get neighbor sum for each cell
    neighbor_sum = F.conv2d(tent_placed_4d, kernel, padding=1)
    neighbor_sum = neighbor_sum.squeeze(0).squeeze(0)  # shape (R, C)
    
    # For each cell with a tent, penalize if any neighbor tent is present.
    # The penalty function is smooth: 0 when neighbor_sum=0, approaching 1 as neighbor_sum increases.
    penalty = tent_placed * (1 - torch.exp(-alpha * neighbor_sum))
    return penalty.sum()

def tree_tent_matching_loss(tent_probs, tree_mask):
    """
    Enforce that each tent with a directional assignment has a tree in its indicated neighboring cell,
    and that each tree is matched by exactly one tent.
    tent_probs: Tensor of shape (6, R, C) (after softmax)
    tree_mask: Binary tensor of shape (R, C) (1 for tree, 0 otherwise)
    """
    loss_tents = 0.0
    R, C = tree_mask.shape
    
    # --- Tent side: Check that tents with directions U, D, L, R find a tree ---
    # For U: a tent at (r, c) expects a tree at (r-1, c)
    tree_up = torch.zeros_like(tree_mask)
    tree_up[1:, :] = tree_mask[:-1, :]  # valid for rows 1...R-1; top row gets 0
    loss_tents += (tent_probs[0] * (1 - tree_up)).sum()
    
    # For D: a tent expects a tree at (r+1, c)
    tree_down = torch.zeros_like(tree_mask)
    tree_down[:-1, :] = tree_mask[1:, :]  # valid for rows 0...R-2; bottom row gets 0
    loss_tents += (tent_probs[1] * (1 - tree_down)).sum()
    
    # For L: a tent expects a tree at (r, c-1)
    tree_left = torch.zeros_like(tree_mask)
    tree_left[:, 1:] = tree_mask[:, :-1]  # valid for columns 1...C-1; first column gets 0
    loss_tents += (tent_probs[2] * (1 - tree_left)).sum()
    
    # For R: a tent expects a tree at (r, c+1)
    tree_right = torch.zeros_like(tree_mask)
    tree_right[:, :-1] = tree_mask[:, 1:]  # valid for columns 0...C-2; last column gets 0
    loss_tents += (tent_probs[3] * (1 - tree_right)).sum()
    
    # Optionally: Penalize tents in the "X" state (index 4) if they are disallowed.
    # Remove this term if you want to allow "X" tent placements.
    loss_tents += tent_probs[4].sum()
    
    # --- Tree side: For each tree, we want exactly one matching tent ---
    # For each tree cell, sum the contributions from the four directions:
    # A tree at (r, c) is matched by:
    #  - a tent below it (at r+1, c) with direction U (index 0)
    #  - a tent above it (at r-1, c) with direction D (index 1)
    #  - a tent to its right (at r, c+1) with direction L (index 2)
    #  - a tent to its left (at r, c-1) with direction R (index 3)
    m = torch.zeros_like(tree_mask)
    # Tent below (r+1, c) matching tree at (r, c)
    m[:-1, :] += tent_probs[0, 1:, :]
    # Tent above (r-1, c) matching tree at (r, c)
    m[1:, :] += tent_probs[1, :-1, :]
    # Tent to the right (r, c+1) matching tree at (r, c)
    m[:, :-1] += tent_probs[2, :, 1:]
    # Tent to the left (r, c-1) matching tree at (r, c)
    m[:, 1:] += tent_probs[3, :, :-1]
    
    # For each tree, penalize if the total matching probability deviates from 1.
    loss_trees = ((m - 1) ** 2 * tree_mask).sum()
    
    return loss_tents + loss_trees

def row_column_count_loss(tent_probs, row_counts, col_counts):
    """
    Enforce that the number of tents in each row and column matches the provided counts.
    tent_probs: Tensor of shape (6, R, C) (after softmax)
    row_counts: Tensor of shape (R,) with the desired tent counts per row.
    col_counts: Tensor of shape (C,) with the desired tent counts per column.
    """
    # Only consider states 0-4 (tent placed)
    tent_placed = tent_probs[:5].sum(dim=0)  # shape (R, C)
    
    # Sum along columns for each row and along rows for each column
    row_sum = tent_placed.sum(dim=1)  # shape (R,)
    col_sum = tent_placed.sum(dim=0)  # shape (C,)
    
    loss_rows = ((row_sum - row_counts) ** 2).sum()
    loss_cols = ((col_sum - col_counts) ** 2).sum()
    return loss_rows + loss_cols

def composite_loss(tent_logits, tree_mask, row_counts, col_counts,
                   lambda_adj=1.0, lambda_tree=1.0, lambda_rc=1.0, alpha=1.0):
    """
    Compute the overall loss as a weighted sum of:
      - Tent adjacency loss,
      - Tree-tent matching loss, and
      - Row/column count loss.
    
    tent_logits: Tensor of shape (6, R, C) (raw logits before softmax)
    tree_mask: Tensor of shape (R, C) (binary mask for trees)
    row_counts: Tensor of shape (R,) with desired tent counts per row.
    col_counts: Tensor of shape (C,) with desired tent counts per column.
    lambda_adj, lambda_tree, lambda_rc: Weights for each loss component.
    alpha: Parameter for the adjacency loss exponential.
    """
    # Convert logits to probabilities with softmax along the state dimension (dim=0)
    tent_probs = torch.softmax(tent_logits, dim=0)
    
    loss_adj = tent_adjacency_loss(tent_probs, alpha=alpha)
    loss_tree = tree_tent_matching_loss(tent_probs, tree_mask)
    loss_rc = row_column_count_loss(tent_probs, row_counts, col_counts)
    
    total_loss = lambda_adj * loss_adj + lambda_tree * loss_tree + lambda_rc * loss_rc
    return total_loss, loss_adj, loss_tree, loss_rc

def discrete_solution(tent_logits):
    """
    Convert the learned logits into a discrete solution.
    Returns a tensor of shape (R, C) where each cell is an integer in {0,1,2,3,4,5}.
    The mapping is:
      0: Tent with tree above (U)
      1: Tent with tree below (D)
      2: Tent with tree to left (L)
      3: Tent with tree to right (R)
      4: Tent with no attached tree (X)
      5: No tent placed.
    """
    tent_probs = torch.softmax(tent_logits, dim=0)
    solution = torch.argmax(tent_probs, dim=0)  # shape (R, C)
    return solution

def count_violations(solution, tree_mask, row_counts, col_counts):
    """
    Count violations for a discrete solution.
    Violations include:
      - A tent with at least one adjacent tent (diagonals included).
      - A tent that does not have a corresponding tree in its indicated direction.
      - A tree that is not matched by exactly one tent (adjacent in the proper direction).
      - A row or column with too many or too few tents (absolute difference per row/col).
    """
    violation = 0
    R, C = solution.shape
    # Mapping from state to letter (for later use)
    state_to_letter = {0: "U", 1: "D", 2: "L", 3: "R", 4: "X", 5: None}
    
    # 1. Tent Adjacency Violations (only count one violation per tent)
    for r in range(R):
        for c in range(C):
            # Only consider cells that can have a tent (i.e. not a tree)
            if tree_mask[r, c] == 1:
                continue
            state = int(solution[r, c])
            if state == 5:
                continue  # no tent here
            # Check 8 neighbors
            has_adjacent = False
            for dr in [-1, 0, 1]:
                for dc in [-1, 0, 1]:
                    if dr == 0 and dc == 0:
                        continue
                    nr, nc = r + dr, c + dc
                    if 0 <= nr < R and 0 <= nc < C:
                        # Only consider neighbor if it is not a tree and a tent is placed.
                        if tree_mask[nr, nc] == 0 and int(solution[nr, nc]) != 5:
                            has_adjacent = True
                            break
                if has_adjacent:
                    break
            if has_adjacent:
                violation += 1

    # 2. Tree-Tent Matching Violations (from the tent side)
    for r in range(R):
        for c in range(C):
            if tree_mask[r, c] == 1:
                continue  # skip tree cells here
            state = int(solution[r, c])
            if state == 5:
                continue  # no tent placed here
            if state == 0:
                # Expects tree above
                if r - 1 < 0 or tree_mask[r - 1, c] != 1:
                    violation += 1
            elif state == 1:
                # Expects tree below
                if r + 1 >= R or tree_mask[r + 1, c] != 1:
                    violation += 1
            elif state == 2:
                # Expects tree to the left
                if c - 1 < 0 or tree_mask[r, c - 1] != 1:
                    violation += 1
            elif state == 3:
                # Expects tree to the right
                if c + 1 >= C or tree_mask[r, c + 1] != 1:
                    violation += 1
            elif state == 4:
                # "X" state: no tree attached, so violation.
                violation += 1

    # 3. Tree-Tent Matching Violations (from the tree side)
    # For each tree, count matching tents in proper positions.
    for r in range(R):
        for c in range(C):
            if tree_mask[r, c] == 1:
                count_match = 0
                # Tent below (r+1, c) should have state 0
                if r + 1 < R and tree_mask[r + 1, c] == 0 and int(solution[r + 1, c]) == 0:
                    count_match += 1
                # Tent above (r-1, c) should have state 1
                if r - 1 >= 0 and tree_mask[r - 1, c] == 0 and int(solution[r - 1, c]) == 1:
                    count_match += 1
                # Tent right (r, c+1) should have state 2
                if c + 1 < C and tree_mask[r, c + 1] == 0 and int(solution[r, c + 1]) == 2:
                    count_match += 1
                # Tent left (r, c-1) should have state 3
                if c - 1 >= 0 and tree_mask[r, c - 1] == 0 and int(solution[r, c - 1]) == 3:
                    count_match += 1
                if count_match == 0:
                    violation += 1
                elif count_match > 1:
                    # Count extra matches as additional violations.
                    violation += (count_match - 1)
    
    # 4. Row and Column Count Violations
    # Count tents only in cells without trees.
    for r in range(R):
        count_tents = 0
        for c in range(C):
            if tree_mask[r, c] == 0 and int(solution[r, c]) != 5:
                count_tents += 1
        violation += abs(count_tents - int(row_counts[r]))
    
    for c in range(C):
        count_tents = 0
        for r in range(R):
            if tree_mask[r, c] == 0 and int(solution[r, c]) != 5:
                count_tents += 1
        violation += abs(count_tents - int(col_counts[c]))
    
    return violation

def extract_solution_details(solution, tree_mask):
    """
    Extracts a list of solution details for output.
    For each cell that is not a tree and has a tent placed (state != 5),
    return a tuple (row, col, letter) using 1-indexed coordinates.
    """
    R, C = solution.shape
    details = []
    # Mapping from state to letter:
    state_to_letter = {0: "U", 1: "D", 2: "L", 3: "R", 4: "X"}
    for r in range(R):
        for c in range(C):
            if tree_mask[r, c] == 1:
                continue  # skip tree cells
            state = int(solution[r, c])
            if state != 5:
                details.append((r + 1, c + 1, state_to_letter[state]))
    return details

def main():
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    # ----- Input Parsing -----
    data = sys.stdin.read().strip().splitlines()
    if not data:
        return
    # First line: rows and columns
    R, C = map(int, data[0].split())
    # Next line: required tent counts per row
    row_targets = torch.tensor(list(map(float, data[1].split())), device=device)
    # Next line: required tent counts per column
    col_targets = torch.tensor(list(map(float, data[2].split())), device=device)
    # Next R lines: grid (each character is either '.' or 'T')
    grid = data[3:3+R]

    # Create a tensor for trees: 1 for tree, 0 for blank.
    tree_grid = torch.zeros((R, C), device=device)
    for i in range(R):
        for j in range(C):
            if grid[i][j] == 'T':
                tree_grid[i, j] = 1.0

    # ----- Set Up Learnable Tent Variables -----
    # We assign a learnable logit for each cell.
    tent_logits = torch.nn.Parameter(torch.randn((6, R, C), device=device))
    optimizer = torch.optim.Adam([tent_logits], lr=0.1)

    # ----- Optimization Loop -----
    iterations = 5000
    for it in range(iterations):
        optimizer.zero_grad()
        
        total_loss, loss_adj, loss_tree, loss_rc = composite_loss(
            tent_logits, tree_grid, row_targets, col_targets,
            lambda_adj=1.0, lambda_tree=1.0, lambda_rc=1.0, alpha=1.0)
        
        total_loss.backward()
        optimizer.step()

        if it % 500 == 0:
            print(f"Iter {it}: Total Loss = {total_loss.item():.4f} | "
                  f"Adjacency Loss = {loss_adj.item():.4f} | "
                  f"Tree-Tent Loss = {loss_tree.item():.4f} | "
                  f"Row/Col Loss = {loss_rc.item():.4f}", file=sys.stderr)

    # ----- Convert to Discrete Solution & Compute Violations -----
    solution = discrete_solution(tent_logits).cpu()
    tree_grid_cpu = tree_grid.cpu()
    total_violations = count_violations(solution, tree_grid_cpu, row_targets.cpu(), col_targets.cpu())
    
    # Extract tent details for output.
    tent_details = extract_solution_details(solution, tree_grid_cpu)
    T = len(tent_details)
    
    # ----- Output in Required Format -----
    # First line: total number of violations.
    # Second line: number of tents added.
    # Next T lines: each tent's 1-indexed row, 1-indexed column, and direction.
    print(total_violations)
    print(T)
    for row, col, direction in tent_details:
        print(f"{row} {col} {direction}")


if __name__ == '__main__':
    main()
