//============================================================================
// Name        : TT.cpp
// Author      : Rasel
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

//valgrind --leak-check=full  --track-origins=yes ./TT -q

#include <iostream>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include "Common.h"
#include "TripleTable.h"
#include "Dictionary.h"
#include "Compression.h"
#include "Query.h"
#include <vector>
#include <regex>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <sys/stat.h>


#pragma pack(push, 1)

using namespace std;
using namespace rapidjson;

// Create a dictionary from a given triple dataset
// ./TT -d /media/rasel/hdd/RDF/datasets/lubm/paper/100m 16

// ./TT -c /media/rasel/hdd/RDF/datasets/lubm/paper/100m 16
// ./TT -q /media/rasel/hdd/RDF/datasets/lubm/paper/100m 16 o x x x


double pet(Query query, char* secondary, const char* s, const char* p, const char* o){

	if(p[0]=='?' && s[0] == '?' && o[0]=='?'){  // ? ? ?
		if(secondary[0] == 's'){
			query.getAllByPSO();
		} else if(secondary[0] == 'o'){
			query.getAllByPOS();
		}
	} else if(p[0]=='?' && o[0]=='?'){			// s ? ?
		query.getPO4S(atoi(s));
	} else if(p[0]=='?' && s[0]=='?'){			// ? ? o
		query.getPS4O(atoi(o));
	} else if(p[0]=='?'){						// s ? o
		if(secondary[0] == 's'){
			query.getP4SO(atoi(s), atoi(o));
		} else if(secondary[0] == 'o'){
			query.getP4OS(atoi(o), atoi(s));
		}
	}
	else if(s[0] == '?' && o[0]=='?'){			// ? p ?
		if(secondary[0] == 's'){
			query.getSO4P(atoi(p));
		} else if(secondary[0] == 'o'){
			query.getOS4P(atoi(p));
		}
	} else if(o[0]=='?'){						// s p ?
		query.getO4PS(atoi(p), atoi(s));
	}
	else if(s[0]=='?'){							// ? p o
		query.getS4PO(atoi(p), atoi(o));
	}
	else {										// s p o
		if(secondary[0] == 's'){
			query.getPSO(atoi(p), atoi(s), atoi(o));
		} else if(secondary[0] == 'o'){
			query.getPOS(atoi(p), atoi(o), atoi(s));
		}
	}
	cout << "#triples: " << query.resultCount << endl;
	return 0.0;
}

//void testSubObj(Dictionary* dic, char* searchFor){
//	cout << "Searching..." << endl;
//	SubObjId subID = dic->getSubObjId(searchFor);
//	if(subID > 0){
//		cout << "subID: " << subID << endl;
//		cout << dic->getSubObj(subID) << endl;
//	} else {
//		cout << "Not available" << endl;
//	}
//}
//
//void testPredicate(Dictionary* dic, char* searchFor){
//	cout << "Searching..." << endl;
//	SubObjId pID = dic->getPredicateId(searchFor);
//	if(pID > 0){
//		cout << "pID: " << pID << endl;
//		cout << dic->getPredicate(pID) << endl;
//	} else {
//		cout << "Not available" << endl;
//	}
//}
ComponentId getSubObjId(Dictionary* dic, string searchFor){
	searchFor.erase(remove(searchFor.begin(), searchFor.end(), '<'), searchFor.end());
	searchFor.erase(remove(searchFor.begin(), searchFor.end(), '>'), searchFor.end());
	searchFor.erase(remove(searchFor.begin(), searchFor.end(), '"'), searchFor.end());
	//cout << searchFor << endl;
	return dic->getSubObjId(&searchFor[0]);
}
ComponentId getPredicateId(Dictionary* dic, string searchFor){
	searchFor.erase(remove(searchFor.begin(), searchFor.end(), '<'), searchFor.end());
	searchFor.erase(remove(searchFor.begin(), searchFor.end(), '>'), searchFor.end());
	searchFor.erase(remove(searchFor.begin(), searchFor.end(), '"'), searchFor.end());
	//cout << searchFor << endl;
	return dic->getPredicateId(&searchFor[0]);
}

struct Triple {
    string subject;
    string predicate;
    string object;
};

