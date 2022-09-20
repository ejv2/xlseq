# xlseq - excel sequence continuer
# Copyright (C) 2022 - Ethan Marshall
.POSIX:

SRC = xlseq.c patt.c sspec.c
OBJ = ${SRC:.c=.o}
HDR = arg.h sspec.h
EXE = xlseq

CFLAGS = -Wall -Wpedantic -Wextra
include config.mk

${EXE}: ${OBJ}
	${CC} -o ${EXE} ${XLLDFLAGS} ${OBJ}

.c.o:
	${CC} -c ${XLCFLAGS} $<

clean:
	rm -f ${EXE} ${OBJ}

install: $(EXE)
	mkdir -p ${DESTDIR}$(PREFIX)/bin
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -f $(EXE) $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(EXE)
	cp -f ${EXE}.1 ${DESTDIR}${MANPREFIX}/man1/${EXE}.1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(EXE)

.PHONY: clean install uninstall
