# xlseq - excel sequence continuer
# Copyright (C) 2022 - Ethan Marshall
.POSIX:

SRC = xlseq.c patt.c
OBJ = ${SRC:.c=.o}
HDR = arg.h
EXE = xlseq

CFLAGS = -Wall -Wpedantic
include config.mk

${EXE}: ${OBJ}
	${CC} -o ${EXE} ${XLLDFLAGS} ${OBJ}

.c.o:
	${CC} -c ${XLCFLAGS} $<

clean:
	rm -f ${EXE} ${OBJ}

install: $(EXE)
	mkdir -p ${DESTDIR}$(PREFIX)/bin
	cp -f $(EXE) $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(EXE)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(EXE)

.PHONY: clean install uninstall
