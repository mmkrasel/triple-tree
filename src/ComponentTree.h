/*
 * ComponentTree.h
 *
 *  Created on: Mar 8, 2023
 *      Author: rasel
 */

#ifndef COMPONENTTREE_H_
#define COMPONENTTREE_H_

#include <boost/regex.hpp>
#include "HashSegmented.h"
//#include "HashBucket16.h"
//#include "HashLiterals.h"
#include "TripleTable.h"

using namespace std;

#pragma pack(push, 1)

typedef unsigned int ProOwnId;
typedef unsigned short PathId;
typedef unsigned short RefId;

typedef struct PROOWNER_NODE {
	ProOwnId count = 0;
	ProOwnId* ids;// = (ProOwnId*)malloc(sizeof(ProOwnId));
	ComponentId* cids;// = (SubObjId*)malloc(sizeof(SubObjId));
}ProOwnerNode;

typedef struct PATH_NODE {
	PathId count = 0;
	PathId* ids;// = (PathId*)malloc(sizeof(PathId));
	ProOwnerNode* childs;// = (ProOwnerNode*) malloc(sizeof(ProOwnerNode));
}PathNode;

typedef struct TREE_INDEX{
	RefId refIndex;
	PathId pathIndex;
	ProOwnId proOwnIndex;
}TreeIndex;

typedef struct REF_NODE {
	RefId count = 0;
	RefId* ids;// = (RefId*)malloc(sizeof(RefId));
	PathNode* childs;// = (PathNode*)malloc(sizeof(PathNode));
	TreeIndex* indices = NULL;
} RefTree;

#pragma pack(pop)

class ComponentTree {
private:
	HashSegmented refH;
	HashSegmented pathH;
	HashSegmented proOwnH;
	HashSegmented lH;
public:
	boost::regex ex = boost::regex("(<http|<https|<file|<ftp)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\x3f?([^ #]*)#?([^ ]*)");
	boost::cmatch what;
	short SZ_REF_ID = sizeof(RefId);
	short SZ_PATH_ID = sizeof(PathId);
	short SZ_PROOWN_ID = sizeof(ProOwnId);
	short SZ_PATH_NODE = sizeof(PathNode);
	short SZ_PROOWN_NODE = sizeof(ProOwnerNode);
	bool LOG;

	RefTree tree;
	Common common;
	char* uri = NULL;
	ComponentId COMPONENT_COUNT = 0;
	//
	ComponentTree();
	virtual ~ComponentTree();
	void init(char* dir, char* type);
	ComponentId add2ComponentTree(RefId refId, PathId pathId, ProOwnId proOwnId);
	ComponentId encodeUri(const char* uri);
	ComponentId encodeLiteral(const char* literal);
	ComponentId getComponentId(char* uri);
	ComponentId getIdFromComponentTree(RefId refId, PathId pathId, ProOwnId proOwnId);
	char* getComponent(ComponentId id);
	unsigned long flush(char* pathDir, char* type);
	void open(char* datasetPath, char* type);
	void close();
	char* getStats();
};

#endif /* COMPONENTTREE_H_ */
