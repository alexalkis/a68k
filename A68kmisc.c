/*------------------------------------------------------------------*/
/*								    */
/*			MC68000 Cross Assembler			    */
/*								    */
/*		  Copyright 1985 by Brian R. Anderson		    */
/*								    */
/*              Miscellaneous routines - April 16, 1991		    */
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

#include "A68kdef.h"
#include "A68kglb.h"

#define OLDAppendSdata
#ifndef OLDAppendSdata
void xputw(struct fs *f,short data);
#endif

char Sdata[MAXSREC]; /* S-record data */
int Sindex; /* Index for Sdata */
int NumRExt, NumR32, NumR16, NumR8;

static char *errmsg[] ={
		"--- Unknown error code ---",
		"Alignment error.",
		"No such op-code.",
		"Duplicate Symbol.",
		"Undefined Symbol.",
		"Addressing mode not allowed here.",
		"Error in operand format.",
		"Error in relative branch.",
		"Address mode error.",
		"Operand size error.",
		"END statement is missing.",
		"Value must be absolute.",
		"Relocatability error.",
		"INCLUDE file cannot be opened.",
		"Illegal forward reference.",
		"Not supported in S-format.",
		"This instruction needs a label.",
		"Pass 1 / Pass 2 phase error.",
		"ENDM statement is missing.",
		"ENDC statement is missing.",
		"Unmatched ENDC statement.",
		"Too much DC data.",
		"Too many SECTIONs.",
		"Duplicate macro definition.",
		"More than one label on this line.",
		"End of string is missing.",
		"Short displacement can't be zero.",
		""
};

long AddrBndW(long v)
/* Advances "v" to the next word boundary. */
{
	if (v & 1L) {
		AppendSdata(0L, 1);
		v++;
	}
	return (v);
}

long 
AddrBndL (register long v)
/* Advances "v" to the next long-word boundary. */
{
	long templong;

	v = AddrBndW(v); /* Bump to a word boundary first. */
	if (v & 2L) { /* If still not aligned, */
		templong = NOP; /*  generate a NOP. */
		AppendSdata(templong, 2);
		v += 2;
	}
	return (v);
}

void 
WriteListLine (struct fs *f)
/* Writes one line to the Listing file, including Object Code. */
{
	register int i, j, printed;
	long templong;
	char macflag;
	char tempstr[12];

	if (!Pass2)
		return; /* Pass 2 only */
	if (FwdShort && (ErrLim == 0)) {
		DisplayLine();
		printf("A short branch can be used here.\n");
	}
	if (SuppList)
		return; /* Listing is suppresed. */
	if (ErrLim == 0)
		if ((Dir == Page) || (Dir == Space) || (Dir == Title) || (Dir == DoList)
				|| (Dir == NoList) || (ListOff))
			return; /* Don't print unless they have errors. */

	CheckPage(f, FALSE); /* Print headings if necessary. */

	if (PrntAddr) {
		if ((Dir == Equ) || (Dir == Set))
			LongPut(f, ObjSrc, 3); /* Equated value */
		else
			LongPut(f, AddrCnt, 3); /* Current location */
		if (!KeepTabs)
			xputs(f, "  ");
	} else if (!KeepTabs)
		xputs(f, "        "); /* Don't print location. */
	if (KeepTabs)
		xputs(f, "\t"); /* Use tabs for spacing. */
	printed = 8; /* We've printed 8 positions. */
	LongPut(f, ObjOp, nO); /* Generated code */
	printed += nO * 2;
	if (nS != 0) {
		xputs(f, " ");
		LongPut(f, ObjSrc, nS);
		printed += nS * 2 + 1;
	}
	if (nD != 0) {
		xputs(f, " ");
		LongPut(f, ObjDest, nD);
		printed += nD * 2 + 1;
	}
	if ((j = nX) > 0) { /* String data */
		if ((j * 2 + printed) > ObjMAX)
			j = (ObjMAX - printed) / 2;
		for (i = 0; i < j; i++) {
			templong = ObjString[i];
			LongPut(f, templong, 1);
		}
		printed += j * 2;
	}
	while (printed < ObjMAX) {
		if (KeepTabs) {
			xputs(f, "\t");
			printed += 8;
			printed &= ~7;
		} else {
			xputs(f, " ");
			printed++;
		}
	}
	if ((InFNum == 0) || (OuterMac == 0))
		macflag = ' '; /* Open code */
	else if (InFNum > OuterMac)
		macflag = '+'; /* Inner macro */
	else if ((InFNum == OuterMac) && (Dir != MacCall))
		macflag = '+'; /* Outermost macro */
	else
		macflag = ' '; /* We're outside macros. */

	sprintf(tempstr, "  %5d%c", LineCount, macflag);
	xputs(f, tempstr);
	xputs(f, Line);
	xputs(f, "\n");

	if (FwdShort && (ErrLim == 0))
		xputs(f, "A short branch can be used here.\n");

	for (i = 0; i < ErrLim; i++) { /* Write error messages. */
		CheckPage(f, FALSE);
		xputs(f, errmsg[ErrCode[i]]);
		printed = strlen(errmsg[ErrCode[i]]);
		while (printed < ObjMAX + 8) {
			if (KeepTabs) {
				xputs(f, "\t");
				printed += 8;
				printed &= ~7;
			} else {
				xputs(f, " ");
				printed++;
			}
		}
		for (j = 0; j < ErrPos[i]; j++) {
			if (Line[j] == '\t') {
				xputs(f, "\t");
				printed += 8;
				printed &= ~7;
			} else {
				xputs(f, " ");
				printed++;
			}
		}
		xputs(f, "^ "); /* Error flag */
		if (i == 0) {
			if (InF->UPtr == 0)
				xputs(f, InF->NPtr); /* Module name */
			else
				xputs(f, "(user macro)"); /* We're in a user macro. */
			sprintf(tempstr, " line %d", InF->Line);
			xputs(f, tempstr); /* Line number */
		}
		xputs(f, "\n");
	}
}

