/*
 * HashSegmented.h
 *
 *  Created on: Dec 3, 2022
 *      Author: rasel
 */

#ifndef HASHSEGMENTED_H_
#define HASHSEGMENTED_H_


using namespace std;

#pragma pack(push, 1)

typedef unsigned short SegmentItem;
typedef unsigned int IndexItem;
typedef unsigned int BucketItem;

typedef struct INDEX32{
	SegmentItem segIdx;
	SegmentItem bktIdx;
	SegmentItem valIdx;
}Index32;

typedef struct BUCKET{
	SegmentItem count;
    char** values;
    BucketItem* ids;
}Bucket;

typedef struct HASHSEGMENT{
	SegmentItem count;
	SegmentItem* keys;
    Bucket* buckets;
}HashSegment;

typedef struct HASHMAP{
    uint32 count;
    HashSegment* segs;
}HashMap;

#pragma pack(pop)


class HashSegmented {

public:
	char SZ_SEG_ITEM = sizeof(SegmentItem);
	char SZ_BUCKET_ITEM = sizeof(BucketItem);
	char SZ_HASHSEG = sizeof(HashSegment);
	char SZ_BUCKET = sizeof(Bucket);
	char SZ_HASH_INDEX = sizeof(Index32);
	char* filePath;
	bool isEnabled;
	int insertCount;
	Common c;
	int SEG_LENGTH = 65536;//0x0000ffff;
	int OVER_FLOW = 65536;//0x0000ffff;
	uint32 LAST_UNIQ_ID;
	HashMap hashMap;
	Index32* indices = NULL;
	long size = 0;
	//
	HashSegmented();
	virtual ~HashSegmented();
	void init(char* dir, char* type);
	unsigned long merge(uint32 proownId, uint16 pathId, uint16 refId);
	int binarySearchForIndex(SegmentItem* arr, int count, SegmentItem id);
	int binarySearchInStrings(char** list_of_words, int size, const char *target);
	uint32 add(const char* str);
	uint32 getId(const char* value);
	char* getValue(uint32 id);
	uint32 getMaxID();
	uint32 getMapSize();
	long getSize();
	unsigned long flushHash();
	void load(char* dir, char* type);
	void close();
};

#endif /* HASHSEGMENTED_H_ */
