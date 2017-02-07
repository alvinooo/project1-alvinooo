
CC=g++
CFLAGS=-ggdb -Wall -Wextra -pedantic -Werror -std=c++0x
DEPS = httpd.h httpd_util.h Stream.h Request.h Response.h Thread.h
SRCS = httpd.cpp httpd_util.cpp Stream.cpp Request.cpp Response.cpp Thread.cpp
MAIN_SRCS = main.c $(SRCS)
MAIN_OBJS = $(MAIN_SRCS:.c=.o)

default: httpd

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

httpd:    $(MAIN_OBJS)
	$(CC) $(CFLAGS) -o httpd $(MAIN_OBJS) -lpthread

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f httpd *.o
