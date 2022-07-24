/* See LICENSE for copyright and license details. */

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <wctype.h>

#include "util.h"
#include "xlseq.h"

int
string_pattern_match(const wchar_t rune)
{
	return 1;
}

void
string_pattern_run(struct string_pattern_state *state, union sample_space samples, int count)
{
	int i, len;
	size_t ind;
	const char *walk;
	const char *pref, *suff;

	if (!state->common_end && !state->common_check) {
		for (walk = samples.ordered.last; *walk; walk++) {
			ind = walk-samples.ordered.last;
			if (!samples.ordered.middle[ind])
				break;
			else if (samples.ordered.middle[ind] != *walk) {
				break;
			}

			state->common_end = walk;
		}
		state->common_check = 1;
	}

	if (samples.samples[2])
		len = 3;
	else
		len = 2;

	if (state->common_end && *state->common_end) {
		state->common_end++;
	} else {
		for (i = 0; i < count; i++) {
			printf("%s ", samples.samples[len-(i%len)-1]);
		}
	}
}

int
number_pattern_match(const wchar_t rune)
{
	return iswdigit(rune) || rune == '.';
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
