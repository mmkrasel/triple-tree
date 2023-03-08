/*
 * Dictionary.cpp
 *
 *  Created on: 9 Nov 2022
 *      Author: rasel
 */

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "Common.h"
#include "HashSegmented.h"
#include "HashBucket16.h"
#include "TripleTable.h"
#include "Dictionary.h"

using namespace std;

Dictionary::Dictionary() {
	pathDir = NULL;
	inputPath = NULL;
	LOG = false;
}
Dictionary::~Dictionary() {}
void Dictionary::create(char* inputPath, char* outputDirectory) {
	int pathLen = strlen(outputDirectory);
	pathDir = new char[pathLen+1];
	memcpy(&this->pathDir[0], &outputDirectory[0], pathLen);
	pathDir[pathLen] = '\0';
	pathLen = strlen(inputPath);
	this->inputPath = new char[pathLen+1];
	memcpy(&this->inputPath[0], &inputPath[0], pathLen);
	this->inputPath[pathLen] = '\0';
	pathH.init(outputDirectory, "path");
	refH.init(outputDirectory, "ref");
	proOwnH.init(outputDirectory, "owner");
	pHash.init(outputDirectory, "predicate");
	tripleTable.init(outputDirectory, true);
	char** properties = (char**)malloc(sizeof(char*)*4);
	properties[0] = (char*) malloc(common.URI_MAX_LENGTH);
	properties[1] = (char*) malloc(common.URI_MAX_LENGTH);
	properties[2] = (char*) malloc(common.URI_MAX_LENGTH);
	properties[3] = (char*) malloc(common.URI_MAX_LENGTH);
	SubObjId subId=0, objId=0;
	PredicateId predId = 0;
	short uriCount = 0;
	TripleId tripleCount = 0;
	TripleId linesCount = 0;
	char c;
	short index = 0;
	bool doubleQouteOpen = false;
	common.startClock();
	ifstream is(inputPath);
	while(is.get(c)){
		switch(c){
			case '\n':
				if(uriCount == 3){
					subId = encodeSubObj(properties[0]);
					//printf("s %u=>%s\n", subID, properties[0]);
					//predID = this->encodePredicate(properties[1]);
					predId = pHash.add(properties[1]);
					//printf("p %u=>%s\n", predID, properties[1]);
					objId = encodeSubObj(properties[2]);
					//printf("o %u=>%s\n\n", objID, properties[2]);
					//if(subId==510){
					//	cout << subId << " " << predId << " " << objId << endl;
					//}
					tripleCount++;
					tripleTable.add(subId, predId, objId);
					if(tripleCount%10000000==0){
						cout << tripleCount/1000000 << ": " << common.getElapsedTime()/1000/1000/1000 << "\t";
						cout << "ref:" << refH.getMaxID() << "(" << refH.getSize() << ")\t" << "path:" << pathH.getMaxID() << "(" << pathH.getSize() << ")\t";
						cout << "proown:" << proOwnH.getMaxID() << "(" << proOwnH.getSize() << "\tpred:" << pHash.getMaxID() << "(" << pHash.getSize() << ")" << endl;
					}
				} else {
					cout << "@"<< uriCount << endl << properties[3] << endl;
				}
				doubleQouteOpen = false;
				linesCount++;
				uriCount = 0;
				index = 0;
				break;
			case '"':
				doubleQouteOpen = doubleQouteOpen ? false : true;
				break;
			case '.':
				if(uriCount < 3){
					if(index<common.URI_MAX_LENGTH){
						properties[uriCount][index++] = c;
					}
				}
			break;
			case '<': break;
			case '>': break;
			case ' ':
				if(doubleQouteOpen){
					if(index<common.URI_MAX_LENGTH){
						if(uriCount < 3){
							properties[uriCount][index++] = c;
						}
					}
				} else if (index>0){
					if(uriCount < 3){
						properties[uriCount][index] = '\0';
						index = 0;
						uriCount++;
					} else {
						if(index<common.URI_MAX_LENGTH){
							if(uriCount < 3){
								properties[uriCount][index++] = c;
							}
						}
					}
				}
				break;
			default:
				if(index<common.URI_MAX_LENGTH){
					if(uriCount < 3){
						properties[uriCount][index++] = c;
					}
				}
				break;
		}
	}
	is.close();
	cout << "#Lines: " << linesCount;
	cout << "; #Triples: " << tripleCount << endl;
	cout << "Elapsed Time: " << common.getElapsedTime() << endl;
	free(properties[0]);
	free(properties[1]);
	free(properties[2]);
	free(properties[3]);
	free(properties);
}
SubObjId Dictionary::add2SubObjTree(RefId refId, PathId pathId, ProOwnId proOwnId){
	int ri = common.binarySearchForIndex16(ref.ids, ref.count, refId);
	if(ri>=0xffff || ri < 0){
		ri = ri >= 0xffff ? 0 : -ri;
		if(ref.count > 0){
			ref.ids = (RefId*)common.xrealloc(ref.ids, SZ_REF_ID*(ref.count+1));
			ref.childs = (PathNode*)common.xrealloc(ref.childs, SZ_PATH_NODE*(ref.count+1));
			if(ri < ref.count) {
				memmove(&ref.ids[ri+1], &ref.ids[ri], 4*(ref.count-ri));
				memmove(&ref.childs[ri+1], &ref.childs[ri], SZ_PATH_NODE*(ref.count-ri));
			}
		}else{
			ref.ids = (RefId*)malloc(SZ_REF_ID);
			ref.childs = (PathNode*)malloc(SZ_PATH_NODE);
		}
        ref.count++;
        ref.ids[ri] = refId;
        ref.childs[ri].count = 0;
        ref.childs[ri].ids = (PathId*)malloc(SZ_PATH_ID);
        ref.childs[ri].childs = (ProOwnerNode*)malloc(SZ_PROOWN_NODE);
    }
	//cout << ri << " " << tree.rids[ri] << endl;
	int pai = common.binarySearchForIndex16(ref.childs[ri].ids, ref.childs[ri].count, pathId);
	if(pai>=0xffff || pai < 0){
		pai = pai >= 0xffff ? 0 : -pai;
		if(ref.childs[ri].count > 0){
			ref.childs[ri].ids = (PathId*)common.xrealloc(ref.childs[ri].ids, SZ_PATH_ID*(ref.childs[ri].count+1));
			ref.childs[ri].childs = (ProOwnerNode*)common.xrealloc(ref.childs[ri].childs, SZ_PROOWN_NODE*(ref.childs[ri].count+1));
			if(pai < ref.childs[ri].count) {
				memmove(&ref.childs[ri].ids[pai+1], &ref.childs[ri].ids[pai], SZ_PATH_ID*(ref.childs[ri].count-pai));
				memmove(&ref.childs[ri].childs[pai+1], &ref.childs[ri].childs[pai], SZ_PROOWN_NODE*(ref.childs[ri].count-pai));
			}
		}
        ref.childs[ri].count++;
        ref.childs[ri].ids[pai] = pathId;
        ref.childs[ri].childs[pai].count = 0;
        ref.childs[ri].childs[pai].ids = (ProOwnId*)malloc(SZ_PROOWN_ID);
        ref.childs[ri].childs[pai].soids = (SubObjId*)malloc(common.SZ_SUB_OBJ_ID);
    }
	long poi = common.binarySearchForIndex32(ref.childs[ri].childs[pai].ids, ref.childs[ri].childs[pai].count, proOwnId);
	if(poi>=0xffffffff || poi < 0){
		poi = poi >= 0xffffffff ? 0 : -poi;
		if(ref.childs[ri].childs[pai].count > 0){
			ref.childs[ri].childs[pai].ids = (ProOwnId*)common.xrealloc(ref.childs[ri].childs[pai].ids, SZ_PROOWN_ID*(ref.childs[ri].childs[pai].count+1));
			ref.childs[ri].childs[pai].soids = (uint32*)common.xrealloc(ref.childs[ri].childs[pai].soids, common.SZ_SUB_OBJ_ID*(ref.childs[ri].childs[pai].count+1));
			if(poi < ref.childs[ri].childs[pai].count) {
				memmove(&ref.childs[ri].childs[pai].ids[poi+1], &ref.childs[ri].childs[pai].ids[poi], SZ_PROOWN_ID*(ref.childs[ri].childs[pai].count-poi));
				memmove(&ref.childs[ri].childs[pai].soids[poi+1], &ref.childs[ri].childs[pai].soids[poi], common.SZ_SUB_OBJ_ID*(ref.childs[ri].childs[pai].count-poi));
			}
		}
        ref.childs[ri].childs[pai].count++;
        ref.childs[ri].childs[pai].ids[poi] = proOwnId;
        ref.childs[ri].childs[pai].soids[poi] = (++SUB_OBJ_COUNT);
    }
	return ref.childs[ri].childs[pai].soids[poi];
}
SubObjId Dictionary::encodeSubObj(char* uri) {
	try {
		if(boost::regex_match(uri, what, ex)) {
			string p1, p2, p3;
			//string protocol = string(what[1].first, what[1].second);
			p1 = string(what[1].first, what[3].second);
			p2 = string(what[4].first, what[5].second);
			p3 = string(what[6].first, what[6].second);
			uint32 ownID = 0;
			if(p1.length()>0){
				ownID = proOwnH.add(p1.data());
			}
			uint16 pathID = 0;
			if(p2.length()>0){
				pathID = pathH.add(p2.c_str());
			}
			uint16 refID = 0;
			if(p3.length()>0){
				refID = refH.add(p3.c_str());
			}
			return add2SubObjTree(refID, pathID, ownID);
		}
	}catch(out_of_range & e){}
     catch(runtime_error & e ){}
    return add2SubObjTree(0, 0, proOwnH.add(uri));
}
void Dictionary::flushDictionary(){
    char* tmp = new char[180];
	sprintf(tmp, "%s/dic_tree.tt", pathDir);
	FILE* fileStream = fopen(tmp, "wb");
	//SubObjId ti;
	//cout << SUB_OBJ_COUNT << endl;
	//treeIndices = (TreeIndex*)malloc(sizeof(TreeIndex)*SUB_OBJ_COUNT);
	fwrite(&SUB_OBJ_COUNT, common.SZ_SUB_OBJ_ID, 1, fileStream);
	fwrite(&ref.count, SZ_REF_ID, 1, fileStream);
	fwrite(ref.ids, SZ_REF_ID, ref.count, fileStream);
	for(RefId ri=0; ri<ref.count; ri++){
		fwrite(&ref.childs[ri].count, SZ_PATH_ID, 1, fileStream);
		fwrite(ref.childs[ri].ids, SZ_PATH_ID, ref.childs[ri].count, fileStream);
		for(PathId pi=0; pi<ref.childs[ri].count; pi++){
			fwrite(&ref.childs[ri].childs[pi].count, SZ_PROOWN_ID, 1, fileStream);
			fwrite(ref.childs[ri].childs[pi].ids, SZ_PROOWN_ID, ref.childs[ri].childs[pi].count, fileStream);
			fwrite(ref.childs[ri].childs[pi].soids, common.SZ_SUB_OBJ_ID, ref.childs[ri].childs[pi].count, fileStream);
			free(ref.childs[ri].childs[pi].ids);
			free(ref.childs[ri].childs[pi].soids);
		}
		free(ref.childs[ri].childs);
		free(ref.childs[ri].ids);
	}
	free(ref.childs);
	free(ref.ids);
	//fwrite(&SUB_OBJ_COUNT, SZ_SUBOBJ_ID, 1, fileStream);
	//fwrite(treeIndices, sizeof(TreeIndex), SUB_OBJ_COUNT, fileStream);
	//free(treeIndices);
	fclose(fileStream);
	unsigned long dicSize = common.getFileSize(tmp);
	delete[] tmp;
	dicSize += pathH.flushHash();
	dicSize += refH.flushHash();
	dicSize += proOwnH.flushHash();
	dicSize += pHash.flushHash();
	unsigned long ttSize = tripleTable.flushBuffer();
	unsigned long inSize = common.getFileSize(inputPath);
	printf("Dictionary: %lu\n", dicSize);
	printf("TriplTable: %lu\n", ttSize);
	printf("Dic+TTable: %lu\n", dicSize+ttSize);
	printf("Input Size: %lu\n", inSize);
	printf("Ratio IvsO: %f\n", (dicSize+ttSize)*100.0/inSize);
	delete[] this->pathDir;
	delete[] this->inputPath;
}
void Dictionary::open(char* datasetPath){
	pHash.load(datasetPath, "predicate");
	pathH.load(datasetPath, "path");
	refH.load(datasetPath, "ref");
	proOwnH.load(datasetPath, "owner");
	uri = new char[512];
	SubObjId ti;
    char* tmp = (char*)malloc(180);
	sprintf(tmp, "%s/dic_tree.tt", datasetPath);
	FILE* fileStream = fopen(tmp, "r");
	free(tmp);
	fread(&SUB_OBJ_COUNT, common.SZ_SUB_OBJ_ID, 1, fileStream);
	treeIndices = (TreeIndex*)malloc(sizeof(TreeIndex)*SUB_OBJ_COUNT);
	fread(&ref.count, SZ_REF_ID, 1, fileStream);
	ref.ids = (RefId*)malloc(SZ_REF_ID*ref.count);
	fread(ref.ids, SZ_REF_ID, ref.count, fileStream);
	ref.childs = (PathNode*)malloc(SZ_PATH_NODE*ref.count);
	for(RefId ri=0; ri<ref.count; ri++){
		fread(&ref.childs[ri].count, SZ_PATH_ID, 1, fileStream);
		ref.childs[ri].ids = (PathId*) malloc(SZ_PATH_ID*ref.childs[ri].count);
		fread(ref.childs[ri].ids, SZ_PATH_ID, ref.childs[ri].count, fileStream);
		ref.childs[ri].childs = (ProOwnerNode*) malloc(SZ_PROOWN_NODE*ref.childs[ri].count);
		for(PathId pi=0; pi<ref.childs[ri].count; pi++){
			fread(&ref.childs[ri].childs[pi].count, SZ_PROOWN_ID, 1, fileStream);
			ref.childs[ri].childs[pi].ids = (ProOwnId*) malloc(SZ_PROOWN_ID*ref.childs[ri].childs[pi].count);
			fread(ref.childs[ri].childs[pi].ids, SZ_PROOWN_ID, ref.childs[ri].childs[pi].count, fileStream);
			ref.childs[ri].childs[pi].soids = (SubObjId*) malloc(common.SZ_SUB_OBJ_ID*ref.childs[ri].childs[pi].count);
			fread(ref.childs[ri].childs[pi].soids, common.SZ_SUB_OBJ_ID, ref.childs[ri].childs[pi].count, fileStream);
			for(SubObjId poi=0; poi<ref.childs[ri].childs[pi].count; poi++){
				ti = ref.childs[ri].childs[pi].soids[poi]-1;
				treeIndices[ti].refIndex = ri;
				treeIndices[ti].pathIndex = pi;
				treeIndices[ti].proOwnIndex = poi;
			}
		}
	}
	//fread(&SUB_OBJ_COUNT, SZ_SUBOBJ_ID, 1, fileStream);
	//treeIndices = (TreeIndex*)malloc(sizeof(TreeIndex)*SUB_OBJ_COUNT);
	//fread(treeIndices, sizeof(TreeIndex), SUB_OBJ_COUNT, fileStream);
	fclose(fileStream);
}
SubObjId Dictionary::getIdFromSubObjTree(RefId refId, PathId pathId, ProOwnId proOwnId){
	if((proOwnId|pathId|refId)==0){
		return 0;
	}
	int ri = common.binarySearchForIndex16(ref.ids, ref.count, refId);
	if(ri>=0xffff || ri < 0){
		return 0;
    }
	int pai = common.binarySearchForIndex16(ref.childs[ri].ids, ref.childs[ri].count, pathId);
	if(pai>=0xffff || pai < 0){
		return 0;
    }
	long poi = common.binarySearchForIndex32(ref.childs[ri].childs[pai].ids, ref.childs[ri].childs[pai].count, proOwnId);
	if(poi>=0xffffffff || poi < 0){
		return 0;
    }
	//cout << "@getIdFromSubObjTree: " << poi << " " << pai << " " << ri << endl;
	return ref.childs[ri].childs[pai].soids[poi];
}
SubObjId Dictionary::getSubObjId(char* uri){
	string p1, p2, p3;
	//cout << endl << endl << "@getSubObjId: " << uri << endl;
	try {
		if(boost::regex_match(uri, what, ex)) {
			string protocol = string(what[1].first, what[1].second);
			p1 = string(what[1].first, what[3].second);
			p2 = string(what[4].first, what[5].second);
			p3 = string(what[6].first, what[6].second);
			//cout << "@getSubObjId: " << p1 << " " << p2 << " " << p3 <<endl;
			ProOwnId ownID = 0;
			if(p1.length()>0){
				ownID = proOwnH.getId(p1.c_str());
			}
			PathId pathID = 0;
			if(p2.length()>0){
				pathID = pathH.getId(p2.c_str());
			}
			RefId refID = 0;
			if(p3.length()>0){
				refID = refH.getId(p3.c_str());
			}
			//cout << "@getSubObjId: " << ownID << " " << pathID << " " << refID <<endl;
			return getIdFromSubObjTree(refID, pathID, ownID);
		}
	}catch(out_of_range & e){}
	 catch(runtime_error & e ){}
	 return getIdFromSubObjTree(0, 0, proOwnH.getId(uri));
}
char* Dictionary::getSubObj(SubObjId uid){
	//cout << "url : " << uid << " " << SUB_OBJ_COUNT << endl;
	if(uid < 0 || uid > SUB_OBJ_COUNT){
		return 0;
	}
	uid--;
	RefId ri = treeIndices[uid].refIndex;
	PathId pai = treeIndices[uid].pathIndex;
	SubObjId poi = treeIndices[uid].proOwnIndex;
    RefId refID = ref.ids[ri];
    PathId pathID = ref.childs[ri].ids[pai];
    ProOwnId ownerID = ref.childs[ri].childs[pai].ids[poi];

    //cout << "@getSubObj: " << uid << " " << ownerID << " " << pathID << " " << refID << endl;

    if(refID == 0 && pathID == 0){
		return proOwnH.getValue(ownerID);
	}
    //char* str = new char[512];
    int len = 0;
    if(ownerID != 0){
    	char* tmp = proOwnH.getValue(ownerID);
    	len = strlen(tmp);
    	memcpy(&uri[0], &tmp[0], len);
    }
    if(pathID != 0){
    	char* tmp = pathH.getValue(pathID);
    	int l = strlen(tmp);
    	memcpy(&uri[len], &tmp[0], l);
    	len += l;
    }
    if(refID != 0){
    	uri[len++] = '#';
    	char* tmp = refH.getValue(refID);
		int l = strlen(tmp);
		memcpy(&uri[len], &tmp[0], l);
		len += l;
    }
    uri[len] = '\0';
    //cout << "@getSubObj: " << uid << " " << proOwnH.getValue(ownerID) << " " << pathH.getValue(pathID) << " " << refH.getValue(refID) << endl;
    return uri;
}
char* Dictionary::getPredicate(PredicateId uid){
	return pHash.getValue(uid);
}
PredicateId Dictionary::getPredicateId(char* uri){
	return pHash.getId(uri);
}
void Dictionary::close(){
	proOwnH.close();
	pathH.close();
	refH.close();
	pHash.close();
	tripleTable.close();
	for(RefId ri=0; ri<ref.count; ri++){
		for(PathId pi=0; pi<ref.childs[ri].count; pi++){
			free(ref.childs[ri].childs[pi].ids);
			free(ref.childs[ri].childs[pi].soids);
		}
		free(ref.childs[ri].childs);
		free(ref.childs[ri].ids);
	}
	free(ref.childs);
	free(ref.ids);
	free(treeIndices);
	delete[] uri;
}

//string Dictionary::getTriple(TripleId id){
//
//
////	Key* ids = tripleTable.getUriIDs(id);
////	cout << ids[0] << " " << ids[1] << " " << ids[2] << endl;
//
//	stringstream urlStr;
////	urlStr << getStringProperty(ids[0], false) << " " << getStringProperty(ids[1], true) << " " << getStringProperty(ids[2], false) << endl;
////
//////	Key* ids = tripleTable.getUriIDs(id);
//////	cout << ids[0] << " " << ids[1] << " " << ids[2] << endl;
//////	stringstream urlStr;
//////	urlStr << (ids[0] < 0 ? literalH.getUriProperty(-ids[0]) : getStringProperty(soHash.getUriProperty(ids[0]))) << " ";
//////	urlStr << (ids[1] < 0 ? literalH.getUriProperty(-ids[1]) : getStringProperty(pHash.getUriProperty(ids[1]))) << " ";
//////	urlStr << (ids[2] < 0 ? literalH.getUriProperty(-ids[2]) : getStringProperty(soHash.getUriProperty(ids[2]))) << endl;
////
////	delete ids;
//	return urlStr.str();
//}
