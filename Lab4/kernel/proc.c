
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
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
                              schedule
 *======================================================================*/

PUBLIC void schedule(){
	PROCESS* p;
	
	//print_str(p_proc_ready->p_name);
	//print_str("scheduling...");

	for(p = proc_table; p < proc_table+NR_TASKS; p++){
		if(p->isWait && p->wait > 0){
			p->wait--;
		}
		else if(p->isWait && p->wait <= 0){
			p->isWait = 0;
			p->wait = 0;
		}
	}

	next();
	while(p_proc_ready->isWait){
		next();
	}
}

void next(){
	p_proc_ready ++;
	//print_str("next...");
	if (p_proc_ready >= proc_table + NR_TASKS) {
		p_proc_ready = proc_table;
	}
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

/*======================================================================*
                              sys_milli_seconds
 *======================================================================*/
PUBLIC void sys_milli_seconds(int milli_sec)
{
	p_proc_ready->isWait = 1;
	p_proc_ready->wait = milli_sec * HZ / 1000 + 1;
	//print_str("sleep...");
	schedule();
}

/*======================================================================*
                              sys_print_str
 *======================================================================*/
PUBLIC void sys_print_str(char* str){
	disp_str(str);
}

/*======================================================================*
                              sys_P
 *======================================================================*/
PUBLIC void sys_P(SIGNAL *signal){
	signal->value--;

	//print_str(p_proc_ready->p_name);
	//print_str("P...");

	if(signal->value < 0){
		p_proc_ready->isWait = 1;
		p_proc_ready->wait = 10000000;
		signal->list[signal->size] = p_proc_ready->pid;
		signal->size++;
		schedule();
	}
	
}

/*======================================================================*
                              sys_V
 *======================================================================*/
PUBLIC void sys_V(SIGNAL *signal){
	signal->value++;

	//print_str(p_proc_ready->p_name);
	//print_str("V...");

	if(signal->value <= 0){
		PROCESS* p = proc_table + signal->list[0];
		p->isWait = 0;
		p->wait = 0;
		signal->size--;
		//print_str(p->p_name);

		if(signal->size>0){
			for(int i=0; i<signal->size; i++){
				signal->list[i] = signal->list[i+1];
			}
		}
	}
}