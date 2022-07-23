/* See LICENSE for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <inttypes.h>
#include <errno.h>
#include <wchar.h>

#include "xlseq.h"
#include "arg.h"

#define TRY_MATCH(TYPE, FUNC) if (match[TYPE-1]) { match[TYPE-1] = FUNC; }
#define MUST_BOUNDED(N) if (N <= 0) {fprintf(stderr, "%s: bounded sequence: need count (-n flag)\n", argv0); return 0;}

typedef enum {
	/* unbounded ranges */
	StringPattern = 1,	/* common prefix of arbitrary type */
	NumberPattern,		/* common format of double */

	/* bounded ranges */
	DaysPattern,
	MonthsPattern,

	UnrecognisedPattern	/* should never happen;
				   used to terminate the buffer */
} PatternType;

struct matcher_state {
	struct days_matcher_state days;
	struct months_matcher_state months;
};

char *argv0;

void
usage()
{
	fprintf(stderr, "%s: [-n count] -u\n", argv0);
	fprintf(stderr, "-n:\titerate n times (default: unbounded)\n");
	fprintf(stderr, "-u:\tthis message\n");
	exit(1);
}

PatternType *
_type_detect(const char *text)
{
	struct matcher_state state;
	int match[UnrecognisedPattern];
	PatternType *buf;
	int used = 0;
	const char *ptr = text;
	wchar_t rune;
	int count, len = strlen(text);

	memset(match, 1, sizeof(match));
	memset(&state, 0, sizeof(state));
	do {
		count = mbtowc(&rune, ptr, len-(ptr-text));
		if (count < 0) {
			fprintf(stderr, "%s: invalid text encoding\n", argv0);
			exit(1);
		}
		ptr += count;

		TRY_MATCH(StringPattern, string_pattern_match(rune));
		TRY_MATCH(NumberPattern, number_pattern_match(rune));
		TRY_MATCH(DaysPattern, days_pattern_match(rune, &state.days));
		TRY_MATCH(MonthsPattern, months_pattern_match(rune, &state.months));
	} while (*ptr);

	buf = malloc(sizeof(match));
	for (int i = 0; i < UnrecognisedPattern; i++) {
		if (match[i]) {
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
run_pattern(PatternType pat, int count, const char *in0, const char *in1)
{
	const char *iter;

	switch (pat) {
	case StringPattern:
		MUST_BOUNDED(count);
		break;
	case NumberPattern:
		MUST_BOUNDED(count);
		break;
	case DaysPattern:
	case MonthsPattern:
	case UnrecognisedPattern:
		break;
	default:
		fprintf(stderr, "%s: unknown pattern type %d\n", argv0, pat);
		return 2;
	}

	return 1;
}

int
main(int argc, char **argv)
{
	PatternType type = UnrecognisedPattern;
	int subcount = 0, count = -1;

	setlocale(LC_ALL, "");

	ARGBEGIN {
	case 'n':
		errno = 0;
		count = strtoimax(EARGF(usage()), NULL, 10);
		if (count == 0 || errno != 0)
			usage();
		break;
	case 'N':
		subcount = 1;
		break;
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

	if (subcount)
		count = abs(count-argc);

	return run_pattern(type, count, argv[argc-2], argv[argc-1]);
}
