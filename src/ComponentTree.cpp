/*
 * ComponentTree.cpp
 *
 *  Created on: Mar 8, 2023
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
//#include "HashBucket16.h"
//#include "HashLiterals.h"
#include "TripleTable.h"
#include "ComponentTree.h"

ComponentTree::ComponentTree() {
	LOG = false;
}

ComponentTree::~ComponentTree() {
	// TODO Auto-generated destructor stub
}

void ComponentTree::init(char* dir, char* type){
	if(type[0]=='p'){
		pathH.init(dir, (char*)"p_path");
		refH.init(dir, (char*)"p_ref");
		proOwnH.init(dir, (char*)"p_owner");
		//lH.init(dir, (char*)"p_ltrl");
	}else {
		pathH.init(dir, (char*)"so_path");
		refH.init(dir, (char*)"so_ref");
		proOwnH.init(dir, (char*)"so_owner");
		lH.init(dir, (char*)"so_ltrl");
	}
}

ComponentId ComponentTree::add2ComponentTree(RefId refId, PathId pathId, ProOwnId proOwnId){
//	if(Common::getLog()){
//		cout << "@ComponentTree::add2ComponentTree 1" << endl;
//	}
	int ri = common.binarySearchForIndex16(tree.ids, tree.count, refId);
	if(ri>=0xffff || ri < 0){
		ri = ri >= 0xffff ? 0 : -ri;
//		if(Common::getLog()){
//			cout << "@ComponentTree::add2ComponentTree 2" << endl;
//		}
		if(tree.count > 0){
			tree.ids = (RefId*)common.xrealloc(tree.ids, SZ_REF_ID*(tree.count+1), (char*)"ComponentTree::add2ComponentTree 1");
			tree.childs = (PathNode*)common.xrealloc(tree.childs, SZ_PATH_NODE*(tree.count+1), (char*)"ComponentTree::add2ComponentTree 2");
			if(ri < tree.count) {
				memmove(&tree.ids[ri+1], &tree.ids[ri], SZ_REF_ID*(tree.count-ri));
				memmove(&tree.childs[ri+1], &tree.childs[ri], SZ_PATH_NODE*(tree.count-ri));
			}
		}else{
			tree.ids = (RefId*)malloc(SZ_REF_ID);
			tree.childs = (PathNode*)malloc(SZ_PATH_NODE);
		}
//		if(Common::getLog()){
//			cout << "@ComponentTree::add2ComponentTree 2 1" << endl;
//		}
        tree.count++;
        tree.ids[ri] = refId;
        tree.childs[ri].count = 0;
        tree.childs[ri].ids = (PathId*)malloc(SZ_PATH_ID);
        tree.childs[ri].childs = (ProOwnerNode*)malloc(SZ_PROOWN_NODE);
    }
	//cout << ri << " " << tree.rids[ri] << endl;
	int pai = common.binarySearchForIndex16(tree.childs[ri].ids, tree.childs[ri].count, pathId);
	if(pai>=0xffff || pai < 0){
		pai = pai >= 0xffff ? 0 : -pai;
//		if(Common::getLog()){
//			cout << "@ComponentTree::add2ComponentTree 3" << endl;
//		}
		if(tree.childs[ri].count > 0){
			tree.childs[ri].ids = (PathId*)common.xrealloc(tree.childs[ri].ids, SZ_PATH_ID*(tree.childs[ri].count+1), (char*)"ComponentTree::add2ComponentTree 3");
			tree.childs[ri].childs = (ProOwnerNode*)common.xrealloc(tree.childs[ri].childs, SZ_PROOWN_NODE*(tree.childs[ri].count+1), (char*)"ComponentTree::add2ComponentTree 4");
			if(pai < tree.childs[ri].count) {
				memmove(&tree.childs[ri].ids[pai+1], &tree.childs[ri].ids[pai], SZ_PATH_ID*(tree.childs[ri].count-pai));
				memmove(&tree.childs[ri].childs[pai+1], &tree.childs[ri].childs[pai], SZ_PROOWN_NODE*(tree.childs[ri].count-pai));
			}
		}
//		if(Common::getLog()){
//			cout << "@ComponentTree::add2ComponentTree 3 1" << endl;
//		}
        tree.childs[ri].count++;
        tree.childs[ri].ids[pai] = pathId;
        tree.childs[ri].childs[pai].count = 0;
        tree.childs[ri].childs[pai].ids = (ProOwnId*)malloc(SZ_PROOWN_ID);
        tree.childs[ri].childs[pai].cids = (ComponentId*)malloc(common.SZ_COMPONENT_ID);
    }
	long poi = common.binarySearchForIndex32(tree.childs[ri].childs[pai].ids, tree.childs[ri].childs[pai].count, proOwnId);
	if(poi>=0xffffffff || poi < 0){
		poi = poi >= 0xffffffff ? 0 : -poi;
//		if(Common::getLog()){
//			cout << "@ComponentTree::add2ComponentTree 3" << endl;
//		}
		if(tree.childs[ri].childs[pai].count > 0){
			tree.childs[ri].childs[pai].ids = (ProOwnId*)common.xrealloc(tree.childs[ri].childs[pai].ids, SZ_PROOWN_ID*(tree.childs[ri].childs[pai].count+1), (char*)"ComponentTree::add2ComponentTree 5");
			tree.childs[ri].childs[pai].cids = (ComponentId*)common.xrealloc(tree.childs[ri].childs[pai].cids, common.SZ_COMPONENT_ID*(tree.childs[ri].childs[pai].count+1), (char*)"ComponentTree::add2ComponentTree 6");
			if(poi < tree.childs[ri].childs[pai].count) {
				memmove(&tree.childs[ri].childs[pai].ids[poi+1], &tree.childs[ri].childs[pai].ids[poi], SZ_PROOWN_ID*(tree.childs[ri].childs[pai].count-poi));
				memmove(&tree.childs[ri].childs[pai].cids[poi+1], &tree.childs[ri].childs[pai].cids[poi], common.SZ_COMPONENT_ID*(tree.childs[ri].childs[pai].count-poi));
			}
		}
//		if(Common::getLog()){
//			cout << "@ComponentTree::add2ComponentTree 3 1" << endl;
//		}
        tree.childs[ri].childs[pai].count++;
        tree.childs[ri].childs[pai].ids[poi] = proOwnId;
        tree.childs[ri].childs[pai].cids[poi] = (++COMPONENT_COUNT);
    }
	return tree.childs[ri].childs[pai].cids[poi];
}












ComponentId ComponentTree::encodeUri(const char* uri) {
//	if(Common::getLog()){
		// cout << "@ComponentTree::encodeUri" << endl;
//	}
	string s = uri;
	boost::algorithm::trim_right(s);
	boost::algorithm::trim_left(s);
	try {
		if(boost::regex_match(s.c_str(), what, ex)) {
			string p1, p2, p3;
			//string protocol = string(what[1].first, what[1].second);
			p1 = string(what[1].first, what[3].second);
			p2 = string(what[4].first, what[5].second);
			p3 = string(what[6].first, what[6].second);

			// cout << "2 @getSubObjId: " << p1 << " " << p2 << " " << p3 <<endl;

			uint32 ownID = 0;
//			if(Common::getLog()){
//				cout << "@ComponentTree::encodeUri 1" << endl;
//			}
			if(p1.length()> 0){
				ownID = proOwnH.add(p1.data());
			}
//			if(Common::getLog()){
//				cout << "@ComponentTree::encodeUri 2" << endl;
//			}
			uint32 pathID = 0;
			if(p2.length()>0){
				pathID = pathH.add(p2.c_str());
			}
			uint32 refID = 0;
//			if(Common::getLog()){
//				cout << "@ComponentTree::encodeUri 3" << endl;
//			}
			if(p3.length()>0){
				refID = refH.add(p3.c_str());
			}
			uint32 id = add2ComponentTree(refID, pathID, ownID);
			// cout << "@encodeUri (in): **" << s.c_str() << "**" << refID << " " << pathID << " " << ownID << " " << id <<endl;
			return id;
		}
	}catch(out_of_range & e){}
     catch(runtime_error & e ){}
    //cout << "@ComponentTree::encodeComponent: " << uri << endl;
// 	if(Common::getLog()){
// 		cout << "@ComponentTree::encodeUri 4" << endl;
// 	}
	uint32 id = add2ComponentTree(0, 0, lH.add(uri));
	// cout << "@encodeUri (out): **" << uri << " " << id <<endl;
    return id;
}
ComponentId ComponentTree::encodeLiteral(const char* literal) {

	string s = literal;
	boost::algorithm::trim_right(s);
	boost::algorithm::trim_left(s);
	if (literal[0]=='"' && literal[1]=='<') {
		s.erase(remove(s.begin(), s.end(), '"'), s.end());
		boost::algorithm::trim_right(s);
		ComponentId id = encodeUri(s.c_str());
		//cout << "@encodeLiteral (in): **" << s.c_str() << "**" << id <<endl;
		return id;
		// uri = s.c_str();
	}
	uint32 id = lH.add(s.c_str());
	// cout << "@encodeLiteral(out): ***" << s.c_str() << "***" << id << endl;
    return add2ComponentTree(0, 0, id);
}












ComponentId ComponentTree::getIdFromComponentTree(RefId refId, PathId pathId, ProOwnId proOwnId){
	if((proOwnId|pathId|refId)==0){
		return 0;
	}
	int ri = common.binarySearchForIndex16(tree.ids, tree.count, refId);
	if(ri>=0xffff || ri < 0){
		return 0;
    }
	int pai = common.binarySearchForIndex16(tree.childs[ri].ids, tree.childs[ri].count, pathId);
	if(pai>=0xffff || pai < 0){
		return 0;
    }
	long poi = common.binarySearchForIndex32(tree.childs[ri].childs[pai].ids, tree.childs[ri].childs[pai].count, proOwnId);
	if(poi>=0xffffffff || poi < 0){
		return 0;
    }
	//cout << "@getIdFromSubObjTree: " << poi << " " << pai << " " << ri << endl;
	return tree.childs[ri].childs[pai].cids[poi];
}
ComponentId ComponentTree::getComponentId(char* uri){
	
	string s = uri;
	if (s.rfind("http://", 0) == 0 || s.rfind("https://", 0) == 0 || s.rfind("ftp://", 0) == 0 || s.rfind("file://", 0) == 0) {
		s = "<" + s + ">";
		//cout << s << endl;
		// uri = s.c_str();
	}
	//cout << endl << endl << "1 @getSubObjId: " << uri << endl;
	try {
		if(boost::regex_match(s.c_str(), what, ex)) {
			string p1, p2, p3;
			//string protocol = string(what[1].first, what[1].second);
			//cout << "2.1 @getSubObjId: " << "" <<endl;
			p1 = string(what[1].first, what[3].second);
			//cout << "2.2 @getSubObjId: " << p1 <<endl;
			p2 = string(what[4].first, what[5].second);
			//cout << "2.3 @getSubObjId: " << p2 << " " << p3 <<endl;
			p3 = string(what[6].first, what[6].second);
			//cout << "2 @getSubObjId: " << p3 <<endl;
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
			ComponentId id = getIdFromComponentTree(refID, pathID, ownID);
			// cout << "3 @getSubObjId: " << ownID << " " << pathID << " " << refID << " " << id << endl;
			return id;
		}
	}catch(out_of_range & e){ cerr << e.what() << endl; }  
	 catch(runtime_error & e ){ cerr << e.what() << endl; }

	 return getIdFromComponentTree(0, 0, lH.getId(uri));
}
char* ComponentTree::getComponent(ComponentId uid){
	//cout << "url : " << uid << " " << SUB_OBJ_COUNT << endl;
	if(uid < 0 || uid > COMPONENT_COUNT){
		return 0;
	}
	uid--;
	RefId ri = tree.indices[uid].refIndex;
	PathId pai = tree.indices[uid].pathIndex;
	ComponentId poi = tree.indices[uid].proOwnIndex;
    RefId refID = tree.ids[ri];
    PathId pathID = tree.childs[ri].ids[pai];
    ProOwnId ownerID = tree.childs[ri].childs[pai].ids[poi];

    // cout << "\n@ComponentTree::getComponent: " << uid << " " << ownerID << " " << pathID << " " << refID << endl;

    if(refID == 0 && pathID == 0){
		return lH.getValue(ownerID);
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
    // cout << "@getSubObj: " << uid << " " << proOwnH.getValue(ownerID) << " " << pathH.getValue(pathID) << " " << refH.getValue(refID) << endl;
    return uri;
}

unsigned long ComponentTree::flush(char* pathDir, char* type){
    char* tmp = new char[180];
	sprintf(tmp, "%s/dic_%s_tree.tt", pathDir, type);
	FILE* fileStream = fopen(tmp, "wb");
	//SubObjId ti;
	//cout << SUB_OBJ_COUNT << endl;
	//treeIndices = (TreeIndex*)malloc(sizeof(TreeIndex)*SUB_OBJ_COUNT);
	fwrite(&COMPONENT_COUNT, common.SZ_COMPONENT_ID, 1, fileStream);
	fwrite(&tree.count, SZ_REF_ID, 1, fileStream);
	fwrite(tree.ids, SZ_REF_ID, tree.count, fileStream);
	for(RefId ri=0; ri<tree.count; ri++){
		fwrite(&tree.childs[ri].count, SZ_PATH_ID, 1, fileStream);
		fwrite(tree.childs[ri].ids, SZ_PATH_ID, tree.childs[ri].count, fileStream);
		for(PathId pi=0; pi<tree.childs[ri].count; pi++){
			fwrite(&tree.childs[ri].childs[pi].count, SZ_PROOWN_ID, 1, fileStream);
			fwrite(tree.childs[ri].childs[pi].ids, SZ_PROOWN_ID, tree.childs[ri].childs[pi].count, fileStream);
			fwrite(tree.childs[ri].childs[pi].cids, common.SZ_COMPONENT_ID, tree.childs[ri].childs[pi].count, fileStream);
			free(tree.childs[ri].childs[pi].ids);
			free(tree.childs[ri].childs[pi].cids);
		}
		free(tree.childs[ri].childs);
		free(tree.childs[ri].ids);
	}
	free(tree.childs);
	free(tree.ids);
	//fwrite(&SUB_OBJ_COUNT, SZ_SUBOBJ_ID, 1, fileStream);
	//fwrite(treeIndices, sizeof(TreeIndex), SUB_OBJ_COUNT, fileStream);
	//free(treeIndices);
	fclose(fileStream);
	unsigned long dicSize = common.getFileSize(tmp);
	delete[] tmp;
	dicSize += pathH.flushHash();
	dicSize += refH.flushHash();
	dicSize += proOwnH.flushHash();
	if(type[0]!='p'){
		dicSize += lH.flushHash();
	}
	return dicSize;
}
void ComponentTree::open(char* datasetPath, char* type){


	//printf("%s %s\n", datasetPath, type);

	if(type[0]=='p'){
		pathH.load(datasetPath, (char*)"p_path");
		refH.load(datasetPath, (char*)"p_ref");
		proOwnH.load(datasetPath, (char*)"p_owner");
		//lH.load(datasetPath, (char*)"p_ltrl");
	}else{
		pathH.load(datasetPath, (char*)"so_path");
		refH.load(datasetPath, (char*)"so_ref");
		proOwnH.load(datasetPath, (char*)"so_owner");
		lH.load(datasetPath, (char*)"so_ltrl");
	}
	uri = new char[512];
	ComponentId ti;
    char* tmp = (char*)malloc(180);
	sprintf(tmp, "%s/dic_%s_tree.tt", datasetPath, type);


	//printf("%s\n", tmp);


	FILE* fileStream = fopen(tmp, "r");
	free(tmp);
	fread(&COMPONENT_COUNT, common.SZ_COMPONENT_ID, 1, fileStream);
	tree.indices = (TreeIndex*)malloc(sizeof(TreeIndex)*COMPONENT_COUNT);
	fread(&tree.count, SZ_REF_ID, 1, fileStream);
	tree.ids = (RefId*)malloc(SZ_REF_ID*tree.count);
	fread(tree.ids, SZ_REF_ID, tree.count, fileStream);
	tree.childs = (PathNode*)malloc(SZ_PATH_NODE*tree.count);
	for(RefId ri=0; ri<tree.count; ri++){
		fread(&tree.childs[ri].count, SZ_PATH_ID, 1, fileStream);
		tree.childs[ri].ids = (PathId*) malloc(SZ_PATH_ID*tree.childs[ri].count);
		fread(tree.childs[ri].ids, SZ_PATH_ID, tree.childs[ri].count, fileStream);
		tree.childs[ri].childs = (ProOwnerNode*) malloc(SZ_PROOWN_NODE*tree.childs[ri].count);
		for(PathId pi=0; pi<tree.childs[ri].count; pi++){
			fread(&tree.childs[ri].childs[pi].count, SZ_PROOWN_ID, 1, fileStream);
			tree.childs[ri].childs[pi].ids = (ProOwnId*) malloc(SZ_PROOWN_ID*tree.childs[ri].childs[pi].count);
			fread(tree.childs[ri].childs[pi].ids, SZ_PROOWN_ID, tree.childs[ri].childs[pi].count, fileStream);
			tree.childs[ri].childs[pi].cids = (ComponentId*) malloc(common.SZ_COMPONENT_ID*tree.childs[ri].childs[pi].count);
			fread(tree.childs[ri].childs[pi].cids, common.SZ_COMPONENT_ID, tree.childs[ri].childs[pi].count, fileStream);
			for(ComponentId poi=0; poi<tree.childs[ri].childs[pi].count; poi++){
				ti = tree.childs[ri].childs[pi].cids[poi]-1;
				tree.indices[ti].refIndex = ri;
				tree.indices[ti].pathIndex = pi;
				tree.indices[ti].proOwnIndex = poi;
			}
		}
	}
	//fread(&SUB_OBJ_COUNT, SZ_SUBOBJ_ID, 1, fileStream);
	//treeIndices = (TreeIndex*)malloc(sizeof(TreeIndex)*SUB_OBJ_COUNT);
	//fread(treeIndices, sizeof(TreeIndex), SUB_OBJ_COUNT, fileStream);
	fclose(fileStream);
}

void ComponentTree::close(){
	proOwnH.close();
	pathH.close();
	refH.close();
	lH.close();
//	for(RefId ri=0; ri<tree.count; ri++){
//		for(PathId pi=0; pi<tree.childs[ri].count; pi++){
//			free(tree.childs[ri].childs[pi].ids);
//			free(tree.childs[ri].childs[pi].cids);
//		}
//		free(tree.childs[ri].childs);
//		free(tree.childs[ri].ids);
//	}
	//free(tree.childs);
	//free(tree.ids);
	//free(tree.indices);
	delete[] uri;
}
char* ComponentTree::getStats(){
	char* stats = (char*) malloc(80);
	sprintf(stats, "%d (%d %d %d %d)", COMPONENT_COUNT, proOwnH.getMaxID(), pathH.getMaxID(), refH.getMaxID(), lH.getMaxID());
	return stats;
}

