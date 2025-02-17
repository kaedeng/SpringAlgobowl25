"""
Autograder/verifier for the tents and trees Algobowl competition

This script takes in two files, and has two functions
1: checks the validity of the input/output
2: if 1 passes, checks the number of violations in the output

Written by Kaelem Deng (unfortunately...)
Not my proudest bit of code; forgive me for my transgressions onto your eyes.
"""

import sys
import argparse
from typing import Optional, List, Tuple

def checkValidity(chart: list) -> bool:
    '''
    This should take in a 2D list and then return a boolean of whether or not this chart follows the rules
    '''
    return True

def countViolations(chart: list) -> int:
    '''
    This should take in a 2D list and then return an integer of how many violations there are
    '''
    return 0

def parseInputFile(file_name: str) -> Optional[Tuple[int, int, List[int], List[int], List[List[Optional[str]]]]]:
    '''
    | Parses files that hold the information on trees and row/col tent counts
    | Returns None if it errors
    | Returns (rows, cols, row_counts, col_counts, grid) otherwise
    '''

    # Try opening the file
    try:
        with open(file_name, 'r') as f:
            lines = f.read().strip().splitlines()
    except Exception as e:
        print(f"Error reading input file {file_name}: {e}")
        return None
    
    # Check if the size of the file matches, need atleast 4 lines
    if len(lines) < 4:
        print("Input file format error: Not enough lines.")
        return None

    # First line (R, C)
    # Then check if they are integers
    parts = lines[0].split()
    if len(parts) < 2:
        print("Input file format error: First line must contain two numbers.")
        return None
    try:
        rows = int(parts[0])
        cols = int(parts[1])
    except ValueError:
        print("Input file format error: Invalid numbers in first line.")
        return None

    # Check if the next lines can be split and mapped as integers, they are the row/column tent count lines.
    try:
        row_counts = list(map(int, lines[1].split()))
        col_counts = list(map(int, lines[2].split()))
    except ValueError:
        print("Input file format error: Row/column counts must be integers.")
        return None

    # Check if the count of the row/col tent counts match the size given on line 1
    if len(row_counts) != rows or len(col_counts) != cols:
        print("Input columns and/or row counts's size does not equal the number of rows/columns")
        return None

    # Check if the rest of the lines match the number of rows
    grid_lines = lines[3:]
    if len(grid_lines) != rows:
        print("Input file format error: Number of grid rows does not match.")
        return None

    # Time to loop through the rest of the input
    grid = []
    for line in grid_lines:
        # check the count of the columns
        if len(line) != cols:
            print("Input file format error: A grid row does not match the specified column count.")
            return None
        
        # Init a list and add the tree info onto it
        row_list = []
        for char in line:
            if char == 'T':
                row_list.append("tree")
            else:
                row_list.append(None)
        grid.append(row_list)

    # return all necessary info
    return rows, cols, row_counts, col_counts, grid

def parseOutputFile(fileName: str, grid: List[List]) -> Optional[Tuple[int, int, List[Tuple[int, int, str]], List[List]]]:
    '''
    | Parses files that hold the information on tents and violations
    | Returns None if it errors
    | Returns (violations, tents_added, grid) otherwise
    '''

    # Try opening the file
    try:
        with open(fileName, 'r') as f:
            lines = f.read().strip().splitlines()
    except Exception as e:
        print(f"Error reading output file {fileName}: {e}")
        return None

    # Needs to be atleast 2 lines
    if len(lines) < 2:
        print("Output file format error: Not enough lines.")
        return None

    # Try setting violations and tents added as integers
    try:
        violations = int(lines[0].strip())
        tents_count = int(lines[1].strip())
    except ValueError:
        print("Output file format error: First two lines must be integers.")
        return None

    # Time for loop
    tents_added = []
    allowed_symbols = {"U", "D", "L", "R", "X"}
    for line in lines[2:]:
        tent_info = line.split()
        
        # Check if the line is valid
        if len(tent_info) != 3:
            print("Output file format error: Each operation line must have 3 values (row, col, symbol).")
            return None

        # Try mapping first two to int
        try:
            row = int(tent_info[0])
            col = int(tent_info[1])
        except ValueError:
            print("Output file format error: Row and column values must be integers.")
            return None

        # Check grid bounds.
        if not (1 <= row <= len(grid) and 1 <= col <= len(grid[0])):
            print(f"Operation ({row}, {col}) is out of grid bounds.")
            return None

        # Check if the symbol is valid
        symbol = tent_info[2]
        if symbol not in allowed_symbols:
            print("Output file format error: Symbol must be one of U, D, L, R, or X.")
            return None

        # Check if it will sit on top of a tree (BAD!!)
        if grid[row - 1][col - 1] == "tree":
            print("Cannot place tent on a tree, invalid operation")
            return None
        
        # If the symbol indicates an associated tree, check that the tree exists in the proper direction.
        if symbol != "X":
            tree_row, tree_col = None, None
            match symbol:
                case "U":
                    tree_row, tree_col = row - 2, col - 1
                case "D":
                    tree_row, tree_col = row, col - 1
                case "L":
                    tree_row, tree_col = row - 1, col - 2
                case "R":
                    tree_row, tree_col = row - 1, col
                case _:
                    print("Invalid symbol")
                    return None
            
            # Check that the tree's position is within the grid.
            if not (0 <= tree_row < len(grid) and 0 <= tree_col < len(grid[0])):
                print(f"Operation ({row}, {col}) with symbol {symbol} is invalid: tree position out of bounds.")
                return None

            # Verify that a tree is present in the expected adjacent cell.
            if grid[tree_row][tree_col] != "tree":
                print(f"Operation ({row}, {col}) with symbol {symbol} is invalid: no tree found in the expected direction.")
                return None

        # If all checks pass, update the grid by placing the tent.
        grid[row - 1][col - 1] = "tent"
        tents_added.append((row, col, symbol))

    return violations, tents_count,tents_added, grid, 
        
