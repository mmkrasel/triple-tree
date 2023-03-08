/*
 * Dictionary.h
 *
 *  Created on: 9 Nov 2022
 *      Author: rasel
 */

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <boost/regex.hpp>
#include "HashSegmented.h"
#include "HashBucket16.h"
#include "TripleTable.h"

using namespace std;

#pragma pack(push, 1)

typedef unsigned int ProOwnId;
typedef unsigned short PathId;
typedef unsigned short RefId;

typedef struct PROOWNER_NODE {
	ProOwnId count = 0;
	ProOwnId* ids;// = (ProOwnId*)malloc(sizeof(ProOwnId));
	SubObjId* soids;// = (SubObjId*)malloc(sizeof(SubObjId));
}ProOwnerNode;

typedef struct PATH_NODE {
	PathId count = 0;
	PathId* ids;// = (PathId*)malloc(sizeof(PathId));
	ProOwnerNode* childs;// = (ProOwnerNode*) malloc(sizeof(ProOwnerNode));
}PathNode;

typedef struct REF_NODE {
	RefId count = 0;
	RefId* ids;// = (RefId*)malloc(sizeof(RefId));
	PathNode* childs;// = (PathNode*)malloc(sizeof(PathNode));
} Ref;

typedef struct TREE_INDEX{
	RefId refIndex;
	PathId pathIndex;
	ProOwnId proOwnIndex;
}TreeIndex;

#pragma pack(pop)


class Dictionary {
public:
	char* inputPath;
	char* pathDir;
	boost::regex ex = boost::regex("(http|https|file)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\x3f?([^ #]*)#?([^ ]*)");
	boost::cmatch what;
	short SZ_REF_ID = sizeof(RefId);
	short SZ_PATH_ID = sizeof(PathId);
	short SZ_PROOWN_ID = sizeof(ProOwnId);
	short SZ_PATH_NODE = sizeof(PathNode);
	short SZ_PROOWN_NODE = sizeof(ProOwnerNode);
	SubObjId SUB_OBJ_COUNT = 0;
	bool LOG;
	HashBucket16 pHash;
	HashBucket16 refH;
	HashBucket16 pathH;
	HashSegmented proOwnH;
	Ref ref;
	TreeIndex* treeIndices = NULL;
	TripleTable tripleTable;
	Common common;
	char* uri = NULL;
	//
	Dictionary();
	virtual ~Dictionary();
	void create(char* inputPath, char* outputDirectory);
	SubObjId add2SubObjTree(RefId refId, PathId pathId, ProOwnId proOwnId);
	SubObjId encodeSubObj(char* uri);
	SubObjId getSubObjId(char* uri);
	SubObjId getIdFromSubObjTree(RefId refId, PathId pathId, ProOwnId proOwnId);
	PredicateId getPredicateId(char* uri);
	void flushDictionary();
	void open(char* datasetPath);
	void close();
	char* getSubObj(SubObjId id);
	char* getPredicate(PredicateId id);
	//string getTriple(TripleId id);
	//string _trim(const string& str);
};

#endif /* DICTIONARY_H_ */
