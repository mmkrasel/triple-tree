/*
 * HashLiterals.h
 *
 *  Created on: Mar 15, 2023
 *      Author: rasel
 */

#ifndef HASHLITERALS_H_
#define HASHLITERALS_H_


#pragma pack(push, 1)

typedef struct INDEX_LTRL{
	uint32 bktIdx;
	uint32 valIdx;
}IndexLiteral;

typedef struct BUCKET_LTRL{
	uint32 count;
    uint32* ids;
    char** values;
}BucketLiteral;

typedef struct HASHMAP_LTRL{
	uint32 count;
    uint32* codes;
    BucketLiteral* buckets;
}HashMapLiteral;

#pragma pack(pop)




class HashLiterals {
public:
	short SZ_HCODE_L;
	short SZ_INDEX_L;
	short SZ_BUCKET_L;
	short SZ_ID_L;
	short SZ_VAL_PNTR_L;
	//short INCREASE_BY = 1;
	const long OVER_FLOW_L = 0xffffffff;
	std::string type;
	//
	char* filePath;
	long size = 0;
	HashMapLiteral hashMap;
	IndexLiteral* indices = NULL;
	uint32 LAST_UNIQ_ID;
	Common c;
	//
	HashLiterals();
	virtual ~HashLiterals();
	void init(char* dir, char* type);
	long binarySearchOnCodes(uint32 code);
	long binarySearchOnValues(uint32 bktIdx, const char *searhFor);
	uint32 add(const char* value);
	uint32 getId(const char* value);
	char* getValue(uint32 id);
	uint32 getMaxID();
	uint32 getMapSize();
	long getSize();
	unsigned long flushHash();
	void load(char* dir, char* type);
	void close();
};

#endif /* HASHLITERALS_H_ */
