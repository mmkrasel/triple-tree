#!/bin/bash

echo "===================================================================================="
echo "Compiling and Buidling ...."

for cpp_file in Common ComponentTree Compression Dictionary HashBucket16 HashLiterals HashSegmented Query TT TripleTable; do
	g++ -I/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -std=gnu++11 -MMD -MP -MF"src/$cpp_file.d" -MT"src/$cpp_file.o" -o "src/$cpp_file.o" "src/$cpp_file.cpp"
done

g++ -o TT  ./src/Common.o ./src/ComponentTree.o ./src/Compression.o ./src/Dictionary.o ./src/HashBucket16.o ./src/HashLiterals.o ./src/HashSegmented.o ./src/Query.o src/TT.o ./src/TripleTable.o -lboost_regex

echo "Executable has been build with name 'TT' ...."
echo "===================================================================================="
echo 
echo "===================================================================================="
echo "# Create the dictionary for an input RDF triples dataset."
echo "# 'tt_directory' is a directory to store all kinds of data produced from triple-tree"
echo "./TT -d path_to_source_dataset.nt tt_directory"
echo "===================================================================================="
echo 
echo "===================================================================================="
echo "# Compress the dictionary using TripleTriple data structure."
echo "# 'page_size' can be 4/8/16/32."
echo "./TT -c tt_directory page_size"
echo "===================================================================================="
echo 
echo "===================================================================================="
echo "# Search for a given pattern."
echo "# 's' indicates that algorithm will search through the path of p->s->o"
echo "# 'o' indicates that algorithm will search through the path of p->o->s"
echo "# Last 3 arguments represent the triple pattern S P O"
echo "# 'x' indicates an unbound triple componet"
echo 
echo "# Both of the following commands find all triples"
echo "./TT -s tt_directory page_size s x x x"
echo "./TT -s tt_directory page_size o x x x"
echo
echo "# Following commands find all triples having subject as http://www.Department0.University0.edu/UndergraduateStudent59"
echo "./TT -s tt_directory page_size s http://www.Department0.University0.edu/UndergraduateStudent59 x x"
echo 
echo "# Following commands find all triples having object as http://www.Department0.University0.edu/UndergraduateStudent59"
echo "./TT -s tt_directory page_size o x x http://www.Department0.University0.edu/UndergraduateStudent59"
echo "===================================================================================="
echo 

