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
days_pattern_match(const wchar_t rune, struct days_matcher_state *state)
{
	if (state->bufpos >= BUFSIZ - 1) {
		return 0;
	}

	state->buf[state->bufpos++] = towlower(rune);
	state->buf[state->bufpos] = 0;

	for (int i = 0; i < LENGTH(days); i++) {
		if (wcsncmp(state->buf, days[i].l, state->bufpos) == 0 ||
		    wcsncmp(state->buf, days[i].s, state->bufpos) == 0) {
			return 1;
		}
	}

	return 0;
}

int
months_pattern_match(const wchar_t rune, struct months_matcher_state *state)
{
	if (state->bufpos >= BUFSIZ - 1) {
		return 0;
	}

	state->buf[state->bufpos++] = towlower(rune);
	state->buf[state->bufpos] = 0;

	for (int i = 0; i < LENGTH(months); i++) {
		if (wcsncmp(state->buf, months[i].l, state->bufpos) == 0 ||
		    wcsncmp(state->buf, months[i].s, state->bufpos) == 0) {
			return 1;
		}
	}

	return 0;
}
