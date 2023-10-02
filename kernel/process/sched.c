#include <kernel/process/sched.h>
#include <kernel/memory/vir_mem.h>
#include <kernel/cpu/cpu.h>
#include <status.h>
#include <common.h>
#include <kernel/cpu/io.h>
#include <libs/mstdio.h>
#include <syscall/syscall.h>
#include <kernel/memory/malloc.h>

#define MAX_PROCESS 512

volatile struct m_pcb* __current;

struct m_pcb dummy;

extern void __proc_table;

struct scheduler sched_ctx;

int sched_init()
{
    size_t pg_size = ROUNDUP(sizeof(struct m_pcb) * MAX_PROCESS, 0x1000);
    if(vmm_alloc_pages(KERNEL_PID, &__proc_table, pg_size, PG_PREM_RW, PP_FGPERSIST)){
        sched_ctx = (struct scheduler){ ._procs = (struct m_pcb*)&__proc_table,
                                    .ptable_len = 0,
                                    .procs_index = 0 };
                                    return 1;}
    return 0;
    
}

void run(struct m_pcb* proc)
{
    if (!(__current->state & ~PROC_RUNNING)) {
        __current->state = PROC_STOPPED;
    }
    proc->state = PROC_RUNNING;


    if (__current->page_table != proc->page_table) {
        __current = proc;
        //asm("cli");
        //__asm__("movl %%ebx,%%cr3"::"b" (__current->page_table));
        __asm__("mov %0, %%cr3" ::"r"(__current->page_table));
        //cpu_lcr3(__current->page_table);
        //asm("sti");
    } else {
        __current = proc;
    }

    // io_outb(0x20, 0x20);
    // io_outb(0xa0, 0x20);//EOI end the int

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
        kprintf("Panic in Ponyville shimmer!");
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

__DEFINE_MXSYSCALL1(unsigned int, sleep, unsigned int, seconds)
{
    if (!seconds) {
        return 0;
    }
    if (__current->pro_ticks) {
        return __current->pro_ticks;
    }

    __current->intr_contxt.registers.eax = seconds;
    __current->state = PROC_BLOCKED;
    schedule();
}

__DEFINE_MXSYSCALL1(void, exit, int, status)
{
    terminate_proc(status);
}

__DEFINE_MXSYSCALL(void, yield)
{
    schedule();
}

__DEFINE_MXSYSCALL1(pid_t, wait, int*, status)
{
    pid_t cur = __current->pid;
    struct m_pcb *proc, *n;
    if (llist_empty(&__current->children)) {
        return -1;
    }
repeat:
    llist_for_each(proc, n, &__current->children, siblings)
    {
        if (proc->state == PROC_TERMNAT) {
            goto done;
        }
    }
    // FIXME: 除了循环，也许有更高效的办法……
    // (在这里进行schedule，需要重写context switch!)
    goto repeat;

done:
    *status = proc->exit_code;
    return destroy_process(proc->pid);
}

extern void __del_pagetable(pid_t pid, uintptr_t mount_point);

pid_t destroy_process(pid_t pid)
{
    int index = pid;
    if (index <= 0 || index > sched_ctx.ptable_len) {
        __current->k_status = MXINVLDPID;
        return;
    }
    struct m_pcb* proc = &sched_ctx._procs[index];
    proc->state = PROC_DESTROY;
    llist_delete(&proc->siblings);

    if (proc->process_mm) {
        struct mm_region *pos, *n;
        llist_for_each(pos, n, &proc->process_mm->head, head)
        {
            malloc_free(pos);
        }
    }

    vmm_mount_pd(PD_MOUNT_2, proc->page_table);

    __del_pagetable(pid, PD_MOUNT_2);

    vmm_unmount_pd(PD_MOUNT_2);

    return pid;
}