void 
WriteSymTab (struct fs *f)
/* Lists the symbol table in alphabetical order. */
{
	int printhunk, i;
	char *p;
	char tempstr[MAXLINE];
	long templong;
	register int j, k;
	register struct SymTab **ss1, **ss2, *sym, **sortlim;
	struct Ref *ref;

	if (NumSyms == 0)
		return; /* The symbol table is empty - exit. */

	/* Build a sorted table of pointers to symbol table entries. */

	templong = NumSyms * sizeof(struct SymTab *);
	SymSort = (struct SymTab **) malloc((unsigned) templong);
	if (SymSort == NULL) {
		fprintf(stderr, "Not enough memory for symbol table sort!\n");
		return;
	}
	sortlim = SymSort + NumSyms;
	sym = SymChunk = SymStart;
	sym++;
	SymChLim = (struct SymTab *) ((char *) SymChunk + CHUNKSIZE);
	ss1 = SymSort;
	while (sym) {
		*ss1++ = sym;
		sym = NextSym(sym); /* Try for another symbol table entry. */
	}
	for (i = NumSyms / 2; i > 0; i /= 2) { /* Shell sort */
		for (ss1 = SymSort + i; ss1 < sortlim; ss1++) { /*  (copied   */
			for (ss2 = ss1 - i; ss2 >= SymSort; ss2 -= i) { /*  from K&R) */
				if (strcmp((*ss2)->Nam, (*(ss2 + i))->Nam) <= 0)
					break;
				sym = *ss2;
				*ss2 = *(ss2 + i);
				*(ss2 + i) = sym;
			}
		}
	}

	/* The table is now sorted - print the listing. */

	LnCnt = LnMax; /* Skip to a new page. */
	for (i = 0, ss1 = SymSort; i < NumSyms; i++) {
		sym = *ss1++;
		CheckPage(f, TRUE);

		p = sym->Nam; /* Pointer to symbol */
		if (sym->Flags & 8)
			p++; /* Skip blank preceding macro name. */
		else if (sym->Flags & 0x10)
			p += 6; /* Skip hunk sequence number. */
		sprintf(tempstr, "%-11s ", p); /* Symbol or macro name */
		xputs(f, tempstr);
		if (strlen(p) > 11)  {/* Long symbol - go to new line. */
			if (KeepTabs)
				xputs(f, "\n\t    ");
			else
				xputs(f, "\n            ");
		}
		printhunk = FALSE; /* Assume no hunk no. to print. */
		if (sym->Defn == NODEF)
			xputs(f, "  *** UNDEFINED *** ");
		else if (sym->Flags & 4)
			xputs(f, "  -- SET Symbol --  ");
		else if (sym->Flags & 8) {
			sprintf(tempstr, " +++ MACRO +++ %5d", sym->Defn);
			xputs(f, tempstr);
		} else if (sym->Flags & 0x10) {
			j = (sym->Hunk & 0x3FFF0000L) >> 16;
			if (j == HunkCode)
				xputs(f, "  CODE    ");
			else if (j == HunkData)
				xputs(f, "  DATA    ");
			else
				xputs(f, "  BSS     ");
			printhunk = TRUE;
		} else if (sym->Flags & 0x20) {
			sprintf(tempstr, "      %c%ld  ", (sym->Val & 8L) ? 'A' : 'D',
					sym->Val & 7L);
			xputs(f, tempstr);
			printhunk = TRUE;
		} else {
			LongPut(f, sym->Val, 4); /* Value */
			xputs(f, "  ");
			printhunk = TRUE;
		}
		if (printhunk) {
			j = sym->Hunk & 0x00007FFFL; /* Hunk number */
			if (sym->Flags & 0x60)
				xputs(f, " Reg"); /* Register or list */
			else if (sym->Flags & 1)
				xputs(f, " Ext"); /* External */
			else if (j == ABSHUNK)
				xputs(f, " Abs"); /* Absolute */
			else {
				sprintf(tempstr, "%4d", j); /* Hunk number */
				xputs(f, tempstr);
			}
			sprintf(tempstr, " %5d", sym->Defn); /* Statement number */
			xputs(f, tempstr);
		}
		if (XrefList) {
			xputs(f, "  ");
			if (sym->Ref1 == NULL)
				xputs(f, " *** UNREFERENCED ***");
			else {
				ref = sym->Ref1;
				j = k = 0;
				while (1) {
					if (ref->RefNum[j] == 0)
						break;
					if (k >= 9) {
						xputs(f, "\n"); /* New line */
						if (KeepTabs)
							xputs(f, "\t\t\t\t  "); /* 34 spaces */
						else
							for (k = 0; k < 34; k++)
								xputs(f, " ");
						k = 0;
					}
					sprintf(tempstr, "%5d", ref->RefNum[j]);
					xputs(f, tempstr);
					j++;
					k++;
					if (j < MAXREF)
						continue; /* Get the next slot. */
					if ((ref = ref->NextRef) == 0)
						break; /* End of last entry */
					j = 0; /* Start the next entry. */
				}
			}
		}
		xputs(f, "\n");
	}
	free(SymSort); /* Free the sort work area. */
	SymSort = NULL;
}

