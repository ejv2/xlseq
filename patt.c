/* See LICENSE for copyright and license details. */

#include <wchar.h>
#include <ctype.h>

int
string_pattern_match(const wchar_t rune)
{
	return 1;
}

int
number_pattern_match(const wchar_t rune)
{
	return isdigit(rune) || rune == '.';
}
