setpixelm1:
	push %a
	push %b
	push %c
	
	mov %a [%sp + 8h]

	mov %b %al
	mul %b 12h
	add %b @T1
	
	mov %c %ah
	mov %a %ah
	
	div %c 8h
	mod %a 8h

	add %b %c
	
	mov %c 7h
	sub %c %a
	mov %a %c

	mov %c 1h
	shl %c %a
	
	mov %a [%b]
	or  %a %c
	mov [%b] %a
	
	pop %c
	pop %b
	pop %a
	ret

resetpixelm1:
	push %a
	push %b
	push %c
	
	mov %a [%sp + 8h]

	mov %b %al
	mul %b 12h
	add %b @T1
	
	mov %c %ah
	mov %a %ah
	
	div %c 8h
	mod %a 8h

	add %b %c
	
	mov %c 7h
	sub %c %a
	mov %a %c

	mov %c 1h
	shl %c %a
	not %c
	
	mov %a [%b]
	and %a %c
	mov [%b] %a
	
	pop %c
	pop %b
	pop %a
	ret

clsm1:
	memset [@T1] 3A98h 0h
	ret
