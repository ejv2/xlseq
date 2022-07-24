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
