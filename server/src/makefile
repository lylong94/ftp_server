SOURCES:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SOURCES))
CFLAGS:= -I../include
server:$(OBJS)
	gcc $^ -o $@ -pthread -lcrypt -lmysqlclient
clean:
	rm -rf $(OBJS) server
