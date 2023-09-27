#include <kernel/cpu/io.h>
#include <stdint.h>
#include <libs/mstdio.h>
#include <kernel/process/peocess.h>
#include <kernel/process/sched.h>
#include <init/idt.h>
#include <kernel/time/time.h>
#include <kernel/interrupt/interupt_fun.h>
#include <kernel/interrupt/int_vector.h>

#define IRQ0_FREQUENCY 100
#define INPUT_FREQUENCY 1193180
#define CURRENTR0_VALUE     INPUT_FREQUENCY/IRQ0_FREQUENCY
#define CURRENTR0_PORT 0X40
#define CURRENTR0_NO 0
#define CURRENTR_MODE 2
#define READ_WRITE_LATCH 3
#define PIT_CONTROL_PORT 0X43

uint32_t ticks;
uint32_t sched_ticks;
void do_intr_time_handler(isr_param int_param);

static void frequence_set(uint8_t counter_port, uint8_t counter_no, 
                         uint8_t rwl, uint8_t counter_mode, uint16_t counter_value)
{
    io_outb(PIT_CONTROL_PORT, (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));
    io_outb(counter_port, (uint8_t)counter_value);
    io_outb(counter_port, (uint8_t)(counter_value >> 8));
}

void timer_init(){
    ticks = 0;
    kprintf("timer intr start\n");
    frequence_set(CURRENTR0_PORT, CURRENTR0_NO, READ_WRITE_LATCH, CURRENTR_MODE, CURRENTR0_VALUE);
    _set_idt_entry(HARDWARE_TIME_INT, 0x08, &intr_time_handler, 0);
    intr_subscribe(MELOX_SYS_CALL, do_intr_time_handler);
    kprintf("timer init down\n");
    asm("sti");
}

void do_intr_time_handler(isr_param int_param)
{
    ticks++;
    __current->intr_contxt = int_param;
    if(__current->pro_ticks == 0)
        schedule();
    else
        __current->pro_ticks--;
}