vector<Triple> parseN3Triples(string n3Data) {
    // Define regular expressions to extract triples
    regex tripleRegex("(.*?)\\s+(.*?)\\s+(.*?)[\\.\\;]");
    regex literalRegex("\"(.*?)\"(@\\w+)?", regex_constants::ECMAScript | regex_constants::icase);

    // Parse N3 data into triples
    vector<Triple> triples;
    smatch match;
    while (regex_search(n3Data, match, tripleRegex)) {
        Triple triple;
        triple.subject = match[1].str();
        triple.predicate = match[2].str();
        triple.object = match[3].str();

        // Check if object is a literal
        if (regex_match(triple.object, literalRegex)) {
            smatch literalMatch;
            regex_search(triple.object, literalMatch, literalRegex);
            triple.object = literalMatch[1].str();

            // Consume additional lines if literal has newlines
            size_t pos = n3Data.find(literalMatch[0].str()) + literalMatch[0].str().size();
            while (pos < n3Data.size() && n3Data[pos] == '\n') {
                pos++;
                size_t endpos = n3Data.find("\".", pos);
                if (endpos == string::npos) {
                    // Literal extends beyond current buffer, read more data
                    char buffer[1024*1024*4];
                    streamsize bytesread = fread(buffer, 1, sizeof(buffer), stdin);
                    n3Data.append(buffer, bytesread);
                    endpos = n3Data.find("\".", pos);
                    if (endpos == string::npos) {
                        // Literal still extends beyond current buffer, give up
                        break;
                    }
                }
                triple.object += n3Data.substr(pos, endpos - pos + 1);
                pos = endpos + 2;
            }
        }

        // Add triple to vector
        //triples.push_back(triple);
        cout << triple.subject << "\n-----\n" << triple.predicate << "\n-----\n" << triple.object << "\n======\n" << endl;

        // Remove parsed triple from N3 data
        n3Data = match.suffix().str();
    }

    return triples;
}

int n3parser() {

    // Open N3 data file
    ifstream file("/media/rasel/hdd/RDF/datasets/dbpedia/dbpedia.nt");
    if (!file.is_open()) {
        cerr << "Failed to open file." << endl;
        return 1;
    }

    // Regular expression to match an NT-triple
    regex pattern(R"(([^ ]+) +([^ ]+) +((?:<[^>]+>)|(?:"(?:\\.|[^\\"])*"))(?:\^\^<[^>]+>)? *\.)");

    smatch match;
    string line;
    string current_triple;
    while (getline(file, line)) {
        // Add current line to current NT-triple
        current_triple += line;
        // If current NT-triple ends with a '.', process it
        if (regex_search(current_triple, match, pattern) && match[0].matched) {
            cout << match[1] << " " << match[2] << " " << match[3] << endl;
            current_triple.clear();
        }
    }

    file.close();

    return 0;
}

int rdfParser() {

  ifstream ifs("/media/rasel/hdd/RDF/datasets/dbpedia/dbpedia.nt");
  IStreamWrapper isw(ifs);
  Document doc;

  doc.ParseStream(isw);

  if (!doc.IsArray()) {
    std::cerr << "Invalid NT triples file format." << std::endl;
    return 1;
  }

  for (SizeType i = 0; i < doc.Size(); i++) {
    Value& triple = doc[i];
    if (!triple.IsArray() || triple.Size() != 3) {
      std::cerr << "Invalid triple format at index " << i << std::endl;
      continue;
    }

    string subject = triple[0].GetString();
    string predicate = triple[1].GetString();
    string object = triple[2].GetString();

    std::cout << "Triple #" << i + 1 << ": " << subject << " " << predicate << " " << object << std::endl;
  }

  return 0;
}

bool createDirTree(const std::string full_path){
    size_t pos = 0;
    bool ret_val = true;
    while(ret_val == true && pos != std::string::npos){
        pos = full_path.find('/', pos + 1);
        if(mkdir(full_path.substr(0, pos).c_str(), 755) == -1){
        	ret_val = false;
        }
    }
    return ret_val;
}

