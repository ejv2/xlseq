/* See LICENSE for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <inttypes.h>
#include <errno.h>
#include <wchar.h>

#include "xlseq.h"
#include "util.h"
#include "arg.h"

#define TRY_MATCH(TYPE, FUNC) if (match[TYPE-1]) { match[TYPE-1] = FUNC; }
#define MUST_BOUNDED(N) if (N <= 0) {fprintf(stderr, "%s: bounded sequence: need count (-n flag)\n", argv0); return 1;}

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
	struct buffered_matcher_state days;
	struct buffered_matcher_state months;
};

char *argv0;

void
usage()
{
	fprintf(stderr, "%s: [-cn count] -u\n", argv0);
	fprintf(stderr, "-c:\tcontinue pattern for n iterations (default: unbounded)\n");
	fprintf(stderr, "-n:\tshow n results in total (default: unbounded)\n");
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
		TRY_MATCH(DaysPattern, buffered_pattern_match(rune, &state.days, days, LENGTH(days)));
		TRY_MATCH(MonthsPattern, buffered_pattern_match(rune, &state.months, months, LENGTH(months)));
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
run_pattern(PatternType pat, int count, int startind, union sample_space samples)
{
	switch (pat) {
	case StringPattern:
		MUST_BOUNDED(count);
		string_pattern_run(samples, count);
		break;
	case NumberPattern:
		MUST_BOUNDED(count);
		number_pattern_run(samples, count, startind);
		break;
	case DaysPattern:
		buffered_pattern_run(samples, count, days, LENGTH(days));
		break;
	case MonthsPattern:
		buffered_pattern_run(samples, count, months, LENGTH(months));
		break;
	case UnrecognisedPattern:
		break;
	default:
		fprintf(stderr, "%s: unknown pattern type %d\n", argv0, pat);
		return 2;
	}

	return 0;
}

int
main(int argc, char **argv)
{
	union sample_space samples;
	PatternType type = UnrecognisedPattern;
	int i, startind, success;
	int subcount = 0, count = -1;

	setlocale(LC_ALL, "");

	ARGBEGIN {
	case 'n':
		subcount = 1;
		/* FALLTHROUGH */
	case 'c':
		errno = 0;
		count = strtoimax(EARGF(usage()), NULL, 10);
		if (count == 0 || errno != 0)
			usage();
		break;
	case 'u':
		usage();
		/* NOTREACHED */
	case 'v':
		printf("xlseq v%s\n", VERSION);
		return 1;
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

	if (subcount) {
		count = count-argc;
		if (count <= 0) {
			fprintf(stderr, "%s: net count %d smaller than sequence sample\n", argv0, count);
			return 1;
		}
	}

	samples = (union sample_space){
		.samples = {
			argv[argc-1],
			argv[argc-2],
			(argc > 2) ? argv[argc-3] : NULL
		}
	};
	startind = (argc > 2) ? argc-2 : argc-1;

	/* print sample set */
	for (i = 0; i < argc; i++) {
		printf("%s ", argv[i]);
	}
	success = run_pattern(type, count, startind, samples);
	putchar('\n');
	return success;
}
