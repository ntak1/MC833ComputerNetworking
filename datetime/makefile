all: cliente.o servidor.o

servidor.o: servidor.c
	gcc -Wall servidor.c -o servidor.o

cliente.o: cliente.c
	gcc -Wall cliente.c -o cliente.o

clean:
	rm -rf *.o