CC = gcc
SHELL := /bin/bash

flags = -Wall -I./include

gateway.o: lib/gateway.c include/libdiscord/bot.h
	${CC} ${flags} -c lib/gateway.c -o build/gateway.o

wsc.o: lib/wsc.c include/libdiscord/wsc.h
	${CC} ${flags} -c lib/wsc.c -o build/wsc.o

rest.o: lib/rest.c include/libdiscord/rest.h
	${CC} ${flags} -c lib/rest.c -o build/rest.o

libdiscord.a: gateway.o wsc.o rest.o
	if [ ! -d "./build" ]; then mkdir build; fi
	ar crs ./build/libdiscord.a ./build/gateway.o ./build/wsc.o ./build/rest.o
	sudo mv build/libdiscord.a /usr/lib
	sudo cp -R include/libdiscord /usr/include

libs: libdiscord.a

clean:
	rm -rf build
	mkdir build