void 
CheckPage (struct fs *f, int xhdr)
/* Checks if end of page reached yet -- if so, advances to next page. */
{
	register int printed;
	char tempstr[12];

	LnCnt++;
	if (LnCnt >= LnMax) {
		PgCnt++;
		if (PgCnt > 1)
			xputs(f, "\f"); /* Skip to new page. */
		xputs(f, TTLstring); /* Title */
		printed = strlen(TTLstring);
		while (printed < 56)
			if (KeepTabs) {
				xputs(f, "\t");
				printed += 8;
				printed &= ~7;
			} else {
				xputs(f, " ");
				printed++;
			}
		xputs(f, SourceFN); /* File name */
		if (KeepTabs)
			xputs(f, "\t");
		else
			xputs(f, "        ");
		sprintf(tempstr, "Page %d\n\n", PgCnt); /* Page number */
		xputs(f, tempstr);
		LnCnt = 2;
		if (xhdr) {
			xputs(f, "Symbol       Value    Hunk  Line");
			if (XrefList)
				xputs(f, "   References"); /* Cross-reference */
			xputs(f, "\n\n");
			LnCnt += 2;
		}
	}
}

void 
StartSrec (struct fs *f, char *idntname)
/* Writes object header record. */
{
	register long CheckSum, templong;
	register char *s;

	if (SFormat) {
		xputs(f, "S0");
		templong = strlen(idntname) + 3; /* extra for addr. & checksum */
		LongPut(f, templong, 1);
		CheckSum = templong;

		xputs(f, "0000"); /* Address is 4 digits, all zero, for S0. */

		s = idntname;
		while (*s) {
			templong = toupper(*s);
			LongPut(f, templong, 1);
			CheckSum += templong;
			s++;
		}
		CheckSum = ~CheckSum; /* Complement the checksum. */
		LongPut(f, CheckSum, 1);
		xputs(f, "\n");
	} else {
		templong = HunkUnit;
		xputl(f, templong);
		DumpName(f, idntname, 0L);
	}
	StartAddr = TempAddr = Sindex = 0;
	NumRExt = NumR32 = NumR16 = NumR8 = 0;
}

void 
WriteSrecLine (struct fs *f)
/* Transfers object code components to output buffer. */
/* Moves long words or portions thereof. */
{
	register int i, j;
	register long templong;

	if (HunkType == HunkBSS)
		return; /* No code in BSS hunk! */

	if (nO + nS + nD + nX) { /* If we have object code... */
		AppendSdata(ObjOp, nO); /* Opcode */
		AppendSdata(ObjSrc, nS); /* Source */
		AppendSdata(ObjDest, nD); /* Destination */
		for (i = 0; i < DupFact; i++) {
			for (j = 0; j < nX; j++) { /* String data */
				templong = ObjString[j];
				AppendSdata(templong, 1);
			}
		}
	}
}

void 
AppendSdata (register long Data, int n)
/* If we are producing S-format records:
 Transfers "n" low-order bytes from "Data" to the output buffer.
 If the buffer becomes full, DumpSdata will be called to flush it.
 S-records will also be broken on 16-byte boundaries.
 If we are producing AmigaDOS format, data will be written
 directly to Srec - we'll go back and fill in the hunk length
 at the end of the hunk.  DumpSdata will never be called from here. */
{
	register int i;
	register char byte;


	if (!Pass2)
		return; /* Pass 2 only */
	if (HunkType == HunkBSS)
		return; /* No data in BSS hunks! */

	if (HunkType == HunkNone) { /* We're not in a hunk yet - */
		DoSection("", 0, "", 0, "", 0); /* start a code hunk. */
		MakeHunk = TRUE;
	}

	if (OrgFlag) { /* If we've had an ORG directive */
		FixOrg(); /*  do necessary adjustments     */
		OrgFlag = FALSE; /*  to the object code file.     */
	}


#ifndef OLDAppendSdata
	if (!n)
		return;
	switch(n) {
	case 1:
		xputc(Data,&Srec);
		++TempAddr;
		break;
	case 2:
		xputw(&Srec,Data);
		TempAddr+=2;
		break;
	case 4:
		xputl(&Srec,Data);
		TempAddr+=4;
		break;
	default:
		printf("uh oh!!!\n");
		break;
	}
#else
//	static c[5];
//	printf("n=%d data=%d (%d)\n",n,Data,++c[n]);
	Data <<= (4 - n) * 8; /* Left-justify data. */

	for (i = 0; i < n; i++) {
		byte = (char) (Data >> ((3 - i) * 8));
		TempAddr++;
		if (!SFormat) {
			xputc(&Srec,byte);
		} else {
			Sdata[Sindex++] = byte;
			if (((TempAddr & 0x0F) == 0) || (Sindex >= MAXSREC))
				DumpSdata(&Srec); /* Break S-record. */
		}
	}
#endif
}

