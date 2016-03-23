/*------------------------------------------------------------------*/
/*								    */
/*		      MC68000 Cross Assembler			    */
/*								    */
/*	         Copyright 1985 by Brian R. Anderson		    */
/*								    */
/*               #define statements - April 16, 1991		    */
/*								    */
/*   This program may be copied for personal, non-commercial use    */
/*   only, provided that the above copyright notice is included	    */
/*   on all copies of the source code.  Copying for any other use   */
/*   without the consent of the author is prohibited.		    */
/*								    */
/*------------------------------------------------------------------*/
/*								    */
/*		Originally published (in Modula-2) in		    */
/*	    Dr. Dobb's Journal, April, May, and June 1986.          */
/*								    */
/*	 AmigaDOS conversion copyright 1991 by Charlie Gibbs.	    */
/*								    */
/*------------------------------------------------------------------*/

#include <stdio.h>
#include <ctype.h>


//#if defined(__SASC) || defined(_GCC_)
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
// #else
// extern char *malloc();
// extern int  open(), creat(), read(), write(), close(), unlink();
// extern long lseek();
// #endif

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#define NODEF 32767	/* High line number for undefined symbols */

/* Assembler configuration parameters */
#define MAXLINE   128	/* Longest source line */
#define MAXFN     41	/* Maximum length of file name */
#define MAXSREC   16	/* Maximum S-record data length */
#define MAXREF    4	/* Number of line numbers in reference entry */
#define DEFHASH   2048	/* Default number of elements in hash table */
#define DEFHEAP2  1024	/* Default size for secondary heap */
#define INCSKSIZ  2048	/* Size of INCLUDE skip table */
#ifdef MSDOS
#define CHUNKSIZE 2048	/* Memory chunks for Itty Bitty Memories */
#else
#define CHUNKSIZE 8192	/* Size of memory chunks allocated for tables */
#endif
#define FWDSIZE   1024	/* Size of forward branch optimization log */
#define BUFFSIZE  4096 //2048	/* File buffer size */
#define ObjMAX    32	/* Max. hex object code digits in listing */

/* Hunk number definitions */
#define HunkNone 0	/* Not in a hunk */
#define HunkUnit 999
#define HunkName 1000
#define HunkCode 1001
#define HunkData 1002
#define HunkBSS  1003
#define HunkR32  1004
#define HunkR16  1005
#define HunkR8   1006
#define HunkExt  1007
#define HunkSym  1008
#define HunkDbg  1009
#define HunkEnd  1010

#define MEMF_FAST 0x80000000L	/* Hunk must load in FAST memory. */
#define MEMF_CHIP 0x40000000L	/* Hunk must load in CHIP memory. */

/* Hunk numbers denoting special symbol attributes */
#define ABSHUNK 32767	/* Absolute */

/* Addressing mode flag values */
#define DReg   1	/* Data Register */
#define ARDir  2	/* Address Register Direct */
#define ARInd  3	/* Address Register Indirect */
#define ARPost 4	/* Address Register with Post-Increment */
#define ARPre  5	/* Address Register with Pre-Decrement */
#define ARDisp 6	/* Address Register with Displacement */
#define ARDisX 7	/* Address Register with Disp. & Index */
#define AbsW   8	/* Absolute Short (16-bit Address) */
#define AbsL   9	/* Absolute Long (32-bit Address) */
#define PCDisp 10	/* Program Counter Relative, with Displacement */
#define PCDisX 11	/* Program Counter Relative, with Disp. & Index */
#define Imm    12	/* Immediate */
#define MultiM 13	/* Multiple Register Move */
#define SR     14	/* Status Register */
#define CCR    15	/* Condition Code Register */
#define USP    16	/* User's Stack Pointer */
#define Null   0	/* Error Condition, or Operand missing */

#define X0   0	/* Register types */
#define Dreg 1
#define Areg 2

#define S0   0	/* Size types */
#define Byte 1
#define Word 2
#define Long 4

#define CMPM  0xB108
#define JMP   0x4EC0
#define JSR   0x4E80
#define LEA   0x41C0
#define LINK  0x4E50
#define NOP   0x4E71
#define PEA   0x4840
#define STOP  0x4E72
#define SWAP  0x4840
#define UNLK  0x4E58

