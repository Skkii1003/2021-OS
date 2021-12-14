section .text	
		global _start
		_start:
			call input
			call split

			call add
			call print1

			call mul

		
		;input x and y
		;read as a string!!!!!!!!!!
		input:
			;;��ӡ������ʾ
			mov eax , 4
			mov ebx , 1
			mov ecx , hint
			mov edx , 23
			int 80h

			;;����x,y
			mov eax , 3
			mov ebx , 1
			mov ecx , nums
			mov edx , 255
			int 80h
			ret

		;;split nums into x and y
		;split when meets space and not a number !!!!!!!!!!
		split:
			mov ebp , nums
			mov eax , x

		.loop1:
			cmp byte[ebp] , ' '
			je .next

			mov bl , [ebp]
			mov [eax] , bl
			add ebp , 1
			add eax , 1
			add dword[x_len] , 1
			jmp .loop1

		.next:
			add ebp , 1
			mov eax , y

		.loop2:
			
			cmp byte[ebp] , '0'
			jb .none
			cmp byte[ebp] , '9'
			ja .none

			mov bl , [ebp]
			mov [eax] , bl
			add ebp , 1
			add eax , 1
			add dword[y_len] , 1

			jmp .loop2

		.none:
			ret


		;;addition
		add:
			mov al , 0
			mov ebp , res1
			mov dword[res1_len] , 0
			mov ebx , [x_len]
			mov ecx , [y_len]
			sub ebx , 1
			sub ecx , 1

		;save middle result in al!!!!!!!!!!!!!!
		.loop:
			cmp ebx , 0
			jl .xless

			mov edx , x
			add edx , ebx
			mov al , [edx]
			sub al , '0'
			sub ebx , 1
			jmp .y_cmp

		.xless:
			mov al , 0

		.y_cmp:
			cmp ecx , 0
			jl .yless

			mov edi , y
			add edi , ecx
			add al , [edi]
			sub al , '0'
			sub ecx , 1
			jmp .cal

		.yless:
			add al , 0

		.cal:
			add al , [carry]
			cmp al , 9
			ja .bigger

			mov dword[carry] , 0
			jmp .save

		.bigger:
			mov dword[carry] , 1
			sub al , 10

		.save:
			add al , 48
			mov [ebp] , al
			add ebp , 1

			;;x and y both are 0
			cmp ebx , 0
			jl .elif1
			jmp .loop
		.elif1:
			cmp ecx , 0
			jl .add_exit

			cmp ecx , 0
			jl .elif2
			jmp .loop
		.elif2:
			cmp ebx , 0
			jl .add_exit


		.add_exit:
			cmp dword[carry] , 0
			je .up

			mov byte[ebp] , 49
			add ebp , 1
		.up:
			sub ebp , res1
			mov [res1_len] , ebp
			ret


		;;multiple
		mul:
			cmp byte[x] , '0'
			je .iszero
			cmp byte[y] , '0'
			je .iszero

			mov dword[res1_len] , 0
			mov esp , 0 ;index of y
			mov eax , [x_len]
			add eax , [y_len]
			sub eax , 1
			mov [res2_len] , eax

			mov eax , 0
			
		.loop_y:
			mov ebp , [x_len] ; index of x
			sub ebp , 1
			mov dword[carry] , 0
			mov edi , res1

			cmp esp , [y_len]
			je .endy

		.loop_x:
			cmp ebp , 0
			jl .endx

			;+x
			mov ebx , x
			add ebx , ebp
			mov al , [ebx]
			sub al , '0'
			;*y + carry
			mov ebx , y
			add ebx , esp
			mov cl , [ebx]
			sub cl , '0'
			mul cl
			add eax , [carry]
			;cal carry
			mov dword[carry] , 0

		.less9:
			cmp eax , 9
			jg .sub10

			;if res2 has carry
			cmp esp , 0
			jne .pro
			cmp ebp , 0
			jne .pro
			cmp dword[carry] , 0
			je .pro
			add dword[res2_len] , 1

		.pro:
			mov [edi] , eax
			add edi , 1
			sub ebp , 1
			jmp .loop_x

		.sub10:
			sub eax , 10
			add dword[carry] , 1
			jmp .less9


		.endx:
			cmp dword[carry] , 0
			je .next_y

			mov eax , [carry]
			mov [edi] , eax
			add edi , 1
			
		.next_y:
			mov eax , edi
			sub eax , res1
			mov dword[res1_len] , eax

			;call res_add
			mov eax , 0
			mov ebp , res2		;;index of res2
			add ebp , esp	
			add ebp , [res1_len]
			sub ebp , 1
			mov ecx , 0		;;index of res1

			mov dword[carry] , 0

		.loopres:
			;+res2
			mov al , [ebp]

			;+res1
			cmp ecx , [res1_len]
			je .add_exitres

			mov edx , res1
			add edx , ecx
			add al , [edx]
			add ecx , 1
			;+carry
			add al , [carry]

			cmp al , 9
			ja .biggerres

			mov dword[carry] , 0
			jmp .saveres

		.biggerres:
			mov dword[carry] , 1
			sub al , 10

		.saveres:
			mov [ebp] , al
			sub ebp , 1
			jmp .loopres

		.add_exitres:
			cmp dword[carry] , 0
			je .upres

			mov eax , 0
			mov al , [ebp]
			add al , 1

			cmp al , 9
			jg .carry_forward
			mov dword[carry] , 0
			mov [ebp] , al

			jmp .upres

		.carry_forward:
			sub al , 10
			mov dword[carry] , 1
			mov [ebp] , al
			sub ebp , 1
			jmp .add_exitres

		.upres:
			;;res_add over

			add esp , 1
			jmp .loop_y

		.iszero:
			mov byte[res2] , '0'
			mov dword[res2_len] , 1

			mov eax , 0
			mov eax , 4
			mov ebx , 1
			mov ecx , res2
			mov edx , 1
			int 80h

			jmp .end

		
		.endy:
			mov ebp , res2
			mov esp , 0

			mov al , [ebp]
			mov al , 0
		
		.not0:
			cmp esp , dword[res2_len]
			je .end

			add byte[ebp] , 48

			mov eax , 0
			mov eax , 4
			mov ebx , 1
			mov ecx , ebp
			mov edx , 1
			int 80h

			add ebp , 1
			add esp , 1
			jmp .not0

		.end:

			mov eax , 4
			mov ebx , 1
			mov ecx , line
			mov edx , 1
			int 80h
			mov ebx,0
			mov eax,1
			int 80h
		

		;;print the add result
		print1:
			mov ebp , res1
			add ebp , [res1_len]
			sub ebp , 1

		.going:
			cmp ebp , res1
			jl .out

			mov dl , [ebp]
			cmp dl , 48
			jl .less48
			jmp .pri

		.less48:
			add byte[ebp] , 48
		
		.pri:
			mov eax , 4
			mov ebx , 1
			mov ecx , ebp
			mov edx , 1
			int 80h

			sub ebp , 1
			jmp .going

		.out:
			mov eax , 4
			mov ebx , 1
			mov ecx , line
			mov edx , 1
			int 80h
			ret


section .data
		hint : db "Please input x and y :" , 0Ah
		done : db "is done" , 0Ah
		on : db "running" , 0Ah
		line : db 10

		x_len : dd 0
		y_len : dd 0

		size : dd 0

		carry : dd 0
		res1_len : dd 0
		res2_len : dd 0
 
section .bss
		nums resb 255
		x resb 21
		y resb 21
		res1 resb 42
		res2 resb 42