void FixOrg(void)
/* Makes necessary adjustments to the object code file if an
 ORG directive has been processed.  This routine is called
 exclusively by AppendSdata and must only be called once for
 each ORG encountered, when writing the next object code (if any). */
{
	register long templong;
	register int i;

	if (SFormat && (AddrCnt != TempAddr)) { /* ORG in S-format -    */
		DumpSdata(&Srec); /*  dump current record */
		StartAddr = TempAddr = AddrCnt; /*  and start afresh.   */
	}
	if (AddrCnt < TempAddr) { /* AmigaDOS backward ORG */
		if (TempAddr > OrgHigh) {
			LenPtr = NULL;
			OrgHigh = TempAddr; /* Save high address for return. */
			xwrite(&Srec); /* Flush the buffer. */
#ifdef USEAMIGAOS
			OrgSeek = Seek(Srec.fd, 0L, OFFSET_CURRENT); /* Remember position. */
			Seek(Srec.fd, (AddrCnt & ~3L) - TempAddr, OFFSET_CURRENT); /* New position */
#else
			OrgSeek = lseek(Srec.fd, 0L, 1); /* Remember position. */
			lseek(Srec.fd, (AddrCnt & ~3L) - TempAddr, 1); /* New position */
#endif
			if (AddrCnt & 3L) { /* If ORG isn't to long-word   */
#ifdef USEAMIGAOS
				Read(Srec.fd, Srec.Buf, AddrCnt & 3L); /*  move ahead */
				Seek(Srec.fd, -(AddrCnt & 3L), OFFSET_CURRENT); /*  to keep    */
#else
				read(Srec.fd, Srec.Buf, AddrCnt & 3L); /*  move ahead */
				lseek(Srec.fd, -(AddrCnt & 3L), 1); /*  to keep    */
#endif
			} /*  the buffer */
			Srec.Ptr = Srec.Buf + (AddrCnt & 3L); /*  aligned.   */
		}
		StartAddr = TempAddr = AddrCnt;

	} else if (AddrCnt > TempAddr) { /* AmigaDOS forward ORG */
		if (OrgHigh > TempAddr) { /* Previous backward ORG */
			if (AddrCnt < OrgHigh)
				templong = AddrCnt; /* Within previous range */
			else
				templong = OrgHigh; /* Beyond previous range */
			i = (int) (templong & 3L); /* Alignment factor */
			templong -= TempAddr; /* Number of bytes to skip */
			LenPtr = NULL;
			xwrite(&Srec); /* Flush the buffer. */
#ifdef USEAMIGAOS
			Seek(Srec.fd, templong - (long) i, OFFSET_CURRENT); /* Skip written data. */
			if (i) { /* If skip isn't to long-word, */
				Read(Srec.fd, Srec.Buf, (long) i); /*  move ahead */
				Seek(Srec.fd, -((long) i), OFFSET_CURRENT); /*  to keep    */
			} /*  the buffer */
#else
			lseek(Srec.fd, templong - (long) i, 1); /* Skip written data. */
			if (i) { /* If skip isn't to long-word, */
				read(Srec.fd, Srec.Buf, (long) i); /*  move ahead */
				lseek(Srec.fd, -((long) i), 1); /*  to keep    */
			} /*  the buffer */
#endif
			Srec.Ptr = Srec.Buf + (long) i; /*  aligned.   */
			TempAddr += templong;
			StartAddr = TempAddr;
		}
		while (TempAddr < AddrCnt) { /* Extend with binary zeros. */
			xputc(&Srec,0);
			TempAddr++;
		}
	}
}

