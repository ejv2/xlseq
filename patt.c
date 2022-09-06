/* See LICENSE for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <wctype.h>

#include "util.h"
#include "sspec.h"
#include "xlseq.h"

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
number_pattern_run(struct full_sample samples, int count)
{
	sspec_t *sq;
	int i;
	long isamp[samples.len], obuf[count];

	for (i = 0; i < samples.len; i++) {
		isamp[i] = strtol(samples.samples[i], NULL, 10);
		/* printf("%d: %ld\n", i, isamp[i]); */
	}

	sq = sspec_analyze(isamp, samples.len);
	if (!sq) {
		fputs("xlseq: no valid arithmetic sequence determined\n", stderr);
		return;
	}

	sspec_continue(sq, obuf, count);

	for (i = 0; i < count; i++) {
		printf("%ld ", obuf[i]);
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

void
buffered_pattern_run(union sample_space samples, int count,
		     const struct long_short *dataset, size_t datalen)
{
	int i;
	const wchar_t *out;
	const char *work = samples.ordered.last;
	const struct long_short *cur;
	int uselong;
	int rlen, rind = 0;
	unsigned int dind = 0;
	int arglen = strlen(samples.ordered.last);
	wchar_t decode[arglen];

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
		if (wcscmp(cur->l, decode) == 0) {
			uselong = 1;
			break;
		} else if (wcscmp(cur->s, decode) == 0) {
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
