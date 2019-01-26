CC = m68k-amigaos-gcc
CFLAGS=-Wall -fomit-frame-pointer -Os -mcrt=nix13 -fbaserel -msmall-code
LDFLAGS=-s -mcrt=nix13
OBJ=A68kmain.o A68kmisc.o Adirect.o Codegen.o embedlvos.o Opcodes.o Operands.o Symtab.o wb_parse.o

a68k: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(CFLAGS)
	ls -l a68k

clean:
	rm -rf a68k *.o *~
