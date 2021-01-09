;******************************************************
;PIC Configuration for PIC16F887
#include "p16F887.inc"

; CONFIG1
; __config 0x2032
 __CONFIG _CONFIG1, _FOSC_HS & _WDTE_OFF & _PWRTE_OFF & _MCLRE_ON & _CP_ON & _CPD_ON & _BOREN_OFF & _IESO_OFF & _FCMEN_OFF & _LVP_OFF
; CONFIG2
; __config 0x3FFF
 __CONFIG _CONFIG2, _BOR4V_BOR40V & _WRT_OFF
;******************************************************

;**************VARIABLES**********************************

CNT_100TH   EQU 0x21	; counter for 0.01s
CNT_10TH    EQU 0x22	; counter for 0.1s
CNT_SEC	    EQU 0x23	; counter for seconds
CNT_10SEC   EQU 0x24	; counter for tens of seconds
CNT_MIN	    EQU 0x25	; counter for minutes
CNT_10MIN   EQU 0x26	; counter for tens of minutes
WCOPY	    EQU 0x70	;ALLOCATE 0x70 to WCOPY
STATCOPY    EQU 0x71	;ALLOCATE 0x71 to STATCOPY
RUN	    	EQU 0x27	;Run Variable
DCOUNT	    EQU 0x28
WC1	    	EQU 0x29
WC2	    	EQU 0x30

;***************MAIN PROGRAM***********************

ORG 000H
GOTO MAIN

ORG 004H
GOTO ISR

ORG 0008H
MAIN
BSF	STATUS, RP0
BSF	STATUS, RP1	;Change to bank 3
CLRF	ANSEL
CLRF	ANSELH		;CONFIGURE ALL PORTS TO DIGITAL
BCF	STATUS, RP1	;BANK1
MOVLW	0xFF		;255 on W
MOVWF	TRISB		;PORTB INPUT
CLRF	TRISD		;PORTD OUTPUT
CLRF	TRISA		;PORTA OUTPUT
BCF	STATUS, RP0	;BANK 0
CLRF	CNT_100TH	;CLEAR ALL VARIABLES
CLRF	CNT_10TH
CLRF	CNT_SEC
CLRF	CNT_10SEC
CLRF	CNT_MIN
CLRF	CNT_10MIN
CLRF	WCOPY
CLRF	STATCOPY
CLRF	RUN
CLRF	DCOUNT
CLRF	WC1
CLRF	WC2
CLRF	INTCON
MOVLW	0x90	    	;SET INTCON BIT7 AND BIT4 (GIE AND INTE)
MOVWF	INTCON

	
DISPLAY_LOOP	    	;displays time in standard western left to right reading mmssdd

MOVFW	CNT_100TH   	;places 100ths of second count on W
CALL	TABLE	    	;references table subroutine
MOVWF	PORTD	    	;moves W from table to PORTD
BCF	PORTA,5	    	;display first seven seg.
CALL	DELAY	    	;CALL DELAY SUBROUTINE   
BSF	PORTA,5  
  
MOVFW	CNT_10TH    	;places 10ths of second count on W
CALL	TABLE	    	;references table subroutine
MOVWF	PORTD	    	;moves W from table to PORTD
BCF	PORTA,4	    	;display on second seven seg.
CALL	DELAY	    	;CALL DELAY SUBROUTINE
BSF	PORTA,4  

MOVFW	CNT_SEC     	;places second count on W
CALL	TABLE	    	;references table subroutine
MOVWF	PORTD	    	;moves W from table to PORTD
BCF	PORTA,3	    	;display on third seven seg.
CALL	DELAY	    	;CALL DELAY SUBROUTINE
BSF	PORTA,3


MOVFW	CNT_10SEC   	;places ten seconds count on W
CALL	TABLE	    	;references table subroutine
MOVWF	PORTD	    	;moves W from table to PORTD
BCF	PORTA,2	    	;display on fourth seven seg.
CALL	DELAY	    	;CALL DELAY SUBROUTINE
BSF	PORTA,2    


MOVFW	CNT_MIN     	;places minute count on W
CALL	TABLE	    	;references table subroutine
MOVWF	PORTD	    	;moves W from table to PORTD
BCF	PORTA,1	    	;display on fifth seven seg.
CALL	DELAY	    	;CALL DELAY SUBROUTINE
BSF	PORTA,1  


MOVFW	CNT_10MIN   	;places ten minute count on W
CALL	TABLE	    	;references table subroutine
MOVWF	PORTD	    	;moves W from table to PORTD
BCF	PORTA,0	    	;display on sixth seven seg.
CALL	DELAY	    	;CALL DELAY SUBROUTINE
BSF	PORTA,0  


RUNCHECK		;CHECK IF CLOCK IS RUNNING
MOVLW	0xFF		;FF ON W
XORWF	RUN,0
BTFSS	STATUS,Z	;STOPWATCH IS ON IF Z=1
GOTO	DISPLAY_LOOP	;STOP WATCH IS OFF GO TO DISPLAY
CALL	INCCNT		;ADD NUMBERS AND DISPLAY THEM
GOTO	DISPLAY_LOOP

;*******************SUBROUTINES***************************

