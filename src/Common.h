/*
 * Common.h
 *
 *  Created on: 8 Nov 2022
 *      Author: rasel
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <chrono>
#include <boost/filesystem.hpp>



using namespace std;
namespace fs = boost::filesystem;

#pragma pack(push, 1)

typedef unsigned int SubObjId;
typedef unsigned short PredicateId;
typedef unsigned int TripleId;


typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned int PropertyID32;
//typedef unsigned int predicateid;
typedef unsigned char uint8;
//typedef unsigned int Key;
typedef unsigned int pageid;
typedef unsigned short offset;




typedef struct Triple_Item{
	PredicateId pred;
	SubObjId sub;
	SubObjId obj;
}TripleItem;

#pragma pack(pop)


class Common {

public:
	std::chrono::steady_clock::time_point begin;
	std::chrono::steady_clock::time_point end;
	int URI_MAX_LENGTH;
	short SZ_TRIPLE;
	short SZ_TRIPLE_ID;
	short SZ_SUB_OBJ_ID = sizeof(SubObjId);
	//
	Common();
	virtual ~Common();
	void* xrealloc(void *ptr, unsigned long size);
	long getFileSize(char path[]);
	long long int getFolderSize(std::string path);
	bool fileExists(char filename[]);
	void removeFile(char filePath[]);
	long diff(struct timespec start, struct timespec end);
	void startClock();
	double getElapsedTime();
	uint32 hashCode(const char* value);
	int parseLine(char* line);
	int getMemUsage();
	int binarySearchForIndex16(ushort* arr, int count, ushort id);
	long binarySearchForIndex32(uint32* arr, uint32 count, uint32 id);
};

#endif /* COMMON_H_ */
