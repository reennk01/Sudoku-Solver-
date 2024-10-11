#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>

int size;
int sqrtSize;
int **sudoku;
pthread_mutex_t mutex;
pthread_cond_t cond;
bool solution_found = false;

typedef struct {
    int row;
    int col;
} ThreadData;

void initializeMutexAndCond() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}

void destroyMutexAndCond() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

bool isSafe(int **board, int row, int col, int num) {
    for (int x = 0; x < size; x++) {
        if (board[row][x] == num || board[x][col] == num)
            return false;
    }

    int startRow = row - row % sqrtSize, startCol = col - col % sqrtSize;
    for (int i = 0; i < sqrtSize; i++) {
        for (int j = 0; j < sqrtSize; j++) {
            if (board[i + startRow][j + startCol] == num)
                return false;
        }
    }

    return true;
}

bool solveSudokuUtil(int **board, int row, int col) {
    if (row == size - 1 && col == size)
        return true;

    if (col == size) {
        row++;
        col = 0;
    }

    if (board[row][col] > 0)
        return solveSudokuUtil(board, row, col + 1);

    for (int num = 1; num <= size; num++) {
        if (isSafe(board, row, col, num)) {
            board[row][col] = num;
            if (solveSudokuUtil(board, row, col + 1))
                return true;
            board[row][col] = 0;
        }
    }
    return false;
}

void* solveSudoku(void* arg) {
    ThreadData *data = (ThreadData*)arg;
    int row = data->row;
    int col = data->col;
    free(data);

    int **local_sudoku = malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++) {
        local_sudoku[i] = malloc(size * sizeof(int));
        for (int j = 0; j < size; j++) {
            local_sudoku[i][j] = sudoku[i][j];
        }
    }

    for (int num = 1; num <= size; num++) {
        if (isSafe(local_sudoku, row, col, num)) {
            local_sudoku[row][col] = num;
            if (solveSudokuUtil(local_sudoku, row, col + 1)) {
                pthread_mutex_lock(&mutex);
                if (!solution_found) {
                    solution_found = true;
                    for (int i = 0; i < size; i++) {
                        for (int j = 0; j < size; j++) {
                            sudoku[i][j] = local_sudoku[i][j];
                        }
                    }
                    pthread_cond_broadcast(&cond);
                }
                pthread_mutex_unlock(&mutex);
                break;
            }
            local_sudoku[row][col] = 0;
        }
    }

    for (int i = 0; i < size; i++) {
        free(local_sudoku[i]);
    }
    free(local_sudoku);

    pthread_exit(NULL);
}

void printSudoku() {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("%2d ", sudoku[i][j]);
        }
        printf("\n");
    }
}

int main() {
    initializeMutexAndCond();

    printf("Enter the size of the Sudoku (must be a perfect square): ");
    scanf("%d", &size);

    sqrtSize = sqrt(size);
    if (sqrtSize * sqrtSize != size) {
        printf("Size must be a perfect square!\n");
        destroyMutexAndCond();
        return 1;
    }

    sudoku = malloc(size * sizeof(int *));
    for (int i = 0; i < size; i++) {
        sudoku[i] = malloc(size * sizeof(int));
    }

    printf("Enter the Sudoku entries (use 0 for empty cells):\n");
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            scanf("%d", &sudoku[i][j]);
        }
    }

    printf("\nOriginal Sudoku:\n");
    printSudoku();

    pthread_t threads[size * size];
    int thread_count = 0;

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (sudoku[i][j] == 0) {
                ThreadData *data = malloc(sizeof(ThreadData));
                data->row = i;
                data->col = j;
                pthread_create(&threads[thread_count++], NULL, solveSudoku, data);
            }
        }
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    if (solution_found) {
        printf("\nSolved Sudoku:\n");
        printSudoku();
    } else {
        printf("\nNo solution exists.\n");
    }

    for (int i = 0; i < size; i++) {
        free(sudoku[i]);
    }
    free(sudoku);

    destroyMutexAndCond();
    return 0;
}