void DumpSdata(struct fs *f)
/* Writes an object code record. */
{
	register long CheckSum, templong;
	register char *s;
	register int i;

	if (!SFormat) {
		if (AddrCnt < OrgHigh) { /* If we did a backwards ORG, */
			LenPtr = NULL; /*  we have to fix things up. */
			xwrite(&Srec); /* Flush the buffer. */
#ifdef USEAMIGAOS
			Seek(f->fd, OrgSeek & ~3L, OFFSET_BEGINING); /* Back to high position */
			if (OrgSeek & 3L) { /* If ORG isn't to long-word,  */
				Read(f->fd, f->Buf, OrgSeek & 3L); /*  move ahead */
				Seek(f->fd, -(OrgSeek & 3L), OFFSET_CURRENT); /*  to keep    */
			} /*  the buffer */
#else
			lseek(f->fd, OrgSeek & ~3L, 0); /* Back to high position */
			if (OrgSeek & 3L) { /* If ORG isn't to long-word,  */
				read(f->fd, f->Buf, OrgSeek & 3L); /*  move ahead */
				lseek(f->fd, -(OrgSeek & 3L), 1); /*  to keep    */
			} /*  the buffer */
#endif
			f->Ptr = f->Buf + (OrgSeek & 3L); /*  aligned.   */
			AddrCnt = StartAddr = TempAddr = OrgHigh;
		}
		AddrCnt = AddrBndL(AddrCnt); /* Finish the last long word. */
		templong = (AddrCnt - SectStart) >> 2;
		if ((s = LenPtr) == NULL) {
			xwrite(&Srec); /* Flush the buffer. */
#ifdef USEAMIGAOS
			CheckSum = Seek(f->fd, 0L, OFFSET_CURRENT); /* Remember position. */
			Seek(f->fd, LenPos, OFFSET_BEGINING); /* Put hunk length here. */
#else
			CheckSum = lseek(f->fd, 0L, 1); /* Remember position. */
			lseek(f->fd, LenPos, 0); /* Put hunk length here. */
#endif
			s = f->Buf;
		}
		*s++ = (char) (templong >> 24);
		*s++ = (char) (templong >> 16);
		*s++ = (char) (templong >> 8);
		*s++ = (char) templong;
		if (LenPtr == NULL) {
			f->Ptr = f->Buf + 4;
			xwrite(f);
#ifdef USEAMIGAOS
			Seek(f->fd, CheckSum, OFFSET_BEGINING); /* Go back to where we were. */
#else
			lseek(f->fd, CheckSum, 0); /* Go back to where we were. */
#endif
			f->Ptr = f->Buf;
		}
		DumpRel(f); /* Write relocation information. */
		templong = HunkEnd;
		xputl(f, templong); /* End of the hunk */
		TempAddr = AddrCnt;
		return;
	}

	if (Sindex == 0)
		return; /* There's nothing to dump. */

	xputs(f, "S2");
	templong = Sindex + 4; /* Record length */
	LongPut(f, templong, 1);
	CheckSum = templong; /* Initialize CheckSum. */

	LongPut(f, StartAddr, 3); /* Address */
	CheckSum += (StartAddr >> 16) & 0x00FFL;
	CheckSum += (StartAddr >> 8) & 0x00FFL;
	CheckSum += StartAddr & 0x00FFL;

	for (i = 0; i < Sindex; i++) {
		templong = Sdata[i];
		LongPut(f, templong, 1); /* Object code */
		CheckSum += templong;
	}
	CheckSum = ~CheckSum; /* Complement the checksum. */
	LongPut(f, CheckSum, 1);
	xputc(f,'\n');//xputs(f, "\n");

	StartAddr += Sindex;
	TempAddr = StartAddr;
	Sindex = 0;
}

void PutRel(long addr, long hunk, int size)
/* Build a relocation entry if necessary. */
{
	register struct RelTab *rel;

	if (!Pass2)
		return; /* Pass 2 only */
	if (SFormat)
		return; /* Not for S-format! */
	if (hunk == ABSHUNK)
		return; /* Absolute */
	if (HunkType == HunkBSS)
		return; /* Not for BSS hunks! */

	rel = RelLim; /* Pointer to new entry. */
	RelLim++; /* Bump limit pointer. */
	if (((char *) RelLim - (char *) RelCurr) > CHUNKSIZE) {
		rel = (struct RelTab *) malloc((unsigned) CHUNKSIZE);
		if (rel == NULL)
			quit_cleanup("Out of memory!\n");
		RelCurr->Link = rel; /* Link from previous chunk */
		RelCurr = rel; /* Make the new chunk current. */
		RelCurr->Link = NULL; /* Clear forward pointer. */
		rel++; /* Skip over pointer entry. */
		RelLim = rel; /* New table limit */
		RelLim++; /* Bump it. */
	}
	if (RelLast != NULL)
		RelLast->Link = rel; /* Link from previous entry */
	rel->Link = NULL; /* End of the chain (so far) */
	rel->Offset = addr; /* Offset */
	rel->Hunk = hunk; /* Hunk number */
	rel->Size = size; /* Size */
	RelLast = rel; /* Pointer to last entry in chain */

	if (hunk < 0) /* Count entries by type. */
		NumRExt++;
	else if (size == Long)
		NumR32++;
	else if (size == Word)
		NumR16++;
	else
		NumR8++;
}

