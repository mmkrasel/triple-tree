/*
 * Query.cpp
 *
 *  Created on: 9 Nov 2022
 *      Author: rasel
 */
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Common.h"
#include "Compression.h"
#include "Query.h"

//#pragma pack(push, 1)

using namespace std;

Query::Query() {
	BITS = new uint32[32];
    for(int i=0; i<32; i++){
    	BITS[i] = (0x80000000 >> i);
    }
	pathDir = NULL;
	SS_fileStream = NULL;
	SO_fileStream = NULL;
	LO_fileStream = NULL;
	LS_fileStream = NULL;
	IDX_PRDCT_fileStream = NULL;
	isExtended = false;
	PAGE_LIMIT = 1024*8;//65536;
	KEEP_EMPTY = PAGE_LIMIT/100;
	buf = NULL;
	PRDCT_COUNT = -1;
	ttrTable = (TertiaryTable*) NULL;
	SS_PAGE_ID = 0;
	SO_PAGE_ID = 0;
	LS_PAGE_ID = 0;
	LO_PAGE_ID = 0;
    //
    size_LO_INDEX = 0;
    INDEX_LO_PAGEIDs = (PageId*) malloc(SZ_PAGEID);
    num_LO_pages = 0;
    //
    size_LS_INDEX = 0;
    INDEX_LS_PAGEIDs = (PageId*) malloc(SZ_PAGEID);
    //num_OS_pages = 0;
    ss_page.numItems = 0;
    so_page.numItems = 0;
    lo_page.numItems = 0;
    ls_page.numItems = 0;

    ss_page.id = -1;
    so_page.id = -1;
    lo_page.id = -1;
    ls_page.id = -1;
    lo_page.ext_page_id = 0xffffffff;
    ls_page.ext_page_id = 0xffffffff;

    ss_page.data = (uint32*) malloc(SZ_ID);
    ss_page.offsets = (Offset16*) malloc(SZ_OFFSET16);
    ss_page.numChildItems = (Offset16*) malloc(SZ_OFFSET16);
    //SS_page.numChildItems = (PageId*) malloc(SZ_PAGEID);
    ss_page.child_page_ids = (PageId*) malloc(SZ_PAGEID*2);
	lo_page.data = (uint32*) malloc(SZ_ID);


    so_page.data = (uint32*) malloc(SZ_ID);
    so_page.offsets = (Offset16*) malloc(SZ_OFFSET16);
    so_page.numChildItems = (Offset16*) malloc(SZ_OFFSET16);
    //SO_page.numChildItems = (PageId*) malloc(SZ_PAGEID);
    so_page.child_page_ids = (PageId*) malloc(SZ_PAGEID*2);
	ls_page.data = (uint32*) malloc(SZ_ID);
    for(int i=0; i<32; i++){
    	BITS[i] = (0x80000000 >> i);
    }
    resultIndex = 0;
    MAX_RESULT_SIZE = 2000;
    resultCount = 0;
    results = (TripleItem*) malloc(sizeof(TripleItem)*MAX_RESULT_SIZE);
	//openedPages = new bool[5000000];
	//for(int i=0; i<5000000; i++){
	//	openedPages[i] = false;
	//}
}
Query::~Query() {}
void Query::load(int ps, const char* ttDirectoryPath){
	PAGE_LIMIT = 1024*ps;
	KEEP_EMPTY = PAGE_LIMIT/100;
	buf = (uint8*)malloc(PAGE_LIMIT);
	for(uint32 i=0; i<PAGE_LIMIT; i++){
		buf[i] = 0;
		//buf[i*2] = 0;
		//buf[i*2+1] = 0;
	}
	//exit(0);
	int pathLen = strlen(ttDirectoryPath);
	pathDir = new char[pathLen+1];
	memcpy(pathDir, ttDirectoryPath, pathLen);
	pathDir[pathLen] = '\0';
	char* tmp = new char[180];
	sprintf(tmp, "%s/%d_pages_ss.tt", pathDir, ps);
	SS_fileStream = fopen(tmp, "rb");
	sprintf(tmp, "%s/%d_pages_so.tt", pathDir, ps);
	SO_fileStream = fopen(tmp, "rb");
	sprintf(tmp, "%s/%d_pages_lo.tt", pathDir, ps);
	LO_fileStream = fopen(tmp, "rb");
	sprintf(tmp, "%s/%d_pages_ls.tt", pathDir, ps);
	LS_fileStream = fopen(tmp, "rb");
	sprintf(tmp, "%s/%d_tertiary_table.tt", pathDir, ps);
	IDX_PRDCT_fileStream = fopen(tmp, "rb");
	fread(&PRDCT_COUNT, sizeof(ComponentId), 1, IDX_PRDCT_fileStream);
	// cout << tmp << " " << PRDCT_COUNT << endl;
	delete[] tmp;
	ttrTable = (TertiaryTable*) malloc(sizeof(TertiaryTable)*PRDCT_COUNT);

	for(ComponentId i=0; i<PRDCT_COUNT; i++){
		fread(&ttrTable[i].numSubPages, sizeof(PageId), 1, IDX_PRDCT_fileStream);
		ttrTable[i].sub_ids = (uint32*) malloc(sizeof(uint32)*ttrTable[i].numSubPages);
		ttrTable[i].sub_page_ids = (PageId*) malloc(sizeof(PageId)*ttrTable[i].numSubPages);
		ttrTable[i].sub_offsets = (Offset16*) malloc(sizeof(Offset16)*ttrTable[i].numSubPages);
		fread(ttrTable[i].sub_ids, sizeof(uint32), ttrTable[i].numSubPages, IDX_PRDCT_fileStream);
		fread(ttrTable[i].sub_page_ids, sizeof(PageId), ttrTable[i].numSubPages, IDX_PRDCT_fileStream);
		fread(ttrTable[i].sub_offsets, sizeof(Offset16), ttrTable[i].numSubPages, IDX_PRDCT_fileStream);

		fread(&ttrTable[i].numObjPages, sizeof(PageId), 1, IDX_PRDCT_fileStream);
		ttrTable[i].obj_ids = (uint32*) malloc(sizeof(uint32)*ttrTable[i].numObjPages);
		ttrTable[i].obj_page_ids = (PageId*) malloc(sizeof(PageId)*ttrTable[i].numObjPages);
		ttrTable[i].obj_offsets = (Offset16*) malloc(sizeof(Offset16)*ttrTable[i].numObjPages);
		fread(ttrTable[i].obj_ids, sizeof(uint32), ttrTable[i].numObjPages, IDX_PRDCT_fileStream);
		fread(ttrTable[i].obj_page_ids, sizeof(PageId), ttrTable[i].numObjPages, IDX_PRDCT_fileStream);
		fread(ttrTable[i].obj_offsets, sizeof(Offset16), ttrTable[i].numObjPages, IDX_PRDCT_fileStream);
	}
}
void Query::clearMemory(){
	for(uint32 i=0; i<PRDCT_COUNT; i++){
		free(ttrTable[i].sub_ids);
		free(ttrTable[i].sub_page_ids);
		free(ttrTable[i].sub_offsets);
		free(ttrTable[i].obj_ids);
		free(ttrTable[i].obj_page_ids);
		free(ttrTable[i].obj_offsets);
	}
	free(ttrTable);
	free(INDEX_LO_PAGEIDs);
	free(INDEX_LS_PAGEIDs);
	free(results);

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
void Query::printResult(){
	for(uint32 i=0; i<resultIndex; i++){
		//if(i==100)break;
		cout << results[i].sub << " " << results[i].pred << " " << results[i].obj << endl;
	}
}
void Query::add2Result(ComponentId s, ComponentId p, ComponentId o){
	if(resultIndex >= MAX_RESULT_SIZE){
		//printResult();
		//resultIndex = 0;
		MAX_RESULT_SIZE += 2000;
		results = (TripleItem*) c.xrealloc((void*)results, sizeof(TripleItem)*MAX_RESULT_SIZE, (char*)"Query::add2Result 1");
	}
	resultCount++;
	results[resultIndex].sub = s;
	results[resultIndex].pred = p;
	results[resultIndex++].obj = o;
}
void Query::loadSSPage(uint32 pid){
	ss_page.id = pid;
	//long offset = (long)pid*PAGE_LIMIT;
	//cout << "@loadSSPage()-offset: " << pid << " " << PAGE_LIMIT << " " << offset << endl;
	fseek(SS_fileStream, (long)pid*PAGE_LIMIT, SEEK_SET);
	fread(buf, 1, PAGE_LIMIT, SS_fileStream);
	uint32 index = 0;
	memcpy(&ss_page.numItems, &buf[index], SZ_OFFSET16);
	index += SZ_OFFSET16;
	memcpy(&ss_page.run_length, &buf[index], SZ_OFFSET16);
	index += SZ_OFFSET16;
	ss_page.data = (uint32*)c.xrealloc((void*)ss_page.data, ss_page.numItems*SZ_ID, (char*)"Query::loadSSPage 1");
	ss_page.offsets = (Offset16*)c.xrealloc((void*)ss_page.offsets, ss_page.numItems*SZ_OFFSET16, (char*)"Query::loadSSPage 2");
	ss_page.numChildItems = (Offset16*)c.xrealloc((void*)ss_page.numChildItems, ss_page.numItems*SZ_OFFSET16, (char*)"Query::loadSSPage 3");
	//SS_page.numChildItems = (PageId*)c.xrealloc((void*)SS_page.numChildItems, SS_page.numItems*SZ_PAGEID, "@loadSSPage-3");
	ss_page.child_page_ids = (PageId*)c.xrealloc((void*)ss_page.child_page_ids, ss_page.run_length*2*SZ_PAGEID, (char*)"Query::loadSSPage 4");
	memcpy(ss_page.data, &buf[index], SZ_ID*ss_page.numItems);
	index += SZ_ID*ss_page.numItems;
	memcpy(ss_page.offsets, &buf[index], SZ_OFFSET16*ss_page.numItems);
	index += SZ_OFFSET16*ss_page.numItems;
	memcpy(ss_page.numChildItems, &buf[index], SZ_OFFSET16*ss_page.numItems);
	index += SZ_OFFSET16*ss_page.numItems;
	//memcpy(SS_page.numChildItems, &buf[index], SZ_PAGEID*SS_page.numItems);
	//index += SZ_PAGEID*SS_page.numItems;
	memcpy(ss_page.child_page_ids, &buf[index], SZ_PAGEID*ss_page.run_length*2);
}
void Query::loadSOPage(uint32 pid){
	so_page.id = pid;
	fseek(SO_fileStream, (long)PAGE_LIMIT*pid, SEEK_SET);
	fread(buf, 1, PAGE_LIMIT, SO_fileStream);
	uint32 index = 0;
	memcpy(&so_page.numItems, &buf[index], SZ_OFFSET16);
	index += SZ_OFFSET16;
	memcpy(&so_page.run_length, &buf[index], SZ_OFFSET16);
	//if(SO_page.run_length == 0 || SO_page.run_length == 65535){
	//	cout << pid << " " << r << " " << PAGE_LIMIT << " " << SO_page.run_length << endl;
	//	exit(1);
	//}
	index += SZ_OFFSET16;
	so_page.data = (uint32*)c.xrealloc((void*)so_page.data, so_page.numItems*SZ_ID, (char*)"Query::loadSOPage 1");
	so_page.offsets = (Offset16*)c.xrealloc((void*)so_page.offsets, so_page.numItems*SZ_OFFSET16, (char*)"Query::loadSOPage 2");
	so_page.numChildItems = (Offset16*)c.xrealloc((void*)so_page.numChildItems, so_page.numItems*SZ_OFFSET16, (char*)"Query::loadSOPage 3");
	//SO_page.numChildItems = (PageId*)c.xrealloc((void*)SO_page.numChildItems, SO_page.numItems*SZ_PAGEID, "@loadSOPage-3");
	so_page.child_page_ids = (PageId*)c.xrealloc((void*)so_page.child_page_ids, so_page.run_length*2*SZ_PAGEID, (char*)"Query::loadSOPage 4");
	memcpy(so_page.data, &buf[index], SZ_ID*so_page.numItems);
	index += SZ_ID*so_page.numItems;
	memcpy(so_page.offsets, &buf[index], SZ_OFFSET16*so_page.numItems);
	index += SZ_OFFSET16*so_page.numItems;
	memcpy(so_page.numChildItems, &buf[index], SZ_OFFSET16*so_page.numItems);
	index += SZ_OFFSET16*so_page.numItems;
	//memcpy(SO_page.numChildItems, &buf[index], SZ_PAGEID*SO_page.numItems);
	//index += SZ_PAGEID*SO_page.numItems;
	memcpy(so_page.child_page_ids, &buf[index], SZ_PAGEID*so_page.run_length*2);
}
void Query::loadLOPage(uint32 pid){
	if(lo_page.id == pid){
		return;
	}
	lo_page.id = pid;
	fseek(LO_fileStream, (long)PAGE_LIMIT*pid,SEEK_SET);
	fread(buf, 1, PAGE_LIMIT, LO_fileStream);
	uint32 index = 0;
	memcpy(&lo_page.numItems, &buf[0], SZ_OFFSET16);
	index += SZ_OFFSET16;
	memcpy(&lo_page.ext_page_id, &buf[index], SZ_PAGEID);
	index += SZ_PAGEID;
	memcpy(&lo_page.ext_page_item_count, &buf[index], SZ_OFFSET16);
	index += SZ_OFFSET16;
	lo_page.data = (uint32*)c.xrealloc((void*)lo_page.data, lo_page.numItems*SZ_ID, (char*)"Query::loadLOPage 1");
	memcpy(lo_page.data, &buf[index], SZ_ID*lo_page.numItems);
	//cout << LO_page.id << " " << LO_page.numItems << " " << LO_page.ext_page_id << " " << LO_page.ext_page_item_count << endl;
//	if(openedPages[lo_page.id]){
//		int c = 0;
//		cout << "\n\n===============================\n";
//		for(Offset16 i=0; i < ss_page.run_length; i++){
//			c += ss_page.child_page_ids[i*2];
//			cout << ss_page.data[i] << " " << ss_page.child_page_ids[i*2+1] << " " << ss_page.child_page_ids[i*2] <<" ("<< c <<")\n";
//		}
//		cout << "\n===============================\n\n";
//		exit(1);
//	}
//	openedPages[lo_page.id] = true;
}
void Query::loadLSPage(uint32 pid){
	if(ls_page.id == pid){
		return;
	}
	ls_page.id = pid;
	fseek(LS_fileStream, (long)PAGE_LIMIT*pid,SEEK_SET);
	fread(buf, 1, PAGE_LIMIT, LS_fileStream);
	uint32 index = 0;
	memcpy(&ls_page.numItems, &buf[0], SZ_OFFSET16);
	index += SZ_OFFSET16;
	memcpy(&ls_page.ext_page_id, &buf[index], SZ_PAGEID);
	index += SZ_PAGEID;
	memcpy(&ls_page.ext_page_item_count, &buf[index], SZ_OFFSET16);
	index += SZ_OFFSET16;
	ls_page.data = (uint32*)c.xrealloc((void*)ls_page.data, ls_page.numItems*SZ_ID, (char*)"Query::loadLSPage 1");
	memcpy(ls_page.data, &buf[index], SZ_ID*ls_page.numItems);
//	itemCount += ls_page.numItems;
//	//cout << LS_page.id << " " << LS_page.numItems << " " << LS_page.ext_page_id << " " << LS_page.ext_page_item_count << endl;
//	if(openedPages[ls_page.id]){
//		int c = 0;
//		cout << "\n\n===============================\n";
//		for(Offset16 i=0; i < so_page.run_length; i++){
//			c += so_page.child_page_ids[i*2];
//			cout << so_page.data[i] << " " << so_page.child_page_ids[i*2+1] << " " << so_page.child_page_ids[i*2] <<" ("<< c <<")\n";
//		}
//		cout << "\n===============================\n\n";
//		exit(1);
//	}
//	openedPages[ls_page.id] = true;
}
PageId Query::get_lo_page_id(Offset16 index){
	Offset16 runOff = 0;
	for(Offset16 i=0; i < ss_page.run_length; i++){
		runOff += ss_page.child_page_ids[i*2];
		if(runOff > index){
			return ss_page.child_page_ids[i*2+1];
		}
	}
	return ss_page.child_page_ids[(ss_page.run_length-1)*2+1];
}
PageId Query::get_ls_page_id(Offset16 index){
	Offset16 runOff = 0;
	for(Offset16 i=0; i < so_page.run_length; i++){
		runOff += so_page.child_page_ids[i*2];
		if(runOff > index){
			//if(SO_page.child_page_ids[i*2+1]==979724129){
			//	cout << i << " " << SO_page.run_length << endl; exit(1);
			//}
			return so_page.child_page_ids[i*2+1];
		}
	}
	return so_page.child_page_ids[(so_page.run_length-1)*2+1];
}
void Query::decodeSubFAW(uint32 fromIndex, uint32 toIndex, uint32 pred, uint32 obj){
	uint32 tmp, j;
	for(uint32 si = fromIndex; si < toIndex; si++){
		if((ls_page.data[si]&0xC0000000)==0xC0000000){
			// FR-word
			tmp = ls_page.data[si]&0x3fffffff;
			this->add2Result(tmp, pred, obj);
			tmp++;
			si++;
			j = 0;
			while(j<ls_page.data[si]){
				this->add2Result(tmp++, pred, obj);
				j++;
			}
		}
		else if((ls_page.data[si]&0xC0000000)==0x80000000){
			// FM-word
			tmp = ls_page.data[si]&0x3fffffff;
			this->add2Result(tmp, pred, obj);
			tmp++;
			si++;
			for(int j=0; j<32; j++){
				if(ls_page.data[si]&BITS[j]){
					this->add2Result(tmp+j, pred, obj);
				}
			}
		}
		else {
			this->add2Result(ls_page.data[si], pred, obj);
		}
	}
}
void Query::decodeObjFAW(uint32 fromIndex, uint32 toIndex, uint32 pred, uint32 sub){
	uint32 tmp, j;
	for(uint32 si = fromIndex; si < toIndex; si++){
		if((lo_page.data[si]&0xC0000000)==0xC0000000){
			// decode FR-word
			tmp = lo_page.data[si]&0x3fffffff;
			this->add2Result(sub, pred, tmp);
			tmp++;
			si++;
			j = 0;
			// decode R-word
			while(j<lo_page.data[si]){
				this->add2Result(sub, pred, tmp++);
				j++;
			}
		}
		else if((lo_page.data[si]&0xC0000000)==0x80000000){
			// decode FM-word
			tmp = lo_page.data[si]&0x3fffffff;
			this->add2Result(sub, pred, tmp);
			tmp++;
			si++;
			// decode M-Word
			for(int j=0; j<32; j++){
				if(lo_page.data[si]&BITS[j]){
					this->add2Result(sub, pred, tmp+j);
				}
			}
		}
		else {
			// S-Word
			this->add2Result(sub, pred, lo_page.data[si]);
		}
	}
}
void Query::getAllByPSO(){
	for(ComponentId pi=0; pi<PRDCT_COUNT; pi++){
		this->getSO4P(pi+1);
	}
	//cout << "itemCount: " << itemCount << endl;
}
void Query::getAllByPOS(){
	for(ComponentId pi=0; pi<PRDCT_COUNT; pi++){
		this->getSO4P(pi+1);
	}
	//cout << "itemCount: " << itemCount << endl;
}
void Query::getSO4P(uint32 p){
	long pi = p-1;
	if(pi < 0 || pi >= PRDCT_COUNT){
		return;
	}
	Offset16 sizeSub, sizeObj;
	uint32 oi;
	for(PageId ss=0; ss<ttrTable[pi].numSubPages; ss++){
		SS_PAGE_ID = ttrTable[pi].sub_page_ids[ss];
		if(SS_PAGE_ID != ss_page.id){
			loadSSPage(SS_PAGE_ID);
		}
		sizeSub = ss_page.numItems;
		if(ss+1 < ttrTable[pi].numSubPages && SS_PAGE_ID == ttrTable[pi].sub_page_ids[ss+1]){
			sizeSub = ttrTable[pi].sub_offsets[ss+1];
		} else if(pi+1 < PRDCT_COUNT && SS_PAGE_ID == ttrTable[pi+1].sub_page_ids[0]){
			sizeSub = ttrTable[pi+1].sub_offsets[0];
		}
		for(Offset16 si=ttrTable[pi].sub_offsets[ss]; si < sizeSub; si++){
			LO_PAGE_ID = get_lo_page_id(si);
			if(LO_PAGE_ID != lo_page.id){
				loadLOPage(LO_PAGE_ID);
			}
			sizeObj = ss_page.offsets[si] + ss_page.numChildItems[si];
			oi = sizeObj < lo_page.numItems ? sizeObj : lo_page.numItems;
			this->decodeObjFAW(ss_page.offsets[si], oi, pi+1, ss_page.data[si]);
			while(oi == lo_page.numItems && lo_page.ext_page_id != 0xffffffff && lo_page.id != lo_page.ext_page_id){
				LO_PAGE_ID = lo_page.ext_page_id;
				loadLOPage(lo_page.ext_page_id);
				this->decodeObjFAW(0, lo_page.ext_page_item_count, pi+1, ss_page.data[si]);
				oi = lo_page.ext_page_item_count;
			}
		}
	}
}
void Query::getOS4P(uint32 p){
	long pi = p-1;
	if(pi < 0 || pi >= PRDCT_COUNT){
		return;
	}
	if(pi >= PRDCT_COUNT){
		return;
	}
	Offset16 sizeSub, sizeObj;
	uint32 si;
	for(PageId so=0; so < ttrTable[pi].numObjPages; so++){
		SO_PAGE_ID = ttrTable[pi].obj_page_ids[so];
		if(SO_PAGE_ID != so_page.id){
			loadSOPage(SO_PAGE_ID);
		}
		sizeObj = so_page.numItems;
		if(so+1 < ttrTable[pi].numObjPages && SO_PAGE_ID == ttrTable[pi].obj_page_ids[so+1]){
			sizeObj = ttrTable[pi].obj_offsets[so+1];
		} else if(pi+1 < PRDCT_COUNT && SO_PAGE_ID == ttrTable[pi+1].obj_page_ids[0]){
			sizeObj = ttrTable[pi+1].obj_offsets[0];
		}
		for(Offset16 oi=ttrTable[pi].obj_offsets[so]; oi < sizeObj; oi++){
			LS_PAGE_ID = get_ls_page_id(oi);
			if(LS_PAGE_ID != ls_page.id){
				loadLSPage(LS_PAGE_ID);
			}
			sizeSub = so_page.offsets[oi] + so_page.numChildItems[oi];
			si = sizeSub < ls_page.numItems ? sizeSub : ls_page.numItems;
			this->decodeSubFAW(so_page.offsets[oi], si, pi+1, so_page.data[oi]);
			while(si == ls_page.numItems && ls_page.ext_page_id != 0xffffffff && ls_page.id != ls_page.ext_page_id){
				LS_PAGE_ID = ls_page.ext_page_id;
				loadLSPage(ls_page.ext_page_id);
				this->decodeSubFAW(0, ls_page.ext_page_item_count, pi+1, so_page.data[oi]);
				si = ls_page.ext_page_item_count;
			}
		}
	}
}
uint32 Query::getSecondarySubIndex(uint32 pi, uint32 s){
	uint32 ssi = 0xffffffff;	// starting subject index
	PageId spidx;			// subject-page id
//	//************************** should perform a binary search here to find the secondary subject page
//	for(spid=0; spid < ttrTable[pi].numSubPages-1; spid++){
//		if(ttrTable[pi].sub_ids[spid]<=s && s <= ttrTable[pi].sub_ids[spid+1]){
//			ssi = ttrTable[pi].sub_offsets[spid];
//			SS_PAGE_ID = ttrTable[pi].sub_page_ids[spid];
//			break;
//		}
//	}
//	cout << spid << " " << ssi << " " << SS_PAGE_ID << " " << ttrTable[pi].sub_ids[spid] << endl;
//	if(ssi == 0xffffffff){
//		ssi = ttrTable[pi].sub_offsets[ttrTable[pi].numSubPages-1];
//		SS_PAGE_ID = ttrTable[pi].sub_page_ids[ttrTable[pi].numSubPages-1];
//	}
	spidx = this->getSecondaryPageIndex(ttrTable[pi].sub_ids, ttrTable[pi].numSubPages, s);			// subject-page id
	//cout << "@getSecondarySubIndex-spidx: " << spidx << endl;
	if(spidx == 0xffffffff){
		ssi = ttrTable[pi].sub_offsets[ttrTable[pi].numSubPages-1];
		SS_PAGE_ID = ttrTable[pi].sub_page_ids[ttrTable[pi].numSubPages-1];
		spidx = ttrTable[pi].numSubPages-1;
	} else {
		ssi = ttrTable[pi].sub_offsets[spidx];
		SS_PAGE_ID = ttrTable[pi].sub_page_ids[spidx];
	}
	//cout << "@getSecondarySubIndex-spidx: " << spidx << " " << ssi << " " << SS_PAGE_ID << " " << ss_page.id << endl;
	if(SS_PAGE_ID != ss_page.id){
		loadSSPage(SS_PAGE_ID);
	}
	uint32 sizeSub = ss_page.numItems;
	if(spidx+1 < ttrTable[pi].numSubPages && SS_PAGE_ID == ttrTable[pi].sub_page_ids[spidx+1]){
		sizeSub = ttrTable[pi].sub_offsets[spidx+1];
	} else if(pi+1 < PRDCT_COUNT && SS_PAGE_ID == ttrTable[pi+1].sub_page_ids[0]){
		sizeSub = ttrTable[pi+1].sub_offsets[0];
	}
	//cout << sizeSub << endl;
	//for(int i=ssi; i<sizeSub; i++){
	//	cout << ss_page.data[i] << " ";
	//}
	//cout << endl;
	return this->binarySearchForIndex(ss_page.data, ssi, sizeSub, s);	// subject index in subject-page
}
uint32 Query::getSecondaryObjectIndex(uint32 pi, uint32 o){
	uint32 soi = 0xffffffff;	// starting object index
	PageId opid = 0;			// object-page id
	//	//************************** should perform a binary search here to find the secondary object page
	//	for(opid=0; opid<ttrTable[pi].numObjPages-1; opid++){
	//		if(ttrTable[pi].obj_ids[opid]<=o && o <= ttrTable[pi].obj_ids[opid+1]){
	//			soi = ttrTable[pi].obj_offsets[opid];
	//			SO_PAGE_ID = ttrTable[pi].obj_page_ids[opid];
	//			break;
	//		}
	//	}
	//	//cout << oi << " " << SO_PAGE_ID << " " << so << " " << ttrTable[pi].obj_ids[so] << endl;
	//	if(soi == 0xffffffff){
	//		soi = ttrTable[pi].obj_offsets[ttrTable[pi].numObjPages-1];
	//		SO_PAGE_ID = ttrTable[pi].obj_page_ids[ttrTable[pi].numObjPages-1];
	//	}
	//	//cout << so << " " << SO_PAGE_ID << endl;
	opid = this->getSecondaryPageIndex(ttrTable[pi].obj_ids, ttrTable[pi].numObjPages, o);			// object-page id
	if(opid == 0xffffffff){
		soi = ttrTable[pi].obj_offsets[ttrTable[pi].numObjPages-1];
		SO_PAGE_ID = ttrTable[pi].obj_page_ids[ttrTable[pi].numObjPages-1];
		opid = ttrTable[pi].numObjPages-1;
	} else {
		soi = ttrTable[pi].obj_offsets[opid];
		SO_PAGE_ID = ttrTable[pi].obj_page_ids[opid];
	}
	//cout << spid << " " << ssi << " " << SS_PAGE_ID << endl;

	if(SO_PAGE_ID != so_page.id){
		loadSOPage(SO_PAGE_ID);
	}
	uint32 sizeObj = so_page.numItems;
	if(opid+1 < ttrTable[pi].numObjPages && SO_PAGE_ID == ttrTable[pi].obj_page_ids[opid+1]){
		sizeObj = ttrTable[pi].obj_offsets[opid+1];
	} else if(pi+1 < PRDCT_COUNT && SO_PAGE_ID == ttrTable[pi+1].obj_page_ids[0]){
		sizeObj = ttrTable[pi+1].obj_offsets[0];
	}
	return this->binarySearchForIndex(so_page.data, soi, sizeObj, o);	// object index in object-page
}
void Query::getO4PS(uint32 p, uint32 s){
	long pi = p-1;	// predicate index
	if(pi < 0 || pi >= PRDCT_COUNT){
		return;
	}
	uint32 si = this->getSecondarySubIndex(pi, s);	// subject index in subject-page
	//cout << "@getO4PS-si: " << si << endl;
	if(si == 0xffffffff){
		return;
	}
	LO_PAGE_ID = get_lo_page_id(si);
	if(LO_PAGE_ID != lo_page.id){
		loadLOPage(LO_PAGE_ID);
	}
	Offset16 sizeObj = ss_page.offsets[si] + ss_page.numChildItems[si];
	uint32 oi = sizeObj < lo_page.numItems ? sizeObj : lo_page.numItems;
	this->decodeObjFAW(ss_page.offsets[si], oi, pi+1, ss_page.data[si]);
	while(oi == lo_page.numItems && lo_page.ext_page_id != 0xffffffff && lo_page.id != lo_page.ext_page_id){
		LO_PAGE_ID = lo_page.ext_page_id;
		loadLOPage(lo_page.ext_page_id);
		this->decodeObjFAW(0, lo_page.ext_page_item_count, pi+1, ss_page.data[si]);
		oi = lo_page.ext_page_item_count;
	}
}
void Query::getS4PO(uint32 p, uint32 o){
	long pi = p-1;		// predicate index
	if(pi < 0 || pi >= PRDCT_COUNT){
		return;
	}
	uint32 oi = this->getSecondaryObjectIndex(pi, o);	// object index in object-page
	if(oi == 0xffffffff){
		return;
	}
	LS_PAGE_ID = get_ls_page_id(oi);
	if(LS_PAGE_ID != ls_page.id){
		loadLSPage(LS_PAGE_ID);
	}
	Offset16 sizeSub = so_page.offsets[oi] + so_page.numChildItems[oi];
	uint32 si = sizeSub < ls_page.numItems ? sizeSub : ls_page.numItems;
	this->decodeSubFAW(so_page.offsets[oi], si, pi+1, so_page.data[oi]);
	while(si == ls_page.numItems && ls_page.ext_page_id != 0xffffffff && ls_page.id != ls_page.ext_page_id){
		LS_PAGE_ID = ls_page.ext_page_id;
		loadLSPage(ls_page.ext_page_id);
		this->decodeSubFAW(0, ls_page.ext_page_item_count, pi+1, so_page.data[oi]);
		si = ls_page.ext_page_item_count;
	}
}
void Query::getPSO(uint32 p, uint32 s, uint32 o){
	long pi = p-1;	// predicate index
	if(pi < 0 || pi >= PRDCT_COUNT){
		return;
	}
	uint32 si = this->getSecondarySubIndex(pi, s);	// subject index in subject-page
	if(si == 0xffffffff){
		return;
	}
	LO_PAGE_ID = get_lo_page_id(si);
	if(LO_PAGE_ID != lo_page.id){
		loadLOPage(LO_PAGE_ID);
	}
	Offset16 sizeObj = ss_page.offsets[si] + ss_page.numChildItems[si];
	uint32 oi = sizeObj < lo_page.numItems ? sizeObj : lo_page.numItems;
	int r = this->probeInFAW(lo_page.data, ss_page.offsets[si], oi, o);
	if(r==1){
		this->add2Result(s, p, o);
	} else {
		while(r==-1 && oi == lo_page.numItems && lo_page.ext_page_id != 0xffffffff && lo_page.id != lo_page.ext_page_id){
			LO_PAGE_ID = lo_page.ext_page_id;
			loadLOPage(lo_page.ext_page_id);
			r = this->probeInFAW(lo_page.data, 0, lo_page.ext_page_item_count, o);
			if(r==1){
				this->add2Result(s, p, o);
			}
			oi = lo_page.ext_page_item_count;
		}
	}
}
void Query::getPOS(uint32 p, uint32 o, uint32 s){
	long pi = p-1;		// predicate index
	if(pi < 0 || pi >= PRDCT_COUNT){
		return;
	}
	uint32 oi = this->getSecondaryObjectIndex(pi, o);	// object index in object-page
	if(oi == 0xffffffff){
		return;
	}
	LS_PAGE_ID = get_ls_page_id(oi);
	if(LS_PAGE_ID != ls_page.id){
		loadLSPage(LS_PAGE_ID);
	}
	Offset16 sizeSub = so_page.offsets[oi] + so_page.numChildItems[oi];
	uint32 si = sizeSub < ls_page.numItems ? sizeSub : ls_page.numItems;
	int r = this->probeInFAW(ls_page.data, so_page.offsets[oi], si, s);
	//cout << r << endl;
	if(r==1){
		this->add2Result(s, p, o);
	} else {
		while(r==-1 && si == ls_page.numItems && ls_page.ext_page_id != 0xffffffff && ls_page.id != ls_page.ext_page_id){
			LS_PAGE_ID = ls_page.ext_page_id;
			loadLSPage(ls_page.ext_page_id);
			r = this->probeInFAW(ls_page.data, 0, ls_page.ext_page_item_count, s);
			if(r==1){
				this->add2Result(s, p, o);
			}
			si = ls_page.ext_page_item_count;
		}
	}
}
int Query::probeInFAW(uint32* faw, uint32 fromIndex, uint32 toIndex, uint32 v){
	// use binary search here
	uint32 tmp;
	for(uint32 si = fromIndex; si < toIndex; si++){
		if((faw[si]&0xC0000000)==0xC0000000){
			// decode FR-word
			tmp = faw[si++]&0x3fffffff;
			if(v < tmp){
				return 0;
			}
			if(tmp <= v && v <= tmp+faw[si]){
				return 1;
			}
		}
		else if((faw[si]&0xC0000000)==0x80000000){
			// decode FM-word
			tmp = faw[si++]&0x3fffffff;
			if(v < tmp){
				return 0;
			}
			//cout << tmp << " f " << faw[si] << " " << v << endl;
			if(v == tmp || (v < tmp+32 && (faw[si]&BITS[v-tmp-1])==BITS[v-tmp-1])){
				return 1;
			}
		}
		else if(v == faw[si]){
			return 1;
		} else if( v < faw[si]){
			return 0;
		}
	}
	return -1;
}
void Query::getPO4S(uint32 s){
	resultCount = 0;
	for(ComponentId p=0; p<PRDCT_COUNT; p++){
		getO4PS(p+1, s);
	}
}
void Query::getPS4O(uint32 o){
	resultCount = 0;
	for(ComponentId p=0; p<PRDCT_COUNT; p++){
		getS4PO(p+1, o);
	}
}
void Query::getP4SO(uint32 s, uint32 o){
	resultCount = 0;
	//cout << "getP4SO(" << s << " " << o << ")"  << endl;
	for(ComponentId p=0; p<PRDCT_COUNT; p++){
		getPSO(p+1, s, o);
	}
}
void Query::getP4OS(uint32 o, uint32 s){
	resultCount = 0;
	//cout << "getP4OS(" << s << " " << o << ")"  << endl;
	for(ComponentId p=0; p<PRDCT_COUNT; p++){
		getPOS(p+1, o, s);
	}
}
uint32* Query::FAW2OSIDs(uint32 f, uint32 t){
	uint32 index = 1;
	uint32* ids = (uint32*) malloc(sizeof(uint32)*2);
	for(uint32 i=f; i<t; i++){
		if((ls_page.data[i]&0xC0000000)==0xC0000000){
			//cout << "Here " << endl;
			// FR-word
			ids = (uint32*)c.xrealloc((void*)ids, (index+1+ls_page.data[i+1])*sizeof(uint32), (char*)"Query::FAW2OSIDs 1");
			ids[index] = ls_page.data[i]&0x3fffffff;
			uint32 k = ids[index++];
			k++;
			i++;
			uint32 j = 0;
			while(j<ls_page.data[i]){
				ids[index++] = k++;
			}
		}
		else if((ls_page.data[i]&0xC0000000)==0x80000000){

			//cout << "Here " << OS_page.data[i] << " " << OS_page.data[i+1] << endl;

			// FM-word
			ids = (uint32*)c.xrealloc((void*)ids, (index+1)*sizeof(uint32), (char*)"Query::FAW2OSIDs 2");
			ids[index] = ls_page.data[i]&0x3fffffff;
			uint32 k = ids[index++];
			k++;
			i++;
		    for(int j=0; j<32; j++){
		    	if(ls_page.data[i]&BITS[j]){
		    		ids = (uint32*)c.xrealloc((void*)ids, (index+1)*sizeof(uint32), (char*)"Query::FAW2OSIDs 3");
		    		ids[index++] = k+j;
		    	}
		    }
		}
		else {
			ids = (uint32*)c.xrealloc((void*)ids, (index+1)*sizeof(uint32), (char*)"Query::FAW2OSIDs 4");
			ids[index++] = ls_page.data[i];
			//cout << "Here " << OS_page.data[i] << endl;
		}
	}
	ids[0] = index;
	return ids;
}
uint32 Query::binarySearchForIndex(uint32* arr, Offset16 from, Offset16 to, uint32 id){
    int first = from;
	int last = to - 1;
	int middle = (first+last)/2;
	while (first <= last) {
		if(arr[middle] < id){
			first = middle + 1;
		}else if (arr[middle] == id) {
			return middle;
		}else{
			last = middle - 1;
		}
		middle = (first + last)/2;
	}
    return 0xffffffff;
}
PageId Query::getSecondaryPageIndex(uint32* arr, PageId size, uint32 id){
    int first = 0;
	int last = size - 1;
	int middle = (first+last)/2;
	while (first <= last) {
		if(arr[middle] < id){
			first = middle + 1;
		}else if (arr[middle] == id) {
			return middle;
		}else{
			last = middle - 1;
		}
		middle = (first + last)/2;
	}
	//cout << first << " m:" << middle << " " << last << endl;
	return first > last ? middle : 0xffffffff;
}

//spo
//spx
//xpo
//xpx
//xxx
//sxx
//xxo
//sxo






