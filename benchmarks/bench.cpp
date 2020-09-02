struct SymTab {				/* Symbol table */
    struct SymTab *Link;	/* Link to next entry in hash chain */
    char *Nam;	/* Pointer to symbol */
    long Val;	/* Value */
    long Hunk;	/* Hunk number (ORed with MEMF_CHIP or MEM_FAST
			if applicable SECTION
		   ~(pointer to symbol) if XREF
		   Pointer to macro text if MACRO		*/
    int  Defn;	/* Line number where defined */
    int  Flags;	/* Flags bits:	0 - XREF
				1 - XDEF
				2 - SET
				3 - MACRO (symbol is preceded by blank)
				4 - SECTION (name preceded by 2 blanks
					and 4-digit hex sequence number)
				5 - register name (EQUR)
				6 - register list (REG)
				7 - PUBLIC (XREF or XDEF will be set) */
    //struct Ref *Ref1;	/* Pointer to first reference entry */
};

int HashSize = 4095;
struct SymTab *Hash;



struct SymTab *HashItOld (char *label) 
  /* Returns a pointer to the hash table entry corresponding to "label". */
{
    register unsigned i;
    i = 0;
    while (*label) {
      i = ((i << 3) - i + *label++) % HashSize;
    }
    return (Hash + i);
}

// djb2 source: http://www.cse.yorku.ca/~oz/hash.html
// Done: if we enforce HashSize to be 2^n-1 then we could get away with & instead of %
struct SymTab *HashIt(char *str)
{
	unsigned long hash = 5381;
	int c;

	while ((c = (unsigned char) *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return Hash + (hash & HashSize);
}



// g++ -O3 -march=native  -fno-exceptions -fno-rtti -o bench bench.cpp -I/home/alex/dev/benchmark/include/ -L/home/alex/dev/benchmark/build/src/ -lbenchmark -lpthread

#include <benchmark/benchmark.h>
#include <unistd.h>

void B_HashIt(benchmark::State& state)
{
  //  state.PauseTiming();
  char* foo = new char[state.range(0)+1];
  static int c = 0;

  sprintf(foo,"L%0*d",(int)state.range(0)-1,++c);
  //  printf("%s\n", foo);
  //  usleep(10000);
  for (auto _ : state) {
    // This code gets timed
    benchmark::DoNotOptimize(HashIt(foo));
  }
  
  //state.ResumeTiming();
  //  benchmark::DoNotOptimize(HashIt(foo));
  delete[] foo;
}

BENCHMARK(B_HashIt)->RangeMultiplier(2)->Range(2, 64);

void B_HashItOld(benchmark::State& state)
{
  //  state.PauseTiming();
  char* foo = new char[state.range(0)+1];
  static int c = 0;

  sprintf(foo,"L%0*d",(int)state.range(0)-1,++c);
  //  printf("%s\n", foo);
  //  usleep(10000);
  for (auto _ : state) {
    // This code gets timed
    benchmark::DoNotOptimize(HashItOld(foo));
  }
  
  //state.ResumeTiming();
  //  benchmark::DoNotOptimize(HashIt(foo));
  delete[] foo;
}

BENCHMARK(B_HashItOld)->RangeMultiplier(2)->Range(2, 64);

//BENCHMARK(B_HashIt)->Apply([](benchmark::internal::Benchmark* b){
//  for (int i = 4; i < 32; ++i)
//    b->Arg({ i });
//});

BENCHMARK_MAIN();
