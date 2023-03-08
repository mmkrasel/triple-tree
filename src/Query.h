/*
 * Query.h
 *
 *  Created on: 9 Nov 2022
 *      Author: rasel
 */

#ifndef QUERY_H_
#define QUERY_H_

//#pragma pack(push, 1)

using namespace std;

class Query {

	char* pathDir;
	FILE *SS_fileStream, *SO_fileStream, *LO_fileStream, *LS_fileStream, *IDX_PRDCT_fileStream;
	bool isExtended;
	uint32 PAGE_LIMIT;
	uint32 KEEP_EMPTY;
	char SZ_OFFSET16 = 2;
	char SZ_PAGEID = 4;
	char SZ_ID = 4;
	uint8* buf;
	PredicateId PRDCT_COUNT;
	TertiaryTable* ttrTable;
	uint32 SS_PAGE_ID, SO_PAGE_ID, LO_PAGE_ID, LS_PAGE_ID, size_LO_INDEX;
	PageId* INDEX_LO_PAGEIDs;
	uint32 num_LO_pages, size_LS_INDEX;
	PageId* INDEX_LS_PAGEIDs;
	//uint16 num_OS_pages;
	uint32* BITS;
	SecondaryPage ss_page, so_page;
	LeafPage lo_page, ls_page;
	Common c;
	//uint32 ec=0, nc = 0;
	uint32 resultIndex;
	uint32 MAX_RESULT_SIZE;
	//bool* openedPages = NULL;
public:
	uint32 resultCount;
	TripleItem* results;
	Query();
	virtual ~Query();
	void load(int ps, const char* ttDirectoryPath);
	void loadSSPage(uint32 pid);
	void loadLOPage(uint32 pid);
	void getAllByPSO();
	PageId get_lo_page_id(Offset16 index);
	void loadSOPage(uint32 pid);
	void loadLSPage(uint32 pid);
	void getOS4P(uint32 p);
	void getSO4P(uint32 p);
	PageId get_ls_page_id(Offset16 index);
	void getAllByPOS();
	uint32* FAW2OSIDs(uint32 f, uint32 t);
	void add2Result(SubObjId s, PredicateId p, SubObjId o);
	void decodeSubFAW(uint32 fromIndex, uint32 toIndex, uint32 pred, uint32 obj);
	void decodeObjFAW(uint32 fromIndex, uint32 toIndex, uint32 pred, uint32 sub);
	void getO4PS(uint32 p, uint32 s);
	void getS4PO(uint32 p, uint32 o);
	void getPSO(uint32 p, uint32 s, uint32 o);
	void getPOS(uint32 p, uint32 o, uint32 s);
	void getPO4S(uint32 s);
	void getPS4O(uint32 o);
	void getP4SO(uint32 s, uint32 o);
	void getP4OS(uint32 o, uint32 s);
	uint32 getSecondaryObjectIndex(uint32 pi, uint32 s);
	uint32 getSecondarySubIndex(uint32 pi, uint32 o);
	int probeInFAW(uint32* faw, uint32 fromIndex, uint32 toIndex, uint32 v);
	uint32 binarySearchForIndex(uint32* arr, Offset16 from, Offset16 to, uint32 id);
	PageId getSecondaryPageIndex(uint32* arr, PageId size, uint32 id);
	void printResult();
	void clearMemory();
};

#endif /* QUERY_H_ */
