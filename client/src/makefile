SOURCES:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SOURCES))
CFLAGS:= -I../include
client:$(OBJS)
	gcc $^ -o $@ -pthread -lcrypt
clean:
	rm -rf $(OBJS) client
