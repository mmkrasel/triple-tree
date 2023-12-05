/*
 * HashBucket16.h
 *
 *  Created on: Dec 2, 2022
 *      Author: rasel
 */

#ifndef HASHBUCKET16_H_
#define HASHBUCKET16_H_

#pragma pack(push, 1)

typedef struct INDEX16{
	uint16 bktIdx;
	uint16 valIdx;
}Index16;

typedef struct BUCKET16{
	uint16 count;
    uint16* ids;
    char** values;
}Bucket16;

typedef struct HASHMAP16{
	uint16 count;
    uint32* codes;
    Bucket16* buckets;
}HashMap16;

#pragma pack(pop)


class HashBucket16 {
public:
	short SZ_HCODE;
	short SZ_INDEX;
	short SZ_BUCKET;
	short SZ_ID;
	short SZ_VAL_PNTR;
	//short INCREASE_BY = 1;
	const int OVER_FLOW = 65536;
	std::string type;
	//
	char* filePath;
	long size = 0;
	HashMap16 hashMap16;
	Index16* indices = NULL;
	uint16 LAST_UNIQ_ID;
	Common c;
	//
	HashBucket16();
	virtual ~HashBucket16();
	void init(char* dir, char* type);
	int binarySearchOnCodes(uint32 code);
	int binarySearchOnValues(uint16 bktIdx, const char *searhFor);
	uint16 add(const char* value);
	uint16 getId(const char* value);
	char* getValue(uint16 id);
	uint16 getMaxID();
	uint32 getMapSize();
	long getSize();
	unsigned long flushHash();
	void load(char* dir, char* type);
	void close();
};

#endif /* HASHBUCKET16_H_ */
