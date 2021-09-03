CC=gcc
CFLAGS=
OBJS=
LDFLAGS=
LDLIBS=-lwiringPi

ex1.so : ex1.o
	$(CC) -shared -o $@ $^ $(LDLIBS)
ex1.o : ex1.c
	$(CC) -c -fPIC $^ $(LDLIBS)

ex2.so : ex2.o
	$(CC) -shared -o $@ $^ 
ex2.o : ex2.c
	$(CC) -c -fPIC $^ -I/usr/include/python3.7

ex3.so : ex3.o
	$(CC) -shared -o $@ $^ $(LDLIBS)
ex3.o : ex3.c
	$(CC) -c -fPIC $^ -I/usr/include/python3.7

clean:
	rm -rf ex1.o ex1.so
	rm -rf ex2.o ex2.so
	rm -rf ex3.o ex3.so

