// SPDX-License-Identifier: BSD-2-Clause-Patent
/*
 * test-str.c - test our string functions.
 */

#ifndef SHIM_UNIT_TEST
#define SHIM_UNIT_TEST
#endif
#include "shim.h"

#include <stdio.h>

#pragma GCC diagnostic ignored "-Wunused-variable"

int
main(void)
{
	const char s0[] = "abcd\0fghi";

	assert(strchrnul(s0, 'a') == &s0[0]);
	assert(strchrnul(s0, 'd') == &s0[3]);
	assert(strchrnul(s0, '\000') == &s0[4]);
	assert(strchrnul(s0, 'i') == &s0[4]);

	return 0;
}

// vim:fenc=utf-8:tw=75:noet
