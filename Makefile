
CXX=g++
CFLAGS = -std=gnu++17 -lgfortran -Wall -O3 -w -mavx
INC=-I faiss -I include/
LFLAGS=faiss/build/faiss/libfaiss.a OpenBLAS/libopenblas.a -lpthread -lm -ldl -lgfortran -fopenmp

index: clean_index
	$(CXX) $(INC) $(CFLAGS) src/readfile.cpp \
							src/utils.cpp \
							src/FilterIndexHamming.cpp \
							src/index.cpp \
	-o index $(LFLAGS)

index_construction: clean_index_construction
	$(CXX) $(INC) $(CFLAGS) src/readfile.cpp \
							src/utils.cpp \
							src/FilterIndexHamming.cpp \
							src/index_construction.cpp \
	-o index_construction $(LFLAGS)


query: clean_query
	$(CXX) $(INC) $(CFLAGS) src/readfile.cpp \
							src/utils.cpp \
							src/FilterIndexHamming.cpp \
							src/query.cpp \
	-o query $(LFLAGS)

query_execution: clean_query_execution
	$(CXX) $(INC) $(CFLAGS) src/readfile.cpp \
							src/utils.cpp \
							src/FilterIndexHamming.cpp \
							src/query_execution.cpp \
	-o query_execution $(LFLAGS)

	
clean_index:
	rm -f index
clean_index_construction:
	rm -f index_construction
clean_query:
	rm -f query
clean_query_execution:
	rm -f query_execution

.PHONY: clean all

debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O2
release: all
