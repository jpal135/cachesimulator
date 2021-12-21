/*Authors: Julia Paley and Tyler Kreider
 * Date: 11/15/2021
 * 
 * csim.c
 *This program is a simulation of a set associative cache and allows the user to establish its size. The user can customize the number of sets and lines per set.It then reads in a txt file that consists of an operation field, address (in hex format, and it's size on each line. After interpreting the file and simulating the cache, it will display the number of hits, misses, and evicts depending on the file and cache type. 
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include "cachelab.h"


typedef unsigned long int mem_addr;

struct Line {
	bool valid;
	mem_addr tag;
	int time_stamp;

};
typedef struct Line Line;

// forward declaration (add your own forward declarations after this point)
void simulateCache(char *trace_file, int e, int s, int b, bool verbose);
int handle_access(Line **cache, mem_addr address, int lines_per_set, int set_bits, int offset, int LRU);

/**
 * Prints out a reminder of how to run the program.
 *
 * @param executable_name Strign containing the name of the executable.
 */

void usage(char *executable_name) {
	printf("Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>", executable_name);
}

int main(int argc, char *argv[]) {

	int set_bits = 1; // default value for s is 1
	int block_offset_bits = 1; // default value for b is 1
	int lines_per_set = 1; // default value for E is 1 (i.e. direct mapped)

	bool verbose_mode = false; // default is for verbose mode to be disabled

	char *trace_filename = NULL;

	opterr = 0;

	int c = -1;

	// Note: adding a colon after the letter states that this option should be
	// followed by an additional value (e.g. "-s 1")
	while ((c = getopt(argc, argv, "vE:s:b:t:")) != -1) {
		switch (c) {
			case 'v':
				// enable verbose mode
				verbose_mode = true;
				break;
			case 's':
				// Note: optarg is set by getopt to the string that follows
				// this option (e.g. "-s 2" would assign optarg to the string "2")
				set_bits = strtol(optarg, NULL, 10);
				break;
			case 't':
				// specify the trace filename
				trace_filename = optarg;
				break;
			case 'b':
				//take in number of block offset bits
				block_offset_bits = strtol(optarg, NULL, 10);
				break;
			case 'E': 
				lines_per_set = strtol(optarg, NULL, 10);
				break;

			default:
				usage(argv[0]);
				exit(1);
		}
	}

	if (verbose_mode) {
		// If user wanted verbose printout, then let's be verbose!
		printf("Verbose mode enabled.\n");
		printf("Trace filename: %s\n", trace_filename);
		printf("Number of sets index bits: %d\n", set_bits);
		printf("Number of block offset bits: %d\n", block_offset_bits);
		printf("Number of lines per set: %d\n", lines_per_set);
	}

	simulateCache(trace_filename, lines_per_set, set_bits, block_offset_bits, verbose_mode);

	return 0;
}

/**
 * This function identifies the location in the cache where a hit occurs or
 * the location that will need to be updated after certain misses. It also
 * controls the LRU. 
 *
 * @param cache - cache lines array
 * @param address - memory address thats accessed
 * @param lines_per_set - user specified lines per set
 * @param set_bits - user specified set_bits
 * @param offset - user specified block offset 
 * @param LRU - least recently used number
 * 
 * @returns a number that indicates a hit or miss
 */

int handle_access(Line **cache, mem_addr address, int lines_per_set, int set_bits, int offset, int LRU){

	int t = address >> (set_bits + offset);// get the tag
	int s = (address >> offset) & ((1<< set_bits) - 1); //get the set number
	
	int minLRU = LRU; 
	int spot = 0; //if miss spot will be used to update tag, valid, and timestamp

	for(int j = 0; j < lines_per_set; j++){ //loop through the lines in the set
		if (cache[s][j].valid == 0){
			spot = j;
			minLRU = 0; //if valid is zero set minLRU to 0
		}else if (cache[s][j].tag == t){ //if a hit
			cache[s][j].time_stamp = LRU; //update LRU
			return 1;
		} else if (cache[s][j].time_stamp < minLRU){ //if current LRU is less than min
			minLRU = cache[s][j].time_stamp; //update min LRU
			spot = j;
		}

	}
	//update tag valid and time_stamp if a miss or eviction
	cache[s][spot].tag = t;
	cache[s][spot].valid = 1;
	cache[s][spot].time_stamp = LRU;

	if (minLRU < 1){ //if miss
		return 2;
	}else { 
		return 0;
	}
}
/**
 * Simulates cache with the specified organization (S, E, B) on the given
 * trace file.
 *
 * @param trace_file Name of the file with the memory addresses.
 * @param verbose Whether we should have verbose printing (helpful for debugging)
 * @param e - number of lines per set
 * @param s - number of set bits
 * @param b - offset
 
 */
void simulateCache(char *trace_file, int e, int s, int b, bool verbose) {
	// Variables to track how many hits, misses, and evictions we've had so
	// far during simulation.
	int hit_count = 0;
	int miss_count = 0;
	int eviction_count = 0;
	long unsigned int read_size = 1;

	Line **cache = calloc(1 << s, sizeof(Line*)); //allocate memory for 2D array
	for (int i = 0; i < 1 << s; i++) {
		cache[i] = calloc(e, sizeof(Line));
	}


	FILE *file = fopen(trace_file, "r"); //open file in read mode
	if(file == NULL){
		printf("Error opening file");
		exit(1);
	}
	char operation[2]; 
	mem_addr address = 0;
	int size = 0; 
	int time_stamp = 1;//track LRU
	while(fscanf(file, "%s %lx, %d",operation, &address, &size) == 3){

		if(strncmp(operation, "I", read_size) != 0){//skip I
			int hit_or_miss = handle_access(cache, address, e, s, b, time_stamp); 
			if(hit_or_miss == 1){ //if a hit
				hit_count ++;
			}
			else if(hit_or_miss == 2){ //if a miss no eviction
				miss_count++;
			}

			else{ //if eviction
				miss_count++;
				eviction_count++;
			}
			if (strncmp(operation, "L", read_size) == 0 || strncmp(operation, "S", read_size) == 0){
				if (verbose){
					printf("%s %lx,%d \n", operation, address, size);			
				}
			}

			else if(strncmp(operation, "M", read_size) == 0){//if M call handle_access again
				if (verbose){
					printf("%s %lx,%d \n", operation, address, size);
				}
				int hit_or_miss = handle_access(cache, address, e, s, b, time_stamp);
				if(hit_or_miss == 1){ //if a hit
					hit_count ++; 
				}   
				else if(hit_or_miss  == 2){ //if a miss
					miss_count++;
				}   

				else{ //if an evictionn
					miss_count++;
					eviction_count++;
				}   
			}
			time_stamp++; //increment LRU

		}
	}

	for (int i = 0; i < 1 << s; i ++){//free cache
		free(cache[i]);
	}
	fclose(file);//close file
	free(cache);
	// DO NOT MODIFY, MOVE, OR REMOVE THE FOLLOWING LINE!
	printSummary(hit_count, miss_count, eviction_count);

}


