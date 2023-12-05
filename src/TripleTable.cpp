/*
 * TripleTable.cpp
 *
 *  Created on: 23 Nov 2022
 *      Author: rasel
 */

#include <boost/regex.hpp>
//#include <boost/algorithm/string.hpp>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include "Common.h"
#include "Dictionary.h"
#include "HashSegmented.h"
#include "TripleTable.h"

//#pragma pack(push, 1)


using namespace std;


TripleTable::TripleTable() {
	// TODO Auto-generated constructor stub
	this->NUM_TRIPLES = 0;
	this->BUF_ITEM_LIMIT = 1024*1024;
	this->BUF_INDEX = 0;
	this->buf = NULL;
	ttFilePath = NULL;
	tripleFile = NULL;
	cache = NULL;
}
TripleTable::~TripleTable() {}
void TripleTable::init(char* dir, bool isCreating){
    char* dataFile = (char*)malloc(512);
    sprintf(dataFile, "%s/triple_table.tt", dir);
	int pathLen = strlen(dataFile);
	this->ttFilePath = new char[pathLen+1];
	memcpy(this->ttFilePath, dataFile, pathLen);
	this->ttFilePath[pathLen] = '\0';
	free(dataFile);
	dataFile = NULL;
	if(isCreating == false){
		this->tripleFile = fopen(this->ttFilePath, "rb");
	} else {
		c.removeFile(this->ttFilePath);
		this->tripleFile = fopen(this->ttFilePath, "wb");
	}
	this->buf = (TripleItem*) malloc(c.SZ_TRIPLE*BUF_ITEM_LIMIT);
}
void TripleTable::add(ComponentId subID, ComponentId predID, ComponentId objID){
	this->NUM_TRIPLES++;
	this->buf[this->BUF_INDEX].sub = subID;
	this->buf[this->BUF_INDEX].pred = predID;
	this->buf[this->BUF_INDEX++].obj = objID;
	if(this->BUF_INDEX==this->BUF_ITEM_LIMIT){
		fwrite(this->buf, c.SZ_TRIPLE, this->BUF_INDEX, this->tripleFile);
		this->BUF_INDEX = 0;
	}
}
uint32 TripleTable::getNumOfTriples(){
	return this->NUM_TRIPLES;
}
unsigned long TripleTable::flushBuffer(){
	printf("#triples: %d\n", this->BUF_INDEX);
	if(this->BUF_INDEX>0){
		fwrite(this->buf, c.SZ_TRIPLE, this->BUF_INDEX, this->tripleFile);
		this->BUF_INDEX = 0;
	}
	//fflush(this->tripleFile);
	fclose(this->tripleFile);
	long fsize = c.getFileSize(this->ttFilePath);
	delete[] this->ttFilePath;
	free(this->buf);
	return fsize;
}
void TripleTable::load(){
	long fsize = c.getFileSize(this->ttFilePath);
	this->NUM_TRIPLES = fsize/c.SZ_TRIPLE;
	this->cache = (TripleItem*) malloc(c.SZ_TRIPLE*this->NUM_TRIPLES);
	fread(this->cache, c.SZ_TRIPLE, this->NUM_TRIPLES, this->tripleFile);
	fclose(this->tripleFile);
	delete[] this->ttFilePath;
	free(this->buf);
	//free(this->buf);
}
void TripleTable::close(){
	free(this->cache);
}
