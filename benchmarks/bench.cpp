// Copyright (C) 2020 by Alex
struct SymTab {
  struct SymTab *Link;  /* Link to next entry in hash chain */
  char *Nam;    /* Pointer to symbol */
  int Val;     /* Value */
  int Hunk;
  int  Defn;    /* Line number where defined */
  int  Flags;
};

int HashSize = 4095;
struct SymTab *Hash;



struct SymTab *HashItOld(char *label) {
  /* Returns a pointer to the hash table entry corresponding to "label". */
  unsigned int i;
  i = 0;
  while (*label) {
    i = ((i << 3) - i + *label++) % HashSize;
  }
  return (Hash + i);
}

// djb2 source: http://www.cse.yorku.ca/~oz/hash.html
// Done: if we enforce HashSize to be 2^n-1
//   then we could get away with & instead of %
struct SymTab *HashIt(char *str) {
  unsigned int hash = 5381;
  int c;

  while ((c = (unsigned char) *str++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return Hash + (hash & HashSize);
}



// g++ -O3 -march=native  -fno-exceptions -fno-rtti -o bench bench.cpp -I/home/alex/dev/benchmark/include/ -L/home/alex/dev/benchmark/build/src/ -lbenchmark -lpthread

#include <unistd.h>
#include "/home/alex/dev/benchmark/include/benchmark/benchmark.h"


void B_HashIt(benchmark::State& state) {
  char* foo = new char[state.range(0)+1];
  static int c = 0;

  snprintf(foo, state.range(0), "L%0*d",
           static_cast<int>(state.range(0)-1), ++c);
  for (auto _ : state) {
    benchmark::DoNotOptimize(HashIt(foo));
  }
  delete[] foo;
}

BENCHMARK(B_HashIt)->RangeMultiplier(2)->Range(2, 64);

void B_HashItOld(benchmark::State& state) {
  char* foo = new char[state.range(0)+1];
  static int c = 0;

  snprintf(foo, state.range(0), "L%0*d",
           static_cast<int>(state.range(0)-1), ++c);
  for (auto _ : state) {
    benchmark::DoNotOptimize(HashItOld(foo));
  }
  delete[] foo;
}

BENCHMARK(B_HashItOld)->RangeMultiplier(2)->Range(2, 64);

// BENCHMARK(B_HashIt)->Apply([](benchmark::internal::Benchmark* b){
//  for (int i = 4; i < 32; ++i)
//    b->Arg({ i });
//});

BENCHMARK_MAIN();
