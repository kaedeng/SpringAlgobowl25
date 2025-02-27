import sys
import torch
import torch.nn.functional as F
import itertools

def tent_adjacency_violations(solution, tree_mask):
    violation = 0
    R, C = solution.shape
    for r in range(R):
        for c in range(C):
            if tree_mask[r, c] == 1 or int(solution[r, c]) == 5:
                continue
            neighbors = [(r, c+1), (r+1, c+1), (r+1, c), (r+1, c-1)]
            for nr, nc in neighbors:
                if 0 <= nr < R and 0 <= nc < C:
                    if tree_mask[nr, nc] == 0 and int(solution[nr, nc]) != 5:
                        violation += 1
                        break
    return violation

def tent_side_matching_violations(solution, tree_mask):
    violation = 0
    R, C = solution.shape
    for r in range(R):
        for c in range(C):
            if tree_mask[r, c] == 1 or int(solution[r, c]) == 5:
                continue
            found_tree = False
            for dr, dc in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
                nr, nc = r + dr, c + dc
                if 0 <= nr < R and 0 <= nc < C:
                    if tree_mask[nr, nc] == 1:
                        found_tree = True
                        break
            if not found_tree:
                print(f"[DEBUG]: Tent at ({r}, {c}) has no adjacent tree in any direction.", file=sys.stderr)
                violation += 1
    return violation

def tree_matching_violations(solution, tree_mask):
    violation = 0
    R, C = solution.shape
    for r in range(R):
        for c in range(C):
            if tree_mask[r, c] != 1:
                continue
            associated = 0
            if r+1 < R and tree_mask[r+1, c] == 0 and int(solution[r+1, c]) == 0:
                associated += 1
            if r-1 >= 0 and tree_mask[r-1, c] == 0 and int(solution[r-1, c]) == 1:
                associated += 1
            if c+1 < C and tree_mask[r, c+1] == 0 and int(solution[r, c+1]) == 2:
                associated += 1
            if c-1 >= 0 and tree_mask[r, c-1] == 0 and int(solution[r, c-1]) == 3:
                associated += 1
            if associated != 1:
                print(f"[DEBUG]: Tree at ({r}, {c}) has {associated} associated tent(s), violation = 1", file=sys.stderr)
                violation += 1
    print(f"[DEBUG]: Oh so lonely tree violations: {violation}", file=sys.stderr)
    return violation

def row_column_violations(solution, tree_mask, row_counts, col_counts):
    violation = 0
    R, C = solution.shape
    for r in range(R):
        count_tents = 0
        for c in range(C):
            if tree_mask[r, c] == 0 and int(solution[r, c]) != 5:
                count_tents += 1
        diff = abs(count_tents - int(row_counts[r]))
        print(f"Row {r}: count_tents = {count_tents}, target = {row_counts[r]}, diff = {diff}", file=sys.stderr)
        violation += diff
    for c in range(C):
        count_tents = 0
        for r in range(R):
            if tree_mask[r, c] == 0 and int(solution[r, c]) != 5:
                count_tents += 1
        diff = abs(count_tents - int(col_counts[c]))
        print(f"Column {c}: count_tents = {count_tents}, target = {col_counts[c]}, diff = {diff}", file=sys.stderr)
        violation += diff
    return violation

def count_violations(solution, tree_mask, row_counts, col_counts):
    rc_violations = row_column_violations(solution, tree_mask, row_counts, col_counts)
    tent_adj = tent_adjacency_violations(solution, tree_mask)
    tent_side = tent_side_matching_violations(solution, tree_mask)
    tree_match = tree_matching_violations(solution, tree_mask)
    total = rc_violations + tent_adj + tent_side + tree_match
    print(f"[INFO]: Computed violations: {total}", file=sys.stderr)
    return total, tent_adj, tent_side, rc_violations, tree_match

def extract_solution_details(solution, tree_mask):
    R, C = solution.shape
    details = []
    state_to_letter = {0: "U", 1: "D", 2: "L", 3: "R", 4: "X"}
    for r in range(R):
        for c in range(C):
            if tree_mask[r, c] == 1:
                continue
            state = int(solution[r, c])
            if state != 5:
                if state == 0 and r == 0:
                    state = 4
                elif state == 1 and r == R - 1:
                    state = 4
                elif state == 2 and c == 0:
                    state = 4
                elif state == 3 and c == C - 1:
                    state = 4
                details.append((r + 1, c + 1, state_to_letter[state]))
    return details

def discrete_solution(tent_logits, tau=0.1, hard=True):
    gumbel_sample = F.gumbel_softmax(tent_logits, tau=tau, hard=hard, dim=0)
    solution = torch.argmax(gumbel_sample, dim=0)
    return solution

