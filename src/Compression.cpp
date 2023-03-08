/*
 * Compression.cpp
 *
 *  Created on: 8 Nov 2022
 *      Author: rasel
 */

//#include <boost/algorithm/string.hpp>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "Common.h"
#include "Compression.h"

//#pragma pack(push, 1)

using namespace std;

Compression::Compression() {
	pathDir = NULL;
	SS_fileStream = NULL;
	SO_fileStream = NULL;
	LO_fileStream = NULL;
	LS_fileStream = NULL;
	IDX_PRDCT_fileStream = NULL;
	PAGE_LIMIT = 1024*8;//65536;
	KEEP_EMPTY = PAGE_LIMIT/100;
	buf = NULL;
	//cout << sizeof(TripleItem) << endl;
	PRDCT_COUNT = 0;
	ttrTable = (TertiaryTable*) malloc(sizeof(TertiaryTable));
	SS_PAGE_ID = 0;
	SO_PAGE_ID = 0;
    //
    //size_LO_INDEX = 0;
    //INDEX_LO_PAGEIDs = (PageId*) malloc(SZ_PAGEID);
    //
    //size_LS_INDEX = 0;
    //INDEX_LS_PAGEIDs = (PageId*) malloc(SZ_PAGEID);
    LS_PAGE_ID = 0;
    LO_PAGE_ID = 0;
	BITS = new uint32[32];
    for(int i=0; i<32; i++){
    	BITS[i] = (0x80000000 >> i);
    }
    ls_page.fWord = 0;
    ls_page.fWordType = 0;
    ls_page.tWord = 0;
    lo_page.fWord = 0;
    lo_page.fWordType = 0;
    lo_page.tWord = 0;
	COMMON_SIZE_OF_SECONDARY_PAGE = SZ_OFFSET16*3 + SZ_PAGEID + KEEP_EMPTY;
	COMMON_SIZE_OF_LEAF_PAGE = SZ_OFFSET16*3 + SZ_PAGEID*2 + KEEP_EMPTY;
}
Compression::~Compression() {}
void Compression::initCompression(int ps, const char* datasetPath){
	PAGE_LIMIT = 1024*ps;
	KEEP_EMPTY = PAGE_LIMIT/100;
	buf = (uint8*)malloc(PAGE_LIMIT);
	for(uint32 i=0; i<PAGE_LIMIT; i++){
		buf[i] = 0;
	}
	//exit(0);
	int pathLen = strlen(datasetPath);
	pathDir = new char[pathLen+1];
	memcpy(pathDir, datasetPath, pathLen);
	pathDir[pathLen] = '\0';
    char* tmp = new char[180];
	sprintf(tmp, "%s/%d_pages_ss.tt",pathDir, ps);
	SS_fileStream = fopen(tmp, "wb");
	sprintf(tmp, "%s/%d_pages_so.tt",pathDir, ps);
	SO_fileStream = fopen(tmp, "wb");
	sprintf(tmp, "%s/%d_pages_lo.tt",pathDir, ps);
	LO_fileStream = fopen(tmp, "wb");
	sprintf(tmp, "%s/%d_pages_ls.tt",pathDir, ps);
	LS_fileStream = fopen(tmp, "wb");
	sprintf(tmp, "%s/%d_tertiary_table.tt",pathDir, ps);
	IDX_PRDCT_fileStream = fopen(tmp, "wb");
	delete[] tmp;
	cout << "Start encoding...." << endl;
	ss_page.id = 0;
    ss_page.numItems = 0;
    ss_page.data = (uint32*) malloc(SZ_ID);
    ss_page.offsets = (Offset16*) malloc(SZ_OFFSET16);
    ss_page.numChildItems = (Offset16*) malloc(SZ_OFFSET16);
    ss_page.run_length = 0;
    ss_page.child_page_ids = (PageId*) malloc(SZ_PAGEID*2);
	ss_page.child_page_ids[ss_page.run_length*2] = 0;
	ss_page.child_page_ids[ss_page.run_length*2+1] = LO_PAGE_ID;
    ss_page.emptySpace = PAGE_LIMIT;

    so_page.id = 0;
    so_page.numItems = 0;
    so_page.data = (uint32*) malloc(SZ_ID);
    so_page.offsets = (Offset16*) malloc(SZ_OFFSET16);
    so_page.numChildItems = (Offset16*) malloc(SZ_OFFSET16);
    so_page.run_length = 0;
    so_page.child_page_ids = (PageId*) malloc(SZ_PAGEID*2);
	so_page.child_page_ids[so_page.run_length*2] = 0;
	so_page.child_page_ids[so_page.run_length*2+1] = LS_PAGE_ID;
    so_page.emptySpace = PAGE_LIMIT;

    lo_page.id = 0;
    lo_page.emptySpace = PAGE_LIMIT;
    lo_page.numItems = 0;
    lo_page.data = (uint32*) malloc(SZ_ID);
    lo_page.ext_page_id = 0xffffffff;
    lo_page.ext_page_item_count = 0;

    ls_page.id = 0;
    ls_page.emptySpace = PAGE_LIMIT;
    ls_page.numItems = 0;
    ls_page.data = (uint32*) malloc(SZ_ID);
    ls_page.ext_page_id = 0xffffffff;
    ls_page.ext_page_item_count = 0;
}
void Compression::updateIndexTable(bool addNew, uint32 startId){
	if(addNew){
		PRDCT_COUNT++;
		ttrTable = (TertiaryTable*)c.xrealloc((void*)ttrTable, (PRDCT_COUNT+1)*sizeof(TertiaryTable));
		ttrTable[PRDCT_COUNT].numSubPages = 0;
		ttrTable[PRDCT_COUNT].sub_ids = (uint32*) malloc(SZ_ID);
		ttrTable[PRDCT_COUNT].sub_page_ids = (PageId*) malloc(SZ_PAGEID);
		ttrTable[PRDCT_COUNT].sub_offsets = (Offset16*) malloc(SZ_OFFSET16);
		ttrTable[PRDCT_COUNT].sub_ids[0] = startId;
		ttrTable[PRDCT_COUNT].sub_page_ids[0] = SS_PAGE_ID;
		ttrTable[PRDCT_COUNT].sub_offsets[0] = ss_page.numItems;
	} else {
		PRDCT_COUNT++;
	    ttrTable[PRDCT_COUNT].numObjPages = 0;
	    ttrTable[PRDCT_COUNT].obj_ids = (uint32*) malloc(SZ_ID);
	    ttrTable[PRDCT_COUNT].obj_page_ids = (PageId*) malloc(SZ_PAGEID);
	    ttrTable[PRDCT_COUNT].obj_offsets = (Offset16*) malloc(SZ_OFFSET16);
	    ttrTable[PRDCT_COUNT].obj_ids[0] = startId;
	    ttrTable[PRDCT_COUNT].obj_page_ids[0] = SO_PAGE_ID;
	    ttrTable[PRDCT_COUNT].obj_offsets[0] = so_page.numItems;
	}
}
void Compression::start(int ps, const char* datasetPath){
	cout << "Page Size: " << ps << endl;
	this->initCompression(ps, datasetPath);
	int strLen = strlen(pathDir);
	char* path = new char[strLen+32];
	sprintf(path, "%s/triple_table.tt", pathDir);
	cout << "Reading dataset: " << path << endl;
	long tripleTableSize = c.getFileSize(path);
	uint32 numTriplesInDB = tripleTableSize / 10;
	cout << "#triples: " << numTriplesInDB << endl;
	cout << "Reading triple table...." << endl;
	TripleItem* ts  = (TripleItem*) malloc(numTriplesInDB*sizeof(TripleItem));
	FILE* fStream = fopen(path, "rb");
	fread(ts, sizeof(TripleItem), numTriplesInDB, fStream);
	fclose(fStream);
	delete[] path;
	//for(uint32 i=0; i<numTriplesInDB; i++){
	//	if(ts[i].sub==510){
	//		cout << ts[i].sub << " x " << ts[i].pred << " x " << ts[i].obj << endl;
	//	}
	//}
	cout << "Sorting by predicate ids...." << endl;
	TripleItem *scratch = (TripleItem*) malloc(numTriplesInDB*sizeof(TripleItem));
//	exit(1);
	this->merge_sort_order_by_predicate(ts, 0, numTriplesInDB, scratch);
	//for(uint32 i=0; i<numTriplesInDB; i++){
	//	if(ts[i].sub==510){
	//		cout << ts[i].sub << " x " << ts[i].pred << " x " << ts[i].obj << endl;
	//	}
	//}
	cout << "Sorting by subject ids with same predicates...." << endl;
	this->sort_by_subject(ts, 0, numTriplesInDB, scratch);
	free(scratch);
	scratch = NULL;
	cout << "compressing for p->s->o paths..." << endl;
	this->subject_order_compression(ts, numTriplesInDB);

	cout << "Sorting by object ids with same predicates...." << endl;
	scratch = (TripleItem*) malloc(numTriplesInDB*sizeof(TripleItem));
	this->sort_by_object(ts, 0, numTriplesInDB, scratch);
	free(scratch);
	scratch = NULL;
	cout << "compressing for p->o->s paths..." << endl;
	this->object_order_compression(ts, numTriplesInDB);
//	for(uint32 i=1; i<numTriplesInDB; i++){
//		if(ts[i].obj==0 || ts[i].obj==ts[i].sub || ts[i-1].obj==ts[i].obj){
//			cout << ts[i].sub << " " << ts[i].pred << " " << ts[i].obj << endl;
//			break;
//		}
//	}
//	exit(1);
//    PRDCT_COUNT = 0;
//    ttrTable[PRDCT_COUNT].numSubPages = 0;
//    ttrTable[PRDCT_COUNT].sub_ids = (uint32*) malloc(SZ_ID);
//    ttrTable[PRDCT_COUNT].sub_page_ids = (PageId*) malloc(SZ_PAGEID);
//    ttrTable[PRDCT_COUNT].sub_offsets = (Offset16*) malloc(SZ_OFFSET16);
//    ttrTable[PRDCT_COUNT].sub_ids[0] = ts[0].sub;
//    ttrTable[PRDCT_COUNT].sub_page_ids[0] = SS_PAGE_ID;
//    ttrTable[PRDCT_COUNT].sub_offsets[0] = SS_page.numItems;
////    IDX_prdct[PRDCT_COUNT].numObjPages = 0;
////    IDX_prdct[PRDCT_COUNT].obj_ids = (uint32*) malloc(SZ_ID);
////    IDX_prdct[PRDCT_COUNT].obj_page_ids = (PageId*) malloc(SZ_PAGEID);
////    //IDX_prdct[PRDCT_COUNT].obj_offsets = (Offset16*) malloc(SZ_OFFSET16);
////    IDX_prdct[PRDCT_COUNT].obj_page_ids[0] = SO_PAGE_ID;
////    //IDX_prdct[PRDCT_COUNT].obj_offsets[0] = SS_page.numItems;
//
//    PropertyID32 sampleP = 0, sampleS = 0;
//    uint32 left = 0;
//    uint32 size = 0;
//    bool isExtended =  false;
//    sampleP = ts[0].pred;
//    sampleS = -1;
//	for(uint32 i=0; i<numTriplesInDB; i++){
//		//cout << i << ". " << ts[i].pred << " " << ts[i].sub << " " << ts[i].obj << endl;
//		if(sampleP != ts[i].pred) {
//			//cout << left << " " << i << " " << PRDCT_COUNT << endl;
//
//			compress_for_objects(ts, left, i);
//
//			this->updateIndexTable(true, ts[i].sub);
//			sampleP = ts[i].pred;
//			sampleS = -1;
//			left = i;
//		}
//		// for subject
//		if(sampleS != ts[i].sub){
//			sampleS = ts[i].sub;
//			size = COMMON_SIZE_OF_SECONDARY_PAGE + (SZ_ID+SZ_OFFSET16*2)*(SS_page.numItems+1) + SZ_PAGEID*SS_page.run_length*2;
//			if(size >= PAGE_LIMIT){
//				store_ss_page(ts[i].sub);
//			}
//			SS_page.numItems++;
//			SS_page.data = (uint32*)c.xrealloc((void*)SS_page.data, SS_page.numItems*SZ_ID, "@start-1");
//			SS_page.offsets = (Offset16*)c.xrealloc((void*)SS_page.offsets, SS_page.numItems*SZ_OFFSET16, "@start-2");
//			SS_page.numChildItems = (Offset16*)c.xrealloc((void*)SS_page.numChildItems, SS_page.numItems*SZ_OFFSET16, "@start-2");
//			SS_page.data[SS_page.numItems-1] = ts[i].sub;
//			SS_page.offsets[SS_page.numItems-1] = LO_page.numItems;
//			SS_page.numChildItems[SS_page.numItems-1] = 0;
//			//cout << PS_page.numItems-1 << " " << ts[i].sub << endl;
//			// run-length encoding on page-ids
//			if(SS_page.child_page_ids[SS_page.run_length*2+1] == num_LO_pages){
//				SS_page.child_page_ids[SS_page.run_length*2]++;
//			} else {
//				SS_page.run_length++;
//				SS_page.child_page_ids = (PageId*)c.xrealloc((void*)SS_page.child_page_ids, (SS_page.run_length+1)*2*SZ_PAGEID, "@start-3");
//				SS_page.child_page_ids[SS_page.run_length*2] = 1;
//				SS_page.child_page_ids[SS_page.run_length*2+1] = num_LO_pages;
//			}
//			isExtended =  false;
//		}
//		size = COMMON_SIZE_OF_LEAF_PAGE + SZ_ID*(LO_page.numItems+2);
//		if(size >= PAGE_LIMIT){
//			isExtended =  store_lo_page();
//		}
//		if(isExtended){
//			LO_page.ext_page_item_count++;
//		}
//		// update code here to perform FAW encoding
//		LO_page.numItems++;
//		LO_page.data = (uint32*)c.xrealloc((void*)LO_page.data, LO_page.numItems*SZ_ID, "@start-5");
//		LO_page.data[LO_page.numItems-1] = ts[i].obj;
//		SS_page.numChildItems[SS_page.numItems-1]++;
//		//cout << i << endl;
//	}
//	compress_for_objects(ts, left, numTriplesInDB);
	store_last_pages();
	free(ts);
	PRDCT_COUNT++;
	fwrite(&PRDCT_COUNT, sizeof(PredicateId), 1, IDX_PRDCT_fileStream);
	for(int i=0; i<PRDCT_COUNT; i++){
		ttrTable[i].numSubPages++;
		fwrite(&ttrTable[i].numSubPages, SZ_PAGEID, 1, IDX_PRDCT_fileStream);
		fwrite(ttrTable[i].sub_ids, SZ_ID, ttrTable[i].numSubPages, IDX_PRDCT_fileStream);
		fwrite(ttrTable[i].sub_page_ids, SZ_PAGEID, ttrTable[i].numSubPages, IDX_PRDCT_fileStream);
		fwrite(ttrTable[i].sub_offsets, SZ_OFFSET16, ttrTable[i].numSubPages, IDX_PRDCT_fileStream);
		ttrTable[i].numObjPages++;
		fwrite(&ttrTable[i].numObjPages, SZ_PAGEID, 1, IDX_PRDCT_fileStream);
		fwrite(ttrTable[i].obj_ids, SZ_ID, ttrTable[i].numObjPages, IDX_PRDCT_fileStream);
		fwrite(ttrTable[i].obj_page_ids, SZ_PAGEID, ttrTable[i].numObjPages, IDX_PRDCT_fileStream);
		fwrite(ttrTable[i].obj_offsets, SZ_OFFSET16, ttrTable[i].numObjPages, IDX_PRDCT_fileStream);
	}
	fflush(IDX_PRDCT_fileStream);
	fclose(IDX_PRDCT_fileStream);
	cout << "\n\nCompression completed\n\n" << endl;
	this->compressionFactor(ps);
}
void Compression::subject_order_compression(TripleItem *ts, uint32 numTriplesInDB){
    PRDCT_COUNT = 0;
    ttrTable[PRDCT_COUNT].numSubPages = 0;
    ttrTable[PRDCT_COUNT].sub_ids = (uint32*) malloc(SZ_ID);
    ttrTable[PRDCT_COUNT].sub_page_ids = (PageId*) malloc(SZ_PAGEID);
    ttrTable[PRDCT_COUNT].sub_offsets = (Offset16*) malloc(SZ_OFFSET16);
    ttrTable[PRDCT_COUNT].sub_ids[0] = ts[0].sub;
    ttrTable[PRDCT_COUNT].sub_page_ids[0] = SS_PAGE_ID;
    ttrTable[PRDCT_COUNT].sub_offsets[0] = ss_page.numItems;
    PropertyID32 sampleP, sampleS;
    uint32 left = 0;
    uint32 size = 0;
    bool isExtended =  false;
    bool isClosed =  true;
    sampleP = ts[0].pred;
    sampleS = -1;
    uint32 dupCount = 0;
    uint32 sub=-1, pred=-1, obj=-1;
	for(uint32 i=0; i<numTriplesInDB; i++){
		//if(ts[i].sub==510){
		//	cout << ts[i].sub << " x " << ts[i].pred << " x " << ts[i].obj << endl;
		//}
		if(sub==ts[i].sub && pred==ts[i].pred && obj==ts[i].obj){
			dupCount++;
			continue;
		}
		//if(ts[i].sub==510){
		//	cout << ts[i].sub << " y " << ts[i].pred << " y " << ts[i].obj << endl;
		//}
		sub=ts[i].sub; pred=ts[i].pred; obj=ts[i].obj;
		//cout << i << ". " << ts[i].pred << " " << ts[i].sub << " " << ts[i].obj << endl;
		if(sampleP != ts[i].pred) {
			if(i > left){
				this->closeObjFAW(isExtended);
				isClosed = true;
			}
			//cout << left << " " << i << " " << PRDCT_COUNT << endl;
			this->updateIndexTable(true, ts[i].sub);
			sampleP = ts[i].pred;
			sampleS = -1;
			left = i;
		}
		if(sampleS != ts[i].sub){
			if(i > left){
				this->closeObjFAW(isExtended);
				isClosed = true;
			}
			sampleS = ts[i].sub;
			size = COMMON_SIZE_OF_SECONDARY_PAGE + (SZ_ID+SZ_OFFSET16*2)*(ss_page.numItems+1) + SZ_PAGEID*ss_page.run_length*2;
			if(size >= PAGE_LIMIT){
				store_ss_page(ts[i].sub);
				//if(ss_page.numItems>0){
					ttrTable[PRDCT_COUNT].numSubPages++;
					ttrTable[PRDCT_COUNT].sub_ids = (uint32*)c.xrealloc((void*)ttrTable[PRDCT_COUNT].sub_ids, (ttrTable[PRDCT_COUNT].numSubPages+1)*SZ_ID);
					ttrTable[PRDCT_COUNT].sub_page_ids = (PageId*) c.xrealloc((void*)ttrTable[PRDCT_COUNT].sub_page_ids, (ttrTable[PRDCT_COUNT].numSubPages+1)*SZ_PAGEID);
					ttrTable[PRDCT_COUNT].sub_offsets = (Offset16*) c.xrealloc((void*)ttrTable[PRDCT_COUNT].sub_offsets, (ttrTable[PRDCT_COUNT].numSubPages+1)*SZ_OFFSET16);
					ttrTable[PRDCT_COUNT].sub_ids[ttrTable[PRDCT_COUNT].numSubPages] = ts[i].sub;
					ttrTable[PRDCT_COUNT].sub_page_ids[ttrTable[PRDCT_COUNT].numSubPages] = SS_PAGE_ID+1;
					ttrTable[PRDCT_COUNT].sub_offsets[ttrTable[PRDCT_COUNT].numSubPages] = 0;
				//}
				SS_PAGE_ID++;
				ss_page.id = SS_PAGE_ID;
				ss_page.numItems = 0;
				ss_page.run_length = 0;
				ss_page.child_page_ids[0] = 0;
				ss_page.child_page_ids[1] = LO_PAGE_ID;
			}
			ss_page.numItems++;
			if(ss_page.numItems==0){
				cout << "SS_page.numItems became overflow" << endl;
			}
			ss_page.data = (uint32*)c.xrealloc((void*)ss_page.data, ss_page.numItems*SZ_ID);
			ss_page.offsets = (Offset16*)c.xrealloc((void*)ss_page.offsets, ss_page.numItems*SZ_OFFSET16);
			ss_page.numChildItems = (Offset16*)c.xrealloc((void*)ss_page.numChildItems, ss_page.numItems*SZ_OFFSET16);
			//ss_page.numChildItems = (PageId*)c.xrealloc((void*)ss_page.numChildItems, ss_page.numItems*SZ_PAGEID, "@start-2");
			ss_page.data[ss_page.numItems-1] = ts[i].sub;
			ss_page.offsets[ss_page.numItems-1] = lo_page.numItems;
			ss_page.numChildItems[ss_page.numItems-1] = 0;
			//cout << PS_page.numItems-1 << " " << ts[i].sub << endl;
			// run-length encoding on page-ids
			if(ss_page.child_page_ids[ss_page.run_length*2+1] == LO_PAGE_ID){
				ss_page.child_page_ids[ss_page.run_length*2]++;
			} else {
				ss_page.run_length++;
				ss_page.child_page_ids = (PageId*)c.xrealloc((void*)ss_page.child_page_ids, (ss_page.run_length+1)*2*SZ_PAGEID);
				ss_page.child_page_ids[ss_page.run_length*2] = 1;
				ss_page.child_page_ids[ss_page.run_length*2+1] = LO_PAGE_ID;
			}
			if(isExtended){
				lo_page.ext_page_item_count = lo_page.numItems;
				isExtended =  false;
			}
		}
		size = COMMON_SIZE_OF_LEAF_PAGE + SZ_ID*(lo_page.numItems+2);
		if(size >= PAGE_LIMIT){
			if(!isClosed){
				this->closeObjFAW(isExtended);
				isClosed = true;
			}
			isExtended =  store_lo_page();
		}
		if(!isClosed){
			this->addID2ObjFAW(ts[i].obj, isExtended);
		} else {
			lo_page.fWord = ts[i].obj;
			lo_page.fWordType = 0;
			lo_page.tWord = 0;
			lo_page.data = (uint32*)c.xrealloc((void*)lo_page.data, (lo_page.numItems+1)*SZ_ID);
			lo_page.data[lo_page.numItems] = ts[i].obj;
		}
		isClosed = false;
		//cout << i << endl;
		if(ts[i].sub==510){
			cout << ts[i].sub << " " << ts[i].pred << " " << ts[i].obj << " " << ttrTable[PRDCT_COUNT].numSubPages << " " << ttrTable[PRDCT_COUNT].sub_page_ids[ttrTable[PRDCT_COUNT].numSubPages] << " ";
			cout << ttrTable[PRDCT_COUNT].sub_offsets[ttrTable[PRDCT_COUNT].numSubPages] << " " << ss_page.numItems << " " << ss_page.data[ss_page.numItems-1] << " " << ss_page.offsets[ss_page.numItems-1] << " ";
			cout << lo_page.data[lo_page.numItems-1] << " " << lo_page.data[lo_page.numItems] << endl;
		}
	}
	if(isExtended){
		lo_page.ext_page_item_count = lo_page.numItems;
	}
	this->closeObjFAW(isExtended);
	cout << "#Duplicates: " << dupCount << "; #Distinct: " << numTriplesInDB-dupCount << endl;
}
void Compression::object_order_compression(TripleItem *ts, uint32 numTriplesInDB){
	SO_PAGE_ID = 0;
	so_page.numItems = 0;
	PRDCT_COUNT = 0;
	ttrTable[PRDCT_COUNT].numObjPages = 0;
	ttrTable[PRDCT_COUNT].obj_ids = (uint32*) malloc(SZ_ID);
	ttrTable[PRDCT_COUNT].obj_page_ids = (PageId*) malloc(SZ_PAGEID);
	ttrTable[PRDCT_COUNT].obj_offsets = (Offset16*) malloc(SZ_OFFSET16);
	ttrTable[PRDCT_COUNT].obj_ids[0] = ts[0].obj;
	ttrTable[PRDCT_COUNT].obj_page_ids[0] = SO_PAGE_ID;
	ttrTable[PRDCT_COUNT].obj_offsets[0] = so_page.numItems;
	PropertyID32 sampleP, sampleO;
	uint32 left = 0;
	uint32 size = 0;
	bool isExtended =  false;
	bool isClosed =  true;
	sampleP = ts[0].pred;
	sampleO = -1;
	uint32 ci = 0;
    uint32 dupCount = 0;
    uint32 sub=-1, pred=-1, obj=-1;
	for(uint32 i=0; i<numTriplesInDB; i++){
		if(sub==ts[i].sub && pred==ts[i].pred && obj==ts[i].obj){
			dupCount++;
			continue;
		}
		sub=ts[i].sub; pred=ts[i].pred; obj=ts[i].obj;
		//cout << i << ". " << ts[i].sub << " " << ts[i].pred << " " << ts[i].obj << endl;
		//cout << ts[i].sub << " " << ts[i].pred << " " << ts[i].obj << endl;
//		if(PRDCT_COUNT==0 && ts[i].obj==5){
//			//cout << ts[i].sub << endl;
//			cout << ts[i].sub << " " << ts[i].pred << " " << ts[i].obj << endl;
//		}
		if(sampleP != ts[i].pred) {
			if(i > left && !isClosed){
				this->closeSubFAW(isExtended);
				isClosed = true;
			}
			//if(i>0){
			//	cout << PRDCT_COUNT << " " << ci << endl;
			//	ci = 0;
			//}
			this->updateIndexTable(false, ts[i].obj);
			sampleP = ts[i].pred;
			sampleO = -1;
			left = i;
		}
		if(sampleO != ts[i].obj){
			if(i > left && !isClosed){
				this->closeSubFAW(isExtended);
				isClosed = true;
			}
//			if(PRDCT_COUNT==0 && i>0 && ts[i-1].obj==5){
//				cout << endl << ts[i-1].obj << " " << ci << " " << so_page.numChildItems[so_page.numItems-1] << " " << so_page.numItems << endl; ci = 0;
//			}
			sampleO = ts[i].obj;
			size = COMMON_SIZE_OF_SECONDARY_PAGE + (SZ_ID+SZ_OFFSET16*2)*(so_page.numItems+1) + SZ_PAGEID*so_page.run_length*2;
			if(size >= PAGE_LIMIT){
				store_so_page(ts[i].obj);
				//if(so_page.numItems>0){
					ttrTable[PRDCT_COUNT].numObjPages++;
					ttrTable[PRDCT_COUNT].obj_ids = (uint32*)c.xrealloc((void*)ttrTable[PRDCT_COUNT].obj_ids, (ttrTable[PRDCT_COUNT].numObjPages+1)*SZ_ID);
					ttrTable[PRDCT_COUNT].obj_page_ids = (PageId*) c.xrealloc((void*)ttrTable[PRDCT_COUNT].obj_page_ids, (ttrTable[PRDCT_COUNT].numObjPages+1)*SZ_PAGEID);
					ttrTable[PRDCT_COUNT].obj_offsets = (Offset16*) c.xrealloc((void*)ttrTable[PRDCT_COUNT].obj_offsets, (ttrTable[PRDCT_COUNT].numObjPages+1)*SZ_OFFSET16);
					ttrTable[PRDCT_COUNT].obj_ids[ttrTable[PRDCT_COUNT].numObjPages] = ts[i].obj;
					ttrTable[PRDCT_COUNT].obj_page_ids[ttrTable[PRDCT_COUNT].numObjPages] = SO_PAGE_ID+1;
					ttrTable[PRDCT_COUNT].obj_offsets[ttrTable[PRDCT_COUNT].numObjPages] = 0;
				//}
				SO_PAGE_ID++;
				so_page.id = SO_PAGE_ID;
				so_page.numItems = 0;
				so_page.run_length = 0;
				so_page.child_page_ids[0] = 0;
				so_page.child_page_ids[1] = LS_PAGE_ID;
			}
			so_page.numItems++;
			if(so_page.numItems==0){
				cout << "SO_page.numItems became overflow" << endl; exit(1);
			}
			so_page.data = (uint32*)c.xrealloc((void*)so_page.data, so_page.numItems*SZ_ID);
			so_page.offsets = (Offset16*)c.xrealloc((void*)so_page.offsets, so_page.numItems*SZ_OFFSET16);
			so_page.numChildItems = (Offset16*)c.xrealloc((void*)so_page.numChildItems, so_page.numItems*SZ_OFFSET16);
			//so_page.numChildItems = (PageId*)c.xrealloc((void*)so_page.numChildItems, so_page.numItems*SZ_PAGEID, "@start-2");
			so_page.data[so_page.numItems-1] = ts[i].obj;
			so_page.offsets[so_page.numItems-1] = ls_page.numItems;
			so_page.numChildItems[so_page.numItems-1] = 0;
			// run-length encoding on page-ids
			if(so_page.child_page_ids[so_page.run_length*2+1] == LS_PAGE_ID){
				so_page.child_page_ids[so_page.run_length*2]++;
			} else {
				so_page.run_length++;
				//if(so_page.run_length==65535){ cout << " Here 1" << endl; exit(1);}
				so_page.child_page_ids = (PageId*)c.xrealloc((void*)so_page.child_page_ids, (so_page.run_length+1)*2*SZ_PAGEID);
				so_page.child_page_ids[so_page.run_length*2] = 1;
				so_page.child_page_ids[so_page.run_length*2+1] = LS_PAGE_ID;
			}
			//if(isExtended){
				//ls_page.ext_page_item_count = ls_page.numItems+1;
				isExtended =  false;
			//}
		}
		size = COMMON_SIZE_OF_LEAF_PAGE + SZ_ID*(ls_page.numItems+2);
		if(size >= PAGE_LIMIT){
			if(!isClosed){
				this->closeSubFAW(isExtended);
				isClosed = true;
			}
			isExtended =  store_ls_page();
		}
		if(!isClosed){
			this->addID2SubFAW(ts[i].sub, isExtended);
		} else {
			ls_page.fWord = ts[i].sub;
			ls_page.fWordType = 0;
			ls_page.tWord = 0;
			//ls_page.data = (uint32*)c.xrealloc((void*)ls_page.data, (ls_page.numItems+1)*SZ_ID, "@compress_for_objects-1");
			//ls_page.data[ls_page.numItems] = ts[i].sub;
		}
		isClosed = false;
		ci++;
	}
	this->closeSubFAW(isExtended);
	//if(isExtended){
		//ls_page.ext_page_item_count = ls_page.numItems+1;
	//}
	//cout << PRDCT_COUNT << " " << ci << endl;
	//cout << nc << " " << ec << endl;
	cout << "#Duplicates: " << dupCount << "; #Distinct: " << numTriplesInDB-dupCount << endl;
}
void Compression::addID2SubFAW(uint32 id, bool isExtended){
	long dif;
	int increment = 0;
	dif = id-ls_page.fWord-1;
	if(dif>-1 && dif<32){                     // within the range of 32
		ls_page.tWord |= BITS[dif];
		if(ls_page.tWord==0xFFFFFFFF){
			ls_page.fWordType = 2;
			ls_page.tWord = 32;
		}
	}
	else  if(dif < 0){
		//cout << "@addSubjectID2FAW (" << calledFrom << "): id=" << id << ", i=" << i << ", obj=" << obj << ", fw="  << OS_page.fWord << ", " << (isExtended ? "true" : "False") << endl;
		//exit(2);
	}
	else {
		if(ls_page.tWord==0){               // word @flag is a S-Word
			ls_page.data = (uint32*)c.xrealloc((void*)ls_page.data, (ls_page.numItems+1)*SZ_ID);
			ls_page.data[ls_page.numItems++] = ls_page.fWord;
			ls_page.fWord = id;
			increment = 1;
		}
		else if(ls_page.fWordType == 2 && dif==1+ls_page.tWord){	// word @flag is a FR-Word
			ls_page.tWord++;
		}
		else {
			ls_page.data = (uint32*)c.xrealloc((void*)ls_page.data, (ls_page.numItems+2)*SZ_ID);
			if(ls_page.fWordType == 2){
				ls_page.data[ls_page.numItems++] = ls_page.fWord|0xC0000000;
			}
			else {
				ls_page.data[ls_page.numItems++] = ls_page.fWord|0x80000000;
			}
			ls_page.data[ls_page.numItems++] = ls_page.tWord;
			ls_page.fWord = id;
			ls_page.tWord = 0;
			ls_page.fWordType = 0;
			increment = 2;
		}
	}
	//if(increment > 0){
		if(!isExtended){
			so_page.numChildItems[so_page.numItems-1]+=increment;
			nc++;
		}else{
			ls_page.ext_page_item_count += increment;
			ec++;
		}
	//}
}
void Compression::closeSubFAW(bool isExtended){
	// close if a new parent is created
	// close if new leaf page is opened :aka: extended page
	int increment = 1;
	if(ls_page.tWord==0){               // word @flag is a S-Word
		ls_page.data = (uint32*)c.xrealloc((void*)ls_page.data, (ls_page.numItems+1)*SZ_ID);
		ls_page.data[ls_page.numItems++] = ls_page.fWord;
	}
	else {
		ls_page.data = (uint32*)c.xrealloc((void*)ls_page.data, (ls_page.numItems+2)*SZ_ID);
		if(ls_page.fWordType == 2){
			ls_page.data[ls_page.numItems++] = ls_page.fWord|0xC0000000;
		}
		else {
			ls_page.data[ls_page.numItems++] = ls_page.fWord|0x80000000;
		}
		ls_page.data[ls_page.numItems++] = ls_page.tWord;
		increment = 2;
	}
	//if(increment > 0){
		if(!isExtended){
			so_page.numChildItems[so_page.numItems-1]+=increment;
			nc++;
		}else{
			ls_page.ext_page_item_count += increment;
			ec++;
		}
	//}
	ls_page.fWordType = 0;
	ls_page.tWord = 0;
	ls_page.fWord = 0;
}
void Compression::flush_lo_page(){
	uint32 index = 0;
	memcpy(&buf[0], &lo_page.numItems, SZ_OFFSET16);
	index += SZ_OFFSET16;
	memcpy(&buf[index], &lo_page.ext_page_id, SZ_PAGEID);
	index += SZ_PAGEID;
	memcpy(&buf[index], &lo_page.ext_page_item_count, SZ_OFFSET16);
	index += SZ_OFFSET16;
	memcpy(&buf[index], lo_page.data, SZ_ID*lo_page.numItems);
	fwrite(buf, 1, PAGE_LIMIT, LO_fileStream);
}
void Compression::flush_ls_page(){
	uint32 index = 0;
	memcpy(&buf[index], &ls_page.numItems, SZ_OFFSET16);
	index += SZ_OFFSET16;
	memcpy(&buf[index], &ls_page.ext_page_id, SZ_PAGEID);
	index += SZ_PAGEID;
	memcpy(&buf[index], &ls_page.ext_page_item_count, SZ_OFFSET16);
	index += SZ_OFFSET16;
	memcpy(&buf[index], ls_page.data, SZ_ID*ls_page.numItems);
	fwrite(buf, 1, PAGE_LIMIT, LS_fileStream);
}
bool Compression::store_lo_page(){
	bool isExtended =  false;
	// If a subject has just started to store objects. so need to update the SS page index for the selected subject
	// check if no siblings have been stored in LO page yet
	if(ss_page.numChildItems[ss_page.numItems-1] == 0){
		ss_page.child_page_ids[ss_page.run_length*2]--;
		isExtended =  false;
		lo_page.ext_page_id = 0xffffffff;	// the page does not have an extended page
		this->flush_lo_page();
		ss_page.offsets[ss_page.numItems-1] = 0;
		ss_page.numChildItems[ss_page.numItems-1] = 0;
		if(ss_page.numItems > 1){
			ss_page.run_length++;
			ss_page.child_page_ids = (PageId*)c.xrealloc((void*)ss_page.child_page_ids, (ss_page.run_length+1)*2*SZ_PAGEID);
		}
		ss_page.child_page_ids[ss_page.run_length*2] = 1;
		ss_page.child_page_ids[ss_page.run_length*2+1] = (LO_PAGE_ID+1);
	}
	// a subject will have objects spread in two different LO pages. Add link to first page
	else {
		isExtended = true;
		lo_page.ext_page_id = (LO_PAGE_ID+1);	// the page has an extended page having id (num_LO_pages+1)
		this->flush_lo_page();
//		SS_page.run_length++;
//		SS_page.child_page_ids = (PageId*)c.xrealloc((void*)SS_page.child_page_ids, (SS_page.run_length+1)*2*SZ_PAGEID, "@store_lo_page-1");
//		SS_page.child_page_ids[SS_page.run_length*2] = 1;
//		SS_page.child_page_ids[SS_page.run_length*2+1] = (LO_PAGE_ID+1);
	}
	LO_PAGE_ID++;
	lo_page.numItems = 0;
	lo_page.ext_page_item_count = 0;
	lo_page.id = LO_PAGE_ID;
	return isExtended;
}
bool Compression::store_ls_page(){
	bool isExtended =  false;
	// If an object has just started to store subjects. so need to update the SO page index for the selected object
	// check if no siblings have been stored in LS page yet
	if(so_page.numChildItems[so_page.numItems-1] == 0){
		so_page.child_page_ids[so_page.run_length*2]--;
		isExtended =  false;
		ls_page.ext_page_id = 0xffffffff;	// the page does not have an extended page
		this->flush_ls_page();
		so_page.offsets[so_page.numItems-1] = 0;
		so_page.numChildItems[so_page.numItems-1] = 0;
		if(so_page.numItems > 1){
			so_page.run_length++;
			//if(so_page.run_length==65535){ cout << " Here 2" << endl; exit(1);}
			so_page.child_page_ids = (PageId*)c.xrealloc((void*)so_page.child_page_ids, (so_page.run_length+1)*2*SZ_PAGEID);
		}
		so_page.child_page_ids[so_page.run_length*2] = 1;
		so_page.child_page_ids[so_page.run_length*2+1] = (LS_PAGE_ID+1);
	}
	// an object will have objects spread in two different pages. Add link to first page
	else {
		isExtended = true;
		ls_page.ext_page_id = (LS_PAGE_ID+1);	// the page has an extended page having id (num_SO_pages+1)
		this->flush_ls_page();
		//so_page.run_length++;
		//so_page.child_page_ids = (PageId*)c.xrealloc((void*)so_page.child_page_ids, (so_page.run_length+1)*2*SZ_PAGEID, "@store_ls_page-2");
		//so_page.child_page_ids[so_page.run_length*2] = 1;
		//so_page.child_page_ids[so_page.run_length*2+1] = (LS_PAGE_ID+1);
	}
	LS_PAGE_ID++;
	ls_page.numItems = 0;
	ls_page.ext_page_item_count = 0;
	ls_page.id = LS_PAGE_ID;
	return isExtended;
}
void Compression::store_ss_page(uint32 startID){
	ss_page.run_length++;
	uint32 index = 0;
	memcpy(&buf[index], &ss_page.numItems, SZ_OFFSET16);
	index += SZ_OFFSET16;
	memcpy(&buf[index], &ss_page.run_length, SZ_OFFSET16);
	index += SZ_OFFSET16;
	memcpy(&buf[index], ss_page.data, SZ_ID*ss_page.numItems);
	index += SZ_ID*ss_page.numItems;
	memcpy(&buf[index], ss_page.offsets, SZ_OFFSET16*ss_page.numItems);
	index += SZ_OFFSET16*ss_page.numItems;
	memcpy(&buf[index], ss_page.numChildItems, SZ_OFFSET16*ss_page.numItems);
	index += SZ_OFFSET16*ss_page.numItems;
	//memcpy(&buf[index], ss_page.numChildItems, SZ_PAGEID*ss_page.numItems);
	//index += SZ_PAGEID*ss_page.numItems;
	memcpy(&buf[index], ss_page.child_page_ids, SZ_PAGEID*ss_page.run_length*2);
	fwrite(buf, 1, PAGE_LIMIT, SS_fileStream);
}
void Compression::store_so_page(uint32 startID){
	so_page.run_length++;
	//if(so_page.run_length==65535){ cout << " Here 3" << endl; exit(1);}
	uint32 index = 0;
	memcpy(&buf[index], &so_page.numItems, SZ_OFFSET16);
	index += SZ_OFFSET16;
	memcpy(&buf[index], &so_page.run_length, SZ_OFFSET16);
	index += SZ_OFFSET16;
	memcpy(&buf[index], so_page.data, SZ_ID*so_page.numItems);
	index += SZ_ID*so_page.numItems;
	memcpy(&buf[index], so_page.offsets, SZ_OFFSET16*so_page.numItems);
	index += SZ_OFFSET16*so_page.numItems;
	memcpy(&buf[index], so_page.numChildItems, SZ_OFFSET16*so_page.numItems);
	index += SZ_OFFSET16*so_page.numItems;
	//memcpy(&buf[index], so_page.numChildItems, SZ_PAGEID*so_page.numItems);
	//index += SZ_PAGEID*so_page.numItems;
	memcpy(&buf[index], so_page.child_page_ids, SZ_PAGEID*so_page.run_length*2);
	fwrite(buf, 1, PAGE_LIMIT, SO_fileStream);
}
void Compression::store_last_pages(){
	if(ss_page.numItems > 0){
		ss_page.run_length++;
		uint32 index = 0;
		memcpy(&buf[index], &ss_page.numItems, SZ_OFFSET16);
		index += SZ_OFFSET16;
		memcpy(&buf[index], &ss_page.run_length, SZ_OFFSET16);
		index += SZ_OFFSET16;
		memcpy(&buf[index], ss_page.data, SZ_ID*ss_page.numItems);
		index += SZ_ID*ss_page.numItems;
		memcpy(&buf[index], ss_page.offsets, SZ_OFFSET16*ss_page.numItems);
		index += SZ_OFFSET16*ss_page.numItems;
		memcpy(&buf[index], ss_page.numChildItems, SZ_OFFSET16*ss_page.numItems);
		index += SZ_OFFSET16*ss_page.numItems;
		//memcpy(&buf[index], ss_page.numChildItems, SZ_PAGEID*ss_page.numItems);
		//index += SZ_PAGEID*ss_page.numItems;
		memcpy(&buf[index], ss_page.child_page_ids, SZ_PAGEID*ss_page.run_length*2);
		fwrite(buf, 1, PAGE_LIMIT, SS_fileStream);
	}
	fflush(SS_fileStream);
	fclose(SS_fileStream);
	// flush so-page data
	if(lo_page.numItems > 0){
		// a subject has just started to store objects. so need to update the PS page index for the selected subject
		lo_page.ext_page_id = 0xffffffff;	// the page does not have an extended page
		this->flush_lo_page();
	}
	fflush(LO_fileStream);
	fclose(LO_fileStream);
	// store po-page
	if(so_page.numItems > 0){
		so_page.run_length++;
		//if(so_page.run_length==65535){ cout << " Here 4" << endl; exit(1);}
		uint32 index = 0;
		memcpy(&buf[index], &so_page.numItems, SZ_OFFSET16);
		index += SZ_OFFSET16;
		memcpy(&buf[index], &so_page.run_length, SZ_OFFSET16);
		index += SZ_OFFSET16;
		memcpy(&buf[index], so_page.data, SZ_ID*so_page.numItems);
		index += SZ_ID*so_page.numItems;
		memcpy(&buf[index], so_page.offsets, SZ_OFFSET16*so_page.numItems);
		index += SZ_OFFSET16*so_page.numItems;
		memcpy(&buf[index], so_page.numChildItems, SZ_OFFSET16*so_page.numItems);
		index += SZ_OFFSET16*so_page.numItems;
		//memcpy(&buf[index], so_page.numChildItems, SZ_PAGEID*so_page.numItems);
		//index += SZ_PAGEID*so_page.numItems;
		memcpy(&buf[index], so_page.child_page_ids, SZ_PAGEID*so_page.run_length*2);
		fwrite(buf, 1, PAGE_LIMIT, SO_fileStream);
	}
	fflush(SO_fileStream);
	fclose(SO_fileStream);
	// flush so-page data
	if(ls_page.numItems > 0){
		// a subject has just started to store objects. so need to update the PS page index for the selected subject
		ls_page.ext_page_id = 0xffffffff;	// the page does not have an extended page
		this->flush_ls_page();
	}
	fflush(LS_fileStream);
	fclose(LS_fileStream);
}
PropertyID32 Compression::min_id(PropertyID32 x, PropertyID32 y){
	if(x < y){
		return x;
	}
	else{
		return y;
	}
}
void Compression::merge_sort_order_by_predicate(TripleItem *input, PropertyID32 left, PropertyID32 right, TripleItem *scratch){
	/* base case: one element */
	if(right == left + 1){
		return;
	}
	else{
		PropertyID32 i = 0;
		PropertyID32 length = right - left;
		PropertyID32 midpoint_distance = length/2;
		PropertyID32 l = left, r = left + midpoint_distance;
		merge_sort_order_by_predicate(input, left, left + midpoint_distance, scratch);
		merge_sort_order_by_predicate(input, left + midpoint_distance, right, scratch);
		for(i = 0; i < length; i++) {
			if(l < left + midpoint_distance &&  (r == right || min_id(input[l].pred, input[r].pred) == input[l].pred)){
				scratch[i] = input[l];
				l++;
			}
			else{
				scratch[i] = input[r];
				r++;
			}
		}
		memcpy(&input[left], &scratch[0], (right-left)*sizeof(TripleItem));
	}
}
void Compression::merge_sort_order_by_object(TripleItem *input, PropertyID32 left, PropertyID32 right, TripleItem *scratch){
	/* base case: one element */
	if(right == left + 1){
		return;
	}
	PropertyID32 i = 0;
	PropertyID32 length = right - left;
	PropertyID32 midpoint_distance = length/2;
	PropertyID32 l = left, r = left + midpoint_distance;
	merge_sort_order_by_object(input, left, left + midpoint_distance, scratch);
	merge_sort_order_by_object(input, left + midpoint_distance, right, scratch);
	for(i = 0; i < length; i++) {
		if(l < left + midpoint_distance &&  (r == right || min_id(input[l].obj, input[r].obj) == input[l].obj)){
			scratch[i] = input[l];
			l++;
		}
		else{
			scratch[i] = input[r];
			r++;
		}
	}
	memcpy(&input[left], &scratch[0], (right-left)*sizeof(TripleItem));
}
void Compression::merge_sort_order_by_subject(TripleItem *input, PropertyID32 left, PropertyID32 right, TripleItem *scratch){
	/* base case: one element */
	if(right == left + 1){
		return;
	}
	PropertyID32 i = 0;
	PropertyID32 length = right - left;
	PropertyID32 midpoint_distance = length/2;
	PropertyID32 l = left, r = left + midpoint_distance;
	merge_sort_order_by_subject(input, left, left + midpoint_distance, scratch);
	merge_sort_order_by_subject(input, left + midpoint_distance, right, scratch);
	for(i = 0; i < length; i++) {
		if(l < left + midpoint_distance &&  (r == right || min_id(input[l].sub, input[r].sub) == input[l].sub)){
			scratch[i] = input[l];
			l++;
		}
		else{
			scratch[i] = input[r];
			r++;
		}
	}
	memcpy(&input[left], &scratch[0], (right-left)*sizeof(TripleItem));
}
void Compression::sort_by_subject(TripleItem *input, PropertyID32 left, PropertyID32 right, TripleItem *scratch){

	PropertyID32 sampleS;
	PropertyID32 sampleP = input[left].pred;
	for(PropertyID32 pi=left; pi<right; pi++){
		//cout << input[pi].pred << " ";
		if(sampleP != input[pi].pred) {
			sampleP = input[pi].pred;
			merge_sort_order_by_subject(input, left, pi, scratch);
			sampleS = input[left].sub;
			for(PropertyID32 i=left; i<pi; i++){
				if(sampleS != input[i].sub) {
					sampleS = input[i].sub;
					merge_sort_order_by_object(input, left, i, scratch);
					left = i;
				}
			}
			merge_sort_order_by_object(input, left, pi, scratch);
			left = pi;
		}
	}
//	for(PropertyID32 pi=left; pi<right; pi++){
//		cout << input[pi].pred << " ";
//	}
	merge_sort_order_by_subject(input, left, right, scratch);
	sampleS = input[left].sub;
	for(PropertyID32 i=left; i<right; i++){
		if(sampleS != input[i].sub) {
			sampleS = input[i].sub;
			merge_sort_order_by_object(input, left, i, scratch);
			left = i;
		}
	}
	merge_sort_order_by_object(input, left, right, scratch);
	//cout << endl;
}
void Compression::sort_by_object(TripleItem *input, PropertyID32 left, PropertyID32 right, TripleItem *scratch){

	PropertyID32 sampleO;
	PropertyID32 sampleP = input[left].pred;
	for(PropertyID32 pi=left; pi<right; pi++){
		//cout << input[pi].pred << " ";
		if(sampleP != input[pi].pred) {
			sampleP = input[pi].pred;
			merge_sort_order_by_object(input, left, pi, scratch);
			sampleO = input[left].obj;
			for(PropertyID32 i=left; i<pi; i++){
				if(sampleO != input[i].obj) {
					sampleO = input[i].obj;
					merge_sort_order_by_subject(input, left, i, scratch);
					left = i;
				}
			}
			merge_sort_order_by_subject(input, left, pi, scratch);
			left = pi;
		}
	}
//	for(PropertyID32 pi=left; pi<right; pi++){
//		cout << input[pi].pred << " ";
//	}
	merge_sort_order_by_object(input, left, right, scratch);
	sampleO = input[left].obj;
	for(PropertyID32 i=left; i<right; i++){
		if(sampleO != input[i].obj) {
			sampleO = input[i].obj;
			merge_sort_order_by_subject(input, left, i, scratch);
			left = i;
		}
	}
	merge_sort_order_by_subject(input, left, right, scratch);
	//cout << endl;
}
void Compression::sort_by_object(TripleItem *input, PropertyID32 left, PropertyID32 right){
	TripleItem *scratch = (TripleItem*) malloc((right-left+1)*sizeof(TripleItem));
	merge_sort_order_by_object(input, left, right, scratch);
	PropertyID32 sampleO = input[left].obj;
	for(PropertyID32 i=left; i<right; i++){
		if(sampleO != input[i].obj) {
			sampleO = input[i].obj;
			merge_sort_order_by_subject(input, left, i, scratch);
	//			if(i==6659 && input[i].obj==2506){
	//				for(int j=i; j<right; j++){
	//					if(input[i].obj != input[j].obj){
	//						cout << endl;
	//						break;
	//					}
	//					cout << input[j].sub << ", ";
	//				}
	//			}
			left = i;
		}
	}
	merge_sort_order_by_subject(input, left, right, scratch);
	free(scratch);
	scratch = NULL;
}
void Compression::addID2ObjFAW(uint32 id, bool isExtended){
	int increment = 0;
	long dif = id-lo_page.fWord-1;
	if(dif>-1 && dif<32){                     // within the range of 32
		lo_page.tWord |= BITS[dif];
		if(lo_page.tWord==0xFFFFFFFF){
			lo_page.fWordType = 2;
			lo_page.tWord = 32;
		}
	}
	else  if(dif < 0){
		//cout << "@addSubjectID2FAW (" << calledFrom << "): id=" << id << ", i=" << i << ", sub=" << sub << ", fw="  << SO_page.fWord << ", " << (isExtended ? "true" : "False") << endl;
		//exit(2);
	}
	else {
		if(lo_page.tWord==0){               // word @flag is a S-Word
			lo_page.data = (uint32*)c.xrealloc((void*)lo_page.data, (lo_page.numItems+1)*SZ_ID);
			lo_page.data[lo_page.numItems++] = lo_page.fWord;
			lo_page.fWord = id;
			increment = 1;
		}
		else if(lo_page.fWordType == 2 && dif==1+lo_page.tWord){	// word @flag is a FR-Word
			lo_page.tWord++;
		}
		else {
			lo_page.data = (uint32*)c.xrealloc((void*)lo_page.data, (lo_page.numItems+2)*SZ_ID);
			if(lo_page.fWordType == 2){
				lo_page.data[lo_page.numItems++] = lo_page.fWord|0xC0000000;
			}
			else {
				lo_page.data[lo_page.numItems++] = lo_page.fWord|0x80000000;
			}
			lo_page.data[lo_page.numItems++] = lo_page.tWord;
			lo_page.fWord = id;
			lo_page.tWord = 0;
			lo_page.fWordType = 0;
			increment = 2;
		}
	}
	if(!isExtended){
		ss_page.numChildItems[ss_page.numItems-1]+=increment;
	}
}
void Compression::closeObjFAW(bool isExtended){
	int increment = 1;
	if(lo_page.tWord==0){               // word @flag is a S-Word
		lo_page.data = (uint32*)c.xrealloc((void*)lo_page.data, (lo_page.numItems+1)*SZ_ID);
		lo_page.data[lo_page.numItems++] = lo_page.fWord;
	}
	else {
		lo_page.data = (uint32*)c.xrealloc((void*)lo_page.data, (lo_page.numItems+2)*SZ_ID);
		if(lo_page.fWordType == 2){
			lo_page.data[lo_page.numItems++] = lo_page.fWord|0xC0000000;
		}
		else {
			lo_page.data[lo_page.numItems++] = lo_page.fWord|0x80000000;
		}
		lo_page.data[lo_page.numItems++] = lo_page.tWord;
	}
	if(!isExtended){
		ss_page.numChildItems[ss_page.numItems-1]+=increment;
	}
	lo_page.fWordType = 0;
	lo_page.tWord = 0;
	lo_page.fWord = 0;
}

