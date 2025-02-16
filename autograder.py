import os
import sys
from typing import Optional
from typing import List
from typing import Tuple

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

def parseInputFile(fileName: str) -> Optional[Tuple[int, int, List[int], List[int], List[List[Optional[str]]]]]:
    '''
    Parses files that hold the information on trees and row/col tent counts
    '''

    try:
        with open(fileName, 'r') as f:
            lines = f.read().strip().splitlines()
    except Exception as e:
        print(f"Error reading input file {fileName}: {e}")
        return None
    
    if len(lines) < 4:
        print("Input file format error: Not enough lines.")
        return None

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

    try:
        row_counts = list(map(int, lines[1].split()))
        col_counts = list(map(int, lines[2].split()))
    except ValueError:
        print("Input file format error: Row/column counts must be integers.")
        return None

    if len(row_counts) != rows or len(col_counts) != cols:
        print("Input columns and/or row counts's size does not equal the number of rows/columns")

    grid_lines = lines[3:]
    if len(grid_lines) != rows:
        print("Input file format error: Number of grid rows does not match.")
        return None

    grid = []
    for line in grid_lines:
        if len(line) != cols:
            print("Input file format error: A grid row does not match the specified column count.")
            return None
        row_list = []
        for char in line:
            if char == 'T':
                row_list.append("tree")
            else:
                row_list.append(None)
        grid.append(row_list)

    return (rows, cols, row_counts, col_counts, grid)

def parseOutputFile(fileName: str, grid: List[List]) -> Optional[Tuple[int, int, List[List]]]:
    try:
        with open(fileName, 'r') as f:
            lines = f.read().strip().splitlines()
    except Exception as e:
        print(f"Error reading output file {fileName}: {e}")
        return None

    if len(lines) < 2:
        print("Output file format error: Not enough lines.")
        return None

    try:
        violations = int(lines[0].strip())
        tents_added = int(lines[1].strip())
    except ValueError:
        print("Output file format error: First two lines must be integers.")
        return None

    operations = []
    allowed_symbols = {"U", "D", "L", "R", "X"}
    for line in lines[2:]:
        tentInfo = line.split()

        if len(tentInfo) != 3:
            print("Output file format error: Each operation line must have 3 values (row, col, symbol).")
            return None

        try:
            row = int(tentInfo[0])
            col = int(tentInfo[1])
        except ValueError:
            print("Output file format error: Row and column values must be integers.")
            return None

        # Check grid bounds.
        if not (1 <= row <= len(grid) and 1 <= col <= len(grid[0])):
            print(f"Operation ({row}, {col}) is out of grid bounds.")
            return None

        symbol = tentInfo[2].upper()

        if symbol not in allowed_symbols:
            print("Output file format error: Symbol must be one of U, D, L, R, or X.")
            return None

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

    return violations, tents_added, grid
        


def getAndParse() -> Optional[Tuple[int, int, List, List, Optional[List[List[Optional[str]]]]]]:
    '''
    Prompt and take command line args (should take 2+ inputs)
    Calls the parseFile function, which should return as a tuple, (numOfViolations, tentsAdded, [<count of tents in rows>], [<count of tents in colunns>], 2D array)
    contains either None, 'tree', or 'tent'.

    This is an example input file (that we need to input), parse through it and return to the above array
    3 4
    0 0 3
    1 1 2 1
    .T..
    T.T.
    ...T

    This is an example output file (that we need to input), parse through this to return the above array
    11
    4
    1 1 R
    1 4 X
    2 2 L
    3 3 U


    This should then be
    (11, 4, [0, 0, 3], [1, 1, 2, 1], 
        [
            ["tent", "tree", None, None],
            ["tree", "tent", "tree", None],
            [None, None, "tent", "tree"],
        ]
    )
    '''
    # Janky conditions and bad logic lol, refactor if needed
    if len(sys.argv) < 2:
        print("Error: Wrong format")
        print("See --help")
        return None

    if "--help" in sys.argv:
        print("Usage: python autograder.py <outputFilename> <inputFilename> <--flags>\n")

        print("File format for input file")
        print("     First line: <row> <column>")
        print("     Second line: <row tent counts>")
        print("     Third line: <column tent counts>")
        print("     Subsequent lines: <space separated row of puzzle>\n")

        print("File Format for output file:")
        print("     First Line: Number of violations")
        print("     Second Line: Number of tents added")
        print("     Subsequent lines: <row> <col> <symbol>")
        return None

    # Need to add more options later.

    fileName = sys.argv[1]
    return parseFile(fileName)


def parseFile(inputFileName: str, outputFileName: str) -> Optional[Tuple[int, int, List, List, int, int, Optional[List[List[Optional[str]]]]]]:
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
    violations, tents_added, grid = outputContents

    return r, c, r_count, c_count, violations, tents_added, grid

if __name__ == "__main__":
    parsed = parseFile("tests/one.test", "tests/one.out")
    r, c, r_count, c_count, violations, tents_added, grid = parsed

    for line in grid:
        print(line)