def lonely_tree_loss(tent_probs, tree_mask, threshold=1.0, beta=20.0):
    kernel = torch.tensor([[0, 1, 0],
                           [1, 0, 1],
                           [0, 1, 0]],
                          device=tent_probs.device,
                          dtype=tent_probs.dtype).unsqueeze(0).unsqueeze(0)
    tent_placed = tent_probs[:5].sum(dim=0)
    # Sharpen the probabilities with a steep sigmoid.
    tent_indicator = torch.sigmoid(beta * (tent_placed - 0.5))
    tent_indicator_4d = tent_indicator.unsqueeze(0).unsqueeze(0)
    m = F.conv2d(tent_indicator_4d, kernel, padding=1).squeeze(0).squeeze(0)
    loss = torch.clamp(threshold - m, min=0) * tree_mask
    return loss.sum()

def tent_count_incentive_loss(tent_probs, row_counts, col_counts):
    """
    Penalize if the expected number of tents (states 0-4) in a row/column is lower than the target.
    We use the absolute error to encourage more tent placement.
    """
    tent_placed = tent_probs[:5].sum(dim=0)
    row_sum = tent_placed.sum(dim=1)
    col_sum = tent_placed.sum(dim=0)
    loss_rows = torch.abs(row_counts - row_sum).sum()
    loss_cols = torch.abs(col_counts - col_sum).sum()
    return loss_rows + loss_cols

def composite_loss(tent_logits, tree_mask, row_counts, col_counts,
                   lambda_adj=1.0, lambda_tree=1.0, lambda_rc=1.0, lambda_lonely=1.0, lambda_incentive=1.0, alpha=1.0):
    tent_probs = torch.softmax(tent_logits, dim=0)
    loss_adj = tent_adjacency_loss(tent_probs, alpha=alpha)
    loss_tree = tree_tent_matching_loss(tent_probs, tree_mask)
    loss_rc = row_column_count_loss(tent_probs, row_counts, col_counts)
    loss_lonely = lonely_tree_loss(tent_probs, tree_mask, threshold=1.0)
    loss_incentive = tent_count_incentive_loss(tent_probs, row_counts, col_counts)
    total_loss = (lambda_adj * loss_adj +
                  lambda_tree * loss_tree +
                  lambda_rc * loss_rc +
                  lambda_lonely * loss_lonely +
                  lambda_incentive * loss_incentive)
    return total_loss, loss_adj, loss_tree, loss_rc, loss_lonely, loss_incentive

def tent_adjacency_loss(tent_probs, alpha=1.0):
    tent_placed = tent_probs[:5].sum(dim=0)
    tent_placed_4d = tent_placed.unsqueeze(0).unsqueeze(0)
    kernel = torch.tensor([[1.0, 1.0, 1.0],
                           [1.0, 0.0, 1.0],
                           [1.0, 1.0, 1.0]], device=tent_probs.device).unsqueeze(0).unsqueeze(0)
    neighbor_sum = F.conv2d(tent_placed_4d, kernel, padding=1)
    neighbor_sum = neighbor_sum.squeeze(0).squeeze(0)
    penalty = tent_placed * (1 - torch.exp(-alpha * neighbor_sum))
    return penalty.sum()

def tree_tent_matching_loss(tent_probs, tree_mask):
    loss_tents = 0.0
    R, C = tree_mask.shape
    tree_up = torch.zeros_like(tree_mask)
    tree_up[1:, :] = tree_mask[:-1, :]
    loss_tents += (tent_probs[0] * (1 - tree_up)).sum()
    tree_down = torch.zeros_like(tree_mask)
    tree_down[:-1, :] = tree_mask[1:, :]
    loss_tents += (tent_probs[1] * (1 - tree_down)).sum()
    tree_left = torch.zeros_like(tree_mask)
    tree_left[:, 1:] = tree_mask[:, :-1]
    loss_tents += (tent_probs[2] * (1 - tree_left)).sum()
    tree_right = torch.zeros_like(tree_mask)
    tree_right[:, :-1] = tree_mask[:, 1:]
    loss_tents += (tent_probs[3] * (1 - tree_right)).sum()
    loss_tents += tent_probs[4].sum()
    
    m = torch.zeros_like(tree_mask)
    m[:-1, :] += tent_probs[0, 1:, :]
    m[1:, :] += tent_probs[1, :-1, :]
    m[:, :-1] += tent_probs[2, :, 1:]
    m[:, 1:] += tent_probs[3, :, :-1]
    
    diff = m - 1
    # Use squared error for under-attachment and cubic for over-attachment.
    loss_trees = torch.where(diff <= 0, diff**2, diff**3)
    loss_trees = (loss_trees * tree_mask).sum()
    
    return loss_tents + loss_trees

