;Модуль индикации 9-х разрядном семисегментном индикаторе
;на AT89C2051 и SN74ALS574B.
;Управление по одному проводу UART 115200,
;формат пакета 12 байт: ^~~123456789
;PIN	PINNAME	FUNK    PIN574
;1	RESET	RESET	
;2	P3.0	RX 115200
;3	P3.1	NC
;4	XL1	Quarz 22,1184MHz+22pF+GND
;5	XL2	Quarz 22,1184MHz+22pF+GND
;6	P3.2	KEY
;7	P3.3	NC
;8	P3.4	0E	1 
;9	P3.5	CLK	11
;10	GND	GND	10
;11	P3.7	9	
;12	P1.0    8,f	9
;13	P1.1    7,b	8
;14	P1.2    6,g	7
;15	P1.3    5,d	6
;16	P1.4    4,e	5
;17	P1.5    3,a	4
;18	P1.6    2,h	3
;19	P1.7    1,c	2
;20	VDD	VDD	20

$MOD2051
kt0	equ	65535 - 1600

CSEG	
ORG 0
	ajmp	init
ORG 3
	reti
ORG 0bh
	ajmp	ind
ORG 13h
	reti
ORG 1Bh
	reti
ORG 23h
	push	ACC
	push	PSW
	jnb	098h,rx3	;SCON bit 0
	mov	PSW,#8
	mov	A,SBUF
	cjne	A,#'^',rx1
	mov	R1,#0dh
rx1:	mov	@R1,A
	inc	R1
	cjne	R1,#19h,rx3
	cjne	R5,#'^',rx2
	cjne	R6,#'~',rx2
	cjne	R7,#'~',rx2
	mov	40h,10h
	mov	41h,11h
	mov	42h,12h
	mov	43h,13h
	mov	44h,14h
	mov	45h,15h
	mov	46h,16h
	mov	47h,17h
	mov	48h,18h
rx2:	mov	R1,#0dh
rx3:	mov	SCON,#50h
	pop	PSW	
	pop	ACC
	reti	

;76543210
;fbgdeahc
;Symbol table:	
; ,c,h,H,L,n,r,o,P,u,U,/u,Г,П,У, ;
;0.,1.,2.,3.,4.,5.,6.,7.,8.,9.,A.,b.,C.,d.,E.,F.;
; ,a,b,c,d,e,f,g,h,ad,ag,dg,adg,ef,bcef,abfg(segments);
;0,1,2,3,4,5,6,7,8,9,A,b,C,d,E,F;
;+40h --> flash "symbol"
;+C0h --> flash "."
tab:	DB	0ffh,038h,0a9h,0e9h,098h,029h,039h,028h,0ech,019h,0d9h,0e8h,08ch,0cdh,051h,0ffh
	DB	0dfh,043h,07eh,077h,0e3h,0b7h,0bfh,047h,0ffh,0f7h,0efh,0bbh,09eh,07bh,0beh,0aeh
	DB	000h,004h,040h,001h,010h,008h,080h,020h,002h,014h,024h,030h,034h,088h,0c9h,0e4h
	DB	0ddh,041h,07ch,075h,0e1h,0b5h,0bdh,045h,0fdh,0f5h,0edh,0b9h,09ch,079h,0bch,0ach
	DB	0ffh,0feh,0fdh,0fbh,0f7h,0efh,0dfh,0bfh,07fh,0ffh,0ffh,0ffh,0ffh,0ffh,0ffh,0ffh

ind:	push	ACC
	push	PSW
	mov	PSW,#8
	mov	TH0,#(high kt0)
	mov	TL0,#(low kt0)
	mov	DPTR,#tab
	mov	A,@R0
	mov	R2,#3fh
	jnb	0e6h,ind1	;ACC bit 6
	jnb	6,ind1		;20h bit 6 if flash 1Hz
	mov	R2,#1fh
	jb	0e7h,ind1	;ACC bit 7
	clr	A
	ajmp	ind2	
ind1:	anl	A,R2
	movc	A,@A+DPTR
ind2:	setb	P3.4
	mov	P1,A
	mov	A,R0
	movc	A,@A+DPTR
	setb	P3.5
	setb	P3.7
	clr	P3.5
	mov	P1,A
	cjne	A,#0ffh,ind4
	clr	P3.7
ind4:	clr	P3.4
	inc	R0
	cjne	R0,#49h,ind5
	mov	R0,#40h
	inc	20h
ind5:	pop	PSW	
	pop	ACC
	reti	

init:	clr	A
	mov	PSW,#8
	mov	R0,#40h
	mov	R1,#0dh
	mov	PSW,A
	mov	SP,#5fh
	mov	R0,#40h
	mov	A,#67h
l000:	mov	@R0,A
	inc	R0
	cjne	R0,#49h,l000
	mov	TMOD,#21h
	mov	TH1,#0ffh
	mov	TH0,#(high kt0)
	mov	TL0,#(low kt0)
	mov	IP,#02h
	mov	IE,#92h
	mov	TCON,#50h
	mov	SCON,#50h
	mov	PCON,#80h

l001:	ajmp	l001
	
END