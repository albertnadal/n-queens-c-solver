#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#define STORE_SOLUTIONS 1 // Set to 0 to disable storing solutions in memory
#define N 8 // Cannot be greater than 16 due to using uint32_t values for bitmasking
#define MAX_SOLUTIONS 92 // Use 92 for N=8, 724 for N=10, 2680 for N=12, 14200 for N=14, 14772512 for N=16

static int solutions_count = 0;
#if STORE_SOLUTIONS
static int solutions[MAX_SOLUTIONS][2 * N];
static int current_solution[2 * N];
#endif

static inline __attribute__((always_inline))
bool is_valid_queen(int x, int y, uint32_t vertical_mask, uint32_t left_diagonal_mask, uint32_t right_diagonal_mask) {
  if (vertical_mask & (1U << x)) {
    // Column is already occupied
    return false;
  }

  if (left_diagonal_mask & (1U << (x + y))) {
    // Left diagonal is already occupied
    return false;
  }

  if (right_diagonal_mask & (1U << (x - y + N - 1))) {
    // Right diagonal is already occupied
    return false;
  }

  return true;
}

void solve_n_queens(int y, int last_col, uint32_t vertical_mask, uint32_t left_diagonal_mask, uint32_t right_diagonal_mask) {
  if (y >= N) {
#if STORE_SOLUTIONS
    // Store the solution in the solutions array
    for (int i = 0; i < 2 * N; i += 2) {
      solutions[solutions_count][i] = current_solution[i];
      solutions[solutions_count][i + 1] = current_solution[i + 1];
      solutions[solutions_count + 1][i] = (N - 1) - current_solution[i];
      solutions[solutions_count + 1][i + 1] = current_solution[i + 1];
    }
#endif
    solutions_count += 2;
    return;
  }

  for (int x = 0; x < last_col; x++) {
    if (is_valid_queen(x, y, vertical_mask, left_diagonal_mask, right_diagonal_mask)) {
#if STORE_SOLUTIONS
      current_solution[y * 2] = x;
      current_solution[y * 2 + 1] = y;
#endif
      solve_n_queens(y + 1, N, vertical_mask | (1U << x), left_diagonal_mask | (1U << (x + y)), right_diagonal_mask | (1U << (x - y + N - 1)));
    }
  }
}

int main(int argc, char *argv[]) {
  assert((N % 2) == 0 && "N value must be even.");

  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC, &start);
  solve_n_queens(0, N / 2, 0, 0, 0);
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