void DumpRel(struct fs *f)
/* Dump relocation information to the object file. */
{
	register struct SymTab *sym;
	register struct RelTab *rel, *rel2;
	int j, size, num, donexhdr, secthlin;
	long currhunk, nexthunk, templong;
	char *s, *t;

	secthlin = LineCount; /* Current section ends here */
	if ((Dir == Section) /*   unless we're starting   */
	|| (Dir == CSeg) /*      a new section.       */
	|| (Dir == DSeg) || (Dir == BSS))
		secthlin--; /* Then it ends at previous line. */

	if (SFormat)
		return; /* S-format is absolute! */

	while (1) {
		if ((num = NumR32) != 0) {
			size = Long; /* Do 32-bit fields */
			templong = HunkR32;
			NumR32 = 0; /* ...but only once. */
		} else if ((num = NumR16) != 0) {
			size = Word; /* Then do 16-bit fields. */
			templong = HunkR16;
			NumR16 = 0;
		} else if ((num = NumR8) != 0) {
			size = Byte; /* Finally do 8-bit fields. */
			templong = HunkR8;
			NumR8 = 0;
		} else
			break; /* We're all done. */

		xputl(f, templong); /* Record type */

		currhunk = 32767;
		num = 0;
		if ((rel = RelStart)) /* If we have anything, */
			rel++; /*  skip over the first chunk's link. */
		while (rel) {
			if ((rel->Size == size) && (rel->Hunk >= 0)) {
				if (rel->Hunk < currhunk) {
					currhunk = rel->Hunk; /* Lowest hunk number */
					num = 1; /* Reset counter. */
				} else if (rel->Hunk == currhunk) {
					num++; /* Count entries. */
				}
			}
			rel = rel->Link;
		}
		while (num > 0) { /* Repeat for all hunk references. */
			templong = num;
			xputl(f, templong); /* Number of entries */
			xputl(f, currhunk); /* Hunk number */
			nexthunk = 32767;
			num = 0; /* Count for next hunk */
			if ((rel = RelStart))
				rel++;
			while (rel) {
				if ((rel->Size == size) && (rel->Hunk >= 0)) {
					if (rel->Hunk < currhunk) {
						rel = rel->Link; /* We already wrote it. */
						continue;
					} else if (rel->Hunk == currhunk) {
						xputl(f, rel->Offset - SectStart);
					} else if (rel->Hunk < nexthunk) {
						nexthunk = rel->Hunk; /* Next hunk number */
						num = 1; /* Reset counter. */
					} else if (rel->Hunk == nexthunk) {
						num++; /* Count entries. */
					}
				}
				rel = rel->Link;
			}
			currhunk = nexthunk; /* Get ready for next hunk. */
		}
		xputl(f, 0L); /* End of relocation information */
	}

	donexhdr = FALSE; /* We haven't written hunk_ext yet. */

	sym = SymChunk = SymStart;
	sym++;
	SymChLim = (struct SymTab *) ((char *) SymChunk + CHUNKSIZE);
	while (sym) {
		if (sym->Flags & 2) { /* Scan for XDEF symbols. */
			j = sym->Defn; /* Defined in current section? */
			if ((j >= SectLine) && (j <= secthlin)) {
				if (!donexhdr) {
					templong = HunkExt; /* We haven't done header yet. */
					xputl(f, templong);
					donexhdr = TRUE;
				}
				if ((sym->Hunk & 0x0000FFFFL) == ABSHUNK)
					templong = 0x02000000;
				else
					templong = 0x01000000; /* Flags */
				DumpName(f, sym->Nam, templong); /* Symbol */
				xputl(f, sym->Val - SectStart); /* Offset */
			}
		}
		sym = NextSym(sym);
	}

	if (NumRExt != 0) { /* External references (XREF) */
		if (!donexhdr) {
			templong = HunkExt; /* We haven't done header yet. */
			xputl(f, templong);
			donexhdr = TRUE;
		}
		if ((rel = RelStart))
			rel++;
		while (rel) {
			if (rel->Hunk < 0) {
				size = rel->Size;
				if (size == Long)
					templong = 0x81000000L; /* ext_ref32 */
				else if (size == Word)
					templong = 0x83000000L; /* ext_ref16 */
				else
					templong = 0x84000000L; /* ext_ref8 */
				s = (char *) ~(rel->Hunk);
				DumpName(f, s, templong); /* Flags and symbol */
				templong = 1;
				rel2 = rel->Link;
				while (rel2) {
					if ((rel2->Hunk == rel->Hunk) && (rel2->Size == size))
						templong++; /* Number of times */
					rel2 = rel2->Link; /*  symbol occurs  */
				}
				xputl(f, templong);
				rel2 = rel; /* Now go back and  */
				while (rel2) { /*  write them out. */
					if ((rel2->Hunk == rel->Hunk) && (rel2->Size == size)) {
						xputl(f, rel2->Offset - SectStart); /* Offset */
						if (rel2 != rel) /* Kill hunk so we    */
							rel2->Hunk = 0; /*  don't do it again */
					} /*  (we're done with  */
					rel2 = rel2->Link; /*  the table anyway). */
				}
			}
			rel = rel->Link;
		}
		NumRExt = 0;
	}
	if (donexhdr)
		xputl(f, 0L); /* End of external information */

	if (DumpSym) { /* Dump the symbol table. */
		donexhdr = FALSE;
		sym = SymChunk = SymStart;
		sym++;
		SymChLim = (struct SymTab *) ((char *) SymChunk + CHUNKSIZE);
		for (; sym; sym = NextSym(sym)) {
			s = DumpSymList;
			if (*s) { /* Select by prefix. */
				if (*s == '!')
					s++; /* Symbols must NOT match prefix. */
				t = sym->Nam;
				j = TRUE; /* Assume we have a match. */
				while (*s) {
					if (*s++ != *t++) {
						j = FALSE; /* Symbol does not match prefix. */
						break;
					}
				}
				if (DumpSymList[0] == '!') {
					if (j)
						continue;
				} else {
					if (!j)
						continue;
				}
			}
			if ((sym->Hunk & 0x0000FFFFL) == CurrHunk) {
				j = sym->Flags & 0x7F; /* Ignore PUBLIC flag. */
				if ((j == 0) || (j == 2)) { /* Defined, may be XDEF. */
					if ((sym->Defn >= SectLine) && (sym->Defn <= secthlin)) {
						if (!donexhdr) { /* It's in current SECTION. */
							templong = HunkSym;
							xputl(f, templong); /* Write header */
							donexhdr = TRUE; /* if necessary. */
						}
						DumpName(f, sym->Nam, 0L); /* Symbol */
						xputl(f, sym->Val - SectStart); /* Offset */
					}
				}
			}
		}
		if (donexhdr)
			xputl(f, 0L); /* End of symbol table dump */
	}

	rel = RelStart->Link;
	while (rel != NULL) {
		rel2 = rel;
		rel = rel2->Link;
		free(rel2); /* Free all but the first chunk. */
	}
	RelCurr = RelStart; /* The first chunk is current. */
	RelCurr->Link = NULL; /* Unlink additional chunks. */
	RelLast = NULL; /* There are no entries left. */
	RelLim = RelStart;
	RelLim++; /* First unused space */
}

