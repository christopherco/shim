// SPDX-License-Identifier: BSD-2-Clause-Patent
/*
 * test-sbat.c - test our sbat functions.
 */

#ifndef SHIM_UNIT_TEST
#define SHIM_UNIT_TEST
#endif
#include "shim.h"
#include "sbat.h"

#include <stdio.h>

#define MAX_SIZE 512

void
test_parse_sbat_null_sbat_base(void)
{
	size_t sbat_size = 20;
	struct sbat sbat = { 0, NULL };
	EFI_STATUS status;

	status = parse_sbat(NULL, sbat_size, &sbat);

	assert(status == EFI_INVALID_PARAMETER);
}

void
test_parse_sbat_zero_sbat_size(void)
{
	char sbat_base[] = "test1,1,SBAT test1,acme,1,testURL";
	size_t sbat_size = 0;
	struct sbat sbat = { 0, NULL };
	EFI_STATUS status;

	status = parse_sbat(sbat_base, sbat_size, &sbat);

	assert(status == EFI_INVALID_PARAMETER);
}

void
test_parse_sbat_null_sbat(void)
{
	char sbat_base[] = "test1,1,SBAT test1,acme,1,testURL";
	size_t sbat_size = sizeof(sbat_base);
	EFI_STATUS status;

	status = parse_sbat(sbat_base, sbat_size, NULL);

	assert(status == EFI_INVALID_PARAMETER);
}

void
test_parse_sbat_single_entry(void)
{
	char sbat_base[] = "test1,1,SBAT test1,acme,1,testURL\n";
	size_t sbat_size = sizeof(sbat_base);
	struct sbat sbat = { 0 };
	EFI_STATUS status;

	status = parse_sbat(sbat_base, sbat_size, &sbat);

	assert(status == EFI_SUCCESS);
	assert(sbat.size == 1);
	assert(strncmp(sbat.entries[0]->component_name, "test1", MAX_SIZE) == 0);
	assert(strncmp(sbat.entries[0]->component_generation, "1", MAX_SIZE) == 0);
	assert(strncmp(sbat.entries[0]->vendor_name, "SBAT test1", MAX_SIZE) == 0);
	assert(strncmp(sbat.entries[0]->vendor_package_name, "acme", MAX_SIZE) == 0);
	assert(strncmp(sbat.entries[0]->vendor_version, "1", MAX_SIZE) == 0);
	assert(strncmp(sbat.entries[0]->vendor_url, "testURL", MAX_SIZE) == 0);
}

void
test_parse_sbat_multiple_entries(void)
{
	char sbat_base[] = "test1,1,SBAT test1,acme,1,testURL\n" \
			   "test2,2,SBAT test2,acme2,2,testURL2\n";
	size_t sbat_size = sizeof(sbat_base);
	struct sbat sbat = { 0 };
	EFI_STATUS status;

	status = parse_sbat(sbat_base, sbat_size, &sbat);

	assert(status == EFI_SUCCESS);
	assert(sbat.size == 2);
	assert(strncmp(sbat.entries[0]->component_name, "test1", MAX_SIZE) == 0);
	assert(strncmp(sbat.entries[0]->component_generation, "1", MAX_SIZE) == 0);
	assert(strncmp(sbat.entries[0]->vendor_name, "SBAT test1", MAX_SIZE) == 0);
	assert(strncmp(sbat.entries[0]->vendor_package_name, "acme", MAX_SIZE) == 0);
	assert(strncmp(sbat.entries[0]->vendor_version, "1", MAX_SIZE) == 0);
	assert(strncmp(sbat.entries[0]->vendor_url, "testURL", MAX_SIZE) == 0);
	assert(strncmp(sbat.entries[1]->component_name, "test2", MAX_SIZE) == 0);
	assert(strncmp(sbat.entries[1]->component_generation, "2", MAX_SIZE) == 0);
	assert(strncmp(sbat.entries[1]->vendor_name, "SBAT test2", MAX_SIZE) == 0);
	assert(strncmp(sbat.entries[1]->vendor_package_name, "acme2", MAX_SIZE) == 0);
	assert(strncmp(sbat.entries[1]->vendor_version, "2", MAX_SIZE) == 0);
	assert(strncmp(sbat.entries[1]->vendor_url, "testURL2", MAX_SIZE) == 0);
}

static struct sbat_var *
create_mock_sbat_var_entry (char* comp_gen, char* comp_name)
{
	struct sbat_var *new_entry = AllocatePool(sizeof(*new_entry));
	if (!new_entry)
		return NULL;
	INIT_LIST_HEAD(&new_entry->list);
	CHAR8 *alloc_comp_gen = AllocatePool((strlen(comp_gen)+1)*sizeof(*alloc_comp_gen));
	if (!alloc_comp_gen)
		return NULL;
	CHAR8 *alloc_comp_name = AllocatePool((strlen(comp_name)+1)*sizeof(*alloc_comp_name));
	if (!alloc_comp_name)
		return NULL;
	CopyMem(alloc_comp_gen, comp_gen, strlen(comp_gen) + 1);
	CopyMem(alloc_comp_name, comp_name, strlen(comp_name)+1);
	new_entry->component_generation = alloc_comp_gen;
	new_entry->component_name = alloc_comp_name;
	return new_entry;
}

