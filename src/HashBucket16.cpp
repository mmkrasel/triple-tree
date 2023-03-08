/*
 * HashBucket16.cpp
 *
 *  Created on: Dec 2, 2022
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
#include "HashBucket16.h"

//#pragma pack(push, 1)

using namespace std;

HashBucket16::HashBucket16() {
	SZ_HCODE = sizeof(uint32);
	SZ_INDEX = sizeof(Index16);
    SZ_BUCKET = sizeof(Bucket16);
    SZ_ID = sizeof(uint16);
    SZ_VAL_PNTR = sizeof(char*);
	LAST_UNIQ_ID = 0;
	//printf("%d %d\n", SZ_INDEX, SZ_BUCKET);
	hashMap.count = 0;
	hashMap.codes = (uint32*)malloc(SZ_HCODE);
	hashMap.buckets = (Bucket16*)malloc(SZ_BUCKET);
	filePath = NULL;
}
HashBucket16::~HashBucket16() {}
void HashBucket16::init(char* dir, char* type){
	this->type = type;
	int pathLen = strlen(dir)+strlen(type)+8;
	this->filePath = new char[pathLen+1];
	sprintf(this->filePath, "%s/dic_%s.tt", dir, type);
	this->filePath[pathLen] = '\0';
}
int HashBucket16::binarySearchOnCodes(uint32 code){
    int first = 0;
    if(this->hashMap.count > 0){
        int last = this->hashMap.count - 1;
        int middle = (first+last)/2;
        while (first <= last) {
            if(hashMap.codes[middle] < code){
                first = middle + 1;
            }else if (hashMap.codes[middle] == code) {
                return middle;
            }else{
                last = middle - 1;
            }
            middle = (first + last)/2;
        }
    }
    return first==0 ? this->OVER_FLOW : -first;
}
int HashBucket16::binarySearchOnValues(uint16 bktIdx, const char *searhFor){
//	cout << searhFor << endl;
//	for(int i=0; i<this->hashMap.buckets[bktIdx].count; i++){
//		cout << this->hashMap.buckets[bktIdx].values[i] << " ";
//	}
	int first= 0;
	int last = this->hashMap.buckets[bktIdx].count - 1;
	int middle = (first+last)/2;
    while (first <= last) {
        if(strcmp(this->hashMap.buckets[bktIdx].values[middle], searhFor) < 0){
            first = middle + 1;
        }else if (strcmp(this->hashMap.buckets[bktIdx].values[middle], searhFor) == 0) {
            return middle;
        }else{
            last = middle - 1;
        }
        middle = (first + last)/2;
    }
    return first==0 ? this->OVER_FLOW : -first;
}
uint16 HashBucket16::add(const char* value){
	int code = c.hashCode(value);
    //printf("%u %s => ", code, value);
	int bktIdx = this->binarySearchOnCodes(code);
    //printf(" %u %ld %ld %d\n", code, bktIdx, OVER_FLOW, this->hashMap.count);
	int valLen = strlen(value);
    if(bktIdx>=OVER_FLOW || bktIdx < 0){
    	bktIdx = bktIdx>=OVER_FLOW ? 0 : -bktIdx;
    	//if(hashMap.count % INCREASE_BY == 0){
    		hashMap.codes = (uint32*)c.xrealloc(hashMap.codes, SZ_HCODE*(hashMap.count+1));
    		hashMap.buckets = (Bucket16*)c.xrealloc(hashMap.buckets, SZ_BUCKET*(hashMap.count+1));
    	//}
        if(bktIdx < hashMap.count) {
            memmove(&hashMap.codes[bktIdx+1], &hashMap.codes[bktIdx], SZ_HCODE*(hashMap.count-bktIdx));
            memmove(&hashMap.buckets[bktIdx+1], &hashMap.buckets[bktIdx], SZ_BUCKET*(hashMap.count-bktIdx));
            //for(int i=0; i<hashMap.buckets[bktIdx].count; i++){
            //	free(hashMap.buckets[bktIdx].values[0]);
            //}
            if(hashMap.buckets[bktIdx].count > 2){
            	cout << "@HB " << hashMap.buckets[bktIdx].count << endl;
            }
        }
        hashMap.count++;
        if(hashMap.count >= OVER_FLOW || hashMap.count==0){
            printf("@HashBucket16: hashMap.count: %d\n", hashMap.count);
            exit(1);
        }
        hashMap.codes[bktIdx] = code;
        hashMap.buckets[bktIdx].count = 1;
        hashMap.buckets[bktIdx].values = (char**)malloc(SZ_VAL_PNTR);
        hashMap.buckets[bktIdx].values[0] = (char*)malloc(valLen+1);
        memcpy(&hashMap.buckets[bktIdx].values[0][0], &value[0], valLen);
        hashMap.buckets[bktIdx].values[0][valLen] = '\0';

        hashMap.buckets[bktIdx].ids = (uint16*)malloc(SZ_ID);
        hashMap.buckets[bktIdx].ids[0] = (++LAST_UNIQ_ID);

//        //if(LAST_UNIQ_ID % INCREASE_BY == 0){
//        	indices = (Index16*)c.xrealloc(indices, SZ_INDEX*LAST_UNIQ_ID, "@addItem2HashMap 3");
//        //}
//        indices[LAST_UNIQ_ID-1].bktIdx = (uint16)bktIdx;
//        indices[LAST_UNIQ_ID-1].valIdx = 0;

        //printf("%s %s => %u %u\n", this->type.c_str(), hashMap.buckets[bktIdx].values[0], bktIdx, LAST_UNIQ_ID);
        size += valLen+1+SZ_ID+SZ_VAL_PNTR+SZ_HCODE+SZ_BUCKET;
        return LAST_UNIQ_ID;
    }
    //printf("%s %u %d\n", this->type.c_str(), this->hashMap.buckets[bktIdx].count, bktIdx);
    int valIdx = this->binarySearchOnValues(bktIdx, value);
    if(valIdx>=OVER_FLOW || valIdx<0){
        valIdx = valIdx>=OVER_FLOW ? 0 : -valIdx;

        //if(hashMap.buckets[bktIdx].count % INCREASE_BY == 0){
        	hashMap.buckets[bktIdx].values = (char**)c.xrealloc(hashMap.buckets[bktIdx].values, SZ_VAL_PNTR*(hashMap.buckets[bktIdx].count+1));
        	hashMap.buckets[bktIdx].ids = (uint16*)c.xrealloc(hashMap.buckets[bktIdx].ids, SZ_ID*(hashMap.buckets[bktIdx].count+1));
        //}
        if(valIdx < hashMap.buckets[bktIdx].count) {
            memmove(&hashMap.buckets[bktIdx].values[valIdx+1], &hashMap.buckets[bktIdx].values[valIdx], SZ_VAL_PNTR*(hashMap.buckets[bktIdx].count-valIdx));
            memmove(&hashMap.buckets[bktIdx].ids[valIdx+1], &hashMap.buckets[bktIdx].ids[valIdx], SZ_ID*(hashMap.buckets[bktIdx].count-valIdx));
            //free(hashMap.buckets[bktIdx].values[valIdx]);
        }
        hashMap.buckets[bktIdx].count++;
        if(hashMap.buckets[bktIdx].count > OVER_FLOW || hashMap.buckets[bktIdx].count==0){
            printf("@HashBucket16: hashMap.buckets[bktIdx].count: %d\n", hashMap.buckets[bktIdx].count);
            exit(1);
        }
        hashMap.buckets[bktIdx].values[valIdx] = (char*)malloc(valLen+1);
        memcpy(&hashMap.buckets[bktIdx].values[valIdx][0], &value[0], valLen);
        hashMap.buckets[bktIdx].values[valIdx][valLen] = '\0';
        hashMap.buckets[bktIdx].ids[valIdx] = (++LAST_UNIQ_ID);
//        //if(LAST_UNIQ_ID % INCREASE_BY == 0){
//        	indices = (Index16*)c.xrealloc(indices, SZ_INDEX*LAST_UNIQ_ID, "@addItem2HashMap");
//        //}
//        indices[LAST_UNIQ_ID-1].bktIdx = (uint16)bktIdx;
//        indices[LAST_UNIQ_ID-1].valIdx = (uint16)valIdx;
        //printf("%s %s => %u %u %u\n", this->type.c_str(), hashMap.buckets[bktIdx].values[valIdx], bktIdx, valIdx, LAST_UNIQ_ID);
        size += valLen+1+SZ_ID+SZ_VAL_PNTR;
        return LAST_UNIQ_ID;
    }
    //printf("* %s %s => %u %u %u\n", this->type.c_str(), hashMap.buckets[bktIdx].values[valIdx], bktIdx, valIdx, hashMap.buckets[bktIdx].ids[valIdx]);
    return hashMap.buckets[bktIdx].ids[valIdx];
}
uint16 HashBucket16::getId(const char* value){
	uint32 code = c.hashCode(value);
    // printf("\n%u %s\n", code, value);
    int bktIdx = this->binarySearchOnCodes(code);
    if(bktIdx >= 0 && bktIdx < OVER_FLOW){
    	int valIdx = this->binarySearchOnValues(bktIdx, value);
		if(valIdx >= 0 && valIdx < OVER_FLOW){
			return hashMap.buckets[bktIdx].ids[valIdx];
		}
    }
    return 0;
}
char* HashBucket16::getValue(uint16 id){
//	cout << "ID: " << id << " " << LAST_UNIQ_ID << endl;
//	for(int i=0; i<LAST_UNIQ_ID+1; i++){
//		cout << i << " " << id2Code[i].segIdx << " " << id2Code[i].bktIdx << " " << id2Code[i].valIdx << " " << hashMap.segs[id2Code[i].segIdx].buckets[id2Code[i].bktIdx].values[id2Code[i].valIdx] << endl;
//	}
    if(id <= LAST_UNIQ_ID){
        return hashMap.buckets[indices[id-1].bktIdx].values[indices[id-1].valIdx];
    }
    return NULL;
}
unsigned long HashBucket16::flushHash(){
//	for(int i=1; i<LAST_UNIQ_ID+1; i++){
//		cout << id2Code[i].segIdx << " " << id2Code[i].bktIdx << " " << id2Code[i].valIdx << "\t" << hashMap.segs[id2Code[i].segIdx].buckets[id2Code[i].bktIdx].unqid[id2Code[i].valIdx] << " " << hashMap.segs[id2Code[i].segIdx].buckets[id2Code[i].bktIdx].values[id2Code[i].valIdx] << endl;
//	}
//	cout << endl;
	////////////////////////////////////////////
	cout << this->type << " " << LAST_UNIQ_ID << endl;
	c.removeFile(this->filePath);
	FILE *wfp;
	wfp = fopen(this->filePath, "wb");
	fwrite(&LAST_UNIQ_ID, sizeof(uint16), 1, wfp);
	if(LAST_UNIQ_ID>0){
		indices = (Index16*)malloc(SZ_INDEX*LAST_UNIQ_ID);
		uint32 id = 0;
		unsigned char len;
		fwrite(&hashMap.count, SZ_ID, 1, wfp);
		fwrite(hashMap.codes, SZ_HCODE, hashMap.count, wfp);
		for(uint16 j=0; j<hashMap.count; j++){
			fwrite(&hashMap.buckets[j].count, SZ_ID, 1, wfp);
			fwrite(hashMap.buckets[j].ids, SZ_ID, hashMap.buckets[j].count, wfp);
			//idCount += hashMap.buckets[j].count;
			for(uint16 k=0; k<hashMap.buckets[j].count; k++){
				len = strlen(hashMap.buckets[j].values[k]);
				fwrite(&len, 1, 1, wfp);
				fwrite(hashMap.buckets[j].values[k], 1, len, wfp);
				free(hashMap.buckets[j].values[k]);
				id = hashMap.buckets[j].ids[k]-1;
				indices[id].bktIdx = j;
				indices[id].valIdx = k;
			}
			free(hashMap.buckets[j].ids);
			free(hashMap.buckets[j].values);
		}
		free(hashMap.codes);
		free(hashMap.buckets);
		fwrite(this->indices, SZ_INDEX, LAST_UNIQ_ID, wfp);
		free(indices);
	}
	fflush(wfp);
	fclose(wfp);
	unsigned long totalSize = c.getFileSize(this->filePath);
	//printf("idCount: %d; offset: %ld\n", idCount, offset);
	delete[] this->filePath;
	return totalSize;
}

void HashBucket16::load(char* dir, char* type){
	this->type = type;
	int pathLen = strlen(dir)+strlen(type)+8;
	this->filePath = new char[pathLen+1];
	sprintf(this->filePath, "%s/dic_%s.tt", dir, type);
	this->filePath[pathLen] = '\0';
	//cout << "LAST_UNIQ_ID: " << maxId << endl;
	FILE *rfp;
	rfp = fopen(this->filePath, "rb");
	unsigned char len;
	fread(&LAST_UNIQ_ID, sizeof(uint16), 1, rfp);
	if(LAST_UNIQ_ID>0){
		fread(&hashMap.count, SZ_ID, 1, rfp);
		//cout << hashMap.count << endl;
		hashMap.codes = (uint32*)c.xrealloc(hashMap.codes, SZ_HCODE*hashMap.count);
		fread(hashMap.codes, SZ_HCODE, hashMap.count, rfp);
		hashMap.buckets = (Bucket16*)c.xrealloc(hashMap.buckets, SZ_BUCKET*hashMap.count);
		for(int j=0; j<hashMap.count; j++){
			fread(&hashMap.buckets[j].count, SZ_ID, 1, rfp);
			//cout << hashMap.segs[i].buckets[j].count << endl;
			//cout << "Reading hashed values" << endl;
			hashMap.buckets[j].ids = (uint16*)malloc(SZ_ID*hashMap.buckets[j].count);
			fread(hashMap.buckets[j].ids, SZ_ID, hashMap.buckets[j].count, rfp);
			hashMap.buckets[j].values = (char**) malloc(SZ_VAL_PNTR*hashMap.buckets[j].count);
			for(int k=0; k<hashMap.buckets[j].count; k++){
				fread(&len, 1, 1, rfp);
				//cout << len << endl;
				hashMap.buckets[j].values[k] = (char*)malloc(len+1);
				fread(hashMap.buckets[j].values[k], 1, len, rfp);
				hashMap.buckets[j].values[k][len] = '\0';
				//cout << ((int)len) << ": " << hashMap.segs[i].buckets[j].values[k] << endl;
			}
		}
		indices = (Index16*)malloc(SZ_INDEX*LAST_UNIQ_ID);
		fread(indices, SZ_INDEX, LAST_UNIQ_ID, rfp);
	}
	fclose(rfp);
	//	for(int i=0; i<LAST_UNIQ_ID; i++){
	//		cout << id2Code[i].segIdx << " " << id2Code[i].bktIdx << " " << id2Code[i].valIdx << endl;
	//	}
}

void HashBucket16::close(){
	free(hashMap.codes);
	for(int j=0; j<hashMap.count; j++){
		free(hashMap.buckets[j].ids);
		for(int k=0; k<hashMap.buckets[j].count; k++){
			free(hashMap.buckets[j].values[k]);
		}
		free(hashMap.buckets[j].values);
	}
	free(this->hashMap.buckets);
	free(this->indices);
	delete[] this->filePath;
}

uint16 HashBucket16::getMaxID(){
	return LAST_UNIQ_ID;
}

uint32 HashBucket16::getMapSize(){
	return this->hashMap.count;
}

long HashBucket16::getSize(){
	return this->size/1024;
}

