all: build

build: server.cpp client.cpp
	g++ -g -lnsl server.cpp -o server
	g++ -g -lnsl client.cpp -o client

clean:
	rm -f *.o *~ server client
