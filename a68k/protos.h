/* Prototype definitions for A68k - last modified April 16, 1991 */


/* Prototypes for functions defined in A68kmain.c */
int main(int argc, char **argv);
int getfilename(char *name,char *arg,char *desc,int needit);
int checkswitch(char *sw,char *name);
void defaultfile(char *name,char *ext);
int checkdupfile(char *name1,char *desc1,char *name2,char *desc2);
void startpass(char pchar,long maxheap2);
void quit_cleanup(char *s);
/* Prototypes for functions defined in Adirect.c */
int ObjDir(void);
void DoSection(char *name,int nameloc,char *type,int typeloc,char *flags,int flagloc);
/* Prototypes for functions defined in Codegen.c */
void GetObjectCode(void);
void PackFwdBranch(void);
/* Prototypes for functions defined in Opcodes.c */
int Instructions(int loc);
/* Prototypes for functions defined in Operands.c */
int GetArgs(char *name);
void EffAdr(struct OpConfig *EA,int Bad);
void OperExt(struct OpConfig *EA);
void GetOperand(char *oper,struct OpConfig *op,int pcconv);
int GetMultReg(char *oper,int loc);
int GetAReg(char *op,int len,int loc);
int IsRegister(char *op,int len);
int GetInstModeSize(int Mode);
/* Prototypes for functions defined in Symtab.c */
int OpenIncl(char *name,char *dirlist);
int LineParts(void);
void GetMacLine(void);
int GetLine(void);
void SubArgs(void);
void GetParts(void);
void ShowFile(int newline);
void ShowLine(int i);
char *GetField(char *s,char *d);
long GetValue(char *operand,int loc);
void CondCalc(int newprec);
int IsOperator(char *o);
long CalcValue(char *operand,int loc);
void AddSymTab(char *label,long value,long hunk,int line,int flags);
char *AddName(char *name,int macflag);
int ReadSymTab(char *label);
struct SymTab **HashIt(char *label);
struct SymTab *NextSym(struct SymTab *sym);
void AddRef(int linenum);
int CountNest(char *s);
void Heap2Space(int n);
void ParseSpace(int n);
/* Prototypes for functions defined in A68kmisc.c */
long AddrBndW(long v);
long AddrBndL(long v);
void WriteListLine(struct fs *f);
void WriteSymTab(struct fs *f);
void CheckPage(struct fs *f,int xhdr);
void StartSrec(struct fs *f,char *idntname);
void WriteSrecLine(struct fs *f);
//void AppendSdata(long Data,int n, ...);
void AppendSdata(long Data,int n);
void FixOrg(void);
void DumpSdata(struct fs *f);
void PutRel(long addr,long hunk,int size);
void DumpRel(struct fs *f);
void EndSdata(struct fs *f,long addr);
void DumpName(struct fs *f,char *name,long flags);
void LongPut(struct fs *f,long data,int length);
int xopen(char *name,struct fs *f,char *desc);
void xputs(struct fs *f,char *s);
void xputl(struct fs *f,long data);
void xputc(char byte, struct fs *f);
void xclose(struct fs *f);
void xwrite(struct fs *f);
void Error(int pos,int errornum);
void DisplayLine(void);


/* Prototypes for functions defined in wb_parse.c */

//void _wb_parse __PROTO((void));
