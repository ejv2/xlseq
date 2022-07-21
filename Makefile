# xlseq - excel sequence continuer
# Copyright (C) 2022 - Ethan Marshall
.POSIX:

SRC = xlseq.c
OBJ = ${SRC:.c=.o}
HDR = arg.h
EXE = xlseq

include config.mk

${EXE}: ${OBJ}
	${CC} -o ${EXE} ${XLLDFLAGS} ${OBJ}

.c.o:
	${CC} -c ${XLCFLAGS} $<

clean:
	rm -f ${EXE} ${OBJ}

.PHONY: clean
