# Sudoku-Solver-
# This is a multithreaded Sudoku solver in C that utilizes backtracking. 
# Multithreading: Each empty cell in the Sudoku board is handled by a separate thread, and the first thread to find a solution broadcasts it to all other threads.
# Backtracking: The solver uses backtracking to recursively try all possibilities for filling empty cells.
# Thread Synchronization: The program ensures that only the first solution is considered using a mutex and condition variables.
# Memory Management: Each thread works on a local copy of the Sudoku board to avoid data races. Memory is properly allocated and freed after use.
