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
#include "TripleTable.h"
#include "ComponentTree.h"
#include "Dictionary.h"
#include <regex>
#include <regex.h>

using namespace std;

Dictionary::Dictionary() {
	pathDir = NULL;
	inputPath = NULL;
	LOG = false;
}
Dictionary::~Dictionary() {}


void Dictionary::stats(){
	cout << tripleTable.NUM_TRIPLES << "\t" << so.getStats() << "\t" << p.getStats() << endl;
}


void Dictionary::create(char* inputPath, char* outputDirectory) {
	int pathLen = strlen(outputDirectory);
	pathDir = new char[pathLen+1];
	memcpy(&this->pathDir[0], &outputDirectory[0], pathLen);
	pathDir[pathLen] = '\0';
	pathLen = strlen(inputPath);
	this->inputPath = new char[pathLen+1];
	memcpy(&this->inputPath[0], &inputPath[0], pathLen);
	this->inputPath[pathLen] = '\0';

	so.init(outputDirectory, (char*)"so");
	p.init(outputDirectory, (char*)"p");
	tripleTable.init(outputDirectory, true);


//	string subject, predicate, object;
//	ComponentId subId=0, objId=0, predId = 0;
//	TripleId tripleCount = 0;
//	TripleId linesCount = 0;
//	common.startClock();
//    // Open N3 data file
//    ifstream file(inputPath);
//    if (!file.is_open()) {
//        cerr << "Failed to open file." << endl;
//        return;
//    }
//    // Regular expression to match an NT-triple
//
//    //regex pattern(R"(([^ ]+) +([^ ]+) +((?:<[^>]+>)|(?:"[^"\\]*(?:\\.[^"\\]*)*"(?:\^\^<[^>]+>)?))(?: *@\w+)? +\.\s*)");
//    //regex pattern(R"(([^ ]+) +([^ ]+) +((?:<[^>]+>)|(?:"(?:\\.|[^\\"])*"))(?:\^\^<[^>]+>)? *+\.)");		// NT-triple ends with a '.'
//    //regex pattern(R"(([^ ]+) +([^ ]+) +((?:<[^>]+>)|(?:"[^"\\]*(?:\\.[^"\\]*)*"(?:\^\^<[^>]+>)?)) *\.)");	// NT-triple with multiple lines and multiple quotation marks
//    //regex pattern(R"(([^ ]+) +([^ ]+) +((?:<[^>]+>)|(?:"[^"\\]*(?:\\.[^"\\]*)*"(?:\^\^<[^>]+>)?)) *\.\s*)");
//    regex pattern(R"(([^ ]+) +([^ ]+) +((?:<[^>]+>)|(?:"[^"\\]*(?:\\.[^"\\]*)*"(?:\^\^<[^>]+>)?)) *\s*\.\s*)");
//
//    smatch match;
//    string line;
//    string current_triple;
//    bool _break = false;
//    while (getline(file, line)) {
//        // Add current line to current NT-triple
//        current_triple += line;
//        //cout << line << endl;
//        // If current NT-triple ends with a '.', process it
//        if (regex_search(current_triple, match, pattern) && match[0].matched) {
//        	//if (current_triple.find("@en .<") != std::string::npos) {
//        	//	cout << current_triple << endl;
//        	//	_break = true;
//        	//}
//        	subject = match[1].str();
//        	predicate = match[2].str();
//        	object = match[3].str();
//        	linesCount++;
//        	//cout << linesCount << ". " << current_triple << endl;
//			if (predicate.front() == '<' && predicate.back() == '>') {
//				predId = p.encodeUri(predicate.c_str());
//				//cout << predId << " u" << endl;
//			} else {
//				// Parse the subject
//				size_t start = current_triple.find("<");
//				size_t end = current_triple.find(">") + 1;
//				subject = current_triple.substr(start, end - start);
//
//				// Parse the predicate
//				start = end + 1; // skip the space and "<"
//				end = current_triple.find(">", start) + 1;
//				predicate = current_triple.substr(start, end - start);
//				predId = p.encodeUri(predicate.c_str());
//
//
//				start = end + 1;
//				object = current_triple.substr(start, current_triple.length()-start-2);
//				//cout << endl << endl << subject << "'\n\n'" << predicate << "'\n\n'" << object <<"'"<< endl;
//				//exit(1);
//			}
//        	//cout << subject << " s => ";
//			if (subject.front() == '<' && subject.back() == '>') {
//				subId = so.encodeUri(subject.c_str());
//				//cout << subId << " u" << endl;
//			} else {
//				subId = so.encodeLiteral(subject.c_str());
//				//cout << subId << " l" << endl;
//			}
//			//cout << object << " o => ";
//			if (object.front() == '<' && object.back() == '>') {
//				objId = so.encodeUri(object.c_str());
//				//cout << objId << " u" << endl;
//			} else {
//				objId = so.encodeLiteral(object.c_str());
//				//cout << objId << " l" << endl;
//			}
////			string tri = subject + " " + predicate + " " + object;
////			std::string::size_type i = current_triple.find(tri);
////			if (i != std::string::npos){
////				current_triple = current_triple.substr(0, i) +".";
////				if(current_triple.length() < 8){
////					current_triple.clear();
////				}
////			}
//			//cout << tri << endl << endl << current_triple << endl << endl;
//            current_triple.clear();
//			cout << linesCount << ". " << subId << " " << predId << " " << objId << endl;
//			//cout << subject << " " << match[2] << " " << object << endl;
//			//printf("s %u=>%s\n", subID, properties[0]);
//			//printf("p %u=>%s\n", predID, properties[1]);
//			//printf("o %u=>%s\n\n", objID, properties[2]);
//			tripleCount++;
//			tripleTable.add(subId, predId, objId);
//			if(tripleCount%1000000==0){
//				cout << "@Dictionary::create: " << tripleCount/1000000 << ": " << common.getElapsedTime()/1000/1000/1000 << "\n";
//				//cout << "ref:" << refH.getMaxID() << "(" << refH.getSize() << ")\t" << "path:" << pathH.getMaxID() << "(" << pathH.getSize() << ")\t";
//				//cout << "proown:" << proOwnH.getMaxID() << "(" << proOwnH.getSize() << endl;// << "\tpred:" << pHash.getMaxID() << "(" << pHash.getSize() << ")" << endl;
//			}
//			//cout << "@1 "<< uriCount << " " << properties[0] << " " << properties[1] << " " << properties[2] << endl << endl;
//        }
//        if(current_triple.length() > 2482900){
//        	//cout << line << endl << "PROBLEM " << endl << subject << endl << predicate << endl << object << endl;
//        	break;
//        }
//    }
//    file.close();

    regex_t uri_regex, literal_regex, literal_regex_type;
    regcomp(&uri_regex, "^<.*>$", REG_EXTENDED);
    regcomp(&literal_regex, "^\".*\"(@[a-zA-Z]+)*$", REG_EXTENDED);
    regcomp(&literal_regex_type, "\"(.+)\"\\^\\^(.+)", REG_EXTENDED);
    regmatch_t matches[2];
	char** properties = (char**)malloc(sizeof(char*)*4);
	properties[0] = (char*) malloc(common.URI_MAX_LENGTH);
	properties[1] = (char*) malloc(common.URI_MAX_LENGTH);
	properties[2] = (char*) malloc(common.URI_MAX_LENGTH);
	properties[3] = (char*) malloc(common.URI_MAX_LENGTH);
	short uriCount = 0;
	TripleId tripleCount = 0;
	TripleId linesCount = 0;
	uint32 subId, objId;
	uint32 predId;
	char c, tmp;
	int index = 0;
	common.startClock();
	ifstream is(inputPath);
	int divisor = 100000;
	while(is.get(c)){
//		if(12288198 < linesCount && linesCount < 12288202){
//			//countDown--;
//			cout << c;
//		} else if (linesCount == 12288202){
//			exit(1);
//		}
//		if(linesCount > 240000){
//			exit(1);
//		}
//		if(linesCount < 1510){
//			if(c=='\n'){
//				linesCount++;
//			}
//			continue;
//		}
//		else {
//			if(c=='\n'){
//				linesCount++;
//			}
//		}
//		cout << c;
//		continue;
//		if(linesCount < 374000000){
//			if(c=='\n'){
//				linesCount++;
//			}
//			cout << c;
//			continue;
//		}
//		exit(1);
//		if(linesCount > 1018000000){
//			Common::setLog(true);
//		}
//		else if(linesCount > 1019000000){
//			Common::setLog(false);
//		}
		if(uriCount==0){
			if(c==' '){
				tmp = properties[uriCount][index-2];
				properties[uriCount][index] = '\0';
			    int uri_matched = regexec(&uri_regex, properties[uriCount], 2, matches, 0);
			    if (uri_matched == 0) {
			    	subId = so.encodeUri(properties[uriCount]);
				    uriCount++;
					index = 0;
					continue;
			    } else {
			    	string str = properties[uriCount];
			    	if (str.find('"') != string::npos) {
			    	} else {
			    	    // shift existing elements to the right
			    	    for (int i = index-1; i >= 0; i--) {
			    	    	properties[uriCount][i+1] = properties[uriCount][i];
			    	    }
			    	    // insert new element at the beginning
			    	    properties[uriCount][0] = '"';
			    	    properties[uriCount][index+1] = '"';
			    	    properties[uriCount][index+2] = '\0';
			    	}
				    int literal_matched = regexec(&literal_regex, properties[uriCount], 2, matches, 0);
			    	if (literal_matched == 0) {
						subId = so.encodeLiteral(properties[uriCount]);
						uriCount++;
						index = 0;
						continue;
					} else {
						properties[uriCount][index-2] = tmp;
						properties[uriCount][index] = '\0';
						cout << index<<"\n\n\nNO S: " << properties[uriCount] << endl << endl << endl;
						exit(1);
					}
			    }
			}
		} else if (uriCount==1){
			if(c==' '){
				tmp = properties[uriCount][index-2];
				properties[uriCount][index] = '\0';
			    int uri_matched = regexec(&uri_regex, properties[uriCount], 2, matches, 0);
			    if (uri_matched == 0) {
			    	predId = p.encodeUri(properties[uriCount]);
				    uriCount++;
					index = 0;
					continue;
			    } else {
			    	string str = properties[uriCount];
			    	if (str.find('"') != string::npos) {
			    	} else {
			    	    // shift existing elements to the right
			    	    for (int i = index-1; i >= 0; i--) {
			    	    	properties[uriCount][i+1] = properties[uriCount][i];
			    	    }
			    	    // insert new element at the beginning
			    	    properties[uriCount][0] = '"';
			    	    properties[uriCount][index+1] = '"';
			    	    properties[uriCount][index+2] = '\0';
			    	}
				    int literal_matched = regexec(&literal_regex, properties[uriCount], 2, matches, 0);
			    	if (literal_matched == 0) {
			    		predId = p.encodeLiteral(properties[uriCount]);
						uriCount++;
						index = 0;
						continue;
					}else {
						properties[uriCount][index-2] = tmp;
						properties[uriCount][index] = '\0';
						cout << index<<"\n\n\nNO P: " << properties[uriCount] << endl << endl << endl;
						exit(1);
					}
			    }
			}
		} else {
			if(c=='\n'){
				linesCount++;
				tmp = properties[uriCount][index-2];
				properties[uriCount][index-2] = '\0';
			    int uri_matched = regexec(&uri_regex, properties[uriCount], 2, matches, 0);
			    if (uri_matched == 0) {
			    	objId = so.encodeUri(properties[uriCount]);
			    	tripleCount++;
			    	//printf("@Dictionary::create 1 *** %d %d %d\n", subId, predId, objId);
			    	tripleTable.add(subId, predId, objId);
					uriCount = 0;
					index = 0;
					if(tripleCount%divisor==0){
						this->stats();
						//cout << linesCount << " " << tripleCount  << "."  << properties[0] << " " << properties[1] << " " << properties[2] << endl;
					}
					continue;
			    } else {
				    int literal_matched = regexec(&literal_regex, properties[uriCount], 2, matches, 0);
			    	if (literal_matched == 0) {
						objId = so.encodeLiteral(properties[uriCount]);
						//printf("@Dictionary::create 2 *** %d %d %d\n", subId, predId, objId);
						tripleTable.add(subId, predId, objId);
			    		tripleCount++;
						uriCount = 0;
						index = 0;
						if(tripleCount%divisor==0){
							this->stats();
							//cout << linesCount << " 1 " << tripleCount <<  "."  << properties[0] << " " << properties[1] << " " << properties[2] << endl;
						}
						continue;
					}else {
						literal_matched = regexec(&literal_regex_type, properties[uriCount], 2, matches, 0);
						if (literal_matched == 0) {
							objId = so.encodeLiteral(properties[uriCount]);
							//printf("@Dictionary::create 3 *** %d %d %d\n", subId, predId, objId);
							tripleTable.add(subId, predId, objId);
							tripleCount++;
							uriCount = 0;
							index = 0;
							if(tripleCount%divisor==0){
								this->stats();
								//cout << linesCount << " 2 " << tripleCount  << "."  << properties[0] << " " << properties[1] << " " << properties[2] << endl;
							}
							continue;
						}
						else if (properties[uriCount][index-3] == '@') {
							objId = so.encodeLiteral(properties[uriCount]);
							//printf("@Dictionary::create 4 *** %d %d %d\n", subId, predId, objId);
							tripleTable.add(subId, predId, objId);
							tripleCount++;
							uriCount = 0;
							index = 0;
							if(tripleCount%divisor==0){
								this->stats();
								//cout << linesCount << " 3 " << tripleCount  << "."  << properties[0] << " " << properties[1] << " " << properties[2] << endl;
							}
							continue;
						}
						else if (index > 1024*5) {
					    	string str = properties[uriCount];
					    	if (str.find('"') != string::npos) {
					    	} else {
					    	    // shift existing elements to the right
					    	    for (int i = index-1; i >= 0; i--) {
					    	    	properties[uriCount][i+1] = properties[uriCount][i];
					    	    }
					    	    // insert new element at the beginning
					    	    properties[uriCount][0] = '"';
					    	    properties[uriCount][index+1] = '"';
					    	    properties[uriCount][index+2] = '\0';
					    	}
							objId = so.encodeLiteral(properties[uriCount]);
							//printf("@Dictionary::create 5 *** %d %d %d\n", subId, predId, objId);
							tripleTable.add(subId, predId, objId);
							tripleCount++;
							uriCount = 0;
							index = 0;
							if(tripleCount%divisor==0){
								this->stats();
								//cout << linesCount << " 4 " << tripleCount  << "."  << properties[0] << " " << properties[1] << " " << properties[2] << endl;
							}
							continue;
						}
						else if (properties[uriCount][index-3] != '@') {
					    	string str = properties[uriCount];
					    	if (str.find('"') != string::npos) {
					    	} else {
					    	    // shift existing elements to the right
					    	    for (int i = index-2; i >= 0; i--) {
					    	    	properties[uriCount][i+1] = properties[uriCount][i];
					    	    }
					    	    // insert new element at the beginning
					    	    properties[uriCount][0] = '"';
					    	    properties[uriCount][index-1] = '"';
					    	    properties[uriCount][index] = '\0';
					    	}
							objId = so.encodeLiteral(properties[uriCount]);
							//printf("@Dictionary::create 6 *** %d %d %d\n", subId, predId, objId);
							tripleTable.add(subId, predId, objId);
							tripleCount++;
							uriCount = 0;
							index = 0;
							if(tripleCount%divisor==0){
								this->stats();
								//cout << linesCount << " 5 " << tripleCount <<  "."  << properties[0] << " " << properties[1] << " " << properties[2] << endl;
							}
							continue;
						}
						else {
							properties[uriCount][index-2] = tmp;
							properties[uriCount][index] = '\0';
							cout << index<<"\n\n\nNO O: " << properties[uriCount] << endl << endl << endl;
							cout << linesCount << " " << tripleCount << "."  << properties[0] << " " << properties[1] << " " << properties[2] << endl;
							exit(1);
							//countDown = 2;
						}
					}
				}
			} else if(index>5 && properties[uriCount][index-1]=='.' && properties[uriCount][index-2]==' ' && ((properties[uriCount][index-3]=='@' && properties[uriCount][index-4]=='"')||(properties[uriCount][index-5]=='@' && properties[uriCount][index-6]=='"'))){
				properties[uriCount][index-2] = '\0';
				objId = so.encodeLiteral(properties[uriCount]);
				//printf("@Dictionary::create 7 *** %d %d %d\n", subId, predId, objId);
				tripleTable.add(subId, predId, objId);
				tripleCount++;
				uriCount = 0;
				index = 0;
				if(tripleCount%divisor==0){
					this->stats();
					//cout << linesCount << " " << tripleCount  << "."  << properties[0] << " " << properties[1] << " " << properties[2] << endl;
				}
				//continue;
			}
		}// else
		if(index < common.URI_MAX_LENGTH) {
			properties[uriCount][index++] = c;
		} else {
			cout << index << ": " << properties[uriCount] << endl;
			exit(1);
		}
	}
	is.close();
    regfree(&uri_regex);
    regfree(&literal_regex);
    regfree(&literal_regex_type);
    //exit(1);
	cout << "#Lines: " << linesCount;
	cout << "; #Triples: " << tripleCount;
	cout << "; Elapsed Time: " << common.getElapsedTime() << endl;
}
void Dictionary::flushDictionary(){
	unsigned long dicSize = so.flush(pathDir, (char*)"so");
	dicSize += p.flush(pathDir, (char*)"p");
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
	p.open(datasetPath, (char*)"p");
	so.open(datasetPath, (char*)"so");
}
void Dictionary::close(){
	p.close();
	so.close();
	tripleTable.close();
	delete[] uri;
}
ComponentId Dictionary::getSubObjId(char* uri){
	//cout << uri << endl;
	return so.getComponentId(uri);
}
ComponentId Dictionary::getPredicateId(char* uri){
	return p.getComponentId(uri);
}
char* Dictionary::getPredicate(ComponentId id){
	return p.getComponent(id);
}
char* Dictionary::getSubObj(ComponentId id){
	return so.getComponent(id);
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
