/* See LICENSE for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>

#include "xlseq.h"
#include "arg.h"

#define TRY_MATCH(TYPE, FUNC) if (!out[TYPE-1]) { out[TYPE-1] = !FUNC; }

typedef enum {
	/* unbounded ranges */
	StringPattern = 1,	/* common prefix of arbitrary type */
	NumberPattern,		/* common format of double */

	/* bounded ranges */
	/* DaysPattern, */
	/* MonthsPattern, */

	UnrecognisedPattern	/* should never happen;
				   used to terminate the buffer */
} PatternType;

struct matcher_state {
};

char *argv0;

void
usage()
{
	exit(1);
}

PatternType *
_type_detect(const char *text)
{
	struct matcher_state state;
	int out[UnrecognisedPattern];
	PatternType *buf;
	int used = 0;
	const char *ptr = text;
	wchar_t rune;
	int count, len = strlen(text);

	memset(out, 0, sizeof(out));
	do {
		count = mbtowc(&rune, ptr, len-(ptr-text));
		if (count < 0) {
			fprintf(stderr, "%s: invalid text encoding\n", argv0);
			exit(1);
		}
		ptr += count;

		TRY_MATCH(StringPattern, string_pattern_match(rune));
		TRY_MATCH(NumberPattern, number_pattern_match(rune));
	} while (*ptr);

	buf = malloc(sizeof(out));
	for (int i = 0; i < UnrecognisedPattern; i++) {
		if (!out[i]) {
			buf[used++] = i+1;
		}
	}

	return buf;
}

PatternType
type_detect(const char *first, const char *second)
{
	PatternType *poss, *tmp;
	PatternType *poss1, *poss2;
	PatternType max = 0;
	poss1 = _type_detect(first);
	poss2 = _type_detect(second);

	/* assumes both buffers are same length */
	for (poss = poss1; *poss != UnrecognisedPattern; poss++) {
		if (*poss > max) {
			for (tmp = poss2; *tmp != UnrecognisedPattern; tmp++) {
				if (*tmp == *poss) {
					max = *poss;
					break;
				}
			}
		}
	}
	free(poss1);
	free(poss2);

	if (!max) {
		fprintf(stderr, "%s: unrecognised pattern\n", argv0);
		exit(1);
	}

	return max;
}

int
main(int argc, char **argv)
{
	PatternType type = UnrecognisedPattern;
	int count = -1;

	setlocale(LC_ALL, "");

	ARGBEGIN {
	case 'u':
		usage();
		/* NOTREACHED */
	default:
		fprintf(stderr, "%s: unknown flag '%c'\n", argv0, ARGC());
		return 1;
	} ARGEND;

	if (argc < 2) {
		fprintf(stderr, "%s: need pattern of at least two items\n", argv0);
		return 1;
	}

	if (type == UnrecognisedPattern)
		type = type_detect(argv[argc-2], argv[argc-1]);
}
