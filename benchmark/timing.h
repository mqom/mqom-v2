#ifndef MQOM_TIMING_H
#define MQOM_TIMING_H

#include <stdint.h>
#include <stddef.h>
/* On POSIX platforms, use the timeval structure */
#if defined(__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <time.h>
#include <sys/time.h>
#else
/* Other non-POSIX platforms */
/* Define our timeval structure */
struct timeval {
    long int tv_sec;
    long int tv_usec;
};
/* The gettimeofday API is external and should be provided by the user */
extern void gettimeofday(struct timeval*, void*);
#endif

typedef struct btimer_t {
    unsigned int counter;
    // gettimeofday
    double nb_milliseconds;
    struct timeval start, stop;
    // rdtscp or RDPMC
    uint64_t nb_cycles;
    unsigned int garbage;
    uint64_t cstart, cstop;
} btimer_t;

void btimer_init(btimer_t* timer);
void btimer_start(btimer_t *timer);
void btimer_count(btimer_t *timer);
void btimer_end(btimer_t *timer);
double btimer_diff(btimer_t *timer);
uint64_t btimer_diff_cycles(btimer_t *timer);
double btimer_get(btimer_t *timer);
double btimer_get_cycles(btimer_t *timer);

#endif /* MQOM_TIMING_H */
