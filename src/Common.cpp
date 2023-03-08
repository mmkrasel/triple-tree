/*
 * Common.cpp
 *
 *  Created on: 8 Nov 2022
 *      Author: rasel
 */
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <chrono>
#include <unistd.h>
#include "Common.h"
#include <boost/filesystem.hpp>
//#pragma pack(push, 1)

namespace fs = boost::filesystem;


using namespace std;

Common::Common() {
	// TODO Auto-generated constructor stub
	URI_MAX_LENGTH = 512;
	SZ_TRIPLE = sizeof(TripleItem);
	SZ_TRIPLE_ID = sizeof(TripleId);
}

Common::~Common() {
	// TODO Auto-generated destructor stub
}

void* Common::xrealloc(void *ptr, unsigned long size){
	void *value = realloc(ptr, size);
    // printf("@xrealloc: %ld %ld %d: %s\n", ptr, value, size, calledFrom);
    if (value == 0 && size>0){
        value = realloc(ptr, size);
        if(value == 0){
            fprintf(stderr, "@xrealloc: Virtual memory exhausted %ld\n", size);
            exit(0);
        }
    }
    if(value == 0){
        fprintf(stderr, "@xrealloc: Virtual memory exhausted %ld\n", size);
        exit(0);
    }
    return value;
}



long long int Common::getFolderSize(string path) {
    // command to be executed
    std::string cmd("du -hs ");
    cmd.append(path);
    //cmd.append(" | cut -f1 2>&1");

    // execute above command and get the output
    FILE *stream = popen(cmd.c_str(), "r");
    if (stream) {
        const int max_size = 256;
        char readbuf[max_size];
        if (fgets(readbuf, max_size, stream) != NULL) {
            return atoll(readbuf);
        }
        pclose(stream);
    }
    // return error val
    return -1;
}

long Common::getFileSize(char path[]){
    struct stat filestatus;
    stat(path, &filestatus);
    return filestatus.st_size;
}
bool Common::fileExists(char filename[]) {
    FILE * file;
    if ((file = fopen(filename, "r")) != NULL) {
        fclose(file);
        return true;
    }
    return false;
}
void Common::removeFile(char filePath[]){
    if(fileExists(filePath)){
        remove(filePath);
    }
}
long Common::diff(struct timespec start, struct timespec end) {
    long diff = (end.tv_sec * 1000000000 + end.tv_nsec) - (start.tv_sec * 1000000000 + start.tv_nsec);
    return diff;
}

void Common::startClock(){
	begin = chrono::steady_clock::now();
}
double Common::getElapsedTime(){
	end = chrono::steady_clock::now();
	double elapsed =  chrono::duration_cast<chrono::nanoseconds>(end - begin).count();
	//auto elapsed =  chrono::duration_cast<chrono::nanoseconds>(end - start).count();
	return elapsed;
}


//-----------------------------------------------------------------------------
// MurmurHashNeutral2, by Austin Appleby
// Same as MurmurHash2, but endian- and alignment-neutral.
// Half the speed though, alas.
uint32 Common::hashCode(const char* key){
    int len = strlen(key);
    uint32 seed = len;
    const unsigned int m = 0x5bd1e995;
    const int r = 24;
    unsigned int h = seed ^ len;
    const unsigned char * data = (const unsigned char *)key;
    while(len >= 4){
        uint32 k;
        k  = data[0];
        k |= data[1] << 8;
        k |= data[2] << 16;
        k |= data[3] << 24;
        k *= m;
        k ^= k >> r;
        k *= m;
        h *= m;
        h ^= k;
        data += 4;
        len -= 4;
    }
    switch(len){
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0];
        h *= m;
    };
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;
    return h;
}




int Common::parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int Common::getMemUsage(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}


//int binarySearchInStrings(char *list_of_words[], int size, char *target){
//    int first= 0;
//
//    //int mid;
//    int last = size - 1;
//    int middle = (first+last)/2;
//    // while(first <= last){
//    //     mid = (first + last)/2;
//    //     if (strcmp(list_of_words[mid], target) == 0){
//    //         printf("%s found at location %d.\n", target, mid+1);
//    //         return;
//    //     } else if (strcmp(list_of_words[mid], target) > 0){
//    //         last    = mid - 1;
//    //     } else if (strcmp(list_of_words[mid], target) < 0){
//    //         first = mid + 1;
//    //     }
//    // }
//    while (first <= last) {
//        if(strcmp(list_of_words[middle], target) < 0){
//            first = middle + 1;
//        }else if (strcmp(list_of_words[middle], target) == 0) {
//            return middle;
//        }else{
//            last = middle - 1;
//        }
//        middle = (first + last)/2;
//    }
//
//    return first==0 ? OVER_FLOW : -first;
//}
//
long Common::binarySearchForIndex32(uint32* arr, uint32 count, uint32 id){
    long first = 0;
    if(count > 0){
        long last = count - 1;
        long middle = (first+last)/2;
        while (first <= last) {
            if(arr[middle] < id){
                first = middle + 1;
            }else if (arr[middle] == id) {
                return middle;
            }else{
                last = middle - 1;
            }
            middle = (first + last)/2;
        }
    }
    return first==0 ? 0xffffffff : -first;
}
int Common::binarySearchForIndex16(ushort* arr, int count, ushort id){
    int first = 0;
    if(count > 0){
        int last = count - 1;
        int middle = (first+last)/2;
        while (first <= last) {
            if(arr[middle] < id){
                first = middle + 1;
            }else if (arr[middle] == id) {
                return middle;
            }else{
                last = middle - 1;
            }
            middle = (first + last)/2;
        }
    }
    return first==0 ? 0xffff : -first;
}



