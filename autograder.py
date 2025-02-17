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
import logging
from typing import Optional, List, Tuple

logger = logging.getLogger(__name__)

def convertToZeroIndex(row: int, col: int) -> Tuple[int, int]:
    """
    Convert 1-indexed coordinates to 0-indexed.
    """
    return row - 1, col - 1

def countRowColViolation(chart: list, row_counts: List[int], col_counts: List[int], active_tents: set) -> int:
    """
    Finds all violations from the row/column tent count specification
    """
    violations = 0
    rows = len(chart)
    cols = len(chart[0]) if rows > 0 else 0
    
    for i in range(rows):
        actual = sum(1 for j in range(cols) if (i, j) in active_tents)
        diff = abs(actual - row_counts[i])
        logger.debug(f"Row {i + 1} has {diff} violations")
        violations += diff
        
    for j in range(cols):
        actual = sum(1 for i in range(rows) if (i, j) in active_tents)
        diff = abs(actual - col_counts[j])
        logger.debug(f"Col {j + 1} has {diff} violations")
        violations += diff
        
    logger.debug(f"Row/Col Violations: {violations}")
    return violations

def countTentViolations(active_tents: set) -> int:
    """
    Counts all the violations of tents being too close to eachother
    """
    dirs = [(-1, -1), (-1, 0), (-1, 1),
            (0, -1),           (0, 1),
            (1, -1),  (1, 0),  (1, 1)]
    tent_violations = 0
    for (i, j) in active_tents:
        for di, dj in dirs:
            ni, nj = i + di, j + dj
            if (ni, nj) in active_tents:
                tent_violations += 1
                logger.debug(f"Tent at location ({i}, {j}) conflicts with tent ({ni}, {nj}), 1 violation")
                break # happens only once per tent

    logger.debug(f"Tent Violations: {tent_violations}")
    return tent_violations

def countTreeViolations(tents_added: set, chart: list):
    """
    Counts the violations from trees being lonely ;( or trees have too many tents (not cool bro)
    """
    rows = len(chart)
    cols = len(chart[0]) if rows > 0 else 0

    tree_associations = {}
    for (r, c, symbol) in tents_added:
        if symbol == "X":
            continue  # nah we don't want you bro
        # back to monkey (0 index)
        tent_i, tent_j = r - 1, c - 1
        # get proper coords with the symbol
        match symbol:
            case "U":
                tree_i, tree_j = tent_i - 1, tent_j
            case "D":
                tree_i, tree_j = tent_i + 1, tent_j
            case "L":
                tree_i, tree_j = tent_i, tent_j - 1
            case "R":
                tree_i, tree_j = tent_i, tent_j + 1
            case _:
                print(f"This should not be in the symbols bro")
        # hashmap fixes lives trademark
        tree_associations[(tree_i, tree_j)] = tree_associations.get((tree_i, tree_j), 0) + 1

    # Check all trees now
    lonely_tree_violations = 0
    for i in range(rows):
        for j in range(cols):
            if chart[i][j] == "tree":
                associated_count = tree_associations.get((i, j), 0)
                # If a tree has 0 or more than 1 associated tent, add 1 violation.
                if associated_count != 1:
                    lonely_tree_violations += 1
                    logger.debug(f"Tree at ({i},{j}) has {associated_count} associated tent(s), 1 violation")
    logger.debug(f"Oh so lonely tree violations: {lonely_tree_violations}")
    return lonely_tree_violations
    

