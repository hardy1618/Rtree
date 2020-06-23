sampleobjects = buffer_manager.o file_manager.o rtree.o

rtree : $(sampleobjects)
	g++ -std=c++11 -o rtree $(sampleobjects)

rtree.o : rtree.cpp
	g++ -std=c++11 -c rtree.cpp

# sample : $(sampleobjects)
# 	g++ -std=c++11 -o rtree $(sampleobjects)

# sample_run.o : sample_run.cpp
# 	g++ -std=c++11 -c sample_run.cpp



buffer_manager.o : buffer_manager.cpp
	g++ -std=c++11 -c buffer_manager.cpp

file_manager.o : file_manager.cpp
	g++ -std=c++11 -c file_manager.cpp

run: rtree
	rm -f Files/*
	./rtree q1.txt 100 2 output.txt


clean :
	rm -f *.o
	rm -f Files/*
	rm -f rtree
