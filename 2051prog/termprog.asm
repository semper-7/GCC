.486
.model flat,stdcall
option casemap:none
.NOLIST
include \masm32\include\windows.inc
include \masm32\include\kernel32.inc
;include \masm32\include\user32.inc
includelib \masm32\lib\kernel32.lib
;includelib \masm32\lib\user32.lib
TProc	proto:DWORD
GetArgCommandLine	PROTO:DWORD,:DWORD

.LIST
.DATA
NumCOM	DB	"COMX",0
Log	DB	"term.log",0
DCB1	DCB	<>
TO	COMMTIMEOUTS	<-1,0,0,0,0>
msgStrt	db	'Start terminal. Press "Ctrl+C" for exit.',13,10
msgErr	db	"COM port operation error!",13,10
flag	db	0

.DATA?
hInst	dd	?
hStdIn	dd	?   
hStdOut	dd	?   
ARG0	dd	?
ARG1	dd	?
hCOM	dd	?
hLog	dd	?
SZRW	dd	?
SZRW2	dd	?
TID	dd	?
buf	db	16 dup (?)
buf2	db	16 dup (?)


.CODE
START:	invoke	GetModuleHandle,0
        mov	hInst,eax
	invoke	GetStdHandle,STD_INPUT_HANDLE
	mov	hStdIn,eax
	invoke	GetStdHandle,STD_OUTPUT_HANDLE
	mov	hStdOut,eax
	invoke	GetArgCommandLine,ADDR ARG0,2
	or	eax,eax
	jnz	error
	mov	eax,ARG1
	mov	eax,[eax]
	mov	dword ptr [NumCOM],eax
	and	eax,00dfdfdfh
	cmp	eax,004d4f43h
	jnz	error
	invoke	CreateFile,ADDR NumCOM,GENERIC_READ or GENERIC_WRITE,0,0,OPEN_EXISTING,0,0
	cmp	eax,INVALID_HANDLE_VALUE
	jz	error
	mov	hCOM,eax
	invoke	WriteFile,hStdOut,ADDR msgStrt,SIZEOF msgStrt,ADDR SZRW,0
	invoke	SetCommTimeouts,hCOM,ADDR TO
	invoke	BuildCommDCB,ARG1,ADDR DCB1
	invoke	SetCommState,hCOM,ADDR DCB1
	invoke	CreateThread,0,0,OFFSET TProc,0,NORMAL_PRIORITY_CLASS,ADDR TID
	invoke	CloseHandle,eax
        invoke	CreateFile,ADDR Log,GENERIC_WRITE,FILE_SHARE_WRITE,0,CREATE_ALWAYS,FILE_ATTRIBUTE_ARCHIVE,0
	mov	hLog,eax
lll0:	invoke	ReadFile,hCOM,ADDR buf,1,ADDR SZRW,0
	cmp	SZRW,0
	jz	lll0
	cmp	buf,'^'
	jz	exit
	cmp	buf,0dh
	jnz	lll1
	mov	flag,0
lll1:	invoke	WriteFile,hStdOut,ADDR buf,1,ADDR SZRW,0
	invoke	WriteFile,hLog,ADDR buf,1,ADDR SZRW,0
	jmp	lll0
error:	invoke	WriteFile,hStdOut,ADDR msgErr,SIZEOF msgErr,ADDR SZRW,0
	invoke	ExitProcess,1
exit:	invoke	ExitProcess,0

TProc	PROC	param:DWORD
	invoke	SetConsoleMode,hStdIn,ENABLE_PROCESSED_INPUT
llt0:	cmp	flag,0
	jnz	llt0
	invoke	ReadFile,hStdIn,ADDR buf2,1,ADDR SZRW2,0
	cmp	SZRW2,0
	jz	llt0
	cmp	buf2,0dh
	jnz	llt1
	mov	flag,1
llt1:	invoke	WriteFile,hCOM,ADDR buf2,1,ADDR SZRW2,0
	jmp	llt0
TProc	ENDP

GetArgCommandLine proc uses edi arg0:DWORD, narg:DWORD
	invoke	GetCommandLine
	mov	edi,arg0
ll0:	stosd
	dec	narg
	jz	ll3
ll1:	inc	eax
	cmp	byte ptr [eax],0
	jz	ll3
	cmp	byte ptr [eax],20h
	jnz	ll1
ll2:	inc	eax
	cmp	byte ptr [eax],20h
	jz	ll2
	cmp	byte ptr [eax],0
	jnz	ll0
ll3:	mov	eax,narg
	ret
GetArgCommandLine endp

END START