def countViolations(chart: list, row_counts: List[int], col_counts: List[int], tents_added: List[Tuple[int, int, str]], debug: bool = False) -> int:
    '''
    | Count the number of rule violations in the grid.
    | 
    | The rules checked here:
    |   1. The number of tents in each row and column must match the expected counts.
    |      (We add abs(expected - actual) for each row/column.)
    |   2. No two tents may touch (including diagonally).
    |      (We count each adjacent pair once.)
    |   3. Each tent must be orthogonally adjacent to exactly one tree.
    |      (If a tent has 0 trees adjacent, or more than 1, count the difference.)
    | 
    | Returns the total violations.
    |
    | On a side note, I think this is horribly inefficient!
    | Also, we should add debug prints to this maybe.
    '''

    violations = 0

    # i hate this
    active_tents = {(r - 1, c - 1) for (r, c, symbol) in tents_added}

    # Count violations in the rows/col tent count
    violations += countRowColViolation(chart, row_counts, col_counts, active_tents)

    # Check the violations of tents touching nearby tents
    violations += countTentViolations(active_tents)

    # Check the violations of lonely trees
    violations += countTreeViolations(tents_added, chart)

    violations += sum(1 for (_, _, symbol) in tents_added if symbol == "X")
    logger.debug(f"Total violations after counting bad X tents: {violations}")

    return violations

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
        logger.error(f"Error reading input file {file_name}: {e}")
        return None
    
    # Check if the size of the file matches, need atleast 4 lines
    if len(lines) < 4:
        logger.error("Input file format error: Not enough lines.")
        return None

    # First line (R, C)
    # Then check if they are integers
    parts = lines[0].split()
    if len(parts) < 2:
        logger.error("Input file format error: First line must contain two numbers.")
        return None
    try:
        rows = int(parts[0])
        cols = int(parts[1])
    except ValueError:
        logger.error("Input file format error: Invalid numbers in first line.")
        return None

    # Check if the next lines can be split and mapped as integers, they are the row/column tent count lines.
    try:
        row_counts = list(map(int, lines[1].split()))
        col_counts = list(map(int, lines[2].split()))
    except ValueError:
        logger.error("Input file format error: Row/column counts must be integers.")
        return None

    # Check if the count of the row/col tent counts match the size given on line 1
    if len(row_counts) != rows or len(col_counts) != cols:
        logger.error("Input columns and/or row counts's size does not equal the number of rows/columns")
        return None

    # Check if the rest of the lines match the number of rows
    grid_lines = lines[3:]
    if len(grid_lines) != rows:
        logger.error("Input file format error: Number of grid rows does not match.")
        return None

    # Time to loop through the rest of the input
    grid = []
    for line in grid_lines:
        # check the count of the columns
        if len(line) != cols:
            logger.error("Input file format error: A grid row does not match the specified column count.")
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
        logger.error(f"Error reading output file {fileName}: {e}")
        return None

    # Needs to be atleast 2 lines
    if len(lines) < 2:
        logger.error("Output file format error: Not enough lines.")
        return None

    # Try setting violations and tents added as integers
    try:
        violations = int(lines[0].strip())
        tents_count = int(lines[1].strip())
    except ValueError:
        logger.error("Output file format error: First two lines must be integers.")
        return None

    # Time for loop
    tents_added = []
    allowed_symbols = {"U", "D", "L", "R", "X"}
    for line in lines[2:]:
        tent_info = line.split()
        
        # Check if the line is valid
        if len(tent_info) != 3:
            logger.error("Output file format error: Each operation line must have 3 values (row, col, symbol).")
            return None

        # Try mapping first two to int
        try:
            row = int(tent_info[0])
            col = int(tent_info[1])
        except ValueError:
            logger.error("Output file format error: Row and column values must be integers.")
            return None

        # Check grid bounds.
        if not (1 <= row <= len(grid) and 1 <= col <= len(grid[0])):
            logger.error(f"Operation ({row}, {col}) is out of grid bounds.")
            return None

        # Check if the symbol is valid
        symbol = tent_info[2]
        if symbol not in allowed_symbols:
            logger.error("Output file format error: Symbol must be one of U, D, L, R, or X.")
            return None

        # Check if it will sit on top of a tree (BAD!!)
        if grid[row - 1][col - 1] == "tree":
            logger.error("Cannot place tent on a tree, invalid operation")
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
                    logger.error("Invalid symbol")
                    return None
            
            # Check that the tree's position is within the grid.
            if not (0 <= tree_row < len(grid) and 0 <= tree_col < len(grid[0])):
                logger.error(f"Operation ({row}, {col}) with symbol {symbol} is invalid: tree position out of bounds.")
                return None

            # Verify that a tree is present in the expected adjacent cell.
            if grid[tree_row][tree_col] != "tree":
                logger.error(f"Operation ({row}, {col}) with symbol {symbol} is invalid: no tree found in the expected direction.")
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
    logger.debug(f"Number of rows: {r}")
    logger.debug(f"Number of columns: {c}")
    logger.debug(f"Row tent counts: {r_count}")
    logger.debug(f"Column tent counts: {c_count}")
    logger.debug(f"Number of violations: {violations}")
    logger.debug(f"Number of tents added: {tents_count}")
    logger.debug(f"Tents added (row, col, symbol):")
    for tent in tents_added:
        logger.debug(tent)
    
    logger.debug("Grid:")
    for row in grid:
        logger.debug(row)

def setupParser():
    parser = argparse.ArgumentParser(description="Autograder/verifier for the tents and trees Algobowl competition")
    subparsers = parser.add_subparsers(dest="command", required=True, help="Sub-command to run (info or grade)")

    info_parser = subparsers.add_parser("info", help="Print useful information about file formats")
   
    grade_parser = subparsers.add_parser("grade", help="Grade input and output files")
    grade_parser.add_argument("input", help="Input file name")
    grade_parser.add_argument("output", help="Output file name")
    grade_parser.add_argument("--debug", action="store_true", help="Print a debug log")

    return parser.parse_args()

def main():
    args = setupParser()

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
        debug = True if args.debug else False
        if debug:
            logging.basicConfig(level=logging.DEBUG, format="[%(levelname)s]: %(message)s")
            logger.setLevel(logging.DEBUG)
        else:
            logging.basicConfig(level=logging.INFO, format="[%(levelname)s]: %(message)s")
            logger.setLevel(logging.INFO)

        parsed = getAndParse(args)
        if parsed is None:
            return
        r, c, r_count, c_count, violations, tents_count, tents_added, grid = parsed

        checked_violations = countViolations(grid, r_count, c_count, tents_added, debug)
        logger.info(f"Computed violations: {checked_violations}")

        if checked_violations != violations:
            logger.info(f"Mismatch: Output file violations ({violations}) do not match computed violations ({checked_violations}).")
            return
        else:
            logger.info("The number of violations matches.")
    
if __name__ == "__main__":
    main()
