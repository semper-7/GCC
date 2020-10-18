.nolist
.include "tn13def.inc"
.list

.MACRO	delay
	ldi	r24,@0
delay:	dec	r24
	brne	delay
.ENDMACRO
.MACRO	ldx
	ldi	r26,low(@0)
	ldi	r27,high(@0)
.ENDMACRO
.MACRO	ldy
	ldi	r28,low(@0)
	ldi	r29,high(@0)
.ENDMACRO
.MACRO	ldz
	ldi	r30,low(@0*2)
	ldi	r31,high(@0*2)
.ENDMACRO

.DSEG

.CSEG
.org 0
	rjmp	init
	reti
	reti
	reti
	reti
	reti
	reti
	reti
	rjmp	wdog
	reti

ROM:	.db	0x69,0x5a,0x4b,0x3c,0x2d,0x1e,0x0f,0x33

wdog:	out	WDTCR,r1
     	reti

init:	eor	r1, r1
	out	SREG,r1
	ldi	r24,RAMEND
	out	SPL,r24
	sbi	ACSR,ACD
	ldi	r24,0x2f
	out	DDRB,r24
	ldi	r24,0x2c
	out	DIDR0,r24

loop:	rcall	WaitRising
	rjmp	loop
loop1:	rcall	SendPresence
	rcall	ReadByte
	cpi	r16,0x55
	brne	loop2
	rcall	ReadROM
	brne	loop
	rcall	ReadByte
	cpi	r16,0x6c
	brcs	loop
	cpi	r16,0x70
	brcc	loop
	andi	r16,3
	out	PORTB,r16
	rjmp	loop
loop2:	cpi	r16,0xCC
	brne	loop
	ldi	r16,(1<<WDTIE)|(1<<WDCE)
	out	WDTCR,r16
	ldi	r16,(1<<WDTIE)|(1<<WDP2)|(1<<WDP1)
	out	WDTCR,r16
	wdr
	sei
	ldi	r16,(1<<SE)|(1<<SM1)
	out	MCUCR,r16
	sleep
	rjmp	loop

WaitRising:
	clr	r24
wrs0:	inc	r24
	sbis	PINB,4
	rjmp	wrs0
	cpi	r24,450*12/4/10
	brcs	wrs1
	ldi	r24,RAMEND
	out	SPL,r24
	rjmp	loop1
wrs1:	sbic	PINB,4
	rjmp	wrs1
	out	PORTB,r1
	ret

SendPresence:
	delay	10*12/3/10
	sbi	DDRB,4
	delay	100*12/3/10
	cbi	DDRB,4
	delay	50*12/3/10
	ret

ReadByte:
	clr	r16
	ldi	r17,8
rb00:	rcall	WaitRising
	delay	20*12/3/10
	lsr	r16
	sbic	PINB,4
	ori	r16,0x80
	dec	r17
	brne	rb00
	ret

ReadROM:
	ldi	r18,8
	ldz	ROM
rr00:	rcall	ReadByte
	lpm	r17,Z+
	cp	r16,r17
        brne	rr01
	dec	r18
	brne	rr00
rr01:	ret
