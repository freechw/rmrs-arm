CPP=arm-linux-gcc
CC=arm-linux-gcc
CFLAGS= -c -Wall -Wshadow -std=gnu99
LDFLAGS= -lpthread -lstdc++

all: test

test: main.o net.o unit.o reader.o si4432.o spi.o c_spi.o record.o
	$(CPP) $(LDFLAGS) -o test main.o net.o unit.o reader.o si4432.o spi.o c_spi.o record.o
main.o:main.cpp
	$(CPP) $(CFLAGS) main.cpp
net.o:net.cpp
	$(CPP) $(CFLAGS) net.cpp
unit.o:unit.cpp
	$(CPP) $(CFLAGS) unit.cpp
reader.o:reader.cpp
	$(CPP) $(CFLAGS) reader.cpp
si4432.o:si4432.cpp
	$(CPP) $(CFLAGS) si4432.cpp
spi.o:spi.cpp
	$(CPP) $(CFLAGS) spi.cpp
c_spi.o:c_spi.c
	$(CC) $(CFLAGS) c_spi.c
record.o:record.cpp
	$(CPP) $(CFLAGS) record.cpp

clean:
	rm -rf *.o test
trans:
	scp ./test arm:/App/newtest