def row_column_count_loss(tent_probs, row_counts, col_counts):
    tent_placed = tent_probs[:5].sum(dim=0)
    row_sum = tent_placed.sum(dim=1)
    col_sum = tent_placed.sum(dim=0)
    loss_rows = ((row_sum - row_counts)**2).sum()
    loss_cols = ((col_sum - col_counts)**2).sum()
    return loss_rows + loss_cols

def grid_search():
    candidates_adj = [5.0, 6.0, 7.0]
    candidates_tree = [9.0, 9.5, 10.0]
    candidates_rc = [17.0, 19.0, 21.0]
    candidates_lonely = [10.0, 12.0]
    candidates_alpha = [2.0, 3.0]
    
    best_violation = float('inf')
    best_params = None

    for lam_adj, lam_tree, lam_rc, lam_lonely, alpha in itertools.product(
            candidates_adj, candidates_tree, candidates_rc, candidates_lonely, candidates_alpha):
        
        tent_logits = torch.nn.Parameter(torch.randn((6, R, C), device=device))
        optimizer = torch.optim.Adam([tent_logits], lr=0.01)
        iterations = 5000

        for it in range(iterations):
            optimizer.zero_grad()
            total_loss, loss_adj, loss_tree, loss_rc, loss_lonely_val, loss_incentive = composite_loss(
                tent_logits, tree_grid, row_targets, col_targets,
                lambda_adj=lam_adj, lambda_tree=lam_tree, lambda_rc=lam_rc,
                lambda_lonely=lam_lonely, lambda_incentive=10.0, alpha=alpha)
            total_loss.backward()
            optimizer.step()

        solution = discrete_solution(tent_logits).cpu()
        tot_v, _, _, _, _ = count_violations(solution, tree_grid.cpu(), row_targets.cpu(), col_targets.cpu())
        print(f"Params: lam_adj={lam_adj}, lam_tree={lam_tree}, lam_rc={lam_rc}, lam_lonely={lam_lonely}, alpha={alpha} -> Violations = {tot_v}", file=sys.stderr)
        if tot_v < best_violation:
            best_violation = tot_v
            best_params = (lam_adj, lam_tree, lam_rc, lam_lonely, alpha)
    
    print("Best combination:", best_params, "with violations:", best_violation)

def main():
    global R, C, tree_grid, row_targets, col_targets, device
    device = torch.device("cpu")
    with open(sys.argv[1], 'r') as f:
        data = f.read().strip().splitlines()
    if not data:
        return
    R, C = map(int, data[0].split())
    row_targets = torch.tensor(list(map(float, data[1].split())), device=device)
    col_targets = torch.tensor(list(map(float, data[2].split())), device=device)
    grid = data[3:3+R]

    tree_grid = torch.zeros((R, C), device=device)
    for i in range(R):
        for j in range(C):
            if grid[i][j] == 'T':
                tree_grid[i, j] = 1.0

    # Uncomment one of the following:
    # Option 1: Run grid search
    # grid_search()
    
    # Option 2: Train with a fixed hyperparameter combination.
    tent_logits = torch.nn.Parameter(torch.randn((6, R, C), device=device))
    optimizer = torch.optim.Adam([tent_logits], lr=0.001)
    iterations = 100000
    for it in range(iterations):
        optimizer.zero_grad()
        total_loss, loss_adj, loss_tree, loss_rc, loss_lonely_val, loss_incentive = composite_loss(
            tent_logits, tree_grid, row_targets, col_targets,
            lambda_adj=9, lambda_tree=10, lambda_rc=500, lambda_lonely=600.0, lambda_incentive=100.0, alpha=2.0)
        total_loss.backward()
        optimizer.step()
        if it % 500 == 0:
            print(f"Iter {it}: Total Loss = {total_loss.item():.4f} | "
                  f"Adjacency Loss = {loss_adj.item():.4f} | "
                  f"Tree-Tent Loss = {loss_tree.item():.4f} | "
                  f"Row/Col Loss = {loss_rc.item():.4f} | "
                  f"Lonely Tree Loss = {loss_lonely_val.item():.4f} | "
                  f"Incentive Loss = {loss_incentive.item():.4f}", file=sys.stderr)
    
    solution = discrete_solution(tent_logits).cpu()
    tree_grid_cpu = tree_grid.cpu()
    total_v, va, vm, vrc, vtm = count_violations(solution, tree_grid_cpu, row_targets.cpu(), col_targets.cpu())
    tent_details = extract_solution_details(solution, tree_grid_cpu)
    T = len(tent_details)
    print(total_v)
    print(T)
    for row, col, direction in tent_details:
        print(f"{row} {col} {direction}")

if __name__ == '__main__':
    main()
