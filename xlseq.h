/* See LICENSE for copyright and license details. */

struct long_short {
	const wchar_t *l, *s;
};

/* string pattern */
int string_pattern_match(const wchar_t rune);

/* number pattern */
int number_pattern_match(const wchar_t rune);

/* days pattern */
static const struct long_short days[] = {
	{L"monday", L"mon"},
	{L"tuesday", L"tue"},
	{L"wednesday", L"wed"},
	{L"thursday", L"thur"},
	{L"friday", L"fri"},
	{L"saturday", L"sat"},
	{L"sunday", L"sun"}
};
struct days_matcher_state {
	wchar_t buf[BUFSIZ];
	long bufpos;
};
int days_pattern_match(const wchar_t rune, struct days_matcher_state *state);

/* months pattern */
static const struct long_short months[] = {
	{L"january", L"jan"},
	{L"february", L"feb"},
	{L"march", L"mar"},
	{L"april", L"apr"},
	{L"may", L"may"},
	{L"june", L"jun"},
	{L"july", L"jul"},
	{L"august", L"aug"},
	{L"september", L"sept"},
	{L"october", L"oct"},
	{L"november", L"nov"},
	{L"december", L"dec"}
};
struct months_matcher_state {
	wchar_t buf[BUFSIZ];
	long bufpos;
};
int months_pattern_match(const wchar_t rune, struct months_matcher_state *state);