uint32 Compression::IDs2FAW(uint32* arr, uint32 count){
	uint32 fWordIndex = 0;
	uint32 fWord = arr[0];
    char fWordType = 0;
    uint32 tWord = 0;
    long dif;
    uint32 i=1;
    while(i<count){
        dif = arr[i]-fWord-1;
        if(dif>-1 && dif<32){                     // within the range of 32
            tWord |= BITS[dif];
            if(tWord==0xFFFFFFFF){
                fWordType = 2;
                tWord = 32;
            }
        }
        else {
            if(tWord==0){               // word @flag is a S-Word
                arr[fWordIndex++] = fWord;
                fWord = arr[i];
            }
            else if(fWordType == 2 && dif==1+tWord){// word @flag is a FR-Word
                tWord++;
            }
            else {
                if(fWordType == 2){
                    arr[fWordIndex++] = fWord|0xC0000000;
                }
                else {
                    arr[fWordIndex++] = fWord|0x80000000;
                }
                arr[fWordIndex++] = tWord;
                fWord = arr[i];
                tWord = 0;
            }
        }
        i++;
    }// while
    if(tWord==0){               // word @flag is a S-Word
        arr[fWordIndex++] = fWord;
    }
    else {
        if(fWordType == 2){
            arr[fWordIndex++] = fWord|0xC0000000;
        }
        else {
            arr[fWordIndex++] = fWord|0x80000000;
        }
        arr[fWordIndex++] = tWord;
    }
    return fWordIndex;
}
void Compression::testFAW(){
	uint32 arrx[] = {1, 12, 20, 67, 70, 80, 150};
	uint32 cl = IDs2FAW(arrx, 7);
	for(uint32 i=0; i<cl; i++){
		cout << arrx[i] << " ";
	}
	// -2147483647 2105344 -2147483581 537395200
	cout << endl << endl;
	int arr1[] = {1, 12, 20, 67, 70, 80, 150, 200};
	ls_page.data = (uint32*) malloc(4*3);

	ls_page.numItems = 2;
	ls_page.fWord = arr1[0];
	ls_page.fWordType = 0;
	ls_page.tWord = 0;
	ls_page.data[0] = 2000;
	ls_page.data[1] = 1000;
	ls_page.data[ls_page.numItems] = arr1[0];
	for(int i=1; i<8; i++){
		addID2SubFAW(arr1[i], false);
		printf("%d: fwi=%d fw=%d fwt=%d tw=%u\n", arr1[i], ls_page.numItems, ls_page.fWord, ls_page.fWordType, ls_page.tWord);//, fw.rootIndex);
	}
	closeSubFAW(false);
	printf("fwi=%d fw=%d fwt=%d tw=%u\n", ls_page.numItems, ls_page.fWord, ls_page.fWordType, ls_page.tWord);//, fw.rootIndex);
	for(Offset16 i=0; i<ls_page.numItems; i++){
		cout << ls_page.data[i] << " ";
	}
}
void Compression::end(){
	for(int i=0; i<PRDCT_COUNT; i++){
		free(ttrTable[i].sub_ids);
		free(ttrTable[i].sub_page_ids);
		free(ttrTable[i].sub_offsets);
		free(ttrTable[i].obj_ids);
		free(ttrTable[i].obj_page_ids);
		free(ttrTable[i].obj_offsets);
	}
	free(ttrTable);
	//free(INDEX_LO_PAGEIDs);
	//free(INDEX_LS_PAGEIDs);
	delete[] BITS;
	free(buf);
	delete[]pathDir;
	free(ss_page.data);
	free(ss_page.offsets);
	free(ss_page.numChildItems);
	free(ss_page.child_page_ids);
	free(so_page.data);
	free(so_page.offsets);
	free(so_page.numChildItems);
	free(so_page.child_page_ids);

	free(lo_page.data);
	free(ls_page.data);
}
void Compression::compressionFactor(uint32 ps){
	char* path = new char[180];
	sprintf(path, "%s/dic_owner.tt", pathDir);
	long dicSize = c.getFileSize(path);
	sprintf(path, "%s/dic_path.tt", pathDir);
	dicSize += c.getFileSize(path);
	sprintf(path, "%s/dic_predicate.tt", pathDir);
	dicSize += c.getFileSize(path);
	sprintf(path, "%s/dic_ref.tt", pathDir);
	dicSize += c.getFileSize(path);
	sprintf(path, "%s/triple_table.tt", pathDir);
	long ttableSize = c.getFileSize(path);
	sprintf(path, "%s.nt", pathDir);
	long inputSize = c.getFileSize(path);
	sprintf(path, "%s/%d_pages_ss.tt",pathDir, ps);
	long size_pages_ss = c.getFileSize(path);
	sprintf(path, "%s/%d_pages_so.tt",pathDir, ps);
	long size_pages_so = c.getFileSize(path);
	sprintf(path, "%s/%d_pages_lo.tt",pathDir, ps);
	long size_pages_lo = c.getFileSize(path);
	sprintf(path, "%s/%d_pages_ls.tt",pathDir, ps);
	long size_pages_ls = c.getFileSize(path);
	sprintf(path, "%s/%d_tertiary_table.tt",pathDir, ps);
	long size_idx_prdct = c.getFileSize(path);
	delete[] path;
	cout << "#pred_index_entry: " << PRDCT_COUNT << endl;
	cout << "size_pages_ss: " << size_pages_ss << endl;
	cout << "size_pages_so: " << size_pages_so << endl;
	cout << "size_pages_lo: " << size_pages_lo << endl;
	cout << "size_pages_ls: " << size_pages_ls << endl;
	cout << "size_idx_prdct: " << size_idx_prdct << endl;
	long ttSize = (size_pages_ss+size_pages_so+size_pages_lo+size_pages_ls+size_idx_prdct);
	cout << "di_size: " << dicSize << endl;
	cout << "tt_size: " << ttSize << endl;
	cout << "input vs. di_size: " << (100*1.0/(inputSize*1.0 / dicSize))  << endl;
	cout << "ttable vs. ttSize: " << (100*1.0/(ttableSize*1.0 / ttSize))  << endl;
	cout << "input vs. output: " << (100*1.0/(inputSize*1.0 / (ttSize+dicSize)))  << endl;
}



