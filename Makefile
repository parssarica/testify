CC = gcc
SRCS = cli.c gui.c json.c main.c tester.c utils.c sds.c
TARGET = testify
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

# Production flags
CFLAGS_PROD = -Wall -Wextra -pedantic -O2

CFLAGS_DEBUG = -ggdb3 -O0 -DFORTIFY_SOURCE=3 -fsanitize=address -fstack-protector -Wall -Wextra -pedantic

all: prod

prod:
	$(CC) -o $(TARGET) $(SRCS) $(CFLAGS_PROD)

debug:
	$(CC) -o $(TARGET) $(SRCS) $(CFLAGS_DEBUG)

clean:
	rm -f $(TARGET)

install: prod
	install -Dm755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)

