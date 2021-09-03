CC=g++

All:
	$(CC) -o sample sample.cpp `pkg-config --cflags --libs opencv`
server:
	$(CC) -o server server.cpp
clean:
	rm -rf sample
	rm -rf server
	rm *.jpg
