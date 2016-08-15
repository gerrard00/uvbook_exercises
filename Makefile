all:
	gcc main.c -o libuv-web-request.o -luv -I /usr/local/include -L /usr/local/lib -framework CoreFoundation -framework CoreServices
