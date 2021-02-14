// SPDX-License-Identifier: BSD-2-Clause-Patent
/*
 * test.c - stuff we need for test harnesses
 * Copyright Peter Jones <pjones@redhat.com>
 */

#ifndef SHIM_UNIT_TEST
#define SHIM_UNIT_TEST
#endif
#include "shim.h"

UINT8 in_protocol = 0;

EFI_STATUS LogError_(const char *file, int line, const char *func, const CHAR16 *fmt, ...)
{
	assert(0);
}

INTN
StrCmp(CONST CHAR16 *s1, CONST CHAR16 *s2) {
	assert(s1 != NULL);
	assert(s2 != NULL);

	int i;
	for (i = 0; s1[i] && s2[i]; i++) {
		if (s1[i] != s2[i])
			return s2[i] - s1[i];
	}
	return 0;
}

INTN
StrnCmp(CONST CHAR16 *s1, CONST CHAR16 *s2, UINTN len) {
	assert(s1 != NULL);
	assert(s2 != NULL);

	UINTN i;
	for (i = 0; i < len && s1[i] && s2[i]; i++) {
		if (s1[i] != s2[i])
			return s2[i] - s1[i];

	}
	return 0;
}

// vim:fenc=utf-8:tw=75:noet
