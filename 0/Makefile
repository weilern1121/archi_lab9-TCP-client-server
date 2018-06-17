all: client

client: client.o common.o line_parser.o
	gcc -g -Wall -m64 -o client client.o common.o line_parser.o
client.o: client.c
	gcc -g -Wall -m64 -c -o client.o client.c
common.o: common.c
	gcc -g -Wall -m64 -c -o common.o common.c
line_parser.o: line_parser.c
	gcc -g -Wall -m64 -c -o line_parser.o line_parser.c
clean:
	rm client client.o
