/* Source: https://github.com/ejv2/seqspec */

/*
 * seqspec - analyze and continue arbitrary sequences of any order
 * Copyright (C) Ethan Marshall - 2022
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

typedef struct {
	int diff;
	unsigned char order;

	const long* seq;
	size_t seqlen;

	struct {
		long diffs[UCHAR_MAX];
	} state;
} sspec_t;

#include "sspec_internal.h"

/*
 * Recursively solves for the difference between elements.
 * Increments iters for each iteration. Space starts at space=len and is
 * decremented for each recursion.
 */
static int analyze_diffs(const long *seq, size_t len, size_t space, unsigned char *iters, int *errf, long *diffbuf)
{
	size_t i;
	int recurse = 0;
	long diff;
	long diffs[space];

	(*iters)++;
	if (*iters == UCHAR_MAX || space < 2) {
		*errf = 1;
		return 0;
	}

	/* start at i=1 and perform difference using i-1, to support any
	 * array size.
	 */
	diff = seq[1] - seq[0];
	for (i = 1; i < len; i++) {
		diffs[i - 1] = seq[i] - seq[i - 1];
		if (seq[i] - seq[i - 1] != diff) {
			recurse = 1;
		}
	}
	diffbuf[*iters - 1] = diffs[space - 1];

	if (recurse)
		diff = analyze_diffs(diffs, space, space-1, iters, errf, diffbuf);

	return diff;
}

sspec_t *sspec_analyze(const long *seq, size_t len)
{
	int err = 0;
	sspec_t *ret;

	if (!seq || len < 2)
		return NULL;

	ret = malloc(sizeof(sspec_t));
	ret->order = 0;
	ret->diff = analyze_diffs(seq, len, len - 1, &ret->order, &err, ret->state.diffs);
	ret->seq = seq;
	ret->seqlen = len;

	if (err) {
		free(ret);
		return NULL;
	}

	return ret;
}

int sspec_diff(sspec_t *spec)
{
	return spec->diff;
}

unsigned char sspec_ord(sspec_t *spec)
{
	return spec->order;
}

void sspec_continue(sspec_t* spec, long* buf, size_t buflen)
{
	long last = spec->seq[spec->seqlen - 1];
	size_t i, j;

	if (!spec || !buf)
		return;

	for (i = 0; i < buflen; i++) {
		for (j = spec->order - 1; j > 0; j--) {
			spec->state.diffs[j - 1] = spec->state.diffs[j - 1] + spec->state.diffs[j];
		}

		buf[i] = last + spec->state.diffs[0];
		last = buf[i];
	}
}
