
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"


PUBLIC	PROCESS			proc_table[NR_TASKS];

PUBLIC	char			task_stack[STACK_SIZE_TOTAL];

PUBLIC	TASK	task_table[NR_TASKS] = {
                    {TaskA, STACK_SIZE_TASKA, "TaskA"},
                    {TaskB, STACK_SIZE_TASKB, "TaskB"},
                    {TaskC, STACK_SIZE_TASKC, "TaskC"},
                    {TaskD, STACK_SIZE_TASKD, "TaskD"},
                    {TaskE, STACK_SIZE_TASKE, "TaskE"},
                    {TaskF, STACK_SIZE_TASKF, "TaskF"},
                    };

PUBLIC	irq_handler		irq_table[NR_IRQ];

PUBLIC	system_call		sys_call_table[NR_SYS_CALL] = {sys_get_ticks,sys_milli_seconds,sys_print_str,sys_P,sys_V};

