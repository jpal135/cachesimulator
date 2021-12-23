# cachesimulator
C program that simulates the behavior of a cache memory. 
Allows the user to specify the parameters of the cache, will read in a trace file that specifies the sequence of memory addresses accessed, simulates the cache, then prints out stats about the cache (e.g. number of misses).
For this project I wrote a cache simulator in the csim.c file that takes a Valgrind memory trace as input, simulates the hit/miss behavior of a cache memory on this trace, and outputs the total number of hits, misses, and evictions.

