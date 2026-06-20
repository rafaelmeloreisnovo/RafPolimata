#include "RAF_rafaelia_common.h"

/*
 * Método M059: RTOS mínimo cooperativo sem heap
 * Alvo: Todos (MCU, Linux, Android)
 * Domínio: RTOS/Scheduler
 * Ganho estimado: multitarefa cooperativa sem alocação dinâmica
 * Status: implementação completa, selftest host-runnable
 *
 * Scheduler round-robin cooperativo para no máximo 4 tasks.
 * Cada task é um function pointer void (*)(void).
 * Sem heap, sem libc: estado em arrays estáticos de tamanho fixo.
 */

#define M059_MAX_TASKS 4u

/* Task registry — static, zero-initialised */
static void (*_m059_tasks[M059_MAX_TASKS])(void);
static uint8_t _m059_ntasks = 0u;

/*
 * rafaelia_m059_rtos_register:
 * Registers a cooperative task function.
 * Returns 0 on success, -1 if the task table is full.
 */
int rafaelia_m059_rtos_register(void (*fn)(void)) {
    if (fn == (void (*)(void))0) {
        return -1;
    }
    if (_m059_ntasks >= (uint8_t)M059_MAX_TASKS) {
        return -1;  /* table full */
    }
    _m059_tasks[_m059_ntasks] = fn;
    _m059_ntasks++;
    return 0;
}

/*
 * rafaelia_m059_rtos_run:
 * Runs the cooperative scheduler for 'iterations' full rounds.
 * Each round calls every registered task once, in registration order.
 * Returns the total number of task invocations performed.
 */
int rafaelia_m059_rtos_run(uint8_t iterations) {
    uint8_t round;
    uint8_t t;
    int     total = 0;

    for (round = 0u; round < iterations; round++) {
        for (t = 0u; t < _m059_ntasks; t++) {
            if (_m059_tasks[t] != (void (*)(void))0) {
                _m059_tasks[t]();
                total++;
            }
        }
    }
    return total;
}

/* ------------------------------------------------------------------ */
/* Selftest infrastructure: two dummy tasks with static counters       */
/* ------------------------------------------------------------------ */

static uint8_t _m059_cnt_a = 0u;
static uint8_t _m059_cnt_b = 0u;

static void _m059_task_a(void) { _m059_cnt_a++; }
static void _m059_task_b(void) { _m059_cnt_b++; }

/*
 * rafaelia_m059_rtos_minimal_no_heap:
 * Main entry: registers 2 dummy tasks, runs 4 iterations,
 * verifies each task was called exactly 4 times.
 * Returns 0=PASS, -1=FAIL.
 */
int rafaelia_m059_rtos_minimal_no_heap(void) {
    int invocations;

    /* Reset scheduler state */
    _m059_ntasks  = 0u;
    _m059_cnt_a   = 0u;
    _m059_cnt_b   = 0u;

    /* Register two tasks */
    if (rafaelia_m059_rtos_register(_m059_task_a) != 0) { return -1; }
    if (rafaelia_m059_rtos_register(_m059_task_b) != 0) { return -1; }

    /* Run 4 iterations: each task should be called 4 times */
    invocations = rafaelia_m059_rtos_run(4u);

    /* Total invocations = 2 tasks × 4 rounds = 8 */
    if (invocations != 8) { return -1; }
    if (_m059_cnt_a != 4u) { return -1; }
    if (_m059_cnt_b != 4u) { return -1; }

    return 0;
}

/*
 * rafaelia_m059_selftest:
 * Registers 2 tasks that increment static counters.
 * After run(4), verifies each counter reached 4.
 * Also verifies registration failure when table is full (4 tasks already).
 * Returns 0=PASS, -1=FAIL.
 */
int rafaelia_m059_selftest(void) {
    int r;

    /* Delegate to main function for core test */
    r = rafaelia_m059_rtos_minimal_no_heap();
    if (r != 0) { return -1; }

    /* Additional: test that a 3rd and 4th task can also be registered */
    _m059_ntasks = 0u;
    if (rafaelia_m059_rtos_register(_m059_task_a) != 0) { return -1; }
    if (rafaelia_m059_rtos_register(_m059_task_b) != 0) { return -1; }
    if (rafaelia_m059_rtos_register(_m059_task_a) != 0) { return -1; }
    if (rafaelia_m059_rtos_register(_m059_task_b) != 0) { return -1; }

    /* 5th registration must fail (table full) */
    if (rafaelia_m059_rtos_register(_m059_task_a) != -1) { return -1; }

    /* NULL registration must fail */
    if (rafaelia_m059_rtos_register((void (*)(void))0) != -1) { return -1; }

    return 0;
}
