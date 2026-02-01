CC = gcc
SRCS = cli.c json.c main.c tester.c utils.c sds.c execute.c create.c regex.c complex.c
LIBS = -lcjson -lpcre2-8 -lm
TARGET = testify
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

# Production flags
CFLAGS_PROD = -Wall -Wextra -pedantic -O2

CFLAGS_ASAN = -ggdb3 -O0 -DFORTIFY_SOURCE=3 -fsanitize=address -fstack-protector -Wall -Wextra -pedantic
CFLAGS_DEBUG = -ggdb3 -O0 -Wall -Wextra -pedantic

all: prod

prod:
	$(CC) -o $(TARGET) $(SRCS) $(LIBS) $(CFLAGS_PROD)

debug:
	$(CC) -o $(TARGET) $(SRCS) $(LIBS) $(CFLAGS_DEBUG)

asan:
	$(CC) -o $(TARGET) $(SRCS) $(LIBS) $(CFLAGS_ASAN)

clean:
	rm -f $(TARGET)

install: prod
	install -Dm755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)

