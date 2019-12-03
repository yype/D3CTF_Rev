## Challenge 

This challenge uses a virtual architecture similar to OISC to implement a classic Sudoku verification. There are 4 types of instructions, and all logical operations are implemented through NAND gates. A branch control and two I / O interrupts are also issued.

Operations like XOR / AND / OR are all implemented by combinations of NAND gates, for example:

```
xor x,y =>
xor_tmp[0] = y NAND y
xor_tmp[1] = x NAND xor_tmp[0]
xor_tmp[2] = x NAND x
xor_tmp[3] = y NAND xor_tmp[2]
x = xor_tmp[1] NAND xor_tmp[3]
```

Which is based on the fact that:

```
Q = A XOR B = [ B NAND ( A NAND A ) ] NAND [ A NAND ( B NAND B ) ]
```

An excerpt of the Sudoku Verifier code:

```
welcome = mkstr("**************************\n**  Welcome To D^3CTF   **\n**   Ancient Game V2    **\n**************************\n\nInput Flag:")
wrong = mkstr("\nSorry, please try again.\n")
correct = mkstr("\nCorrect.\n")

flag = new(50)
// distract = new(1000)
grid = new(81)


// initialize the puzzle
set(grid[0],9)
set(grid[5],8)
set(grid[9],1)
set(grid[10],3)
set(grid[14],9)
set(grid[16],7)
...
set(grid[71],6)
set(grid[75],9)
set(grid[80],1)

__code_start__

// print the welcome message
print(welcome)

// get input
input(flag[0])
input(flag[1])
input(flag[2])
input(flag[3])
input(flag[4])
input(flag[5])
...
input(flag[46])
input(flag[47])
input(flag[48])
input(flag[49])

// transfer chars in the flag into the grids

long_transfer(flag[0],grid[1])
long_transfer(flag[1],grid[2])
...
long_transfer(flag[47],grid[77])
long_transfer(flag[48],grid[78])
long_transfer(flag[49],grid[79])

// xor with xor_table, which is introduced 
//   for generating different flags to different teams

grid[1] = grid[1] ^ xor_table[0]
grid[2] = grid[2] ^ xor_table[1]
grid[3] = grid[3] ^ xor_table[2]
grid[4] = grid[4] ^ xor_table[3]
grid[6] = grid[6] ^ xor_table[4]
grid[7] = grid[7] ^ xor_table[5]
...
grid[77] = grid[77] ^ xor_table[47]
grid[78] = grid[78] ^ xor_table[48]
grid[79] = grid[79] ^ xor_table[49]

// verify the sudoku game

// rows
jmp _label_wrong if grid[4] == grid[5]
jmp _label_wrong if grid[4] == grid[6]
jmp _label_wrong if grid[4] == grid[7]
...
jmp _label_wrong if grid[3] == grid[7]
jmp _label_wrong if grid[3] == grid[8]

// columns
jmp _label_wrong if grid[0] == grid[9]
jmp _label_wrong if grid[0] == grid[18]
jmp _label_wrong if grid[0] == grid[27]
...
jmp _label_wrong if grid[62] == grid[80]
jmp _label_wrong if grid[71] == grid[80]

// subgrids
jmp _label_wrong if grid[0] == grid[1]
jmp _label_wrong if grid[0] == grid[2]
jmp _label_wrong if grid[0] == grid[9]
jmp _label_wrong if grid[0] == grid[10]
...
jmp _label_wrong if grid[78] == grid[79]
jmp _label_wrong if grid[78] == grid[80]
jmp _label_wrong if grid[79] == grid[80]

// check range

jmp _label_wrong if outofnumbers(grid[1])
jmp _label_wrong if outofnumbers(grid[2])
jmp _label_wrong if outofnumbers(grid[3])
jmp _label_wrong if outofnumbers(grid[4])
...
jmp _label_wrong if outofnumbers(grid[76])
jmp _label_wrong if outofnumbers(grid[77])
jmp _label_wrong if outofnumbers(grid[78])
jmp _label_wrong if outofnumbers(grid[79])

_label_correct:
print(correct)
return

_label_wrong:
print(wrong)
return
```

Write a compiler(not available for now) for this architecture. After the above code is compiled, it is the challenge that the players get.

To solve this problem, there is no need to simplify all logical operations. Since there is no complicated loop, you can find the conditions that prevent the control flow from jumping to the output "Sorry" through simple control flow tracking and symbol analysis. Finally, a constraint satisfaction process can be performed.

During the competition, due to the negligence of the author, the implementation of the `outofnumbers (var)` function was incorrectly written as `return var in range (10)`, resulting in multiple solutions. Since the target Sudoku should only be filled with 1 ~ 9, the correct implementation should be `return var in range (1, 10)`, It has been fixed now.

**Sudoku Map**

![](https://i.imgur.com/PrzmyEu.png)

**Solution**

Flag: d3ctf{g5lk9t28zz47y3l6m2kosbajd2vk9e2dwghxgfktcki}

> Referenceable solution script: [sol.py](https://github.com/0h2o/D3CTF_Rev/blob/master/AncientGameV2/sol.py) by [Byaidu](https://github.com/byaidu)