#define None	 0	/* Assembler directives */
#define Org	 1
#define DC	 2
#define DS	 3
#define Even	 4
#define End	 5
#define Cnop	 6
#define Section	 7
#define CSeg	 8
#define DSeg	 9
#define BSS	10
#define Idnt	11
#define DCB	12
#define Near	13
#define Far	14
#define BadMac	15
#define Incbin  16
#define SkipDir	17	/* Skippable INCLUDE directives start here. */
#define Equ	17
#define Public	18
#define Xdef	19
#define Xref	20
#define Page	21
#define DoList	22
#define NoList	23
#define Space	24
#define Title	25
#define Include	26
#define Set	27
#define Macro	28
#define IfEQ	29
#define IfNE	30
#define IfGT	31
#define IfGE	32
#define IfLT	33
#define IfLE	34
#define IfC	35
#define IfNC	36
#define IfD	37
#define IfND	38
#define EndC	39
#define Equr	40
#define Reg	41
#define MacCall	42

/* BITSETs of the modes MISSING from effective address modes  */
#define  ea 0x0000	/* Effective addressing - all modes */
#define dea 0x0002	/* Data effective addressing        */
#define mea 0x0003	/* Memory effective addressing      */
#define cea 0x081B	/* Control effective addressing     */
#define aea 0x0E00	/* Alterable effective addressing   */
#define xxx 0xE000	/* extra modes: CCR/SR/USP          */

#define IN &		/* Simulated BITSET test */

/* AdrModeA bit definitions */
#define RegMem3	0x0001	/* 0 = register, 1 = memory */
#define Ry02	0x0002	/* Register Rx - bits 0-2 */
#define Rx911	0x0004	/* Register Ry - bits 9-11 */
#define Data911	0x0008	/* Immediate data - bits 9-11 */
#define CntR911	0x0010	/* Count register or immediate data */
#define Brnch	0x0020	/* Relative branch */
#define DecBr	0x0040	/* Decrement and branch */
#define Data03	0x0080	/* TRAP vector in 0-3 */
#define Data07	0x0100	/* Data in 0-7 (MOVEQ) */
#define OpM68D	0x0200	/* Data register in 6-8 */
#define OpM68A	0x0400	/* Address register in 6-8 (ADDA/CMPA/SUBA) */
#define OpM68C	0x0800	/* CMP (Compare) */
#define OpM68X	0x1000	/* EOR (Exclusive or) */
#define OpM68S	0x2000	/* EXT (Sign extension) */
#define OpM68R	0x4000	/* MOVEP (Register/memory) */
#define OpM37	0x8000	/* EXG (Exchange registers) */
#define TwoOpsA	0xDF4D	/* Two operands are required. */

/* AdrModeB bit definitions */
#define Bit811	0x0001	/* Bit operations - bits 8-11 as switch */
#define Size67	0x0002	/* 00 = byte, 01 = word, 10 = long */
#define Size6	0x0004	/* 0 = word, 1 = long */
#define Sz1213A	0x0008	/* 01 = byte, 11 = word, 10 = long */
#define Sz1213	0x0010	/* 11 = word, 10 = long */
#define Exten	0x0020	/* Opcode extension is required. */
#define EA05a	0x0040	/* Effective address - all */
#define EA05b	0x0080	/* All except ARDir */
#define EA05c	0x0100	/* All except ARDIR and Imm */
#define EA05d	0x0200	/* All except PCDisp, PCDisx, and Imm */
#define EA05e	0x0400	/* All except ARDir, PCDisp, PCDisx, and Imm */
#define EA05f	0x0800	/* All except Dreg, ARDir, ARPost, ARPre, Imm */
#define EA05x	0x1000	/* Dual mode - AND/OR */
#define EA05y	0x2000	/* Dual mode - ADD/SUB */
#define EA05z	0x4000	/* Dual mode - MOVEM */
#define EA611	0x8000	/* Eff. Adr. in 6-11 (used only by MOVE) */
#define TwoOpsB	0xF3DD	/* Two operands are required. */
#define ImmMode	0x0422	/* Immediate instructions */
#define SrcPC	0xF8C0	/* Source operand may be PCDisp. */

#define Dummy     0	/* Error codes */
#define AlignErr  1
#define NoCode    2
#define SymDup    3
#define Undef     4
#define ModeErr   5
#define OperErr   6
#define BraErr    7
#define AddrErr   8
#define SizeErr   9
#define EndErr   10
#define AbsReq   11
#define RelErr   12
#define NoIncl   13
#define FwdRef   14
#define NotSFmt  15
#define NeedLab  16
#define Phase    17
#define NoENDM   18
#define NoENDC   19
#define ManyENDC 20
#define DCOflo   21
#define ManySect 22
#define DupMac   23
#define MultLab  24
#define NoStrEnd 25
#define BccSDsp0 26

#define ERRMAX 10	/* Size of error message table */
