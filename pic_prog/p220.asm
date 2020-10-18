	list      p=12F635
	#include <p12f635.inc>
	errorlevel	-302
	__CONFIG	30d4h
;2МГц - 13,46мкГ - 470пФ
;****** Регистры ***************************************************************
wtmp	equ	0x40	; W temp
stmp	equ	0x41	; Status temp
nbit	equ	0x42	; счетчик бит
nbyte	equ	0x43	; счетчик байт
us2	equ	0x44	; счетчик us*2
ms1	equ	0x45	; счетчик ms
txbyte	equ	0x46	; текущий передаваемый байт
buf	equ	0x48	; начало буфера приема

;*******************************************************************************
	org	0
	goto	start		; к началу
;****** Обработчик прерываний **************************************************
	org	4
	movwf   wtmp
	movf	STATUS,w
	movwf	stmp
	btfss	PIR1,C1IF
	goto	i000
	bcf	PORTA,2
	call	RX
;	call	ONOFF
	movf	buf,w
	movwf	txbyte
	call	TX
	bcf	INTCON,PEIE
	bcf	PIR1,C1IF
i000	movf    stmp,w
	movwf	STATUS
	swapf   wtmp,f
	swapf   wtmp,w
	retfie
;*******************************************************************************
start	bsf	CMCON0,CM1
	movlw	30h
	movwf	PORTA
	bsf	STATUS,RP0	; 1 Bank
	bsf	OSCCON,IRCF0	; 8 MHz
	movlw	0bh
	movwf	TRISA
	bsf	PIE1,C1IE
	bcf	STATUS,RP0	; 0 Bank

	movlw	.10
	movwf	nbyte
l004	call	MS256
	decfsz	nbyte,f
	goto	l004

	clrf	nbyte
l002	movf	nbyte,w
	call	szstart
	movwf	txbyte
	movf	txbyte,f
	btfsc	STATUS,Z
	goto	l003
	call	TX
	incf	nbyte,f
	goto	l002

l003	bsf	INTCON,GIE

l000	btfsc	PORTA,2
	goto	l000
	btfsc	INTCON,PEIE
	goto	l001
	call	MS256
	bsf	PORTA,2
	call	US514
	clrf	PIR1
	bsf	INTCON,PEIE
l001	bsf	PORTA,2
	goto	l000

szstart	addwf	PCL,f
	retlw	's'
	retlw	't'
	retlw	'a'
	retlw	'r'
	retlw	't'
	retlw	0dh
	retlw	0ah
	retlw	0

ONOFF	movlw	55h
	subwf	buf+1,w
	btfss	STATUS,Z
	return
	movlw	65h
	subwf	buf,f
	btfss	STATUS,C
	return
	movlw	6
	subwf	buf,w
	btfsc	STATUS,C
	return
	btfsc	buf,1
	return
	movf	buf,w
	call	TAB
	movwf	PORTA
	bcf	INTCON,PEIE
	return

TAB	addwf	PCL,f
	retlw	30h
	retlw	20h
	retlw	30h
	retlw	30h
	retlw	10h
	retlw	00h

RX	movlw	buf
	movwf	FSR
	movlw	2
	movwf	nbyte
	movlw	.8000000/.16/.38400/2*3-4
	call	US		;
RX00	movlw	8
	movwf	nbit		; счетчик бит=8
RX01	bcf	STATUS,C	; сбрасываем флаг С
	rrf	INDF,f		; сдвигаем регистр вправо
	btfsc	CMCON0,COUT	; если вход=1, то пишем 1, если нет - пропускаем
	bsf	INDF,7		; ставим бит=1
	movlw	.8000000/.16/.38400-3
	call	US
	decfsz	nbit,f		; уменьшаем счетчик и проверяем - приняли 8 бит или нет
	goto	RX01		; если нет - принимаем следующий бит
	incf	FSR,f		; указатель на сл. байт в буфере
	decfsz	nbyte,f		; уменьшаем счетчик и проверяем - приняли 16 бит или нет
	goto	RX00		; если нет - принимаем следующие 8
	return

US	movwf	us2
US514	nop
	decfsz	us2,f
	goto	US514
	return
MS	movwf	ms1
MS256	call	US514
	movlw	(.1000-.514-4)/2
	call	US
	decfsz	ms1,1
	goto	MS256
	return

TXnext	bsf	STATUS,0
	rrf	txbyte,1
	btfss	STATUS,0
	goto	zero
	bsf	PORTA,5
	goto	one
TX	movlw	.10
	movwf	nbit
zero	bcf	PORTA,5
one	movlw	(.8000000/4/.9600-12)/4
	call	US
	decfsz	nbit,1
	goto	TXnext
	return


	end