// Test: .sbat component generation read from binary is equal to SBAT EFI
//       variable component generation for the test component.
// Expected Result: verify_sbat() should return EFI_SUCCESS
int
test_verify_sbat_valid_sbat_pass(void)
{
	char sbat_base[] = "test1,1,SBAT test1,acme,1,testURL\n" \
			   "test2,2,SBAT test2,acme2,2,testURL2\n";
	size_t sbat_size = sizeof(sbat_base);
	struct sbat sbat = { 0 };
	assert(parse_sbat(sbat_base, sbat_size, &sbat) == EFI_SUCCESS);

	list_t sbat_entries;
	INIT_LIST_HEAD(&sbat_entries);
	struct sbat_var *test_entry;
	char comp_name[] = "test1";
	char comp_gen[] = "1";
	test_entry = create_mock_sbat_var_entry(comp_gen, comp_name);
	if (!test_entry)
		return -1;
	list_add(&test_entry->list, &sbat_entries);

	// Note: verify_sbat also frees the underlying sbat_entries.
	assert(verify_sbat(&sbat, &sbat_entries) == EFI_SUCCESS);
	return 0;
}

// Test: .sbat component generation read from binary is greater than SBAT EFI
//       variable component generation for the test component.
// Expected Result: verify_sbat() should return EFI_SUCCESS
int
test_verify_sbat_valid_sbat_pass2(void)
{
	char sbat_base[] = "test1,1,SBAT test1,acme,1,testURL\n" \
			   "test2,2,SBAT test2,acme2,2,testURL2\n";
	size_t sbat_size = sizeof(sbat_base);
	struct sbat sbat = { 0 };
	assert(parse_sbat(sbat_base, sbat_size, &sbat) == EFI_SUCCESS);

	list_t sbat_entries;
	INIT_LIST_HEAD(&sbat_entries);
	struct sbat_var *test_entry;
	char comp_name[] = "test2";
	char comp_gen[] = "1";
	test_entry = create_mock_sbat_var_entry(comp_gen, comp_name);
	if (!test_entry)
		return -1;
	list_add(&test_entry->list, &sbat_entries);

	// Note: verify_sbat also frees the underlying sbat_entries.
	assert(verify_sbat(&sbat, &sbat_entries) == EFI_SUCCESS);
	return 0;
}

// Test: .sbat component generation read from binary is lower than the
//       SBAT EFI variable component generation for the test component
// Expected result: verify_sbat() should reject with EFI_SECURITY_VIOLATION
int
test_verify_sbat_valid_sbat_reject(void)
{
	char sbat_base[] = "test1,1,SBAT test1,acme,1,testURL\n" \
			   "test2,2,SBAT test2,acme2,2,testURL2\n";
	size_t sbat_size = sizeof(sbat_base);
	struct sbat sbat = { 0 };
	assert(parse_sbat(sbat_base, sbat_size, &sbat) == EFI_SUCCESS);

	list_t sbat_entries;
	INIT_LIST_HEAD(&sbat_entries);
	struct sbat_var *test_entry;
	char comp_name[] = "test2";
	char comp_gen[] = "3";
	test_entry = create_mock_sbat_var_entry(comp_gen, comp_name);
	if (!test_entry)
		return -1;
	list_add(&test_entry->list, &sbat_entries);

	// Note: verify_sbat also frees the underlying sbat_entries.
	assert(verify_sbat(&sbat, &sbat_entries) == EFI_SECURITY_VIOLATION);
	return 0;
}

void
test_verify_sbat_empty_entries(void)
{
	char sbat_base[] = "test1,1,SBAT test1,acme,1,testURL\n" \
			   "test2,2,SBAT test2,acme2,2,testURL2\n";
	size_t sbat_size = sizeof(sbat_base);
	struct sbat sbat = { 0 };
	assert(parse_sbat(sbat_base, sbat_size, &sbat) == EFI_SUCCESS);
	list_t sbat_entries;
	INIT_LIST_HEAD(&sbat_entries);
	EFI_STATUS status;

	status = verify_sbat(&sbat, &sbat_entries);

	assert(status == EFI_INVALID_PARAMETER);
}

void
test_parse_sbat_var_invalid_list(void)
{
	EFI_STATUS status;

	status = parse_sbat_var(NULL);

	assert(status == EFI_INVALID_PARAMETER);
}

int
main(void)
{
	// parse_sbat_var tests
	test_parse_sbat_var_invalid_list();

	// parse_sbat tests
	test_parse_sbat_null_sbat_base();
	test_parse_sbat_zero_sbat_size();
	test_parse_sbat_null_sbat();
	test_parse_sbat_single_entry();
	test_parse_sbat_multiple_entries();

	// verify_sbat tests
	test_verify_sbat_valid_sbat_pass();
	test_verify_sbat_valid_sbat_pass2();
	test_verify_sbat_valid_sbat_reject();
	test_verify_sbat_empty_entries();

	return 0;
}

// vim:fenc=utf-8:tw=75:noet
