#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <omp.h>

#define STORE_SOLUTIONS 1 // Set to 0 to disable storing solutions in memory
#define N 8 // Cannot be greater than 16 due to using uint32_t values for bitmasking
#define MAX_SOLUTIONS 92 // Use 92 for N=8, 724 for N=10, 2680 for N=12, 14200 for N=14, 14772512 for N=16

typedef struct {
  int solutions_count;
#if STORE_SOLUTIONS
  int solutions[MAX_SOLUTIONS][2 * N];
#endif
  int current_solution[2 * N];
} thread_data;

bool static inline is_valid_queen(int x, int y, uint32_t vertical_mask, uint32_t left_diagonal_mask, uint32_t right_diagonal_mask) {
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

void solve_n_queens(int start_x, int end_x, int y, uint32_t vertical_mask, uint32_t left_diagonal_mask, uint32_t right_diagonal_mask, int thread_idx, thread_data *thread_data) {
  if (y >= N) {
#if STORE_SOLUTIONS
    // Store the solution in the solutions array
    for (int i = 0; i < 2 * N; i += 2) {
      thread_data[thread_idx].solutions[thread_data[thread_idx].solutions_count][i] = thread_data[thread_idx].current_solution[i];
      thread_data[thread_idx].solutions[thread_data[thread_idx].solutions_count][i + 1] = thread_data[thread_idx].current_solution[i + 1];
      thread_data[thread_idx].solutions[thread_data[thread_idx].solutions_count + 1][i] = thread_data[thread_idx].current_solution[i];
      thread_data[thread_idx].solutions[thread_data[thread_idx].solutions_count + 1][i + 1] = (N - 1) - thread_data[thread_idx].current_solution[i + 1];
    }
#endif
    thread_data[thread_idx].solutions_count += 2;
    return;
  }

  for (int x = start_x; x < end_x; x++) {
    if (is_valid_queen(x, y, vertical_mask, left_diagonal_mask, right_diagonal_mask)) {
#if STORE_SOLUTIONS
      thread_data[thread_idx].current_solution[y * 2] = x;
      thread_data[thread_idx].current_solution[y * 2 + 1] = y;
#endif
      solve_n_queens(0, N, y + 1, vertical_mask | (1U << x), left_diagonal_mask | (1U << (x + y)), right_diagonal_mask | (1U << (x - y + N - 1)), thread_idx, thread_data);
    }
  }
}

int main(int argc, char *argv[]) {
  assert((N % 2) == 0 && "N value must be even.");

  struct timespec start, end;
  int num_threads = (int)sysconf(_SC_NPROCESSORS_ONLN);
  printf("Solving %d-Queen Problem with %d parallel threads.\n", N, num_threads);
  thread_data *threads_data = malloc(sizeof(thread_data) * num_threads);

  if (threads_data == NULL) {
    fprintf(stderr, "Error: cannot allocate memory for threads_data.\n");
    return 1;
  }

  for (int i = 0; i < num_threads; i++) {
    threads_data[i].solutions_count = 0;
  }

  clock_gettime(CLOCK_MONOTONIC, &start);
  int range_per_thread = (num_threads >= N / 2) ? 1 : (N / 2) / num_threads;
  int max_usable_threads = (num_threads >= N / 2) ? N / 2 : num_threads;
#pragma omp parallel for schedule(static)
  for (int thread_idx = 0; thread_idx < max_usable_threads; thread_idx++) {
    int start_x = thread_idx * range_per_thread;
    int end_x = (thread_idx == max_usable_threads - 1) ? N / 2 : start_x + range_per_thread;
    solve_n_queens(start_x, end_x, 0, 0, 0, 0, thread_idx, threads_data);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  long long elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000LL +
                         (end.tv_nsec - start.tv_nsec);
  double elapsed_s = elapsed_ns / 1000000000.0;

  int total_solutions = 0;
  for (int thread_idx = 0; thread_idx < num_threads; thread_idx++) {
    total_solutions += threads_data[thread_idx].solutions_count;
  }

#if STORE_SOLUTIONS
  printf("\n");
  for (int thread_idx = 0; thread_idx < num_threads; thread_idx++) {
    for (int i = 0; i < threads_data[thread_idx].solutions_count; i++) {
      for (int j = 0; j < N; j++) {
        printf("(%d,%d)", threads_data[thread_idx].solutions[i][j * 2], threads_data[thread_idx].solutions[i][j * 2 + 1]);
      }
      printf("\n");
    }
  }
#endif

  printf("\nFound %d solutions in %lld ns (%.9f s)\n", total_solutions, elapsed_ns, elapsed_s);

  free(threads_data);
  return 0;
}
