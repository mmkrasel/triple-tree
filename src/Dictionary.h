/*
 * Dictionary.h
 *
 *  Created on: 9 Nov 2022
 *      Author: rasel
 */

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <boost/regex.hpp>
#include "TripleTable.h"
#include "ComponentTree.h"

using namespace std;


class Dictionary {
public:
	char* inputPath;
	char* pathDir;
	bool LOG;
	ComponentTree so, p;
	TripleTable tripleTable;
	Common common;
	char* uri = NULL;
	//
	Dictionary();
	virtual ~Dictionary();
	void stats();
	void create(char* inputPath, char* outputDirectory);
	ComponentId getSubObjId(char* uri);
	ComponentId getPredicateId(char* uri);
	char* getSubObj(ComponentId id);
	char* getPredicate(ComponentId id);
	void flushDictionary();
	void open(char* datasetPath);
	void close();
};

#endif /* DICTIONARY_H_ */
