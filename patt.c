/* See LICENSE for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <wctype.h>

#include "util.h"
#include "xlseq.h"

/* abstraction over mathematical sequence
 * sequence is defined as ax^n ... nx^1 + c
 * highest supported order is x^3, although it could (in theory) be extended to
 * x^255 with the current design
 */
typedef struct {
	unsigned char order;
	int coff[3];	/* coefficients in LE order (coff[0] is coefficient of x^0) */
			/* coff[0] is therefore the constant term (nx^0 === n === c) */
} NumSeq;

int
string_pattern_match(const wchar_t rune)
{
	return 1;
}

void
string_pattern_run(union sample_space samples, int count)
{
	int i;
	long suffix;
	size_t ind;
	const char *cend = NULL;
	char *endptr, *buf;
	const char *walk;

	for (walk = samples.ordered.last; *walk; walk++) {
		ind = walk-samples.ordered.last;
		if (!samples.ordered.middle[ind])
			break;
		else if (samples.ordered.middle[ind] != *walk) {
			break;
		}

		cend = walk;
	}

	if (cend && *cend) {
		cend++;
		suffix = strtol(cend, &endptr, 10);
		buf = calloc(sizeof(char), cend - samples.ordered.last + 1);
		if (!buf) {
			perror("buf allocation");
			return;
		}
		strncpy(buf, samples.ordered.last, cend - samples.ordered.last);

		for (int i = 0; i < count; i++) {
			if (suffix) {
				printf("%s%ld%s ", buf, ++suffix, endptr);
			} else {
				printf("%s%s ", buf, endptr);
			}
		}
		free(buf);
	} else {
		for (i = 0; i < count; i++) {
			printf("%s ", samples.samples[1-i%2]);
		}
	}
}

int
number_pattern_match(const wchar_t rune)
{
	return iswdigit(rune) || rune == '.';
}

void
number_pattern_run(union sample_space samples, int count, int ind)
{
	NumSeq seq;
	long start = strtol(samples.ordered.last, NULL, 10);
	long work;
	int cur = start;
	int prev = strtoimax(samples.ordered.middle, NULL, 10);
	int startind;

	if (samples.ordered.first)
		startind = ind + 3;
	else
		startind = ind + 2;

	if (cur == prev) {
		seq.order = 0;
		seq.coff[0] = cur;
	} else if (!samples.ordered.first) {
		seq.order = 1;
		seq.coff[1] = cur - prev;
		seq.coff[0] = cur - ((ind + 1) * seq.coff[1]);
	} else {
		for (seq.order = 0; cur != prev && seq.order < 4; seq.order++) {
		}
	}

	for (int i = 0; i < count; i++) {
		work = 0;
		for (int ord = 0; ord <= seq.order; ord++) {
			work += pow(startind, ord) * seq.coff[ord];
		}
		printf("%ld ", work);
		startind++;
	}
}

int
buffered_pattern_match(const wchar_t rune, struct buffered_matcher_state *state,
		       const struct long_short *dataset, size_t datalen)
{
	if (state->bufpos >= BUFSIZ - 1) {
		return 0;
	}

	state->buf[state->bufpos++] = towlower(rune);
	state->buf[state->bufpos] = 0;

	for (int i = 0; i < datalen; i++) {
		if (wcsncmp(state->buf, dataset[i].l, state->bufpos) == 0 ||
		    wcsncmp(state->buf, dataset[i].s, state->bufpos) == 0) {
			return 1;
		}
	}

	return 0;
}
