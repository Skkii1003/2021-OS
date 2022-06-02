
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               clock.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"


/*======================================================================*
                           clock_handler
 *======================================================================*/
PUBLIC void clock_handler(int irq)
{
	ticks++;

	if(p_proc_ready->ticks > 0){
		p_proc_ready->ticks--;
	}

	if (k_reenter != 0) {
		return;
	}

    if(p_proc_ready->ticks < p_proc_ready->priority && p_proc_ready->ticks % TICKS == 0){
		//print_str(p_proc_ready->p_name);
		if(p_proc_ready->pid != 5) 
			print_state();
		schedule();
    }
	else{
		return;
	}

}

/*======================================================================*
                              milli_delay
 *======================================================================*/
PUBLIC void milli_delay(int milli_sec)
{
        int t = get_ticks();

        while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}

/*======================================================================*
                           init_clock
 *======================================================================*/
PUBLIC void init_clock()
{
        /* 初始化 8253 PIT */
        out_byte(TIMER_MODE, RATE_GENERATOR);
        out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
        out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

        put_irq_handler(CLOCK_IRQ, clock_handler);      /* 设定时钟中断处理程序 */
        enable_irq(CLOCK_IRQ);                        /* 让8259A可以接收时钟中断 */
}

void print_state_(){
	ptr = p_proc_ready;
	p_proc_ready = proc_table + 5;
	if(p_proc_ready->pid == 5){
		p_proc_ready = ptr;
		schedule();
	}
}


