import sys
import torch
import torch.nn.functional as F

def main():
    # ----- Input Parsing -----
    data = sys.stdin.read().strip().splitlines()
    if not data:
        return
    # First line: rows and columns
    R, C = map(int, data[0].split())
    # Next line: required tent counts per row
    row_targets = list(map(float, data[1].split()))
    # Next line: required tent counts per column
    col_targets = list(map(float, data[2].split()))
    # Next R lines: grid (each character is either '.' or 'T')
    grid = data[3:3+R]

    # Create a tensor for trees: 1 for tree, 0 for blank.
    tree_grid = torch.zeros((R, C))
    for i in range(R):
        for j in range(C):
            if grid[i][j] == 'T':
                tree_grid[i, j] = 1.0
    # Only blank cells (value 1) can receive a tent.
    tent_mask = 1.0 - tree_grid

    # ----- Set Up Learnable Tent Variables -----
    # We assign a learnable logit for each cell; later we’ll use sigmoid to get a probability.
    # We multiply by tent_mask so that cells with trees remain 0.
    tent_logits = torch.nn.Parameter(torch.randn((R, C)))
    optimizer = torch.optim.Adam([tent_logits], lr=0.1)

    # ----- Define Convolution Kernels for Neighbor Relationships -----
    # For adjacent-tent checks: 8-neighbors (all surrounding cells)
    kernel_adj = torch.ones((1, 1, 3, 3))
    kernel_adj[0, 0, 1, 1] = 0.0  # exclude center

    # For tree-tent pairing, consider only horizontal and vertical neighbors.
    kernel_pair = torch.tensor([[0.0, 1.0, 0.0],
                                [1.0, 0.0, 1.0],
                                [0.0, 1.0, 0.0]]).reshape(1, 1, 3, 3)

    # Preformat the tree grid for convolution.
    tree_grid_tensor = tree_grid.unsqueeze(0).unsqueeze(0)  # shape: (1,1,R,C)

    # ----- Optimization Loop -----
    iterations = 5000
    for it in range(iterations):
        optimizer.zero_grad()
        # Compute tent probability in each cell; force 0 on trees by multiplying with tent_mask.
        tent_prob = torch.sigmoid(tent_logits) * tent_mask  # shape: (R,C)

        # 1. Adjacent tent penalty: if a tent’s probability is high and its 8-neighbor sum is high, that’s bad.
        tent_prob_4d = tent_prob.unsqueeze(0).unsqueeze(0)  # shape: (1,1,R,C)
        neighbor_tents = F.conv2d(tent_prob_4d, kernel_adj, padding=1)[0, 0]
        loss_adj = (tent_prob * neighbor_tents).sum()

        # 2. Tent-to-tree pairing (tent must have at least one horizontal/vertical tree)
        # Convolve the fixed tree grid with the kernel so that each cell gets the number of adjacent trees.
        tree_neighbors = F.conv2d(tree_grid_tensor, kernel_pair, padding=1)[0, 0]
        # For cells with no adjacent tree, add penalty for any tent probability.
        tree_neighbor_mask = (tree_neighbors > 0).float()
        loss_tent_tree = (tent_prob * (1 - tree_neighbor_mask)).sum()

        # 3. Tree-to-tent pairing (each tree must have at least one adjacent tent)
        conv_tent = F.conv2d(tent_prob_4d, kernel_pair, padding=1)[0, 0]
        loss_tree_tent = (tree_grid * F.relu(1 - conv_tent)).sum()

        # 4. Row count penalty: square error between actual tent sum and required count.
        loss_row = 0.0
        for i in range(R):
            row_sum = tent_prob[i, :].sum()
            loss_row += (row_sum - row_targets[i])**2

        # 5. Column count penalty.
        loss_col = 0.0
        for j in range(C):
            col_sum = tent_prob[:, j].sum()
            loss_col += (col_sum - col_targets[j])**2

        # Weights for each loss term
        w_adj = 1.0
        w_tent_tree = 5.0
        w_tree_tent = 5.0
        w_row = 1.0
        w_col = 1.0

        total_loss = (w_adj * loss_adj +
                      w_tent_tree * loss_tent_tree +
                      w_tree_tent * loss_tree_tent +
                      w_row * loss_row +
                      w_col * loss_col)

        total_loss.backward()
        optimizer.step()

        if it % 500 == 0:
            # Write progress info to stderr so as not to interfere with final output.
            print(f"Iter {it}: Loss = {total_loss.item():.4f}, "
                  f"loss_adj = {loss_adj.item():.4f}, "
                  f"loss_tent_tree = {loss_tent_tree.item():.4f}, "
                  f"loss_tree_tent = {loss_tree_tent.item():.4f}, "
                  f"loss_row = {loss_row:.4f}, "
                  f"loss_col = {loss_col:.4f}", file=sys.stderr)

    # ----- Extract Final Tent Placements -----
    with torch.no_grad():
        tent_prob = torch.sigmoid(tent_logits) * tent_mask
        # Threshold at 0.5 to decide whether a tent is placed.
        final_tents = (tent_prob > 0.5).float()

    # ----- Count Violations as Specified -----
    violations = 0

    # a. For each tent: if it has any adjacent tent (in any of 8 directions), add 1 violation.
    final_tents_4d = final_tents.unsqueeze(0).unsqueeze(0)
    neighbor_tents_final = F.conv2d(final_tents_4d, kernel_adj, padding=1)[0, 0]
    for i in range(R):
        for j in range(C):
            if final_tents[i, j] > 0.5:  # a tent is placed here
                if neighbor_tents_final[i, j] > 0:
                    violations += 1

    # b. For each tent: if no adjacent tree horizontally or vertically, add 1 violation.
    for i in range(R):
        for j in range(C):
            if final_tents[i, j] > 0.5:
                paired = False
                # Check Up
                if i > 0 and tree_grid[i-1, j] > 0:
                    paired = True
                # Check Down
                if i < R-1 and tree_grid[i+1, j] > 0:
                    paired = True
                # Check Left
                if j > 0 and tree_grid[i, j-1] > 0:
                    paired = True
                # Check Right
                if j < C-1 and tree_grid[i, j+1] > 0:
                    paired = True
                if not paired:
                    violations += 1

    # c. For each tree: if it does not have a tent adjacent horizontally or vertically, add 1 violation.
    for i in range(R):
        for j in range(C):
            if tree_grid[i, j] > 0.5:
                paired = False
                if i > 0 and final_tents[i-1, j] > 0.5:
                    paired = True
                if i < R-1 and final_tents[i+1, j] > 0.5:
                    paired = True
                if j > 0 and final_tents[i, j-1] > 0.5:
                    paired = True
                if j < C-1 and final_tents[i, j+1] > 0.5:
                    paired = True
                if not paired:
                    violations += 1

    # d. Row count violations: each row gets one violation for every tent too many or too few.
    for i in range(R):
        row_count = int(final_tents[i, :].sum().item())
        violations += abs(row_count - int(row_targets[i]))

    # e. Column count violations.
    for j in range(C):
        col_count = int(final_tents[:, j].sum().item())
        violations += abs(col_count - int(col_targets[j]))

    # ----- Prepare Output: For each tent, also decide its associated tree’s direction.
    # For each tent cell, check its horizontal/vertical neighbors in order: Up, Down, Left, Right.
    tent_positions = []
    for i in range(R):
        for j in range(C):
            if final_tents[i, j] > 0.5:
                direction = 'X'
                if i > 0 and tree_grid[i-1, j] > 0.5:
                    direction = 'U'
                elif i < R-1 and tree_grid[i+1, j] > 0.5:
                    direction = 'D'
                elif j > 0 and tree_grid[i, j-1] > 0.5:
                    direction = 'L'
                elif j < C-1 and tree_grid[i, j+1] > 0.5:
                    direction = 'R'
                # Use 1-indexed coordinates.
                tent_positions.append((i+1, j+1, direction))

    # ----- Output the Final Solution -----
    # First line: total violations.
    print(violations)
    # Second line: number of tents placed.
    print(len(tent_positions))
    # Next T lines: each tent’s row, column, and pairing direction.
    for pos in tent_positions:
        print(pos[0], pos[1], pos[2])

if __name__ == '__main__':
    main()
