# uwp version
VERSION = 0.0.1

# Customize the below to fit your system
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

LIBS = -lm
XLCPPFLAGS = -DVERSION=\"$(VERSION)\" -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=600
XLCFLAGS   = $(XLCPPFLAGS) $(CFLAGS)
XLLDFLAGS  = $(LIBS) $(LDFLAGS)
