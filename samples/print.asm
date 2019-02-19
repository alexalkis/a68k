	    ;incdir /opt/amiga/gcc6/m68k-amigaos/ndk-include/
	    include exec/macros.i
        include lvo/exec_lib.i
        include lvo/dos_lib.i

DEBUG	EQU	1

DBUG	macro
	ifnd	NDEBUG

	; save all regs
	movem.l	d0/d1/a0/a1,-(a7)
	IFGE	NARG-9
		move.l	\9,-(sp)		; stack arg8
	ENDC
	IFGE	NARG-8
		move.l	\8,-(sp)		; stack arg7
	ENDC
	IFGE	NARG-7
		move.l	\7,-(sp)		; stack arg6
	ENDC
	IFGE	NARG-6
		move.l	\6,-(sp)		; stack arg5
	ENDC
	IFGE	NARG-5
		move.l	\5,-(sp)		; stack arg4
	ENDC
	IFGE	NARG-4
		move.l	\4,-(sp)		; stack arg3
	ENDC
	IFGE	NARG-3
		move.l	\3,-(sp)		; stack arg2
	ENDC
	IFGE	NARG-2
		move.l	\2,-(sp)		; stack arg1
	ENDC

PULLSP	SET	(NARG)<<2		
	pea.l	.n1\@
	;; 	jsr	_kprintf
	bsr	myprintf
	lea.l	PULLSP(sp),sp			
	movem.l	(a7)+,d0/d1/a0/a1
	bra.s	.n2\@

.n1\@	dc.b	\1,0
	cnop	0,2
.n2\@
	endc
	endm



	moveq	#0,d0
	moveq	#1,d1
	move.l	#63001,d2

.addloop:
	add.l	d1,d0
	addq.l	#1,d1
	cmp	d1,d2
	bne	.addloop
	DBUG <"Sum = %ld\n">,d0
	rts


MPIB    EQU 512

myprintf:
        movem.l d2/d3/a2/a3/a6,-(sp)
        movea.l 6*4(sp),a0              ; fmt string
        lea     7*4(sp),a1              ; pointer to variables
        lea     -MPIB(sp),sp            ; allocate stack space for produced string
        movea.l sp,a3                   ; a3 points to where RawDoFmt will write (on stack)
        lea.l   stuffChar(pc),a2
        move.l  ($4).w,a6
        JSRLIB  RawDoFmt


        moveq   #0,d0
        lea.l   dosname,a1
        JSRLIB  OpenLibrary 
        move.l  d0,a6
        JSRLIB  Output
        move.l  d0,d1                   ;file to write to (Output())

        move.l  a3,d2                   ;string start for Write
        move.l  a3,a1
.strlen tst.b   (a1)+
        bne.s   .strlen
        sub.l   a3,a1
        move.l  a1,d3                   ;length
        subq.l  #1,d3                   ;account for the fact that a1 has passed the \0 by one byte

        JSRLIB  Write

        lea     MPIB(sp),sp             ; deallocate buffer 
        movem.l (sp)+,d2/d3/a2/a3/a6
        rts

stuffChar:
        move.b  d0,(a3)+
        rts

dosname:	dc.b "dos.library",0
