all: admin client

admin: main.c main.h
	clang -g -o admin main.c -lssl -lcrypto

client: client.c
	clang -g -o client client.c -lssl -lcrypto
