import os
import sys
from typing import Optional
from typing import List
'''
This should take in a 2D list and then return a boolean of whether or not this chart follows the rules
'''
def checkValidity(chart: list) -> bool:
    return True

'''
This should take in a 2D list and then return an integer of how many violations there are
'''
def countViolations(chart: list) -> int:
    return 0

'''
Prompt and take command line args (should take 2+ inputs)
return as a 2D array
contains either None, 'tree', or 'tent'.

This is an example input, parse through this to return the above array
11
4
1 1 R
1 4 X
2 2 L
3 3 U
'''
def getCLA() -> Optional[List[List[Optional[str]]]]:
    if len(sys.argv) == 1:
        print("Error, wrong input format. See --help") 
        return None

    fileName = sys.argv[1]
    # Janky conditions, refactor if needed
    if len(sys.argv) == 2 and sys.argv[1] != "--help":
        fileContents = parseFile(fileName)
        os.remove(fileName)
    else: # This will hold contents of -- arguments
        for arg in sys.argv[1::]:
            if arg[0:2] == "--":
                # This will be the implementation of the -- arguements, so far, not much here
                match arg[2::]:
                    case "help":
                        print(
                            "Useful stuff put here!!!"
                        )
                    case "no-remove":
                        fileContents = parseFile(fileName)
                    case _:
                        print("UNIMPLEMENTED")

    return [[]]

def runMain(fileName: str) -> None:
    pass

def parseFile(fileName: str) -> Optional[str]:
    with open(fileName, 'r') as f:
        parsed = f.read()
    f.close()
    
    # Check the number of new lines or if the file is empty, then return None
    if len(parsed) == 0 or parsed.count('\n') < 3:
        return None

    return parsed

if __name__ == "__main__":
    getCLA() 
z
