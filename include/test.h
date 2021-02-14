// SPDX-License-Identifier: BSD-2-Clause-Patent
/*
 * test.h - fake a bunch of EFI types so we can build test harnesses with libc
 * Copyright Peter Jones <pjones@redhat.com>
 */

#ifdef SHIM_UNIT_TEST
#ifndef TEST_H_
#define TEST_H_

#include <stdarg.h>

#if defined(__aarch64__)
#include <aa64/efibind.h>
#elif defined(__arm__)
#include <arm/efibind.h>
#elif defined(__i386__) || defined(__i486__) || defined(__i686__)
#include <ia32/efibind.h>
#elif defined(__x86_64__)
#include <x64/efibind.h>
#else
#error what arch is this
#endif

#include <efidef.h>

#include <efidevp.h>
#include <efiprot.h>
#include <eficon.h>
#include <efiapi.h>
#include <efierr.h>

#include <efipxebc.h>
#include <efinet.h>
#include <efiip.h>

#include <stdlib.h>

#define ZeroMem(buf, sz) memset(buf, 0, sz)
#define SetMem(buf, sz, value) memset(buf, value, sz)
#define CopyMem(dest, src, len) memcpy(dest, src, len)
#define CompareMem(dest, src, len) memcmp(dest, src, len)

#include <assert.h>

#define strlena(x) __builtin_strlen(x)

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

#define AllocateZeroPool(x) calloc(1, (x))
#define AllocatePool(x) malloc(x)
#define FreePool(x) free(x)
#define ReallocatePool(old, oldsz, newsz) realloc(old, newsz)

#endif /* !TEST_H_ */
#endif /* SHIM_UNIT_TEST */
// vim:fenc=utf-8:tw=75:noet
