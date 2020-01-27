#Author: Michael Brown
#Date: 11/22/2019
#Filename: makefile
#Purpose: Makefile for download server application

all: client server

client: client.c libraryname.a
	gcc client.c libraryname.a -o client

server: server.c libraryname.a
	gcc server.c libraryname.a -o server

libraryname.a: downloadlibrary.o
	ar rc libraryname.a downloadlibrary.o

downloadlibrary.o: downloadlibrary.c
	gcc -c downloadlibrary.c