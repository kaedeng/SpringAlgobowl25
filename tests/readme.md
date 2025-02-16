# Test specific info

Autograder needs to generate a file from the main function
CLA will be something like this

'''
python autograder.py tests/one.test
'''

This will then (probably) run the main function in our code, which will (hopefully) generate a file, and then the autograder will read that file, perhaps "tests/one.out." Ill try adding something that will allow us to keep the output, but I think I will delete after parsing.



# Input format
The first line of the input file contains two positive integers separated by a single space, R and C, indicating the number of rows and columns in the puzzle. <br>
ie. 3 4

The next line contains R non-negative integers separated by spaces r1 . . . rR. ri indicates the number of tents which must be in row i. In other words, the first number corresponds to the number of tents in the first row, the second number corresponds to the number of tents in the second row, and so forth. <br>
ie. 0 0 3 <br>
first row has 0 tents, second row has 0 tents, third row has 3 tents

The next line contains C non-negative integers separated by spaces c_1 . . . c_C . c_i indicates the number of tents which must be in column i. <br>
ie. 1, 1, 2, 1

The next R lines each contain a row of the puzzle. Each line should contain exactly C characters. The characters used are: 
- . Blank cell.
- T Tree.

ie. <br>
T.. <br>
T.T. <br>
...T

## Input Restrictions
- 1 ≤ R × C ≤ 105
- 0 ≤ ri ≤ C
- 0 ≤ ci ≤ R

# Output format
The first line of output should contain a single integer: the total number of violations in the solution. <br>
ie. 11

The next line should contain a single integer T : the number of tents added. <br>
ie. 4

The next T lines indicate where each tent was added, and should be encoded as each of these fields separated by
spaces:
1. The 1-indexed row number of the tent’s position.
2. The 1-indexed column number of the tent’s position.
3. The letter U, D, L, R, or X, indicating if the associated tree is located up (U), down (D), left (L), or right (R) relative to this tent, or X if there is not an associated tree.

For example, the example solution shown in the Your Task section would be encoded in a text file as: <br>
11 <br>
4<br>
1 1 R<br>
1 4 X<br>
2 2 L<br>
3 3 U<br>

Here’s a slightly better solution for the same input:<br>
10<br>
4<br>
1 1 R<br>
2 2 L<br>
2 4 D<br>
3 3 U

While the second solution is more optimal than the first, both solutions are valid. A solution does not need to be optimal to be valid, and many valid solutions exist for any given input.<br>
Caution: Please double-check your output has 1-indexed coordinates. It’s a common mistake to output 0-indexed coordinates.


# Note for us;
- There is not really a good way of checking for **correctness** given we just want to minimize the number of violations in a good time.
- Thus, we should probably have a few hand checked optimal solutions to check as well as a few *difficult*, unsolvable solutions; this is where an some type of python autograder will go far.
