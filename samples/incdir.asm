	incdir  /opt/amiga/gcc6/m68k-amigaos/ndk-include/
        include exec/types.i
        include exec/macros.i
        include lvo/exec_lib.i
        include lvo/dos_lib.i


        STRUCTURE vars_struct,0
        LONG    _SysBase
        LONG    _DOSBase
        LONG    _stdin
        LONG    _stdout
        LONG    vars_sizeof


;NDEBUG	EQU	1

DBG	macro
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

.PULLSP	SET	(NARG)<<2
	pea.l	.n1\@
	;; 	jsr	_kprintf
	bsr	myprintf
	lea     .PULLSP(sp),sp
	movem.l	(a7)+,d0/d1/a0/a1
	;; 	bra.s	.n2\@

	section texts,data
.n1\@	DC.b 	\1,0
   ;; 	cnop	0,2
	;; .n2\@
        code

	ENDC
	ENDM

	;;;	------------   Initialisation   ---------------
	lea.l	variables,a4
	move.l	($4).w,a6
	move.l	a6,_SysBase(a4)
	moveq   #0,d0
    ;lea.l   dosname(pc),a1
    lea.l   dosname,a1
    JSRLIB  OpenLibrary
    move.l  d0,a6
    move.l  d0,_DOSBase(a4)

	JSRLIB  Input
    move.l  d0,_stdin(a4)
	JSRLIB	Output
	move.l  d0,_stdout(a4)
	;;;	------------   End of Initialisation   ---------------

	
	DBG <"All is good\n">
	DBG <"D0 = %ld\n">, d0

	;;;	------------   Closing up things   ---------------
	move.l	_SysBase(a4),a6
	move.l	_DOSBase(a4),a1
	JSRLIB	CloseLibrary
	moveq	#0,d0
	rts

        ifnd	NDEBUG
MPIB 	EQU	512
myprintf:
        movem.l d2/d3/a2/a3/a6,-(sp)
        movea.l 6*4(sp),a0              ; fmt string
        lea     7*4(sp),a1              ; pointer to variables
        lea     -MPIB(sp),sp            ; allocate stack space for produced string
        movea.l sp,a3                   ; a3 points to where RawDoFmt will write (on stack)
        lea.l   .stuffChar(pc),a2
        move.l  ($4).w,a6
        JSRLIB  RawDoFmt


        moveq   #0,d0
        lea.l   dosname(pc),a1
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

        ; Close DOS library
        move.l  a6,a1
        move.l  ($4).w,a6
        JSRLIB  CloseLibrary

        movem.l (sp)+,d2/d3/a2/a3/a6
        rts

.stuffChar:
        move.b  d0,(a3)+
        rts
        ENDC


dosname:	dc.b "dos.library",0
        SECTION variables,BSS
variables:
        ds.b    vars_sizeof