def parseFile(inputFileName: str, outputFileName: str) -> Optional[Tuple[int, int, List, List, int, int, List[Tuple[int, int, str]], Optional[List[List[Optional[str]]]]]]:
    '''
    As said above, return the tuple.
    Need to take two files and parse it in
    '''

    inputContents = parseInputFile(inputFileName)

    if inputContents == None:
        return None
    r, c, r_count, c_count, grid = inputContents

    outputContents = parseOutputFile(outputFileName, grid)
    
    if outputContents == None:
        return None
    violations, tents_count, tents_added, grid = outputContents

    return r, c, r_count, c_count, violations, tents_count, tents_added, grid



def getAndParse(args: argparse.Namespace) -> Optional[Tuple[int, int, List, List, int, int, List[Tuple[int, int, str]], Optional[List[List[Optional[str]]]]]]:
    '''
    | Uses the provided arguments (from argparse) to parse both input and output files.
    | Returns the parsed data or None if parsing fails.
    | Calls the parseFile function, which should return as a tuple, (numOfViolations, tentsAdded, [<count of tents in rows>], [<count of tents in colunns>], 2D array)
    | contains either None, 'tree', or 'tent'.

    | This is an example input file (that we need to input), parse through it and return to the above array
    | 3 4
    | 0 0 3
    | 1 1 2 1
    | .T..
    | T.T.
    | ...T
    |
    | This is an example output file (that we need to input), parse through this to return the above array
    | 11
    | 4
    | 1 1 R
    | 1 4 X
    | 2 2 L
    | 3 3 U
    |
    | This should then be
    | (11, 4, [0, 0, 3], [1, 1, 2, 1], 
    |     [
    |         ["tent", "tree", None, None],
    |         ["tree", "tent", "tree", None],
    |         [None, None, "tent", "tree"],
    |     ]
    | )
    '''
    # Janky conditions and bad logic lol, refactor if needed
    if len(sys.argv) < 3:
        print("Error: Wrong format")
        print("See --info")
        return None

    input_file_name = args.input
    output_file_name = args.output

    result = parseFile(input_file_name, output_file_name)
    if result is None:
        return None

    r, c, r_count, c_count, violations, tents_count, tents_added, grid = result

    if args.debug:
        debugPrint(r, c, r_count, c_count, violations, tents_count, tents_added, grid)

    return r, c, r_count, c_count, violations, tents_count, tents_added, grid

def debugPrint(r, c, r_count, c_count, violations, tents_count, tents_added, grid):
    '''
    Debug print function to print all the info from running the parseFile() function
    '''
    print(f"Number of rows: {r}")
    print(f"Number of columns: {c}")
    print(f"Row tent counts: {r_count}")
    print(f"Column tent counts: {c_count}")
    print(f"Number of violations: {violations}")
    print(f"Number of tents added: {tents_count}")
    print("Tents added (row, col, symbol):")
    for tent in tents_added:
        print(tent)
    
    print("Grid:")
    for row in grid:
        print(row)


def main():
    parser = argparse.ArgumentParser(description="Autograder/verifier for the tents and trees Algobowl competition")
    subparsers = parser.add_subparsers(dest="command", required=True, help="Sub-command to run (info or grade)")

    info_parser = subparsers.add_parser("info", help="Print useful information about file formats")
   
    grade_parser = subparsers.add_parser("grade", help="Grade input and output files")
    grade_parser.add_argument("input", help="Input file name")
    grade_parser.add_argument("output", help="Output file name")
    grade_parser.add_argument("--debug", action="store_true", help="Print a debug log")

    args = parser.parse_args()

    if args.command == "info":
        print("Usage: python autograder.py <outputFilename> <inputFilename> <--flags>\n")

        print("File format for input file")
        print("     First line: <row> <column>")
        print("     Second line: <row tent counts>")
        print("     Third line: <column tent counts>")
        print("     Subsequent lines: <space separated row of puzzle>\n")

        print("File Format for output file:")
        print("     First Line: Number of violations")
        print("     Second Line: Number of tents added")
        print("     Subsequent lines: <row> <col> <symbol>\n")

        return None

    elif args.command == "grade":
        parsed = getAndParse(args)
        if parsed is None:
            return
        r, c, r_count, c_count, violations, tents_count, tents_added, grid = parsed

        if not checkValidity(grid):
            print("The grid is not valid according to the puzzle rules.")
            return

        checked_violations = countViolations(grid)
        print("Computed violations:", checked_violations)

        if checked_violations != violations:
            print(f"Mismatch: Output file violations ({violations}) do not match computed violations ({checked_violations}).")
            return
        else:
            print("The number of violations matches.")
    
if __name__ == "__main__":
    main()
