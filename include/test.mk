# SPDX-License-Identifier: BSD-2-Clause-Patent
#
# test.mk - makefile to make local test programs
#

.SUFFIXES:

CC = gcc
CFLAGS = -O2 -ggdb -std=gnu89 \
	 -Iinclude -I/usr/include/efi -iquote . \
	 -fshort-wchar -flto -fno-builtin \
	 -Wall -Wsign-compare -Werror \
	 -Wno-deprecated-declarations -Wno-pointer-sign \
	 -DEFI_FUNCTION_WRAPPER -DGNU_EFI_USE_MS_ABI -DPAGE_SIZE=4096 \
	 -DSHIM_UNIT_TEST

tests := $(patsubst %.c,%,$(wildcard test-*.c))

$(tests) :: test-% : test-%.c test.c sbat.c $(wildcard %.c)
	$(CC) $(CFLAGS) -o $@ $^
	./$@

test : $(tests)

all : test

.PHONY: $(tests) all test

# vim:ft=make
