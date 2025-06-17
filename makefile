CC = gcc
CFLAGS = -Wall -g

SRCS = neHosServer.c neHosClient.c neHosLib.h
OBJS = neHosServer.o neHosClient.o
TARGETS = neHosServer neHosClient

all: $(TARGETS)

neHosServer: neHosServer.c
	$(CC) $(CFLAGS) neHosServer.c -o neHosServer -lpthread

neHosClient: neHosClient.c
	$(CC) $(CFLAGS) neHosClient.c -o neHosClient -lpthread

clean:
	rm -f $(TARGETS)
