/* See LICENSE for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <wctype.h>
#include <time.h>

#include "util.h"
#include "sspec.h"
#include "xlseq.h"

int
string_pattern_match()
{
	return 1;
}

void
string_pattern_run(union sample_space samples, unsigned long count)
{
	unsigned long i;
	long suffix;
	size_t ind;
	const char *cend = NULL;
	char *endptr, *buf;
	const char *walk;

	for (walk = samples.ordered.last; *walk; walk++) {
		ind = walk - samples.ordered.last;
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

		for (i = 0; i < count; i++) {
			if (suffix) {
				printf("%s%ld%s ", buf, ++suffix, endptr);
			} else {
				printf("%s%s ", buf, endptr);
			}
		}
		free(buf);
	} else {
		for (i = 0; i < count; i++) {
			printf("%s ", samples.samples[1 - i % 2]);
		}
	}
}

int
number_pattern_match(const wchar_t rune)
{
	return iswdigit(rune) || rune == '.' || rune == '-';
}

void
number_pattern_run(struct full_sample samples, unsigned long count)
{
	sspec_t *sq;
	unsigned long i;
	long isamp[samples.len], obuf[count];
	long diff;

	for (i = 0; i < samples.len; i++) {
		isamp[i] = strtol(samples.samples[i], NULL, 10);
		/* printf("%d: %ld\n", i, isamp[i]); */
	}

	/*
	 * As a special case, do a naive extrapolation over two elements if
	 * they are the only two provided, as sspec requires at least three
	 */
	if (samples.len == 2) {
		diff = isamp[1] - isamp[0];
		for (i = 1; i <= count; i++) {
			printf("%ld ", isamp[1] + (i * diff));
		}
		return;
	}

	sq = sspec_analyze(isamp, samples.len);
	if (!sq) {
		fputs("xlseq: no valid arithmetic sequence determined\n",
		      stderr);
		return;
	}

	sspec_continue(sq, obuf, count);

	for (i = 0; i < count; i++) {
		printf("%ld ", obuf[i]);
	}

	free(sq);
}

/* NOTE: not a normal matcher - called once and overrides any other matches */
int
date_pattern_match(const char *in)
{
	unsigned long i;
	char any = 0;
	time_t ts;
	struct tm tm;
	const char *cmp;

	ts = time(NULL);
	tm = *localtime(&ts);
	for (i = 0; i < LENGTH(datefmt); i++) {
		cmp = strptime(in, datefmt[i], &tm);
		if (cmp && *cmp == 0) {
			any++;
			break;
		}
	}

	return any;
}

void
date_pattern_run(union sample_space samples, unsigned long count)
{
	unsigned long i;
	unsigned long parsed0 = 0, parsed1 = 0;
	time_t ts, diff, start;
	struct tm tm0, tm1, tmp;
	const char *cmp;
	char buf[BUFSIZ];

	ts = time(NULL);
	tm0 = tm1 = *localtime(&ts);

	for (i = 0; i < LENGTH(datefmt); i++) {
		if (!parsed0) {
			cmp = strptime(samples.ordered.middle, datefmt[i], &tm0);
			if (cmp && *cmp == 0) {
				parsed0 = i;
			}
		}

		if (!parsed1) {
			cmp = strptime(samples.ordered.last, datefmt[i], &tm1);
			if (cmp && *cmp == 0) {
				parsed1 = i;
			}
		}
	}
	if (!parsed0 || !parsed1) {
		fprintf(stderr,
			"xlseq: date_pattern_match didn't catch bad format(s)\ntm0:\t'%s'\ntm1:\t'%s'\n",
			samples.ordered.last, samples.ordered.middle);
		abort();
	}

	start = mktime(&tm1);
	diff = start - mktime(&tm0);

	for (i = 0; i < count; i++) {
		start += diff;
		tmp = *localtime(&start);
		if (!strftime(buf, BUFSIZ, datefmt[parsed1], &tmp)) {
			fputs("xlseq: warning: time truncated\n", stderr);
			continue;
		}
		printf("%s ", buf);
	}
}

int
buffered_pattern_match(const wchar_t rune, struct buffered_matcher_state *state,
		       const struct long_short *dataset, size_t datalen)
{
	unsigned long i;

	if (state->bufpos >= BUFSIZ - 1) {
		return 0;
	}

	state->buf[state->bufpos++] = towlower(rune);
	state->buf[state->bufpos] = 0;

	for (i = 0; i < datalen; i++) {
		if (wcsncmp(state->buf, dataset[i].l, state->bufpos) == 0 ||
		    wcsncmp(state->buf, dataset[i].s, state->bufpos) == 0) {
			return 1;
		}
	}

	return 0;
}

void
buffered_pattern_run(union sample_space samples, unsigned long count,
		     const struct long_short *dataset, size_t datalen)
{
	unsigned long i;
	const wchar_t *out;
	const char *work = samples.ordered.last;
	const struct long_short *cur;
	int uselong;
	int rlen, rind = 0;
	unsigned int dind = 0;
	int arglen = strlen(samples.ordered.last);
	wchar_t decode[arglen + 1];

	memset(decode, 0, sizeof(wchar_t) * arglen);
	do {
		rlen = mbtowc(decode + rind, work, arglen);
		if (rlen < 0) {
			fprintf(stderr, "xlseq: invalid text encoding\n");
			return;
		}
		work += rlen;
		rind++;
	} while (*work);
	decode[rind] = 0;

	for (i = 0; i < datalen; i++) {
		cur = &dataset[i];
		if (wcscasecmp(cur->l, decode) == 0) {
			uselong = 1;
			break;
		} else if (wcscasecmp(cur->s, decode) == 0) {
			uselong = 0;
			break;
		}
	}
	dind = i;
	if (count <= 0)
		count = datalen - 1 - (cur - dataset);

	for (i = 1; i <= count; i++) {
		out = (uselong) ? dataset[(dind + i) % datalen].l :
				  dataset[(dind + i) % datalen].s;
		printf("%ls ", out);
	}
}
