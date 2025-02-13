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
def getAndParse() -> Optional[List[List[Optional[str]]]]:
    if len(sys.argv) == 1:
        print("Error, wrong input format. See --help") 
        return None
    
    fileName = sys.argv[1]
    # Janky conditions and bad logic lol, refactor if needed
    if len(sys.argv) == 2 and sys.argv[1] != "--help":
        fileContents = parseFile(fileName)
    else: # This will hold contents of -- arguments
        fileContents = parseFile(fileName)
        for arg in sys.argv[2::]:
            if arg[0:2] == "--":
                # This will be the implementation of the -- arguements, so far, not much here
                match arg[2::]:
                    case "help":
                        print(
                            "Useful stuff put here!!!"
                        )
                    case _:
                        print("UNIMPLEMENTED")



    return [[]]

def runMain(fileName: str) -> None:
    pass

def parseFile(fileName: str) -> OOptional[List[List[Optional[str]]]]:
    with open(fileName, 'r') as f:
       file = f.read()
    f.close()
    
    # Check the number of new lines or if the file is empty, then return None
    if len(file) == 0 or file.count('\n') < 3:
        return None

    # Need to parse it into the list thingie mcbobber idk bruh
    parsed =  [[]]

    return parsed

if __name__ == "__main__":
    getCLA() 

