/* Source: https://github.com/ejv2/seqspec */

/*
 * seqspec - analyze and continue arbitrary sequences of any order
 * Copyright (C) Ethan Marshall - 2022
 */

#ifndef SEQSPEC_INTERNAL_H
#define SEQSPEC_INTERNAL_H

/*
 * Analyzes a sequence to find the order and differences.
 * If seq is NULL or of length < 2, NULL is returned.
 * If the properties of seq could not be determined in 255 iterations,
 * NULL is returned.
 */
sspec_t *sspec_analyze(const long *seq, size_t len);

/*
 * Returns the term difference of the sequence
 */
int sspec_diff(sspec_t *spec);

/*
 * Returns the logical order of the sequence.
 * This is *not* necessarily the same as the order of the nth term formula,
 * although it almost always is.
 */
unsigned char sspec_ord(sspec_t *spec);

/*
 * Fills buf up to buflen with a continuation of the sequence stored in spec.
 * The original sequence buffer must remain valid while this function is being
 * called.
 */
void sspec_continue(sspec_t* spec, long* buf, size_t buflen);

#endif
