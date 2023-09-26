#include <kernel/process/sched.h>
#include <kernel/memory/vir_mem.h>
#include <kernel/cpu/cpu.h>
#include <status.h>
#include <common.h>
#define MAX_PROCESS 512

volatile struct m_pcb* __current;

struct m_pcb dummy;

extern void __proc_table;

struct scheduler sched_ctx;

void sched_init()
{
    size_t pg_size = ROUNDUP(sizeof(struct m_pcb) * MAX_PROCESS, 0x1000);
    vmm_alloc_pages(KERNEL_PID, &__proc_table, pg_size, PG_PREM_RW, PP_FGPERSIST);

    sched_ctx = (struct scheduler){ ._procs = (struct m_pcb*)&__proc_table,
                                    .ptable_len = 0,
                                    .procs_index = 0 };
}

void run(struct m_pcb* proc)
{
    if (!(__current->state & ~PROC_RUNNING)) {
        __current->state = PROC_STOPPED;
    }
    proc->state = PROC_RUNNING;

    // FIXME: 这里还是得再考虑一下。
    // tss_update_esp(__current->intr_ctx.esp);

    if (__current->page_table != proc->page_table) {
        __current = proc;
        cpu_lcr3(__current->page_table);
        // from now on, the we are in the kstack of another process
    } else {
        __current = proc;
    }

    //apic_done_servicing();

    asm volatile("pushl %0\n"
                 "jmp soft_iret\n" ::"r"(&__current->intr_contxt)
                 : "memory");
}

void schedule()
{
    if (!sched_ctx.ptable_len) {
        return;
    }

    struct m_pcb* next;
    int prev_ptr = sched_ctx.procs_index;
    int ptr = prev_ptr;
    // round-robin scheduler
    do {
        ptr = (ptr + 1) % sched_ctx.ptable_len;
        next = &sched_ctx._procs[ptr];
    } while (next->state != PROC_STOPPED && ptr != prev_ptr);

    sched_ctx.procs_index = ptr;

    run(next);
}

pid_t alloc_pid()
{
    pid_t i = 0;
    for (;
         i < sched_ctx.ptable_len && sched_ctx._procs[i].state != PROC_DESTROY;
         i++)
        ;

    if (i == MAX_PROCESS) {
        panick("Panic in Ponyville shimmer!");
    }
    return i;
}

void
push_process(struct m_pcb* process)
{
    int index = process->pid;
    if (index < 0 || index > sched_ctx.ptable_len) {
        __current->k_status = MXINVLDPID;
        return;
    }

    if (index == sched_ctx.ptable_len) {
        sched_ctx.ptable_len++;
    }

    sched_ctx._procs[index] = *process;

    process = &sched_ctx._procs[index];

    // make sure the address is in the range of process table
    llist_init_head(&process->children);
    // every process is the child of first process (pid=1)
    if (process->parent) {
        llist_append(&process->parent->children, &process->siblings);
    } else {
        process->parent = &sched_ctx._procs[0];
    }

    process->state = PROC_STOPPED;
}

void terminate_proc(int exit_code)
{
    __current->state = PROC_TERMNAT;
    __current->exit_code = exit_code;

    schedule();
}