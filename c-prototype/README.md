# Proof-of-Concept in C

The repository contains a proof-of-concept implementation of our DecoyAuth protocol. Note that this is currently only a proof-of-concept, we are still analyzing and verifying security properties, and seeing where the protocol can be optimized. _Do not use this code in production._ **Help on analyzing the security of this design, or other feedback, is welcome!**

## 1. Compilation

First compile the code and create the appropriate data files:

```
cd c-prototype
cmake CMakeLists.txt
cd src
make

# This generates the data files that will be used in the benchmark
./generate.sh

# Compile the benchmark
cd ../bench
make
```


# 2. Benchmark

After running the steps in [Compilation](#1-Compilation) you can run the benchmarks:

```
./MultiPassWPA3_bench --benchmark_min_time=1s
```


## 3. Example Output

Example output of the benchmark on my laptop, a HP ZBook Power G8, is shown below:

```
$ ./MultiPassWPA3_bench --benchmark_min_time=10s
[..]
-------------------------------------------------------------
Benchmark                   Time             CPU   Iterations
-------------------------------------------------------------
BM_encoding            380537 ns       379921 ns        37884
BM_decoding            100838 ns       100734 ns       138847
BM_precompute/3         66090 ns        66033 ns       211635
BM_precompute/10       831614 ns       830546 ns        17255
BM_precompute/20      3158636 ns      3154907 ns         4474
BM_precompute/30      7144717 ns      7135761 ns         1990
BM_precompute/40     12320683 ns     12304475 ns         1121
BM_precompute/50     19249801 ns     19224834 ns          724
BM_precompute/100    75232883 ns     75157253 ns          183
BM_precompute/200   306269876 ns    305918800 ns           46
BM_precompute/500  1894198325 ns   1892140426 ns            7
BM_precompute/1000 8233069212 ns   8205861175 ns            2
BM_weave/3               2925 ns         2929 ns      4823042
BM_weave/10             27703 ns        27685 ns       486969
BM_weave/20            109198 ns       109075 ns       129225
BM_weave/30            245700 ns       245429 ns        58408
BM_weave/40            429396 ns       428974 ns        32480
BM_weave/50            674779 ns       673920 ns        19975
BM_weave/100          2689594 ns      2685601 ns         5078
BM_weave/200         10693650 ns     10679224 ns         1246
BM_weave/500         68001436 ns     67892379 ns          205
BM_weave/1000       266698493 ns    266349142 ns           52
BM_evaluate/3             845 ns          847 ns     16973277
BM_evaluate/10           2802 ns         2806 ns      4372524
BM_evaluate/20           5414 ns         5416 ns      2560306
BM_evaluate/30           8074 ns         8073 ns      1746775
BM_evaluate/40          10693 ns        10682 ns      1321315
BM_evaluate/50          13368 ns        13357 ns      1054179
BM_evaluate/100         26521 ns        26502 ns       519979
BM_evaluate/200         52946 ns        52901 ns       267012
BM_evaluate/500        136054 ns       135953 ns       107142
BM_evaluate/1000       265693 ns       265444 ns        53349
```

Correspondence to the descriptions in the [white paper](../docs/whitepaper.pdf) are as follows:

- `BM_encoding`: corresponds to Algorithm 1, and is implemented in [encode.c:es_encode](src/encode.c#L297).

- `BM_decoding`: corresponds to Algorithm 2 and 3, and is implemented in [encode.c:es_decode](src/encode.c#L351).

- `BM_weave`: corresponds to the _encode_ function in Figure 1 and Section 3.1, and is implemented in [weaver.c:weave](src/weaver.c#L105).

- `BM_evaluate`: corresponds to the _decode_ function in Figure 1 and Section 3.2, and is implemented in [weaver.c:evaluate](src/weaver.c#134).

- `BM_precompute`: this is an implementation optimization related to `BM_weave`. It enables to reuse of intermediate values of the configuration of the DecoyAuth protocol stays the same. It is implemented in [weaver.c:precompute](src/weaver.c#L3).


## Acknowledgments

Brecht Van de Sijpe wrote the initial version of this code as part of his master's thesis.
