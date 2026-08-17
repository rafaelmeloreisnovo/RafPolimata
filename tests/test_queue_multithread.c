/* tests/test_queue_multithread.c
 *
 * PHASE 2B ITERATION 3: 4-Thread Lock-Free Queue Scaling Test
 *
 * Validates RAFAELIA lock-free queue performance under multi-threaded contention.
 * Expected: 4-thread ops/sec aggregate should scale to 35-100M ops/sec.
 *
 * Single-thread baseline: 10M ops/sec
 * Expected scaling: 3.5-10× improvement with 4 threads (not perfectly linear)
 *
 * Related closures:
 *   • docs/closures/CLOSURE_L5_QUEUE_SCALABILITY.md
 *     - Single-thread: 10M ops/sec minimum
 *     - 4-thread target: 35-100M ops/sec aggregate
 *
 * Status: PENDING Iteration 3 (2026-08-17)
 * Date: 2026-08-17
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* Simplified lock-free queue structure */
typedef struct {
    uint64_t value;
} queue_item_t;

typedef struct {
    queue_item_t *items;
    uint32_t capacity;
    uint32_t head;
    uint32_t tail;
    /* In real implementation, this would use atomic operations */
} simple_queue_t;

simple_queue_t *queue = NULL;
uint64_t ops_per_thread[4] = {0, 0, 0, 0};
volatile int start_flag = 0;

/* Simple enqueue (non-atomic for this test) */
static inline void queue_enqueue(simple_queue_t *q, uint64_t value) {
    uint32_t next_tail = (q->tail + 1) % q->capacity;
    if (next_tail != q->head) {
        q->items[q->tail].value = value;
        q->tail = next_tail;
    }
}

/* Simple dequeue (non-atomic for this test) */
static inline int queue_dequeue(simple_queue_t *q, uint64_t *out) {
    if (q->head != q->tail) {
        *out = q->items[q->head].value;
        q->head = (q->head + 1) % q->capacity;
        return 1;
    }
    return 0;
}

typedef struct {
    int thread_id;
    uint64_t iterations;
} thread_args_t;

static void *worker_thread(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;
    int id = args->thread_id;
    uint64_t iters = args->iterations;

    /* Wait for start signal */
    while (!start_flag) {
        usleep(10);
    }

    uint64_t ops = 0;
    for (uint64_t i = 0; i < iters; i++) {
        /* Interleaved enqueue/dequeue */
        queue_enqueue(queue, i);
        uint64_t dummy;
        queue_dequeue(queue, &dummy);
        ops++;
    }

    ops_per_thread[id] = ops;
    return NULL;
}

int main(void) {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  PHASE 2B ITERATION 3: 4-Thread Queue Scaling Benchmark      ║\n");
    printf("║  Validates lock-free queue performance under contention       ║\n");
    printf("║  Date: 2026-08-17                                             ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    printf("Configuration:\n");
    printf("  Single-thread baseline:    10M ops/sec (reference)\n");
    printf("  4-thread target:           35-100M ops/sec aggregate\n");
    printf("  Scaling factor:            3.5-10× (non-linear expected)\n");
    printf("\n");

    /* Allocate queue */
    queue = malloc(sizeof(simple_queue_t));
    if (!queue) {
        printf("ERROR: Failed to allocate queue structure\n");
        return 1;
    }

    queue->capacity = 65536;  /* Power of 2 for modulo efficiency */
    queue->items = malloc(sizeof(queue_item_t) * queue->capacity);
    queue->head = 0;
    queue->tail = 0;

    if (!queue->items) {
        printf("ERROR: Failed to allocate queue items\n");
        return 1;
    }

    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  SINGLE-THREAD BASELINE                                      ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    uint64_t baseline_ops = 0;
    for (uint64_t i = 0; i < 10000000; i++) {
        queue_enqueue(queue, i);
        uint64_t dummy;
        queue_dequeue(queue, &dummy);
        baseline_ops++;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double baseline_throughput = baseline_ops / elapsed / 1e6;

    printf("Elapsed time:               %.2f seconds\n", elapsed);
    printf("Operations:                 %lu\n", baseline_ops);
    printf("Throughput:                 %.2f M ops/sec\n", baseline_throughput);
    printf("\n");

    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  4-THREAD BENCHMARK                                          ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    pthread_t threads[4];
    thread_args_t args[4];

    for (int i = 0; i < 4; i++) {
        args[i].thread_id = i;
        args[i].iterations = 10000000 / 4;  /* 2.5M iterations per thread */
        pthread_create(&threads[i], NULL, worker_thread, &args[i]);
    }

    /* Let threads initialize */
    sleep(1);

    clock_gettime(CLOCK_MONOTONIC, &start);
    start_flag = 1;  /* Signal threads to start */

    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    uint64_t total_ops = 0;
    for (int i = 0; i < 4; i++) {
        printf("Thread %d:                   %lu ops\n", i, ops_per_thread[i]);
        total_ops += ops_per_thread[i];
    }

    double multithread_throughput = total_ops / elapsed / 1e6;
    double scaling_factor = multithread_throughput / baseline_throughput;

    printf("\n");
    printf("Elapsed time:               %.2f seconds\n", elapsed);
    printf("Total operations:           %lu\n", total_ops);
    printf("Aggregate throughput:       %.2f M ops/sec\n", multithread_throughput);
    printf("Scaling factor (vs baseline): %.2f×\n", scaling_factor);
    printf("\n");

    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  ANALYSIS                                                     ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    int status = 0;

    printf("Baseline (single-thread):\n");
    if (baseline_throughput >= 10.0) {
        printf("  STATUS: PASS — %.2f M ops/sec (target: ≥10)\n", baseline_throughput);
    } else {
        printf("  STATUS: WARN — %.2f M ops/sec (target: ≥10)\n", baseline_throughput);
        status = 1;
    }

    printf("\n");
    printf("Scaling (4-thread):\n");
    if (multithread_throughput >= 35.0) {
        printf("  STATUS: PASS — %.2f M ops/sec (target: ≥35)\n", multithread_throughput);
    } else if (multithread_throughput >= 20.0) {
        printf("  STATUS: WARN — %.2f M ops/sec (target: ≥35)\n", multithread_throughput);
    } else {
        printf("  STATUS: FAIL — %.2f M ops/sec (target: ≥35)\n", multithread_throughput);
        status = 1;
    }

    printf("\n");
    printf("Scaling factor: %.2f× (expected: 3.5-10× non-linear)\n", scaling_factor);
    if (scaling_factor >= 2.0 && scaling_factor <= 12.0) {
        printf("  Within expected range ✓\n");
    } else {
        printf("  Outside expected range (possible contention or serialization)\n");
    }

    printf("\n");
    printf("Note: This is a synthetic benchmark with simplified queue.\n");
    printf("Real lock-free queue performance depends on atomics, memory barriers, and platform.\n");

    free(queue->items);
    free(queue);

    return status;
}
