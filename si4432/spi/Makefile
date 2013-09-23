si4432:si4432.o spi.o si4432interrupt.o
	arm-linux-gcc -o si4432 si4432.o spi.o si4432interrupt.o --std=gnu99

si4432.o:si4432.c spi.h si4432interrupt.h
	arm-linux-gcc -c si4432.c --std=gnu99

spi.o:spi.c spi.h
	arm-linux-gcc -c spi.c --std=gnu99

si4432interrupt.o:si4432interrupt.c si4432interrupt.h
	arm-linux-gcc -c si4432interrupt.c --std=gnu99

clean:
	rm *.o

doen:
	arm-linux-gcc -o myspi main.c spi.c -std=gnu9
