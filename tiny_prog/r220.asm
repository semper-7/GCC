.nolist
.include "tn13def.inc"
.list

; F_CPU = 1.2MHz, Fuse: default Value 0xff6a
.equ	ADDR = 0x34	; Address of receiver

.DSEG

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

comp:	in	r0,SREG
      	out	PORTB,r1
	rcall	l220
	ldi	r24,0x1b
	out	ACSR,r24
	clr	r25
	out	SREG,r0
	reti

init:	clr	r1
	out	SREG,r1
	ldi	r16,RAMEND
	out	SPL,r16
	ldi	r16,0x1c
	out	DDRB,r16
	ldi	r16,0x1b
	out	ACSR,r16
	clr	r17

l000:	cli
	out	PORTB,r17
	ldi	r16,8
	sei
l001:	dec	r16			;imp 20us (8*3/1.2MHz)
	brne	l001
	cli
      	out	PORTB,r1
	sei
	ldi	r16,128
l002:	dec	r16			;pause 320us (256*3/1.2MHz)
	brne	l002
	inc	r25
	cpi	r25,17500/(320+20)	; no int >17,5ms
	brcs	l000
	clr	r25
	clr	r19
	rjmp	l000

l220:	clr	r18
l221:	clr     r24
l222:	inc	r24
	cpi	r24,12			;imp < 70us (12*7/1.2MHz)
	brcc	l229
	sbic	ACSR,ACO
	rjmp	l222
	inc	r18
	clr     r24
l224:	inc	r24
	cpi	r24,12			;pause < 70us (12*7/1.2MHz)
	brcc	l225
	sbis	ACSR,ACO
	rjmp	l224
	rjmp	l221
l225:	cpi	r18,7
	breq	l226
	cpi	r18,9
	brne	l229
l226:	cpi	r18,8
 	ror	r23
	inc	r19
	cpi	r19,8
	brne	l229
	mov	r22,r23
	andi	r22,0xfc
	cpi	r22,ADDR
	brne	l229
	swap	r23
	lsr	r23
	andi	r23,0x18
	mov	r17,r23
	clr	r19
l229:	ret
