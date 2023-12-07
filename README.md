# Triple Tree
A technique to compress RDF triples and query on compressed RDF triples data

**# Dependencies**

	Packages: (Install boost and rapidjson library and keep it in the path of /usr/include/)
 		libboost-all-dev version 1.71.0
   		rapidjson-dev=1.1.0
	Compiler: g++ 
	Note: User **build.sh** to compile and build

**#Create dictionary from triple datatsets**

	./TT -d path_to_source_NT_dataset_file path_to_triple_tree_directory


**#Compress TripleTable to TripleTree**

	./TT -c path_to_triple_tree_directory page_size
	
	Note: 
		- page_size can be 1 or 2 or 3 or 4 .........16..........32...
		- Here, 1 means 1kb
	
	


**#Search for triple pattern (with uri-component ids)**

	./TT -q path_to_destination_triple_tree_directory subject_or_object_path triple_pattern


 
	Note: 
		- subject_or_object_path: 
			- value can be either s or o. 
			- If s is used, then the search query will use the p->s->o paths
			- If o is used, then the search query will use the p->o->s paths
			
		- triple_pattern: s p o
			- unbounded subject/predicate/object will be represented wit x;   x x x
			- bounded subject/predicate/object is represented by their ID; such as 1 1 2, where 1, 1, and 2 represent the subject, predicate, and object ID, respectively.
			- Some patters: 
				x x x: Extracts all triples
				x 1 x: Extracts all triples with predicate id 1
				1 x x: Extracts all triples with subject id 1
				x x 2: Extracts all triples with object id 2
				1 1 x: Extracts all triples with predicate id 1 and subject id 1
				x 1 2: Extracts all triples with predicate id 1 and object id 2
				1 x 2: Extracts all triples with subject id 1 and object id 2
				1 1 2: Find the triple in triple tree

**#Example query: 	Find all triples having object_id "2" and predicate_id "1". Use the p->o->s path of the triple tree for searching the triples.**
 
    ./TT -q ../dbpedia/triple_tree o x 1 2	




**#Search for triple pattern (with uri)**

	./TT -s path_to_triple_tree_directory subject_or_object_path triple_pattern

  Note: 
    - subject_or_object_path: 
      - value can be either s or o. 
      - If s is used, then the search query will use the p->s->o paths
      - If o is used, then the search query will use the p->o->s paths
      
    - triple_pattern: s p o
      - unbounded subject/predicate/object will be represented wit x;   x x x
      - Some patters: 
   
      - Extracts all triples 
        x x x
  
      - Extracts all triples with predicate "http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#telephone" 
          x http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#telephone x
          
      - Extracts all triples with subject "http://www.Department6.University81.edu/GraduateStudent33"
        http://www.Department6.University81.edu/GraduateStudent33 x x
        
      - Extracts all triples with predicate "http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#telephone" and subject "http://www.Department6.University81.edu/GraduateStudent33"
        http://www.Department6.University81.edu/GraduateStudent33 http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#telephone x



**#Example query: 	Find all triples having subject "http://www.Department6.University81.edu/GraduateStudent33". Use the p->s->o path of the triple tree for searching the triples.**
		
    ./TT -q ../lubm/triple_tree s http://www.Department6.University81.edu/GraduateStudent33 x x


  
