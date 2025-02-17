import argparse
import numpy as np
import os


def parse_arguments():
    parser = argparse.ArgumentParser(description='Generate Tents and Trees test case')

    # Add the two mandatory arguments
    parser.add_argument("Rows", type=int, help="The number of rows the graph will contain")
    parser.add_argument("Cols", type=int, help="The number of columns")

    # Add the two optional arguments
    parser.add_argument("-o", help="What and where you want the file to be made")
    parser.add_argument("--trees", type=int, help="Number of trees you want generated")

    # Add row and columns bell curve weight constant (more is more tentability and less is less tentability)
    parser.add_argument("-w", type=float, help="Constant weight added to tent curve higher number means higher row and column values")

    return parser.parse_args()


def generate_trees(board, rows, cols, num_of_trees):
    num_of_tiles = rows * cols

    # If num of trees wasn't set
    if num_of_trees is None:
        # Set the number of trees to a bell curve around 1/4 of the tiles
        num_of_trees = int(np.random.normal(loc=(num_of_tiles / 4), scale=4.0))
        # If trees are outside of bounds set to either max or min value
        num_of_trees = max(0, min(num_of_trees, num_of_tiles))

    trees_remaining = num_of_trees
    tiles_remaining = num_of_tiles

    # For each tile, iterate to see if a tree gets placed
    for x in range(cols):
        for y in range(rows):
            probability = trees_remaining / tiles_remaining
            if np.random.uniform(0, 1) < probability:
                board[y][x] = 'T'
                trees_remaining -= 1
            tiles_remaining -= 1

    return board


def calculate_tents_for_rows(board, rows, cols, weight):
    # Initialize variables
    previous_row = 0
    current_row = 0
    next_row = 0
    row_tent_num = []

    for y in range(rows):
        num_adjacent_row = 3
        adjacent_row_total = 0

        # If this row is the first on the board
        if y == 0:
            for x in range(cols):
                if board[y][x] == 'T':
                    current_row += 1
                if rows != 1:
                    if board[y + 1][x] == 'T':
                        next_row += 1
            adjacent_row_total = current_row + next_row
            num_adjacent_row = 2

        elif y != 0 and y != rows - 1:  # Middle rows
            for x in range(cols):
                if board[y + 1][x] == 'T':
                    next_row += 1
            adjacent_row_total = previous_row + current_row + next_row

        elif y == rows - 1:  # Last row
            adjacent_row_total = previous_row + current_row
            num_adjacent_row = 2

        # Calculate tents for the row
        num_tents = int(np.random.normal((adjacent_row_total / num_adjacent_row) + weight, 1))
        row_tent_num.append(max(0, num_tents))  # Ensure non-negative

        # Update row counters
        previous_row = current_row
        current_row = next_row
        next_row = 0

    return row_tent_num


def calculate_tents_for_columns(board, rows, cols, weight):
    # Initialize variables
    previous_col = 0
    current_col = 0
    next_col = 0
    col_tent_num = []

    for x in range(cols):
        num_adjacent_col = 3
        adjacent_col_total = 0

        # If this col is the first on the board
        if x == 0:
            for y in range(rows):
                if board[y][x] == 'T':
                    current_col += 1
                if cols != 1:
                    if board[y][x + 1] == 'T':
                        next_col += 1
            adjacent_col_total = current_col + next_col
            num_adjacent_col = 2

        elif x != 0 and x != cols - 1:  # Middle columns
            for y in range(rows):
                if board[y][x + 1] == 'T':
                    next_col += 1
            adjacent_col_total = previous_col + current_col + next_col

        elif x == cols - 1:  # Last column
            adjacent_col_total = previous_col + current_col
            num_adjacent_col = 2

        # Calculate tents for the column
        num_tents = int(np.random.normal((adjacent_col_total / num_adjacent_col) + weight, 1))
        col_tent_num.append(max(0, num_tents))  # Ensure non-negative

        # Update column counters
        previous_col = current_col
        current_col = next_col
        next_col = 0

    return col_tent_num


def draw(board, row_tent_num, col_tent_num):
    
    rows = len(board)
    cols = len(board[0])

    # Print column numbers on top
    print("    ", end="")  # Space for row numbers on the left
    for col_num in col_tent_num:
        print(f"{col_num:2} ", end="")  # Adjust spacing as needed
    print()  # Move to the next line

    # Print a separating line for clarity
    print("   +" + "---" * cols + "+")

    # Print each row
    for row_idx in range(rows):
        # Print row number (row_tent_num) on the left
        print(f"{row_tent_num[row_idx]:2} |", end="")  # Row tent count with border

        # Print the board's content
        for col_idx in range(cols):
            print(f" {board[row_idx][col_idx]} ", end="")
        print("|")  # Close the row with a border

    # Print the bottom border
    print("   +" + "---" * cols + "+")


def print_output(output_location, board, row_tent_num, col_tent_num):

    # Check if file already exists
    if os.path.exists(output_location):
        print("Error: There is already a file in this location")
        exit()

    # Ensure the directory exists
    os.makedirs(os.path.dirname(output_location), exist_ok=True)

    with open(output_location, "w") as file:
        # Write the dimensions of the board
        file.write(f"{len(row_tent_num)} {len(col_tent_num)}\n")

        # Write row tent numbers
        for row in row_tent_num:
            file.write(f"{row} ")
        file.write("\n")  # Newline after row tent numbers

        # Write column tent numbers
        for col in col_tent_num:
            file.write(f"{col} ")
        file.write("\n")  # Newline after column tent numbers

        # Write the board
        for y in range(len(row_tent_num)):
            for x in range(len(col_tent_num)):
                file.write(board[y][x])  # Write cell content
            file.write("\n")  # Newline after each row

    print("Saved to " + output_location)


def find_unique_filename():

    base_dir = "tests"
    prefix = "test"

    #Does directory exist if not make it
    os.makedirs("tests", exist_ok=True)

    #Find next index test that doesn't already exist
    index = 1
    while True:
        file_path = os.path.join(base_dir, f"{prefix}{index}")
        if os.path.exists(file_path) == False:
            return file_path
        index += 1


def main():

    # Parse arguments
    args = parse_arguments()

    # Set variables
    rows = args.Rows
    cols = args.Cols
    weight = args.w
    output_location = args.o
    num_of_trees = args.trees

    if output_location == None:
        output_location = find_unique_filename()

    if weight == None:
        weight = np.random.normal(0.5, 0.15)
        print(weight)

    while True:

        # Initialize board
        board = [['.' for _ in range(cols)] for _ in range(rows)]

        # Generate trees
        board = generate_trees(board, rows, cols, num_of_trees)

        # Calculate tents for rows
        row_tent_num = calculate_tents_for_rows(board, rows, cols, weight)

        # Calculate tents for columns
        col_tent_num = calculate_tents_for_columns(board, rows, cols, weight)

        # Draw the board 
        draw(board, row_tent_num, col_tent_num)

        print("Do you like this output? Y|N")
        response = input()

        if response == "Y":
            break

    # Print the output to file or stdout
    print_output(output_location, board, row_tent_num, col_tent_num)


if __name__ == "__main__":
    main()
