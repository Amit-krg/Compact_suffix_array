using namespace std;
#include <divsufsort.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "rank9b.h"
#include <ctime>
#define VALUE_A 0
#define VALUE_T 1
#define VALUE_G 2
#define VALUE_C 3

static inline unsigned char get_value(unsigned char *data, long index) {
	return (data[index/4] >> ((index % 4) * 2)) & 3;
}
	

int main(int argc, char *argv[]) {
	char *in;
	char tmp;
	unsigned char *data = NULL;
	saidx_t *SA;
	unsigned long actual_size = 0;
	unsigned long in_size = 0;
	unsigned long packed_size;
	string pat;
	unsigned long low = 0;
	unsigned long high = 0;
	unsigned long mid = 0;
	unsigned long val = 0;
	char cval;
        rank9b *rds;
	uint64_t *bit_arr;
	uint64_t start_rank;
	FILE *fp = NULL;
	char *temp_buf = NULL;
	int read = 0;
	struct stat stat_buf;
	unsigned long max_index = 0;
	unsigned long max_l = 0;
	unsigned long match_l = 0;
	clock_t Start = clock();
	clock_t End;

	temp_buf = (char *)malloc(500000);
	if (temp_buf == NULL)
		return 1;

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		printf("Could not open file\n");
		return 1;
	}
	int rc = stat(argv[1], &stat_buf);

	in = (char *)malloc(stat_buf.st_size);
	if (!in)
		return 1;
	bit_arr = (uint64_t *) malloc(stat_buf.st_size/8 + 8);
	while(1) {
		read = fscanf(fp, "%s", temp_buf);
		if(read < 1)
			break;
		memset(temp_buf, 0, 500000);
		fscanf(fp, "%s", temp_buf);
		strcpy(in + actual_size, temp_buf);
		actual_size = actual_size + strlen(temp_buf);
		bit_arr[(actual_size - 1)/64] |= (1 << ((actual_size - 1)%64));
	}
	in_size = actual_size;
	packed_size = in_size/4 + 1;
	rds = new rank9b(bit_arr, in_size);

	SA = (saidx_t *) malloc(actual_size * sizeof(saidx_t));
	if (SA == NULL)
		return 1;

	Start = clock();
	saint_t ret = divsufsort((unsigned char *) in, SA, in_size);
	cout<<"Time to create SA = "<<(clock() - Start)/CLOCKS_PER_SEC<<"s\n";
	
	data = (unsigned char *) malloc(packed_size);
	if (data == NULL) {
		return 1;
	}
	memset(data, 0, packed_size);
	Start = clock();
	for (int i = 0; i < in_size; i++) {
		if (in[i] == 'A') {
			data[i/4] = data[i/4] | (VALUE_A <<((i % 4) * 2));
		}
		else if(in[i] == 'T') {
                        data[i/4] = data[i/4] | (VALUE_T <<((i % 4) * 2));
                }
		else if (in[i] == 'G') {
                        data[i/4] = data[i/4] | (VALUE_G <<((i % 4) * 2));
                }
		else if (in[i] == 'C') {
                        data[i/4] = data[i/4] | (VALUE_C <<((i % 4) * 2));
                }
	}
	cout<<"Time to convert to packed format = "<<(clock() - Start)/CLOCKS_PER_SEC<<"s\n";
	free(in);
	free(temp_buf);
	while(1) {
		cout<<"Please enter a pattern or enter 'q' to quit\n";
		cin>>pat;
		if (pat.at(0) == 'q')
			break;
		Start = clock();
		low = 0;
		high = in_size - 1;
		int i;
		max_index = 0;
		max_l = 0;
		while (low <= high)
		{
			mid = low + (high - low) / 2;
			start_rank = rds->rank(SA[mid]);
			match_l = 0;
			for (i = 0; i < pat.length() && SA[mid] + i < in_size; i++)
			{
				val = get_value(data, SA[mid] + i);
				if (val == VALUE_A) {
					cval = 'A';
				}
				else if (val == VALUE_T) {
                	                cval = 'T';
                        	}
				else if (val == VALUE_G) {
	                                cval = 'G';
        	                }
				else {
					cval = 'C';
				}
				if (cval == pat.at(i) && start_rank == rds->rank(SA[mid] + i)) {
					match_l++;
					if (match_l > max_l) {
                                                max_l = match_l;
                                                max_index = SA[mid];
                                        }
					continue;
				}
				else {
					break;
				}
			}
			if (i == pat.length())
			{
				End = clock();
				cout << "Pattern found at index "<< SA[mid]<<"\n";
				break;
			}
 
			if (pat.at(i) - cval < 0) {
				high = mid - 1;
			} 
			else {
				low = mid + 1;
			}
		}
		if (max_l < pat.length()) {
			End = clock();
			cout<<"Exact match not found. Length "<<max_l<<" prefix of pattern found is : "<<pat.substr(0,max_l)<<"\n";
		}
		cout<<"Time to search = "<<((double)((double)End-(double)Start)/(double)CLOCKS_PER_SEC)*(double)1000<<"ms\n";
	}
	return 0;
}
