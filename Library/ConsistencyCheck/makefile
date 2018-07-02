CC=g++
CXXFLAGS = -g $(shell root-config --cflags)
LDFLAGS = -g $(shell root-config --ldflags)
LDLIBS = $(shell root-config --libs) $(shell root-config --libs)

ConsistencyCheck: ConsistencyCheck.c
	$(CC) $(LDFLAGS) -o ConsistencyCheck ConsistencyCheck.c $(CXXFLAGS) $(LDLIBS)