INCCNT
INCF	CNT_100TH	;INCREASE HUNDREDTS
MOVLW	0x0A		;PUT 10 ON W
XORWF	CNT_100TH, 0	;XOR CNT_100TH WITH W
BTFSC	STATUS, Z	;IF 10 Z=1
CALL	INCREMENT_10TH	;INCREASE TENTHS SUB
MOVFW	CNT_100TH	
CALL	WAIT		;STALL TO MAKE 100THS OF SEC

RETURN

INCREMENT_10TH
CLRF	CNT_100TH	;100THS GO TO ZERO
INCF	CNT_10TH	;INCREMENT TENTHS DIGIT
MOVLW	0x0A		;PUT 10 ON W
XORWF	CNT_10TH, 0	;XOR CNT_10TH WITH W
BTFSC	STATUS, Z	;IF 10 Z=1
CALL	INCREMENT_SEC

RETURN

INCREMENT_SEC
CLRF	CNT_10TH	;CLEAR TENTHS
INCF	CNT_SEC		;INCREMENT SECONDS
MOVLW	0x0A		;PUT 10 ON W
XORWF	CNT_SEC, 0	;XOR CNT_SEC WITH W
BTFSC	STATUS, Z	;IF 10 Z=1
CALL	INCREMENT_10SEC

RETURN

INCREMENT_10SEC
CLRF	CNT_SEC		;CLEAR SECONDS
INCF	CNT_10SEC	;INCREMENT TENS DIGIT
MOVLW	0x06		;PUT 6 ON W
XORWF	CNT_10SEC, 0	;XOR CNT_10SEC WITH W
BTFSC	STATUS, Z	;IF 6 Z=1
CALL	INCREMENT_MIN

RETURN

INCREMENT_MIN
CLRF	CNT_10SEC	;CLEAR TENS
INCF	CNT_MIN		;INCREMENT MINUTES
MOVLW	0x0A		;PUT 10 ON W
XORWF	CNT_MIN, 0	;XOR CNT_MIN WITH W
BTFSC	STATUS, Z	;IF 10 Z=1
CALL	INCREMENT_10MIN

RETURN

INCREMENT_10MIN
CLRF	CNT_MIN		;CLEAR MINUTES
INCF	CNT_10MIN	;INCREMENT TEN MINUTES
MOVLW	0x06		;PUT 6 ON W
XORWF	CNT_10MIN, 0	;XOR CNT_10MIN WITH W
BTFSS	STATUS, Z	;IF 6 Z=1
GOTO	BACK
CLRF	CNT_10MIN

BACK
RETURN


TABLE
 ADDWF   PCL,1                ;increment PC by index
 RETLW   0C0H                 ;the code of 0( the common LED is anode)    
 RETLW   0F9H                 ;the code of 1                              
 RETLW   0A4H                 ;the code of 2                              
 RETLW   0B0H                 ;the code of 3                              
 RETLW   99H                  ;the code of 4                              
 RETLW   92H                  ;the code of 5  
 RETLW   82H                  ;the code of 6                              
 RETLW   0F8H                 ;the code of 7                              
 RETLW   80H                  ;the code of 8
 RETLW   90H                  ;the code of 9  



DELAY ;DO NOT MODIFY
    MOVLW   0FFH
    MOVWF   DCOUNT
DLOOP    DECFSZ  DCOUNT
         GOTO   DLOOP
         RETURN


ISR
MOVWF	WCOPY		;MOVE W TO WCOPY
SWAPF	STATUS, W	;SWAP STATUS INTO W
MOVWF	STATCOPY	;MOVE W INTO STATCOPY	 
MOVLW	0xFF		;SET W REGISTER
XORWF	RUN,0		;BITCHECK RUN
BTFSS	STATUS,Z
GOTO	SETRUN	
GOTO	CLEARRUN


ISRFINISH			;INTERRUPT ENDING SUBROUTINE
BCF		INTCON,1	;CLEAR EXTERNAL INTERRUPT FLAG (INTF)
SWAPF	STATCOPY,W	;PUT ORIGINAL STATUS INTO W
MOVWF	STATUS		;PUT STATUS INTO STATUS
SWAPF	WCOPY,F		;SWAP WCOPY
SWAPF	WCOPY,W		;PUT WCOPY INTO W
RETFIE				;RETURN FROM INTERRUPT


SETRUN
MOVLW	0xFF
MOVWF	RUN	 	;SET RUN TO 1
GOTO	ISRFINISH

CLEARRUN
CLRF	RUN		;SET RUN TO 0
GOTO	ISRFINISH
 


WAIT	;stalls code to increment CNT_100TH approximately 0.01 seconds
		;loses .25 seconds over 1 minute run time
		;adds 2532us cycles to running time 
		
MOVLW	0x54		;1us y=54
MOVWF	WC1			;1us	 

OUTER	
MOVLW	0x14		;1us  x=14	y
MOVWF	WC2			;1us		y

INNER 	
DECFSZ	WC2			;1 or 2us	y(x+1)
GOTO	INNER		;2 us		2y(x-1)
DECFSZ	WC1			;1 or 2 us	(y+1)
GOTO	OUTER		;2 us		2(y-1)					
RETURN				;2 us

END
