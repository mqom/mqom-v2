#ifndef __BENCHMARK_H__
#define __BENCHMARK_H__

#define NUMBER_OF_BENCHES 22

// Signing
#define BS_EXPAND_MQ 0
#define BS_BLC_COMMIT 1
#define BS_PIOP_COMPUTE 2
#define BS_SAMPLE_CHALLENGE 3
#define BS_BLC_OPEN 4

#define BS_BLC_EXPAND_TREE 5
#define BS_BLC_KEYSCH_COMMIT 6
#define BS_BLC_SEED_COMMIT 7
#define BS_BLC_PRG 8
#define BS_BLC_XOF 9
#define BS_BLC_ARITH 10
#define BS_BLC_GLOBAL_XOF 11

#define BS_PIOP_MAT_MUL_EXT 12
#define BS_PIOP_COMPUTE_T1 13
#define BS_PIOP_COMPUTE_PZI 14
#define BS_PIOP_EXPAND_BATCHING_MAT 15
#define BS_PIOP_BATCH 16
#define BS_PIOP_ADD_MASKS 17

// Verification
// Currently no benchmark is performed on the
//   verification algorithm

// Others
#define B_PIN_A 18
#define B_PIN_B 19
#define B_PIN_C 20
#define B_PIN_D 21

#ifndef BENCHMARK
#define __BENCHMARK_START__(label) {}
#define __BENCHMARK_STOP__(label) {}

#else
#include "benchmark/timing.h"
__attribute__((weak)) btimer_t timers[NUMBER_OF_BENCHES];
void btimer_start(btimer_t *timer);
void btimer_end(btimer_t *timer);

/* NOTE: dummy definitions to be replaced when benchmarks needed */
__attribute__((weak,noinline)) void btimer_start(btimer_t *timer)
{
	(void)timer;
}
__attribute__((weak,noinline)) void btimer_end(btimer_t *timer)
{
	(void)timer;
}

#define __BENCHMARK_START__(label) btimer_start(&timers[label])
#define __BENCHMARK_STOP__(label) btimer_end(&timers[label])
#endif

#endif /* __BENCHMARK_H__ */
