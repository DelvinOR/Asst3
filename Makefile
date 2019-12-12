all:
	gcc DUMBserver.c -o DUMBserver -lpthread
	gcc DUMBclient.c -o DUMBclient -lpthread

clean:
	rm -rf DUMBclient
	rm -rf DUMBserver
	