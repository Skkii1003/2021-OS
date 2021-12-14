
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_do_read(TTY* p_tty);
PRIVATE void tty_do_write(TTY* p_tty);
PRIVATE void put_key(TTY* p_tty, u32 key);

PUBLIC void find(TTY* p_tty);
PUBLIC void exitFind(TTY* p_tty);

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty()
{
	TTY*	p_tty;

	init_keyboard();

	isFind = 0;
	isWrite = 1;

	for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
		init_tty(p_tty);
	}
	select_console(0);

	int time = 0;

	

	while (1) {
		for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
			tty_do_read(p_tty);
			tty_do_write(p_tty);

			if(p_tty->p_console->cursor>=(p_tty->p_console->original_addr + p_tty->p_console->v_mem_limit - 1))
				clean(TTY_FIRST);
		}

		if(!isFind)
			time++;
		else
			time = 0;

			if(time > 250000){
				time = 0;
				clean(TTY_FIRST);
				clean(TTY_FIRST+1);
				clean(TTY_END);
			}
	}
}


/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY* p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	init_screen(p_tty);
}

/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty, u32 key)
{
        char output[2] = {'\0', '\0'};

        if (!(key & FLAG_EXT)) {
			put_key(p_tty, key);
        }
        else {
            int raw_code = key & MASK_RAW;
            switch(raw_code) {
				//re:find
				case ESC: 
					if(!isFind){
						isFind = 1;
						init_cursor = p_tty->p_console->cursor;
					}
					else{
						isFind = 0;
						isWrite = 1;
						exitFind(p_tty);
						init_cursor = 0;
					}
					break;
				//re5:tab
				case TAB:
					put_key(p_tty,'\t');
					break;
            	case ENTER:
					if(isFind && isWrite){
						isWrite = 0;
						find(p_tty);
						break;
					}
					put_key(p_tty, '\n');
					break;
            	case BACKSPACE:
					put_key(p_tty, '\b');
					break;
            	case UP:
                	if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
						scroll_screen(p_tty->p_console, SCR_DN);
                	}
					break;
				case DOWN:
					if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
						scroll_screen(p_tty->p_console, SCR_UP);
					}
					break;
				case F1:
				case F2:
				case F3:
				case F4:
				case F5:
				case F6:
				case F7:
				case F8:
				case F9:
				case F10:
				case F11:
				case F12:
					/* Alt + F1~F12 */
					if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
						select_console(raw_code - F1);
					}
					break;
            	default:
                    break;
            }
        }
}

PUBLIC void find(TTY* p_tty){
	int len = p_tty->p_console->cursor - init_cursor;
	u8* p_vmem = (u8*)(V_MEM_BASE + p_tty->p_console->cursor * 2 - len * 2);
	u8 key[20];
	for (int i = 0; i < len * 2; i+=2){
		key[i/2] = *p_vmem;
		p_vmem += 2;
	}

	//printf(key);

	u8* st;
	p_vmem -= len * 2;
	int i = 0;
	for(u8* p = (u8*)V_MEM_BASE;p<p_vmem;p+=2){

		if(*p == '\n'){
			i = 0;
			continue;
		}
		if(key[i] == *p){
			if(i==0)
				st = p;
			
			if(i==len-1){
				for(int j=0;j< len;j++){
					*(st+1+j*2) = RED;
				}
				i = 0;
			}
			else
				i++;
		}
		else{
			i = 0;
		}
	}
	
}

PUBLIC void exitFind(TTY* p_tty){
	int len = p_tty->p_console->cursor - init_cursor;
	u8* p_vmem = (u8*)(V_MEM_BASE + p_tty->p_console->cursor * 2 - len * 2);
	while(len>0){
		out_char(p_tty->p_console,'\b',0);
		len--;
	}
	for(u8* p = (u8*)V_MEM_BASE;p < p_vmem; p++){
		if(*p == RED)
			*p = DEFAULT_CHAR_COLOR;
	}
	//printf("exit\n");
}

/*======================================================================*
			      put_key
*======================================================================*/
PRIVATE void put_key(TTY* p_tty, u32 key)
{
	if (p_tty->inbuf_count < TTY_IN_BYTES) {
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}
}


/*======================================================================*
			      tty_do_read
 *======================================================================*/
PRIVATE void tty_do_read(TTY* p_tty)
{
	if (is_current_console(p_tty->p_console)) {
		keyboard_read(p_tty);
	}
}


/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY* p_tty)
{
	int isTab = 0;
	if (p_tty->inbuf_count) {
		char ch = *(p_tty->p_inbuf_tail);
		
		if (*(p_tty->p_inbuf_tail - 1)=='\t') // \ti\b
		{
			isTab = 1;
		}

		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;
		out_char(p_tty->p_console, ch, isTab);
	}
}

/*======================================================================*
                              tty_write
*======================================================================*/
PUBLIC void tty_write(TTY* p_tty, char* buf, int len)
{
        char* p = buf;
        int i = len;

        while (i) {
                out_char(p_tty->p_console, *p++, 0);
                i--;
        }
}

/*======================================================================*
                              sys_write
*======================================================================*/
PUBLIC int sys_write(char* buf, int len, PROCESS* p_proc)
{
        tty_write(&tty_table[p_proc->nr_tty], buf, len);
        return 0;
}


/*
A：进入查找模式 flag
不清空屏幕
输入，显红色，直到enter
屏蔽输入
查找输入的内容，匹配的改颜色

B：退出查找模式
删除关键字
恢复文本颜色
*/