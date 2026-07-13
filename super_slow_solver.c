#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define STORE_SOLUTIONS 1 // Set to 0 to disable storing solutions in memory
#define N 8
#define MAX_SOLUTIONS 92 // Use 92 for N=8, 724 for N=10, 2680 for N=12, 14200 for N=14, 14772512 for N=16

static int solutions_count = 0;
static int solutions[MAX_SOLUTIONS][2 * N];
static int current_solution[2 * N];
static bool board[N][N] = {0};

static inline __attribute__((always_inline))
bool is_valid_queen(int x, int y) {
  for (int row = y - 1; row >= 0; row--) {
    if (board[row][x]) {
      return false;
    }
  }

  for (int row = y - 1, col = x - 1; row >= 0 && col >= 0; row--, col--) {
    if (board[row][col]) {
      return false;
    }
  }

  for (int row = y - 1, col = x + 1; row >= 0 && col < N; row--, col++) {
    if (board[row][col]) {
      return false;
    }
  }

  return true;
}

static void solve_n_queens(int y) {
  if (y >= N) {
#ifdef STORE_SOLUTIONS
    // Store the solution in the solutions array
    for (int i = 0; i < 2 * N; i++) {
      solutions[solutions_count][i] = current_solution[i];
    }
#endif
    solutions_count++;
    return;
  }

  for (int x = 0; x < N; x++) {
    if (is_valid_queen(x, y)) {
      board[y][x] = true;
      current_solution[y * 2] = x;
      current_solution[y * 2 + 1] = y;
      solve_n_queens(y + 1);
      board[y][x] = false;
    }
  }
}

int main(int argc, char *argv[]) {
  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC, &start);
  solve_n_queens(0);
  clock_gettime(CLOCK_MONOTONIC, &end);

  long long elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000LL +
                         (end.tv_nsec - start.tv_nsec);
  double elapsed_s = elapsed_ns / 1000000000.0;

#if STORE_SOLUTIONS
  printf("\n");
  for (int i = 0; i < solutions_count; i++) {
    for (int j = 0; j < N; j++) {
      printf("(%d,%d)", solutions[i][j * 2], solutions[i][j * 2 + 1]);
    }
    printf("\n");
  }
#endif

  printf("\nFound %d solutions in %lld ns (%.9f s)\n", solutions_count, elapsed_ns, elapsed_s);

  return 0;
}
