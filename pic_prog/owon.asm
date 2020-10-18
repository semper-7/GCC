	list      p=12F635
	#include <p12f635.inc>
	errorlevel	-302
	__CONFIG	30d4h

mov	macro	reg,var 
	movlw	var
	movwf	reg
	endm

delay	macro	cnt
	local	ldl0
	mov	count,cnt
ldl0	decfsz	count,f
	goto	ldl0
	endm


;****** Регистры ***************************************************************
count	equ	0x40	; счетчик задержки
nbit	equ	0x41	; счетчик бит
nbyte	equ	0x42	; счетчик байт
xbyte	equ	0x43	; байт приемника
ibyte	equ	0x44	; index

;*******************************************************************************
	org	0
	goto	init		; к началу
;****** Обработчик прерываний **************************************************
	org	4
	retfie
;*******************************************************************************
rom	addwf	PCL,f
	retlw	69h
	retlw	5ah
	retlw	4bh
	retlw	3ch
	retlw	2dh
	retlw	1eh
	retlw	0fh
	retlw	33h

init	mov	CMCON0,7
	mov	PORTA,3
	bsf	STATUS,RP0	; 1 Bank
	bcf	OSCCON,IRCF1	; 1 MHz
	mov	TRISA,3ch
	bcf	STATUS,RP0	; 0 Bank

loop	call	WaitReset
	call	SendPresence
	goto	loop
	call	ReadByte
	movlw	55h
	subwf	xbyte,w
	btfss	STATUS,Z
	goto	loop
	call	ReadROM
	btfss	STATUS,Z
	goto	loop
	call	ReadByte
	movlw	6ch
	subwf	xbyte,w
	btfss	STATUS,C
	goto	loop
	movlw	6fh
	subwf	xbyte,w
	btfsc	STATUS,C
	goto	loop
	movf	xbyte,w
	andlw	3
	movwf	PORTA
	goto	loop

WaitRising
wrs0	btfss	PORTA,2
	goto	wrs0
wrs1	btfsc	PORTA,2
	goto	wrs1
	clrf	PORTA
	return

WaitReset
	call	WaitRising
	clrf	count
wr00	incf	count,f
	btfss	PORTA,2
	goto	wr00
	movlw	.440/4
	subwf	count,w
;	btfss	STATUS,C
;	goto	WaitReset
	return

SendPresence
	delay	.12/3
	bsf	STATUS,RP0	; 1 Bank
	bcf	TRISA,2
	bcf	STATUS,RP0	; 0 Bank
	clrf	PORTA
	delay	.250
	bsf	STATUS,RP0	; 1 Bank
	bsf	TRISA,2
	bcf	STATUS,RP0	; 0 Bank
	delay	.60/3
	return

ReadByte
	mov	nbit,8
rb00	call	WaitRising
	delay	24/3
	bcf	STATUS,C	; сбрасываем флаг С
	rrf	xbyte,f		; сдвигаем регистр вправо
	btfsc	PORTA,2		; если вход=1, то пишем 1, если нет - пропускаем
	bsf	xbyte,7		; ставим бит=1
	decfsz	nbit,f
	goto	rb00
	return

ReadROM
	clrf	ibyte
	mov	nbyte,8
rr00	call	ReadByte
	movf	ibyte,w
	call	rom	
	subwf	xbyte,w
	btfss	STATUS,Z
	return
	incf	ibyte,f
	decfsz	nbyte,f
	goto	rr00
	return


	end
