/*
 * Compression.h
 *
 *  Created on: 8 Nov 2022
 *      Author: rasel
 */

#ifndef COMPRESSION_H_
#define COMPRESSION_H_


typedef unsigned int PageId;
typedef unsigned short Offset16;

#pragma pack(push, 1)

typedef struct Tertiary_Table{
	PageId numSubPages;				// number of secondary-subject-pages that contain the children
	ComponentId* sub_ids;				// contains the id of the first child subject
    PageId* sub_page_ids;			// contains the page-id of the first child subject
    Offset16* sub_offsets;
    //
    PageId numObjPages;				// number of secondary-object-pages that contain the children
    ComponentId* obj_ids;				// contains the id of the first child object
    PageId* obj_page_ids;			// contains the page-id of the first child object
    Offset16* obj_offsets;
}TertiaryTable;

typedef struct Secondary_Page{
	PageId id;						// Page Id
	Offset16 emptySpace;			// available emptSpace in the page
	Offset16 numItems;				// total number of items in FAW-encoded data. Use it to know the number of encoded items for last parent
	ComponentId* data;					// stores subject or object ids
    Offset16* offsets;				// offsets of FAW-encoded child-data in leaf page
    Offset16* numChildItems;		// number of items in FAW-encoded child-data in leaf page
    PageId* child_page_ids;			// run-length encoded child page ids
    Offset16 run_length;
    //PageId ext_page_id;				// extended page id if the children (secondary subs/objs) of last parents are also in next page
    //Offset16 ext_page_item_count;	// number of items in this current extended page ()
}SecondaryPage;

typedef struct Leaf_Page{
	PageId id;						// page id
	Offset16 emptySpace;			// available emptSpace in the page
	Offset16 numItems;				// total number of items in FAW-encoded data. Use it to know the number of encoded items for last parent
	ComponentId* data;					// FAW encoded data. There may several group of FAW encoded items
    PageId ext_page_id;				// extended page id if the child of last parents are also in next page
    Offset16 ext_page_item_count;	// number of items in this current extended page ()
    ComponentId fWord;
    ComponentId fWordType;
    ComponentId tWord;
}LeafPage;

#pragma pack(pop)


using namespace std;

class Compression {
private:
	char* pathDir;
	FILE *SS_fileStream, *SO_fileStream, *LO_fileStream, *LS_fileStream, *IDX_PRDCT_fileStream;
	//bool isLSPExtended;
	char SZ_OFFSET16 = 2;
	char SZ_PAGEID = 4;
	char SZ_ID = 4;
	uint32 KEEP_EMPTY;
	uint32 PAGE_LIMIT;
	uint8* buf;
	int PRDCT_COUNT;
	TertiaryTable* ttrTable;
	//PageId* INDEX_LO_PAGEIDs;
	//PageId* INDEX_LS_PAGEIDs;
	PageId LO_PAGE_ID, LS_PAGE_ID;
	PageId SS_PAGE_ID, SO_PAGE_ID;
	//PageId size_LS_INDEX, size_LO_INDEX;
	Offset16 COMMON_SIZE_OF_SECONDARY_PAGE;
	Offset16 COMMON_SIZE_OF_LEAF_PAGE;
	uint32* BITS;
	SecondaryPage ss_page;
	SecondaryPage so_page;		// secondary subject and secondary-object page
	LeafPage lo_page;
	LeafPage ls_page;				// leaf-object and leaf-subject page
	Common c;
	//
	uint32 ec=0, nc = 0;
	//
	void initCompression(int ps, const char* datasetPath);
public:
	Compression();
	virtual ~Compression();
	void start(int ps, const char* datasetPath);
	void subject_order_compression(TripleItem *ts, uint32 numTriplesInDB);
	void object_order_compression(TripleItem *ts, uint32 numTriplesInDB);
	void end();
	void updateIndexTable(bool addNew, uint32 startId);
	void store_ss_page(uint32 startID);
	void store_so_page(uint32 startID);
	void flush_lo_page();
	bool store_lo_page();
	void flush_ls_page();
	bool store_ls_page();
	void store_last_pages();
	ComponentId min_id(ComponentId x, ComponentId y);
	void merge_sort_order_by_predicate(TripleItem *input, ComponentId left, ComponentId right, TripleItem *scratch);
	void merge_sort_order_by_object(TripleItem *input, ComponentId left, ComponentId right, TripleItem *scratch);
	void merge_sort_order_by_subject(TripleItem *input, ComponentId left, ComponentId right, TripleItem *scratch);
	void sort_by_subject(TripleItem *input, ComponentId left, ComponentId right, TripleItem *scratch);
	void sort_by_object(TripleItem *input, ComponentId left, ComponentId right, TripleItem *scratch);
	void sort_by_object(TripleItem *input, ComponentId left, ComponentId right);
	void addObjectID2FAW(uint32 id, bool isClosed, int calledFrom, uint32 i, uint32 sub);

	void addID2ObjFAW(uint32 id, bool isExtended);
	void closeObjFAW(bool isExtended);
	void addID2SubFAW(uint32 id, bool isExtended);
	void closeSubFAW(bool isExtended);
	uint32 IDs2FAW(uint32* arr, uint32 count);
	void compressionFactor(uint32 ps);
	void testFAW();
};

#endif /* COMPRESSION_H_ */
