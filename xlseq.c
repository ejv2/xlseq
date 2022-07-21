/* See LICENSE for copyright and license details. */

#include <stdio.h>
#include "arg.h"
#include <stdlib.h>

char *argv0;

void
usage()
{
	exit(1);
}

int
main(int argc, char **argv)
{
	ARGBEGIN {
	case 'u':
		usage();
		/* NOTREACHED */
	default:
		fprintf(stderr, "%s: unknown flag '%c'\n", argv0, ARGC());
		return 1;
	} ARGEND;
}
