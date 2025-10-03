/**
 * @file sched_stubs.c
 * Stub implementations for NuttX scheduling instrumentation
 */

#include <nuttx/config.h>
#include <nuttx/sched.h>
#include <syslog.h>

/* These are the exact functions the linker says are missing */
void sched_note_add(FAR struct tcb_s *tcb)
{
    /* Empty stub - required by NuttX scheduling instrumentation */
    (void)tcb;
}

void sched_note_remove(FAR struct tcb_s *tcb)
{
    /* Empty stub - required by NuttX scheduling instrumentation */
    (void)tcb;
}

void sched_note_start(FAR struct tcb_s *tcb)
{
    /* Empty stub - required by NuttX scheduling instrumentation */
    (void)tcb;
}

void sched_note_stop(FAR struct tcb_s *tcb)
{
    /* Empty stub - required by NuttX scheduling instrumentation */
    (void)tcb;
}

void sched_note_suspend(FAR struct tcb_s *tcb)
{
    /* Empty stub - required by NuttX scheduling instrumentation */
    (void)tcb;
}

void sched_note_resume(FAR struct tcb_s *tcb)
{
    /* Empty stub - required by NuttX scheduling instrumentation */
    (void)tcb;
}