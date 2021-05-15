CC = gcc

flags = -Wall -I./include

gateway.o: lib/gateway.c include/libdiscord/bot.h
	${CC} ${flags} -c lib/gateway.c -o build/gateway.o

wsc.o: lib/wsc.c include/libdiscord/wsc.h
	${CC} ${flags} -c lib/wsc.c -o build/wsc.o

rest.o: lib/rest.c include/libdiscord/rest.h
	${CC} ${flags} -c lib/rest.c -o build/rest.o

libdiscord.a: gateway.o wsc.o rest.o
	ar crs ./build/libdiscord.a ./build/gateway.o ./build/wsc.o ./build/rest.o

libs: libdiscord.a

clean:
	rm -rf build
	mkdir build