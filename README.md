# Multithreaded Game Of Life

This program runs Conway's Game of Life on the commandline, allowing the user to customize the starting seed file and the number of threads used in the program. It can be displayed either in terminal in "ASCII Mode" or using the paraVisi library. In running the program with various thread counts, the user can see the increased speed from running the program with multiple cores. In practice, I have found the most efficient number to be around 8 for my machine.

### Demo

![](golDemo.gif)

## Getting Started

To run, install the necessary dependencies, and run the makefile. Then run: 
 - `./gol file.txt output_mode[0,1,2] num_threads partition[0,1] print_partition[0,1]`

This takes 5 arguments

 - file.txt: the seed file to run. Provided files are in the repo(e.g. explode.txt)
 - output_mode[0,1,2]: There are 3 options for how to display the game(0: no visualization, 1: ASCII, 2: ParaVisi)
 - num_threads: the number of threads for the program to use
 - partition[0,10]: the program partitions the board into rows or columns, and assigns each row/col to a thread. 0: row, 1: col. 
 - print_partition[0,1]: If you are curious, a 1 in this argument shows how the board was split up. 

## Authors

 - Alex Schrader
 - Jonathon Lee
 - Swarthmore College starter code