void EndSdata(struct fs *f, long addr)
/* Write end record to object file. */
{
	register long checksum;

	if (SFormat) {
		DumpSdata(f); /* Write any remaining data. */
		xputs(f, "S804"); /* Record header */
		checksum = 4;
		LongPut(f, addr, 3); /* Transfer address */
		checksum += (addr >> 16) & 0x00FFL;
		checksum += (addr >> 8) & 0x00FFL;
		checksum += addr & 0x00FFL;
		checksum = ~checksum;
		LongPut(f, checksum, 1); /* Checksum */
		xputs(f, "\n");
	} else {
		if (HunkType != HunkNone) {
			DumpSdata(f); /* Last hunk's data */
		}
	}
}

void DumpName(struct fs *f,char *name,long flags)
/* Writes a name preceded by a long word containing the
 length of the name in long words.  The length word has
 the contents of "flags" ORed into it.  The name is padded
 with binary zeros to the next long word boundary. */
{
	register int i;
	register long templong;

	i = strlen(name);
	templong = (i + 3) >> 2; /* Length of name (long words) */
	templong |= flags; /* Add flag bits. */
	xputl(f, templong); /* Write length and flags. */
	xputs(f, name); /* Write the name itself. */
	while (i & 3) {
		xputc(f,'\0'); /* Pad the last word. */
		i++;
	}
}

void LongPut(struct fs *f, long data, int length)
/* Writes to file "f" the hexadecimal interpretation of
 the bytes in "data".  The number of bytes written
 (two hex digits per byte) is given in "length" -
 if less than 4, only low-order bytes are written. */
{
	register int i, j;
	register char *t;
	char xstr[9];

	t = xstr;
	for (i = length * 8 - 4; i >= 0; i -= 4) {
		j = (int) ((data >> i) & 0x0FL);
		*t++ = (char) ((j > 9) ? (j - 10 + 'A') : (j + '0'));
	}
	*t = '\0';
	xputs(f, xstr);
}

