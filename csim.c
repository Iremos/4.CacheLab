/* name : ±èÀç¿ø
 * ID : biopetjaewon99
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include "cachelab.h"

typedef struct {
	bool valid;
	bool dirty;
	long tag;
	int counter;
} Line;

Line** initCache(int S, int E);
void freeCache(Line **cache, int S);
int getOpt(int argc, char **argv, bool *verbose, int *s, int *E, int *b, char *traceFile);
bool hit(Line **cache, long s, int E, long tag, int round, char *op, char *message);
int getLRU(Line **cache, long s, int E);

int main (int argc, char **argv) {
	int s, S, E, b, size, lru;
	long addr, setId, tag;
	char traceFile[20], op[2], message[20];
	FILE *trace;
	Line **cache;
	bool verbose = false;
	long hit_count = 0, miss_count = 0, eviction_count = 0, dirty_count = 0, dirty_evicted = 0;
	int round = 0;
	getOpt(argc, argv, &verbose, &s, &E, &b, traceFile);
	S = 1 << s;
	cache = initCache(S, E);
	trace = fopen(traceFile, "r");
	while (fscanf(trace, "%s %lx,%d", op, &addr, &size) != EOF) {
		round++;
		setId = ((1 << s) - 1) & (addr >> b);
		tag = (addr >> b) >> s;
		if (hit(cache, setId, E, tag, round, op, message)) ++hit_count;
		else {
			miss_count++;
			lru = getLRU(cache, setId, E);
			cache[setId][lru].tag = tag;
			cache[setId][lru].counter = round;
			if (cache[setId][lru].valid) {
				++eviction_count;
				if (cache[setId][lru].dirty) {
					++dirty_evicted;
					cache[setId][lru].dirty = false;
				}
				strcpy(message, "miss eviction");
			}
			else {
				cache[setId][lru].valid = true;
				strcpy(message, "miss");
			}
			if (strcmp(op, "S") == 0) cache[setId][lru].dirty = true;
		}
		if (verbose) printf("%s %lx,%d\t%s\n", op, addr, size, message);
	}
	for (int i = 0; i < S; ++i) {
		for (int j = 0; j < E; ++j) {
			if (cache[i][j].dirty) ++dirty_count;
		}
	}
	printSummary(hit_count, miss_count, eviction_count, (dirty_count << b), (dirty_evicted << b));
	freeCache(cache, S);
	return 0;
}

int getOpt(int argc, char **argv, bool *verbose, int *s, int *E, int *b,
	char *traceFile) {
	int opt;
	while ((opt = getopt(argc, argv, "hvs:E:b:t:")) > 0) {
		switch (opt) {
		case 'v':
			*verbose = true;
			break;
		case 's':
			*s = atoi(optarg);
			break;
		case 'E':
			*E = atoi(optarg);
			break;
		case 'b':
			*b = atoi(optarg);
			break;
		case 't':
			strcpy(traceFile, optarg);
			break;
		default:
			printf("Wrong argument.\n");
			break;
		}
	}
	return 1;
}

Line** initCache(int S, int E) {
	Line **cache = (Line **)calloc(S, sizeof(Line*));
	for (int i = 0; i < S; ++i) {
		cache[i] = (Line *)calloc(E, sizeof(Line));
	}
	return cache;
}

void freeCache(Line **cache, int S) {
	for (int i = 0; i < S; ++i) {
		free(cache[i]);
	}
	free(cache);
}

bool hit(Line **cache, long s, int E, long tag, int round, char *op,
	char *message) {
	for (int j = 0; j < E; ++j) {
		if (cache[s][j].tag == tag && cache[s][j].valid) {
			cache[s][j].counter = round;
			if (strcmp(op, "S") == 0) cache[s][j].dirty = true;
			strcpy(message, "hit");
			return true;
		}
	}
	return false;
}

int getLRU(Line **cache, long s, int E) {
	int lruCounter = cache[s][0].counter;
	int lru = 0;
	for (int j = 0; j < E; ++j) {
		if (cache[s][j].counter < lruCounter) {
			lruCounter = cache[s][j].counter;
			lru = j;
		}
	}
	return lru;
}