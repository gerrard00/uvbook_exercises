CFLAGS=
LDFLAGS= -L /usr/local/lib

all:
	$(CC) main.c -o libuv-web-request.o -luv -I /usr/local/include $(LDFLAGS) $(CFLAGS)