int xopen(char *name, struct fs *f, char *desc)
/* Opens the output file whose name is in "name",
 setting up the file structure pointed to by "f".
 This routine first allocates a file buffer -
 if unsuccessful, it calls quit_cleanup.
 Otherwise, it opens the file - if unsuccessful,
 displays an error message using "desc" and returns TRUE.
 If the file is successfully opened, this routine returns FALSE. */
{
	if ((f->Buf = (char *) malloc(BUFFSIZE)) == NULL)
		quit_cleanup("Out of memory!\n");
#ifdef USEAMIGAOS
	if ((f->fd = Open(name, MODE_NEWFILE)) == 0) {
#else
	if ((f->fd = creat(name, 0644)) < 0) {
#endif
		fprintf(stderr, "Unable to open %s file.\n", desc);
		f->fd = 0;
		return (TRUE);
	}
	f->Ptr = f->Buf;
	f->Lim = f->Buf + BUFFSIZE;
	return (FALSE);
}

void xputs(struct fs *f, char *s)
/* Writes the string pointed to by "s"
 to the output file whose structure is pointed to by "f". */
{
	register char *t, *l;

//	static int c = 0;
//	printf("string %d times...(%s) Len: %d\n",++c,s, strlen(s));

	t = f->Ptr; /* Current position (use registers for speed) */
	l = f->Lim; /* End of buffer */

	while (*s) {
		*t++ = *s++;
		if (t >= l) {
			f->Ptr = t;
			xwrite(f); /* Flush the buffer. */
			t = f->Buf; /* Reset pointer. */
		}
	}
	f->Ptr = t; /* Update pointer. */
}

#ifndef OLDAppendSdata
void xputw(struct fs *f,short data)
/* Writes to file "f" the contents of the short word in "data". */
{
#ifdef USEAMIGAOS
    if ((f->Ptr+2)>=f->Lim) {
    	xwrite(f);
    	f->Ptr=f->Buf;
    }
    *((short *)f->Ptr)=data; /* this needs big-endian machine which amiga is */
    f->Ptr+=2;				/* and we can save 2 calls to xputc...			*/
#else
	xputc((char) (data >> 8), f);
	xputc((char) data, f);
#endif
}
#endif

void xputl(struct fs *f, long data)
/* Writes to file "f" the contents of the long word in "data". */
{
#ifdef USEAMIGAOS
    if ((f->Ptr+4)>=f->Lim) {
    	xwrite(f);
    	f->Ptr=f->Buf;
    }
    *((long *)f->Ptr)=data; /* this needs big-endian machine which amiga is */
    f->Ptr+=4;				/* and we can save 4 calls to xputc...			*/
#else
	xputc(f,(char) (data >> 24));
	xputc(f,(char) (data >> 16));
	xputc(f,(char) (data >> 8));
	xputc(f,(char) data);
#endif
}

#define NEWXPUTC
#ifdef NEWXPUTC
void xputc(struct fs *f, char byte)
/* Writes the byte contained in "byte" to file "f". */
{
	*f->Ptr++ = byte;
	if (f->Ptr >= f->Lim) {
		xwrite(f); /* Flush the buffer. */
		f->Ptr = f->Buf; /* Reset pointer. */
	}
}

#else
void xputc(struct fs *f, char byte)
/* Writes the byte contained in "byte" to file "f". */
{
	register char *t;

	t = f->Ptr; /* Current position (use a register for speed) */

	*t++ = byte;
	if (t >= f->Lim) {
		f->Ptr = t;
		xwrite(f); /* Flush the buffer. */
		t = f->Buf; /* Reset pointer. */
	}
	f->Ptr = t; /* Update pointer. */
}
#endif

void xclose(struct fs *f)
/* Closes the output file whose structure is pointed to by "f".
 The buffer is flushed if necessary, then freed.		*/
{
	xwrite(f); /* Flush the buffer. */
#ifdef USEAMIGAOS
	Close(f->fd);
#else
	close(f->fd); /* Close the file. */
#endif
	f->fd = 0;
	free(f->Buf); /* Free the buffer. */
	f->Buf = NULL;
}

void xwrite(struct fs *f)
/* General write routine */
{
	int len;

	len = (int) (f->Ptr - f->Buf); /* Number of bytes to write */
	if (len > 0) {
#ifdef USEAMIGAOS
		if (Write(f->fd, f->Buf, len) != len) { /* Write the buffer. */
#else
		if (write(f->fd, f->Buf, len) != len) { /* Write the buffer. */
#endif
			f->Ptr = f->Buf; /* Avoid loop in xclose()! */
			if (f == &List)
				quit_cleanup("\nListing file write error!\n");
			else if (f == &Eq)
				quit_cleanup("\nEquate file write error!\n");
			else
				quit_cleanup("\nObject file write error!\n");
		} else {
			if (f == &Srec)
			   LenPtr = NULL; /* Hunk length is no longer in buffer. */
		}
	}
}

void Error(int pos, int errornum)
/* Displays error message #errornum.  If this is the first error for
 the current line, the line itself is displayed, preceded by a
 message giving the current position in the current module.
 If the line is in a macro or include file, the position in
 each nested module is given, working out to the source file.
 A flag is placed under the column indicated by "pos".       */
{
	int i;

	if (!Pass2 && (errornum != NoIncl)) {
		if (IncStart != 0) { /* Don't skip this     */
			IncStart = 0; /*  INCLUDE file in    */
			if (SkipLim->Set1 != NULL) { /*  pass 2 - we must   */
				SetFixLim = SkipLim->Set1; /*  re-read it to      */
				SetFixLim++; /*  report its errors. */
			}
		}
		return; /* Report during pass 2 only. */
	}
	if (ErrLim < ERRMAX) { /* Save error data. */
		ErrCode[ErrLim] = errornum;
		ErrPos[ErrLim] = pos;
		ErrLim++;
	}
	if (ErrLim == 1) /* If this is the first error for this line, */
		DisplayLine(); /*  display the line and its number(s). */


	printf ("\t");
	for (i = 0; i < pos; i++)
		if (Line[i] == '\t')
			printf ("\t");
		else
			printf(" ");		/* Space over to error column. */
	printf("^ %s\n", errmsg[errornum]); /* Error flag and message */
	ErrorCount++; /* Count errors. */
}

void DisplayLine(void)
/* Displays the current line and its position
 in all current files - used by Error, etc. */
{
	struct InFCtl *inf;
	int i;

	printf ("\n");
	for (i = InFNum, inf = InF; i >= 0; i--, inf++) { /* Nested? */
		if (inf->UPtr == 0)
			printf("%s", inf->NPtr); /* Module name */
		else
			printf("(user macro)"); /* We're in a user macro. */
		printf(" line %d\n", inf->Line); /* Line number in module */
	}
	printf("%5d   %s\n", LineCount, Line); /* The line itself */
}
