/*
 * SegmentedHash.cpp
 *
 *  Created on: 29 Nov 2022
 *      Author: rasel
 */

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "Common.h"
#include "Compression.h"
#include "HashSegmented.h"

//#pragma pack(push, 1)

using namespace std;

HashSegmented::HashSegmented() {
	isEnabled = false;
	insertCount = 0;
	LAST_UNIQ_ID = 0;
	hashMap.count = OVER_FLOW;
	//cout << "Initializing\n";
	hashMap.segs = NULL;
	filePath = NULL;
}
HashSegmented::~HashSegmented() {}
void HashSegmented::init(char* dir, char* type){
	int pathLen = strlen(dir)+strlen(type)+8;
	this->filePath = new char[pathLen+1];
    sprintf(this->filePath, "%s/dic_%s.tt", dir, type);
	this->filePath[pathLen] = '\0';
	hashMap.segs = (HashSegment*)malloc(SZ_HASHSEG*OVER_FLOW);
	for(int i=0; i<OVER_FLOW; i++){
        hashMap.segs[i].count = 0;
        hashMap.segs[i].keys = (SegmentItem*)malloc(SZ_SEG_ITEM);
        hashMap.segs[i].buckets = (Bucket*)malloc(SZ_BUCKET);
        hashMap.segs[i].keys[0] = 0;
        hashMap.segs[i].buckets[0].count = 0;
	}
    //printf("Dictionary-TT: %s\n", mapFilePath);
}
uint32 HashSegmented::getMaxID(){
	return LAST_UNIQ_ID;
}
unsigned long HashSegmented::merge(uint32 proownId, uint16 pathId, uint16 refId){
    unsigned long value = proownId;
    //cout << value << " ";
    value = value << 16;
    //cout << value << " ";
    value = value | pathId;
    //cout << value << " ";
    value = value << 16;
    //cout << value << " ";
    value = value | refId;
    //cout << value << endl;
    return value;
}
int HashSegmented::binarySearchInStrings(char** list_of_words, int size, const char *target){
	int first= 0;
    int last = size-1;
    int middle = (first+last)/2;
    while (first <= last) {
        if(strcmp(list_of_words[middle], target) < 0){
            first = middle + 1;
        }else if (strcmp(list_of_words[middle], target) == 0) {
            return middle;
        }else{
            last = middle - 1;
        }
        middle = (first + last)/2;
    }
    return first==0 ? OVER_FLOW : -first;
}
int HashSegmented::binarySearchForIndex(SegmentItem* arr, int count, SegmentItem id){
    int first = 0;
    if(count > 0){
        int last = count-1;
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
    return first==0 ? OVER_FLOW : -first;
}

uint32 HashSegmented::add(const char* value){

	//if((LAST_UNIQ_ID+1)%100000==0){
	//	cout << c.getMemUsage()*1024 << " " << size << " " << c.getMemUsage()*1024 - size << " " << mapFilePath << endl;
	//}
	uint32 key = c.hashCode(value);
    //printf("%u %s => ", key, value);
    ushort bsgIdx = (SegmentItem)(key/SEG_LENGTH);
    if(bsgIdx > SEG_LENGTH){
        printf("bsgId: %d\n", bsgIdx);
        //exit(1);
    }
    int valLen = strlen(value);
   // printf("2 bsgId: %d %d %d\n", hashMap.count, bsgIdx, bsgIdx);
    ushort bucketId  = (ushort)(key%SEG_LENGTH);
    int bucketIdx = binarySearchForIndex(hashMap.segs[bsgIdx].keys, hashMap.segs[bsgIdx].count, bucketId);
    //cout << bucketIdx << endl;
    if(bucketIdx>=OVER_FLOW || bucketIdx<0){
        bucketIdx = bucketIdx>=OVER_FLOW ? 0 : -bucketIdx;
        hashMap.segs[bsgIdx].keys = (SegmentItem*)c.xrealloc(hashMap.segs[bsgIdx].keys, SZ_SEG_ITEM*(1+hashMap.segs[bsgIdx].count));
        hashMap.segs[bsgIdx].buckets = (Bucket*)c.xrealloc(hashMap.segs[bsgIdx].buckets, SZ_BUCKET*(1+hashMap.segs[bsgIdx].count));
        if(bucketIdx < hashMap.segs[bsgIdx].count) {
            memmove(&hashMap.segs[bsgIdx].keys[bucketIdx+1], &hashMap.segs[bsgIdx].keys[bucketIdx], SZ_SEG_ITEM*(hashMap.segs[bsgIdx].count-bucketIdx));
            memmove(&hashMap.segs[bsgIdx].buckets[bucketIdx+1], &hashMap.segs[bsgIdx].buckets[bucketIdx], SZ_BUCKET*(hashMap.segs[bsgIdx].count-bucketIdx));
            //for(int i=0; i < hashMap.segs[bsgIdx].buckets[bucketIdx].count; i++){
            	//hashMap.segs[bsgIdx].buckets[bucketIdx].count = 0;
            	//free(hashMap.segs[bsgIdx].buckets[bucketIdx].values[i]);
            //}
            if(hashMap.segs[bsgIdx].buckets[bucketIdx].count > 3){
            	cout << "@HS " << hashMap.segs[bsgIdx].buckets[bucketIdx].count << endl;
            }
        }
        hashMap.segs[bsgIdx].count++;
        if(hashMap.segs[bsgIdx].count > OVER_FLOW || hashMap.segs[bsgIdx].count==0){
            printf("hashMap.segs[bsgIdx].count: %d\n", hashMap.segs[bsgIdx].count);
            exit(1);
        }
        hashMap.segs[bsgIdx].keys[bucketIdx] = bucketId;
        hashMap.segs[bsgIdx].buckets[bucketIdx].count = 1;
        hashMap.segs[bsgIdx].buckets[bucketIdx].values = (char**)malloc(sizeof(char*));
        hashMap.segs[bsgIdx].buckets[bucketIdx].values[0] = (char*)malloc(valLen+1);
        memcpy(&hashMap.segs[bsgIdx].buckets[bucketIdx].values[0][0], &value[0], valLen);
        hashMap.segs[bsgIdx].buckets[bucketIdx].values[0][valLen] = '\0';
        hashMap.segs[bsgIdx].buckets[bucketIdx].ids = (BucketItem*)malloc(SZ_BUCKET_ITEM);
        hashMap.segs[bsgIdx].buckets[bucketIdx].ids[0] = (++LAST_UNIQ_ID);
        size += valLen+1+sizeof(char*)+SZ_BUCKET_ITEM+SZ_SEG_ITEM+SZ_BUCKET;
        return LAST_UNIQ_ID;
    }
    int valIdx = binarySearchInStrings(hashMap.segs[bsgIdx].buckets[bucketIdx].values, hashMap.segs[bsgIdx].buckets[bucketIdx].count, value);
    if(valIdx>=OVER_FLOW || valIdx<0){
        valIdx = valIdx>=OVER_FLOW ? 0 : -valIdx;
        hashMap.segs[bsgIdx].buckets[bucketIdx].values = (char**)c.xrealloc(hashMap.segs[bsgIdx].buckets[bucketIdx].values, sizeof(char*)*(1+hashMap.segs[bsgIdx].buckets[bucketIdx].count));
        hashMap.segs[bsgIdx].buckets[bucketIdx].ids = (BucketItem*)c.xrealloc(hashMap.segs[bsgIdx].buckets[bucketIdx].ids, SZ_BUCKET_ITEM*(1+hashMap.segs[bsgIdx].buckets[bucketIdx].count));

        if(valIdx < hashMap.segs[bsgIdx].buckets[bucketIdx].count) {
            memmove(&hashMap.segs[bsgIdx].buckets[bucketIdx].values[valIdx+1], &hashMap.segs[bsgIdx].buckets[bucketIdx].values[valIdx], sizeof(char*)*(hashMap.segs[bsgIdx].buckets[bucketIdx].count-valIdx));
            memmove(&hashMap.segs[bsgIdx].buckets[bucketIdx].ids[valIdx+1], &hashMap.segs[bsgIdx].buckets[bucketIdx].ids[valIdx], SZ_BUCKET_ITEM*(hashMap.segs[bsgIdx].buckets[bucketIdx].count-valIdx));
            //free(hashMap.segs[bsgIdx].buckets[bucketIdx].values[valIdx]);
        }
        hashMap.segs[bsgIdx].buckets[bucketIdx].count++;
        if(hashMap.segs[bsgIdx].buckets[bucketIdx].count > OVER_FLOW || hashMap.segs[bsgIdx].buckets[bucketIdx].count==0){
            printf("hashMap.segs[bsgIdx].buckets[bucketIdx].count: %d\n", hashMap.segs[bsgIdx].buckets[bucketIdx].count);
            exit(1);
        }
        hashMap.segs[bsgIdx].buckets[bucketIdx].values[valIdx] = (char*)malloc(valLen+1);
        memcpy(&hashMap.segs[bsgIdx].buckets[bucketIdx].values[valIdx][0], &value[0], valLen);
        hashMap.segs[bsgIdx].buckets[bucketIdx].values[valIdx][valLen] = '\0';
        hashMap.segs[bsgIdx].buckets[bucketIdx].ids[valIdx] = (++LAST_UNIQ_ID);
        size += valLen+1+sizeof(char*)+SZ_BUCKET_ITEM;
        return LAST_UNIQ_ID;
    }
    return hashMap.segs[bsgIdx].buckets[bucketIdx].ids[valIdx];
}
uint32 HashSegmented::getId(const char* value){
	uint32 key = c.hashCode(value);
    // printf("\n%u %s\n", key, value);
    ushort bsgIdx = (SegmentItem)(key/SEG_LENGTH);
    //int bsgIdx = this->binarySearchForIndex(hashMap.bids, hashMap.count, bsgId);
    if(bsgIdx>=0 && bsgIdx<OVER_FLOW){
        ushort bucketId  = (ushort)(key%SEG_LENGTH);
        int bucketIdx = binarySearchForIndex(hashMap.segs[bsgIdx].keys, hashMap.segs[bsgIdx].count, bucketId);
        if(bucketIdx>=0 && bucketIdx<OVER_FLOW){
        	int valIdx = binarySearchInStrings(hashMap.segs[bsgIdx].buckets[bucketIdx].values, hashMap.segs[bsgIdx].buckets[bucketIdx].count, value);
			if(valIdx>=0 && valIdx<OVER_FLOW){
				return hashMap.segs[bsgIdx].buckets[bucketIdx].ids[valIdx];
			}
        }
    }
    return 0;
}
char* HashSegmented::getValue(uint32 id){
	//	cout << "ID: " << id << " " << LAST_UNIQ_ID << endl;
	//	for(uint32 i=0; i<LAST_UNIQ_ID; i++){
	//		cout << i << " " << id2Code[i].segIdx << " " << id2Code[i].bktIdx << " " << id2Code[i].valIdx << " " << hashMap.segs[id2Code[i].segIdx].buckets[id2Code[i].bktIdx].values[id2Code[i].valIdx] << endl;
	//	}
    if(id > 0 && id <= LAST_UNIQ_ID){
        return hashMap.segs[indices[id-1].segIdx].buckets[indices[id-1].bktIdx].values[indices[id-1].valIdx];
    }
    return NULL;
}
uint32 HashSegmented::getMapSize(){
	return this->hashMap.count;
}
long HashSegmented::getSize(){
	return this->size/1024;
}
unsigned long HashSegmented::flushHash(){
	indices = (Index32*)malloc(SZ_HASH_INDEX*LAST_UNIQ_ID);
	////////////////////////////////////////////
	c.removeFile(this->filePath);
	FILE *wfp;
	wfp = fopen(this->filePath, "wb");
	uint32 id = 0;
	short len;
	fwrite(&LAST_UNIQ_ID, sizeof(uint32), 1, wfp);
	fwrite(&hashMap.count, sizeof(uint32), 1, wfp);
	for(uint32 i=0; i<hashMap.count; i++){
		fwrite(&hashMap.segs[i].count, SZ_SEG_ITEM, 1, wfp);
		if(hashMap.segs[i].count>0){
			fwrite(hashMap.segs[i].keys, SZ_SEG_ITEM, hashMap.segs[i].count, wfp);
			for(SegmentItem j=0; j<hashMap.segs[i].count; j++){
				fwrite(&hashMap.segs[i].buckets[j].count, SZ_SEG_ITEM, 1, wfp);
				if(hashMap.segs[i].buckets[j].count>0){
					fwrite(hashMap.segs[i].buckets[j].ids, SZ_BUCKET_ITEM, hashMap.segs[i].buckets[j].count, wfp);
					for(SegmentItem k=0; k<hashMap.segs[i].buckets[j].count; k++){
						len = strlen(hashMap.segs[i].buckets[j].values[k]);
						fwrite(&len, 1, 1, wfp);
						fwrite(hashMap.segs[i].buckets[j].values[k], 1, len, wfp);
						free(hashMap.segs[i].buckets[j].values[k]);
						id = hashMap.segs[i].buckets[j].ids[k]-1;
						indices[id].segIdx = i;
						indices[id].bktIdx = j;
						indices[id].valIdx = k;
					}
				}
				free(hashMap.segs[i].buckets[j].ids);
				free(hashMap.segs[i].buckets[j].values);
			}
		}
		free(hashMap.segs[i].keys);
		free(hashMap.segs[i].buckets);
	}
	free(hashMap.segs);
	fwrite(indices, SZ_HASH_INDEX, LAST_UNIQ_ID, wfp);
	fflush(wfp);
	fclose(wfp);
	free(indices);
	unsigned long totalSize = c.getFileSize(this->filePath);
	delete[] this->filePath;
	return totalSize;
}
void HashSegmented::load(char* dir, char* type){
	int pathLen = strlen(dir)+strlen(type)+8;
	this->filePath = new char[pathLen+1];
    sprintf(this->filePath, "%s/dic_%s.tt", dir, type);
	this->filePath[pathLen] = '\0';


	//cout << "LAST_UNIQ_ID: " << maxId << endl;
	FILE *rfp;
	rfp = fopen(this->filePath, "rb");
	short len = 0;
	fread(&LAST_UNIQ_ID, sizeof(uint32), 1, rfp);
	fread(&hashMap.count, sizeof(uint32), 1, rfp);
	//cout << hashMap.count << endl;
	//hashMap.bids = (SegmentItem*)c.xrealloc(hashMap.bids, SZ_SEG_ITEM*(1+hashMap.count), "@addItem2HashMap");
	//fread(hashMap.bids, SZ_SEG_ITEM, hashMap.count, rfp);
	hashMap.segs = (HashSegment*)c.xrealloc(hashMap.segs, SZ_HASHSEG*(1+hashMap.count));
	for(long i=0; i<hashMap.count; i++){
		fread(&hashMap.segs[i].count, SZ_SEG_ITEM, 1, rfp);
		if(hashMap.segs[i].count>0){
			//cout << hashMap.segs[i].count << endl;
			hashMap.segs[i].keys = (SegmentItem*) malloc(SZ_SEG_ITEM * hashMap.segs[i].count);
			hashMap.segs[i].buckets = (Bucket*) malloc(SZ_BUCKET * hashMap.segs[i].count);
			fread(hashMap.segs[i].keys, SZ_SEG_ITEM, hashMap.segs[i].count, rfp);
			for(long j=0; j<hashMap.segs[i].count; j++){
				fread(&hashMap.segs[i].buckets[j].count, SZ_SEG_ITEM, 1, rfp);
				//cout << hashMap.segs[i].buckets[j].count << endl;
				//cout << "Reading hashed values" << endl;
				if(hashMap.segs[i].buckets[j].count>0){
					hashMap.segs[i].buckets[j].ids = (BucketItem*) malloc(SZ_BUCKET_ITEM*hashMap.segs[i].buckets[j].count);
					fread(hashMap.segs[i].buckets[j].ids, SZ_BUCKET_ITEM, hashMap.segs[i].buckets[j].count, rfp);
					hashMap.segs[i].buckets[j].values = (char**) malloc(sizeof(char*)*hashMap.segs[i].buckets[j].count);
					for(long k=0; k<hashMap.segs[i].buckets[j].count; k++){
						fread(&len, 1, 1, rfp);
						if(((int)len)>0){
							//cout << (int)len << endl;
							hashMap.segs[i].buckets[j].values[k] = (char*)malloc(len+1);
							fread(hashMap.segs[i].buckets[j].values[k], 1, len, rfp);
							hashMap.segs[i].buckets[j].values[k][len] = '\0';
							//cout << ((int)len) << ": " << hashMap.segs[i].buckets[j].values[k] << endl;
						}
					}
				}
			}
		}
	}
	indices = (Index32*)malloc(SZ_HASH_INDEX*LAST_UNIQ_ID);
	fread(indices, SZ_HASH_INDEX, LAST_UNIQ_ID, rfp);
	fclose(rfp);
	//	for(int i=0; i<LAST_UNIQ_ID+1; i++){
	//		cout << id2Code[i].segIdx << " " << id2Code[i].bktIdx << " " << id2Code[i].valIdx << endl;
	//	}
}
void HashSegmented::close(){
	//free(hashMap.bids);
	for(long i=0; i<OVER_FLOW; i++){
		if(hashMap.segs[i].count>0){
			free(hashMap.segs[i].keys);
			for(long j=0; j<hashMap.segs[i].count; j++){
				free(hashMap.segs[i].buckets[j].ids);
				for(long k=0; k<hashMap.segs[i].buckets[j].count; k++){
					free(hashMap.segs[i].buckets[j].values[k]);
				}
				free(hashMap.segs[i].buckets[j].values);
			}
			free(hashMap.segs[i].buckets);
		}
	}
	free(hashMap.segs);
	free(indices);
	delete[] this->filePath;
}



