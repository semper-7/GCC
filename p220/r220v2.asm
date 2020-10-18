.nolist
.include "tn13def.inc"
.list

.MACRO	delay
	ldi	r16,@0
delay:	dec	r16
	brne	delay
.ENDMACRO
.MACRO	delayus
	ldi	r24,low(@0)
	ldi	r25,high(@0)
	rcall	dus
.ENDMACRO
.MACRO	delayms
	ldi	r24,low(@0)
	ldi	r25,high(@0)
	rcall	dms
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
.MACRO	print
	ldz	@0
	rcall	print
.ENDMACRO

.equ	F_CPU = 8000000
.equ	ADDR = 0x34

.DSEG
buf:	.byte	16

.CSEG

.org 0
	rjmp	init
	reti
	reti
	reti
	reti
	rjmp	comp
	reti
	reti
	reti
	reti
;szst:	.db	"Start P220"
;	.db	13,10,0,0

comp:	mov	r8,r16
	in	r0,SREG
	rcall	rx220
	ldi	r16,0x1b
	out	ACSR,r16
	out	SREG,r0
	mov	r16,r8
	reti

init:	clr	r1
	out	SREG,r1
	ldi	r16,RAMEND
	out	SPL,r16
	ldi	r16,0x40
	out	OSCCAL,r16
	sbi	ADMUX,MUX1
	sbi	ADCSRB,ACME
	ldi	r16,0x0c
	out	DDRB,r16
;	out	PORTB,r16
;	delayms	2000
;	print	szst

l000:	ldy	buf
l001:	clr	r27
	ldi	r16,0x1b
	out	ACSR,r16
	sei
l002:	nop
	adiw	r26,0
	adiw	r26,1
	cpi	r27,0x32
	brcs	l002
	cpi	r28,buf+8
	brne	l000
	cli
	ldy	buf
l004:	ld	r22,Y+
	ror	r22
	ror	r22
	ror	r23
	cpi	r28,buf+8
	brne	l004
	mov	r22,r23
	andi	r22,0xfc
	cpi	r22,ADDR
	brne	l000
	lsl	r23
	lsl	r23
	andi	r23,0x0c
	out	PORTB,r23
;	ldi	r22,'.'
;	rcall	tx
;	mov	r22,r23
;	rcall	tx
	rjmp	l000

rx220:	clr	r18
rx221:  clr     r24
rx222:	inc	r24
	breq	rx229
	sbic	ACSR,ACO
	rjmp	rx222
	inc	r18
	clr     r24
rx224:	inc	r24
	breq	rx225
	sbis	ACSR,ACO
	rjmp	rx224
	rjmp	rx221
rx225:	cpi	r18,11
	breq	rx228
	cpi	r18,13
	brne	rx229
rx228:	st	Y+,r18
 	clr	r27
rx229:	ret

;rx:	delay	F_CPU/115200/2
;	ldi	r17,8
;rx1:	lsr	r23
;	sbic	PINB,3
;	ori	r23,0x80
;	delay	F_CPU/115200/3-3
;	dec	r17
;	brne	rx1
;	ret

;tx1:	sec
;	ror	r22
;	brcc	tx2
;	sbi	PORTB,2
;	rjmp	tx3
;tx:	ldi	r17,10
;tx2:	cbi	PORTB,2
;tx3:	delay	F_CPU/115200/3-3
;	dec	r17
;	brne	tx1
;lret:	ret
;print:	lpm	r22,Z+
;	or	r22,r22
;	breq	lret
;	rcall	tx
;	rjmp	print

;dus:	push	r24
;	pop	r24
;	sbiw	r24,1
;	brne	dus
;	ret
;dms:	push	r24
;	push	r25
;	delayus	998
;	pop	r25
;	pop	r24
;	sbiw	r24,1
;	brne	dms
;	ret
