
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
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
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

		p_proc->isWait = 0;
		p_proc->wait = 0;

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	proc_table[0].ticks = proc_table[0].priority = 2 * TICKS;
	proc_table[1].ticks = proc_table[1].priority = 3 * TICKS;
	proc_table[2].ticks = proc_table[2].priority = 3 * TICKS;
	proc_table[3].ticks = proc_table[3].priority = 3 * TICKS;
	proc_table[4].ticks = proc_table[4].priority = 4 * TICKS;
	proc_table[5].ticks = proc_table[5].priority = 1 * TICKS;

	proc_table[0].isWait = 0;
	proc_table[1].isWait = 0;
	proc_table[2].isWait = 0;
	proc_table[3].isWait = 0;
	proc_table[4].isWait = 0;
	proc_table[5].isWait = 0;

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;


	init_clock();
    init_keyboard();
	
	reader_first = 0;		//reader_first or writer_first
	reader_cnt = 0;			//reader_num
	status = -1;
	readers = 1;

	pass = 0;

	read.value = 1;
	write.value = 1;
	limit.value = readers;
	read.size = 0;
	write.size = 0;
	limit.size = 0;
	queue.value = 1;
	queue.size = 0;


	disp_pos = 0;
	for(int i=0;i<80*25;i++){
		disp_str(" ");
	}
	disp_pos = 0;

	restart();


	while(1){}
}

/*======================================================================*
                               Task
 *======================================================================*/

void print(){
	print_str("limit:");
	disp_int(limit.value);
	print_str("read:");
	disp_int(read.value);
	print_str("write:");
	disp_int(write.value);
	print_str("...");
}

char* name[5] = {"A","B","C","D","E"};


void Read_rf(){
	int proc = p_proc_ready->pid;

	if((reader_cnt+1) <= readers){
		P(&read);
		if(reader_cnt == 0){
			P(&queue);
			P(&write);
		}
		reader_cnt++;
		status = 0;
		disp_color_str(name[proc],RED);
		disp_color_str(":Begin Read...",RED);
		V(&read);
	}
	else{
		P(&write);
		P(&read);
		reader_cnt++;
		status = 0;
		disp_color_str(name[proc],RED);
		disp_color_str(":Begin Read...",RED);
		V(&read);
	}


	disp_color_str(name[proc],GREEN);
	disp_color_str(":Reading...",GREEN);
	while(p_proc_ready->ticks){
		if(p_proc_ready->ticks == 1){
			disp_color_str(name[proc],BLUE);
			disp_color_str(":Finish Read...",BLUE);

			P(&read);
			reader_cnt--;
			if(reader_cnt == 0 || reader_cnt < readers){
				V(&write);
				if(reader_cnt == 0){
					V(&queue);
				}
			
			}
			V(&read);
			milli_delay(TICKS);
		}
	}

	p_proc_ready->ticks = p_proc_ready->priority;
	milli_seconds(TICKS);
}

void Write_rf(){
	int proc = p_proc_ready->pid;
	P(&queue);
	P(&write);

	status = 1;
	disp_color_str(name[proc],RED);
	disp_color_str(":Begin Write...",RED);


	disp_color_str(name[proc],GREEN);
	disp_color_str(":Writing...",GREEN);

	while(p_proc_ready->ticks){
		if(p_proc_ready->ticks == 1){
			disp_color_str(name[proc],BLUE);
			disp_color_str(":Finish Write...",BLUE);

			V(&write);
			V(&queue);
			milli_delay(TICKS);
		}
	}
	p_proc_ready->ticks = p_proc_ready->priority;
	milli_seconds(TICKS);
}

void Read_wf(){
	//print_str("start...");
	int proc = p_proc_ready->pid;

	P(&limit);
	P(&queue);
	P(&read);

	//print();

	if(reader_cnt == 0){
		P(&write);
	}
	reader_cnt++;
	status = 0;

	disp_color_str(name[proc],RED);
	disp_color_str(":Begin Read...",RED);

	V(&read);
	V(&queue);

	disp_color_str(name[proc],GREEN);
	disp_color_str(":Reading...",GREEN);

	while(p_proc_ready->ticks){
		if(p_proc_ready->ticks == 1){
			disp_color_str(name[proc],BLUE);
			disp_color_str(":Finish Read...",BLUE);
			P(&read);
			reader_cnt--;
			if(reader_cnt == 0)
				V(&write);
			V(&read);
			V(&limit);
			milli_delay(TICKS);
		}
	}
	//print_str("end...");
	p_proc_ready->ticks = p_proc_ready->priority;
	milli_seconds(TICKS);
}

void Write_wf(){
	int proc = p_proc_ready->pid;
	P(&queue);
	P(&write);

	status = 1;
	disp_color_str(name[proc],RED);
	disp_color_str(":Begin Write...",RED);


	disp_color_str(name[proc],GREEN);
	disp_color_str(":Writing...",GREEN);

	while(p_proc_ready->ticks){
		if(p_proc_ready->ticks == 1){
			disp_color_str(name[proc],BLUE);
			disp_color_str(":Finish Write...",BLUE);
			V(&write);
			V(&queue);
			milli_delay(TICKS);
		}
	}

	p_proc_ready->ticks = p_proc_ready->priority;
	milli_seconds(TICKS);
}

void TaskA(){
	while(1){
		if(reader_first)
			Read_rf();
		else
			Read_wf();
	}
}

void TaskB(){
	while(1){
		if(reader_first)
			Read_rf();
		else
			Read_wf();
	}
}

void TaskC(){
	while(1){
		if(reader_first)
			Read_rf();
		else
			Read_wf();
	}
}

void TaskD(){
	while(1){
		if(reader_first)
			Write_rf();
		else
			Write_wf();
	}
}

void TaskE(){
	while(1){
		if(reader_first)
			Write_rf();
		else
			Write_wf();
	}
}

void TaskF(){
	while(1){
		if(pass){
			if(status==0){
				print_str("On Reading ");
				disp_int(reader_cnt);
				print_str("...");
			}
			if(status==1){
				print_str("On Writing...");
			}
		}
		while(p_proc_ready->ticks);

		proc_table[5].ticks = proc_table[5].priority;
	}
}


void print_state(){
	if(status==0){
		print_str("On Reading ");
		disp_int(reader_cnt);
		print_str("...");
	}
	if(status==1){
		print_str("On Writing...");
	}
}