/*
 * TripleTable.h
 *
 *  Created on: 23 Nov 2022
 *      Author: rasel
 */

#ifndef TRIPLETABLE_H_
#define TRIPLETABLE_H_

//#pragma pack(push, 1)

using namespace std;

class TripleTable {
public:
	char* ttFilePath;
	FILE *tripleFile;
	TripleItem* buf;
	TripleItem* cache;
	uint32 BUF_ITEM_LIMIT;
	uint32 BUF_INDEX;
	uint32 NUM_TRIPLES;
	Common c;
	//
	TripleTable();
	virtual ~TripleTable();
	void init(char* dir, bool isCreating);
	void add(uint32 subID, uint16 predID, uint32 objID);
	unsigned long flushBuffer();
	void load();
	void close();
	uint32 getNumOfTriples();
};

#endif /* TRIPLETABLE_H_ */