int main(int argc, char** argv) {
	//rdfParser();
	//n3parser();
	//exit(1);
	//cout << "#params: " << argc << endl;
	if(argc > 1){
		if(argv[1][1] == 'd' && argc == 4){
			Dictionary dic;
			createDirTree(argv[3]);
			dic.create(argv[2], argv[3]);
			//test(dic);b
			//dic.uri_to_ID(argv[2], false);
			dic.flushDictionary();
			dic.close();
		}
//		else if(argv[1][1] == 'l' && argc >= 3){
//			Dictionary dic;
//			dic.open(argv[2]);
//			//test(dic);
//			char* searchFor = "http://www.Department0.University0.edu/UndergraduateStudent58";
//			if(argc>=4){
//				searchFor = argv[3];
//			}
//			char type = 'p';
//			if(argc == 5){
//				type = argv[4][0];
//			}
//			cout << "Searching..." << type << ": " << searchFor << endl;
//			if(type=='p'){
//				testPredicate(&dic, searchFor);
//			} else {
//				testSubObj(&dic, searchFor);
//			}
//			dic.close();
//		}
		else if(argv[1][1] == 'c' && argc == 4){
			Compression com;
			com.start(atoi(argv[3]), argv[2]);
			com.end();
		}
//		else if(argv[1][1] == 't' && argc == 5){
//			Query query;
//			query.load(atoi(argv[4]), argv[2]);
//			ifstream infile(argv[3]);
//			if (infile.is_open()) {
//				string s;
//				string p;
//				string o;
//				string str = "";
//				char s1, s2;
//				int i = 0;
//				while (getline (infile, str)){
//					istringstream stream(str);
//					stream >> s >> p >> o;
//					if(atoi(p.c_str())>15){ continue;}
//					cout << s << " " << p << " " << o << "\n";
//					pet(query, "o", s.c_str(), p.c_str(), o.c_str());
//				}
//			}
//			infile.close();
//		}
		else if(argv[1][1] == 'q' && argc == 8){
			Query query;
			query.load(atoi(argv[argc-5]), argv[2]);
			//exit(1);
			// pattern as s p o
			char* s = argv[argc-3];
			char* p = argv[argc-2];
			char* o = argv[argc-1];

			if(p[0]=='x' && s[0] == 'x' && o[0]=='x'){  // ? ? ?
				if(argv[argc-4][0] == 's'){
					query.getAllByPSO();
				} else if(argv[argc-4][0] == 'o'){
					query.getAllByPOS();
				}
			} else if(p[0]=='x' && o[0]=='x'){			// s ? ?
				query.getPO4S(atoi(s));
			} else if(p[0]=='x' && s[0]=='x'){			// ? ? o
				query.getPS4O(atoi(o));
			} else if(p[0]=='x'){						// s ? o
				if(argv[argc-4][0] == 's'){
					query.getP4SO(atoi(s), atoi(o));
				} else if(argv[argc-4][0] == 'o'){
					query.getP4OS(atoi(o), atoi(s));
				}
			}
			else if(s[0] == 'x' && o[0]=='x'){			// ? p ?
				if(argv[argc-4][0] == 's'){
					query.getSO4P(atoi(p));
				} else if(argv[argc-4][0] == 'o'){
					query.getOS4P(atoi(p));
				}
			} else if(o[0]=='x'){						// s p ?
				query.getO4PS(atoi(p), atoi(s));
			}
			else if(s[0]=='x'){							// ? p o
				query.getS4PO(atoi(p), atoi(o));
			}
			else {										// s p o
				if(argv[argc-4][0] == 's'){
					query.getPSO(atoi(p), atoi(s), atoi(o));
				} else if(argv[argc-4][0] == 'o'){
					query.getPOS(atoi(p), atoi(o), atoi(s));
				}
			}
			//
			query.printResult();
			cout << "#triples: " << query.resultCount << endl;
			query.clearMemory();
		}
		else if(argv[1][1] == 's' && argc == 8){
			cout << "Loading Dictionary..." << endl;
			Dictionary dic;
			dic.open(argv[2]);
			cout << "Loading Tertiary Table..." << endl;
			Query query;
			query.load(atoi(argv[argc-5]), argv[2]);

			//cout << dic.getSubObj(510) << "\t" << dic.getPredicate(1) << "\t" << dic.getSubObj(33) << endl;
			//cout << dic.getSubObj(510) << "\t" << dic.getPredicate(3) << "\t" << dic.getSubObj(511) << endl;
			//cout << dic.getSubObj(510) << "\t" << dic.getPredicate(10) << "\t" << dic.getSubObj(512) << endl;
			//cout << dic.getSubObj(510) << "\t" << dic.getPredicate(11) << "\t" << dic.getSubObj(20) << endl;
			//cout << dic.getSubObj(510) << "\t" << dic.getPredicate(14) << "\t" << dic.getSubObj(7) << endl;
			//cout << dic.getSubObj(510) << "\t" << dic.getPredicate(15) << "\t" << dic.getSubObj(65) << endl;
			//cout << dic.getSubObj(510) << "\t" << dic.getPredicate(15) << "\t" << dic.getSubObj(75) << endl;
			//cout << dic.getSubObj(510) << "\t" << dic.getPredicate(15) << "\t" << dic.getSubObj(91) << endl;
			//cout << dic.getSubObj(510) << "\t" << dic.getPredicate(15) << "\t" << dic.getSubObj(322) << endl;
			//exit(1);
			// pattern as s p o
			char* s = argv[argc-3];
			char* p = argv[argc-2];
			char* o = argv[argc-1];
			cout << "Start Searching..." << endl;
			if(p[0]=='x' && s[0] == 'x' && o[0]=='x'){  // ? ? ?
				if(argv[argc-4][0] == 's'){
					query.getAllByPSO();
				} else if(argv[argc-4][0] == 'o'){
					query.getAllByPOS();
				}
			} else if(p[0]=='x' && o[0]=='x'){			// s ? ?
				uint32 subId = getSubObjId(&dic, s);
				if(subId > 0){
					query.getPO4S(subId);
				}
			} else if(p[0]=='x' && s[0]=='x'){			// ? ? o
				uint32 objId = getSubObjId(&dic, o);
				if(objId > 0){
					query.getPS4O(objId);
				}
			} else if(p[0]=='x'){						// s ? o
				uint32 subId = getSubObjId(&dic, s);
				if(subId > 0){
					uint32 objId = getSubObjId(&dic, o);
					if(objId > 0){
						if(argv[argc-4][0] == 's'){
							query.getP4SO(subId, objId);
						} else if(argv[argc-4][0] == 'o'){
							query.getP4OS(objId, subId);
						}
					}
				}
			}
			else if(s[0] == 'x' && o[0]=='x'){			// ? p ?
				uint32 pid = getPredicateId(&dic, p);
				if(pid > 0){
					if(argv[argc-4][0] == 's'){
						query.getSO4P(pid);
					} else if(argv[argc-4][0] == 'o'){
						query.getOS4P(pid);
					}
				}
			} else if(o[0]=='x'){						// s p ?
				uint32 pid = getPredicateId(&dic, p);
				if(pid > 0){
					uint32 sid = getSubObjId(&dic, s);
					if(sid > 0){
						query.getO4PS(pid, sid);
					}
				}
			}
			else if(s[0]=='x'){							// ? p o
				uint32 pid = getPredicateId(&dic, p);
				if(pid > 0){
					uint32 oid = getSubObjId(&dic, o);
					if(oid > 0){
						query.getS4PO(pid, oid);
					}
				}
			}
			else {										// s p o
				uint32 pid = getPredicateId(&dic, p);
				if(pid > 0){
					uint32 oid = getSubObjId(&dic, o);
					if(oid > 0){
						uint32 sid = getSubObjId(&dic, s);
						if(sid > 0){
							if(argv[argc-4][0] == 's'){
								query.getPSO(pid, sid, oid);
							} else if(argv[argc-4][0] == 'o'){
								query.getPOS(pid, oid, sid);
							}
						}
					}
				}
			}
			//
			query.printResult();
			if(query.resultCount > 0){
				for(uint32 i=0; i<query.resultCount; i++){
					cout << dic.getSubObj(query.results[i].sub) << "\t" << dic.getPredicate(query.results[i].pred) << "\t" << dic.getSubObj(query.results[i].obj) << endl;
				}
			}
			cout << "#triples: " << query.resultCount << endl;
			query.clearMemory();
			dic.close();
		}
		else {
			cout << "Unknown action type" << endl;
		}
	} else {
		cout << "Please provide action type" << endl;
	}
	return 0;
}

