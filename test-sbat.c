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

/*
 * Mock test helpers
 */
static struct sbat_entry *
create_mock_sbat_entry(const char* comp_name, const char* comp_gen,
		       const char* vend_name, const char* vend_pkg_name,
		       const char* vend_ver, const char* vend_url)
{
	struct sbat_entry *new_entry = AllocatePool(sizeof(*new_entry));
	if (!new_entry)
		return NULL;
	new_entry->component_name = comp_name;
	new_entry->component_generation = comp_gen;
	new_entry->vendor_name = vend_name;
	new_entry->vendor_package_name = vend_pkg_name;
	new_entry->vendor_version = vend_ver;
	new_entry->vendor_url = vend_url;
	return new_entry;
}

void
free_mock_sbat_entry(struct sbat_entry *entry)
{
	if (entry)
		FreePool(entry);
}

static struct sbat *
create_mock_sbat_one_entry(char* comp_name, char* comp_gen, char* vend_name,
			   char* vend_pkg_name, char* vend_ver, char* vend_url)
{
	struct sbat *new_entry = AllocatePool(sizeof(*new_entry));
	if (!new_entry)
		return NULL;
	struct sbat_entry *test_entry;
	struct sbat_entry **entries = AllocatePool(sizeof(*entries));
	if (!entries)
		return NULL;
	test_entry = create_mock_sbat_entry(comp_name, comp_gen, vend_name,
					     vend_pkg_name, vend_ver, vend_url);
	if (!test_entry)
		return NULL;
	entries[0] = test_entry;
	new_entry->size = 1;
	new_entry->entries = entries;
	return new_entry;
}

static struct sbat *
create_mock_sbat_multiple_entries(struct sbat_entry *entry_array,
				  size_t num_elem)
{
	unsigned int i;
	struct sbat *new_entry = AllocatePool(sizeof(*new_entry));
	if (!new_entry)
		return NULL;
	struct sbat_entry *test_entry;
	struct sbat_entry **entries = AllocatePool(num_elem * sizeof(*entries));
	if (!entries)
		return NULL;
	for (i = 0; i < num_elem; i++) {
		test_entry = create_mock_sbat_entry(entry_array[i].component_name,
						    entry_array[i].component_generation,
						    entry_array[i].vendor_name,
						    entry_array[i].vendor_package_name,
						    entry_array[i].vendor_version,
						    entry_array[i].vendor_url);
		if (!test_entry)
			return NULL;
		entries[i] = test_entry;
	}
	new_entry->size = num_elem;
	new_entry->entries = entries;

	return new_entry;
}

void
free_mock_sbat(struct sbat *sbat)
{
	unsigned int i;
	if (sbat) {
		for (i = 0; i < sbat->size; i++) {
			if (sbat->entries[i]) {
				FreePool(sbat->entries[i]);
			}
		}
		FreePool(sbat);
	}
}

static struct sbat_var *
create_mock_sbat_var_entry(const char* comp_name, const char* comp_gen)
{
	struct sbat_var *new_entry = AllocatePool(sizeof(*new_entry));
	if (!new_entry)
		return NULL;
	INIT_LIST_HEAD(&new_entry->list);
	int comp_name_size = strlen(comp_name) + 1;
	CHAR8 *alloc_comp_name = AllocatePool(comp_name_size * sizeof(*alloc_comp_name));
	if (!alloc_comp_name)
		return NULL;
	int comp_gen_size = strlen(comp_gen) + 1;
	CHAR8 *alloc_comp_gen = AllocatePool(comp_gen_size * sizeof(*alloc_comp_gen));
	if (!alloc_comp_gen)
		return NULL;
	CopyMem(alloc_comp_name, comp_name, comp_name_size);
	CopyMem(alloc_comp_gen, comp_gen, comp_gen_size);
	new_entry->component_name = alloc_comp_name;
	new_entry->component_generation = alloc_comp_gen;
	return new_entry;
}

static list_t *
create_mock_sbat_entries_one_entry(char* name, char* gen)
{
	list_t *test_sbat_entries = AllocatePool(sizeof(*test_sbat_entries));
	if (!test_sbat_entries)
		return NULL;
	INIT_LIST_HEAD(test_sbat_entries);
	struct sbat_var *test_entry;
	test_entry = create_mock_sbat_var_entry(name, gen);
	if (!test_entry)
		return NULL;
	list_add(&test_entry->list, test_sbat_entries);
	return test_sbat_entries;
}

static list_t *
create_mock_sbat_entries_multiple_entries(struct sbat_var *var_array,
					  size_t num_elem)
{
	unsigned int i;
	list_t *test_sbat_entries = AllocatePool(sizeof(*test_sbat_entries));
	if (!test_sbat_entries)
		return NULL;
	INIT_LIST_HEAD(test_sbat_entries);
	struct sbat_var *test_entry;
	for (i = 0; i < num_elem; i++) {
		test_entry = create_mock_sbat_var_entry(var_array[i].component_name,
							var_array[i].component_generation);
		if (!test_entry)
			return NULL;
		list_add(&test_entry->list, test_sbat_entries);
	}
	return test_sbat_entries;
}

void
free_mock_sbat_entries(list_t *entries)
{
	list_t *pos = NULL;
	list_t *n = NULL;
	struct sbat_var *entry;

	if (entries)
	{
		list_for_each_safe(pos, n, entries)
		{
			entry = list_entry(pos, struct sbat_var, list);
			list_del(&entry->list);
			if (entry->component_name)
				FreePool((CHAR8 *)entry->component_name);
			if (entry->component_generation)
				FreePool((CHAR8 *)entry->component_generation);
			FreePool(entry);
		}
		FreePool(entries);
	}
}

/*
 * parse_sbat() tests
 */
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
	char sbat_base[] = "test1,1,SBAT test1,acme,1,testURL\n";
	size_t sbat_size = 0;
	struct sbat sbat = { 0, NULL };
	EFI_STATUS status;

	status = parse_sbat(sbat_base, sbat_size, &sbat);

	assert(status == EFI_INVALID_PARAMETER);
}

void
test_parse_sbat_null_sbat(void)
{
	char sbat_base[] = "test1,1,SBAT test1,acme,1,testURL\n";
	size_t sbat_size = sizeof(sbat_base);
	EFI_STATUS status;

	status = parse_sbat(sbat_base, sbat_size, NULL);

	assert(status == EFI_INVALID_PARAMETER);
}

void
test_parse_sbat_no_newline(void)
{
	char sbat_base[] = "test1,1,SBAT test1,acme,1,testURL";
	size_t sbat_size = sizeof(sbat_base);
	struct sbat sbat = { 0 };
	EFI_STATUS status;

	status = parse_sbat(sbat_base, sbat_size, &sbat);

	assert(status == EFI_INVALID_PARAMETER);
}

void
test_parse_sbat_too_few_elem(void)
{
	char sbat_base[] = "test1,1,SBAT test1,acme,1\n";
	size_t sbat_size = sizeof(sbat_base);
	struct sbat sbat = { 0 };
	EFI_STATUS status;

	status = parse_sbat(sbat_base, sbat_size, &sbat);

	assert(status == EFI_INVALID_PARAMETER);
}

void
test_parse_sbat_too_many_elem(void)
{
	char sbat_base[] = "test1,1,SBAT test1,acme,1,testURL,testURL2\n";
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
	assert(strncmp(sbat.entries[0]->vendor_url, "testURL,testURL2", MAX_SIZE) == 0);
}

void
test_parse_sbat_no_newline_multiple_entries(void)
{
	char sbat_base[] = "test1,1,SBAT test1,acme,1,testURL\n" \
			   "test2,2,SBAT test2,acme2,2,testURL2";
	size_t sbat_size = sizeof(sbat_base);
	struct sbat sbat = { 0 };
	EFI_STATUS status;

	status = parse_sbat(sbat_base, sbat_size, &sbat);

	assert(status == EFI_INVALID_PARAMETER);
}

void
test_parse_sbat_too_few_elem_multiple_entries(void)
{
	char sbat_base[] = "test1,1,SBAT test1,acme,1,testURL\n" \
			   "test2,2,SBAT test2,acme2,2\n";
	size_t sbat_size = sizeof(sbat_base);
	struct sbat sbat = { 0 };
	EFI_STATUS status;

	status = parse_sbat(sbat_base, sbat_size, &sbat);

	assert(status == EFI_INVALID_PARAMETER);
}

void
test_parse_sbat_too_many_elem_multiple_entries(void)
{
	char sbat_base[] = "test1,1,SBAT test1,acme,1,testURL\n" \
			   "test2,2,SBAT test2,acme2,2,testURL2,test3\n";
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
	assert(strncmp(sbat.entries[1]->vendor_url, "testURL2,test3", MAX_SIZE) == 0);
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

/*
 * parse_sbat_var() tests
 */
void
test_parse_sbat_var_invalid_list(void)
{
	EFI_STATUS status;

	status = parse_sbat_var(NULL);

	assert(status == EFI_INVALID_PARAMETER);
}

/*
 * verify_sbat() tests
 * Note: verify_sbat also frees the underlying "sbat_entries" memory.
 */
int
test_verify_sbat_null_sbat(void)
{
	list_t *test_sbat_entries;
	test_sbat_entries = create_mock_sbat_entries_one_entry("test1", "1");
	if (!test_sbat_entries)
		return -1;
	EFI_STATUS status;

	status = verify_sbat(NULL, test_sbat_entries);

	assert(status == EFI_INVALID_PARAMETER);
	return 0;
}

int
test_verify_sbat_null_sbat_entries(void)
{
	struct sbat *test_sbat;
	test_sbat = create_mock_sbat_one_entry("test1","1","SBAT test1",
					       "acme","1","testURL");
	if (!test_sbat)
		return -1;

	list_t sbat_entries;
	INIT_LIST_HEAD(&sbat_entries);
	EFI_STATUS status;

	status = verify_sbat(test_sbat, &sbat_entries);

	assert(status == EFI_INVALID_PARAMETER);
	free_mock_sbat(test_sbat);
	return 0;
}

int
test_verify_sbat_match_one_exact(void)
{
	struct sbat *test_sbat;
	struct sbat_entry sbat_entry_array[2];
	sbat_entry_array[0].component_name = "test1";
	sbat_entry_array[0].component_generation = "1";
	sbat_entry_array[0].vendor_name = "SBAT test1";
	sbat_entry_array[0].vendor_package_name = "acme";
	sbat_entry_array[0].vendor_version = "1";
	sbat_entry_array[0].vendor_url = "testURL";
	sbat_entry_array[1].component_name = "test2";
	sbat_entry_array[1].component_generation = "2";
	sbat_entry_array[1].vendor_name = "SBAT test2";
	sbat_entry_array[1].vendor_package_name = "acme2";
	sbat_entry_array[1].vendor_version = "2";
	sbat_entry_array[1].vendor_url = "testURL2";
	test_sbat = create_mock_sbat_multiple_entries(sbat_entry_array, 2);
	if (!test_sbat)
		return -1;

	list_t *test_sbat_entries;
	test_sbat_entries = create_mock_sbat_entries_one_entry("test1", "1");
	if (!test_sbat_entries)
		return -1;
	EFI_STATUS status;

	status = verify_sbat(test_sbat, test_sbat_entries);

	assert(status == EFI_SUCCESS);
	free_mock_sbat(test_sbat);
	free_mock_sbat_entries(test_sbat_entries);
	return 0;
}

int
test_verify_sbat_match_one_higher(void)
{
	struct sbat *test_sbat;
	struct sbat_entry sbat_entry_array[2];
	sbat_entry_array[0].component_name = "test1";
	sbat_entry_array[0].component_generation = "1";
	sbat_entry_array[0].vendor_name = "SBAT test1";
	sbat_entry_array[0].vendor_package_name = "acme";
	sbat_entry_array[0].vendor_version = "1";
	sbat_entry_array[0].vendor_url = "testURL";
	sbat_entry_array[1].component_name = "test2";
	sbat_entry_array[1].component_generation = "2";
	sbat_entry_array[1].vendor_name = "SBAT test2";
	sbat_entry_array[1].vendor_package_name = "acme2";
	sbat_entry_array[1].vendor_version = "2";
	sbat_entry_array[1].vendor_url = "testURL2";
	test_sbat = create_mock_sbat_multiple_entries(sbat_entry_array, 2);
	if (!test_sbat)
		return -1;

	list_t *test_sbat_entries;
	test_sbat_entries = create_mock_sbat_entries_one_entry("test2", "1");
	if (!test_sbat_entries)
		return -1;
	EFI_STATUS status;

	status = verify_sbat(test_sbat, test_sbat_entries);

	assert(status == EFI_SUCCESS);
	free_mock_sbat(test_sbat);
	free_mock_sbat_entries(test_sbat_entries);
	return 0;
}

int
test_verify_sbat_reject_one(void)
{
	struct sbat *test_sbat;
	struct sbat_entry sbat_entry_array[2];
	sbat_entry_array[0].component_name = "test1";
	sbat_entry_array[0].component_generation = "1";
	sbat_entry_array[0].vendor_name = "SBAT test1";
	sbat_entry_array[0].vendor_package_name = "acme";
	sbat_entry_array[0].vendor_version = "1";
	sbat_entry_array[0].vendor_url = "testURL";
	sbat_entry_array[1].component_name = "test2";
	sbat_entry_array[1].component_generation = "2";
	sbat_entry_array[1].vendor_name = "SBAT test2";
	sbat_entry_array[1].vendor_package_name = "acme2";
	sbat_entry_array[1].vendor_version = "2";
	sbat_entry_array[1].vendor_url = "testURL2";
	test_sbat = create_mock_sbat_multiple_entries(sbat_entry_array, 2);
	if (!test_sbat)
		return -1;

	list_t *test_sbat_entries;
	test_sbat_entries = create_mock_sbat_entries_one_entry("test2", "3");
	if (!test_sbat_entries)
		return -1;
	EFI_STATUS status;

	status = verify_sbat(test_sbat, test_sbat_entries);

	assert(status == EFI_SECURITY_VIOLATION);
	free_mock_sbat(test_sbat);
	free_mock_sbat_entries(test_sbat_entries);
	return 0;
}

int
test_verify_sbat_reject_many(void)
{
	struct sbat *test_sbat;
	unsigned int sbat_entry_array_size = 2;
	struct sbat_entry sbat_entry_array[sbat_entry_array_size];
	sbat_entry_array[0].component_name = "test1";
	sbat_entry_array[0].component_generation = "1";
	sbat_entry_array[0].vendor_name = "SBAT test1";
	sbat_entry_array[0].vendor_package_name = "acme";
	sbat_entry_array[0].vendor_version = "1";
	sbat_entry_array[0].vendor_url = "testURL";
	sbat_entry_array[1].component_name = "test2";
	sbat_entry_array[1].component_generation = "2";
	sbat_entry_array[1].vendor_name = "SBAT test2";
	sbat_entry_array[1].vendor_package_name = "acme2";
	sbat_entry_array[1].vendor_version = "2";
	sbat_entry_array[1].vendor_url = "testURL2";
	test_sbat = create_mock_sbat_multiple_entries(sbat_entry_array,
						      sbat_entry_array_size);
	if (!test_sbat)
		return -1;

	list_t *test_sbat_entries;
	unsigned int sbat_var_array_size = 2;
	struct sbat_var sbat_var_array[sbat_var_array_size];
	sbat_var_array[0].component_name = "test1";
	sbat_var_array[0].component_generation = "1";
	sbat_var_array[1].component_name = "test2";
	sbat_var_array[1].component_generation = "3";
	test_sbat_entries = create_mock_sbat_entries_multiple_entries(sbat_var_array,
								      sbat_var_array_size);
	if (!test_sbat_entries)
		return -1;
	EFI_STATUS status;

	status = verify_sbat(test_sbat, test_sbat_entries);

	assert(status == EFI_SECURITY_VIOLATION);
	free_mock_sbat(test_sbat);
	free_mock_sbat_entries(test_sbat_entries);
	return 0;
}

int
test_verify_sbat_match_many_higher(void)
{
	struct sbat *test_sbat;
	unsigned int sbat_entry_array_size = 2;
	struct sbat_entry sbat_entry_array[sbat_entry_array_size];
	sbat_entry_array[0].component_name = "test1";
	sbat_entry_array[0].component_generation = "3";
	sbat_entry_array[0].vendor_name = "SBAT test1";
	sbat_entry_array[0].vendor_package_name = "acme";
	sbat_entry_array[0].vendor_version = "1";
	sbat_entry_array[0].vendor_url = "testURL";
	sbat_entry_array[1].component_name = "test2";
	sbat_entry_array[1].component_generation = "5";
	sbat_entry_array[1].vendor_name = "SBAT test2";
	sbat_entry_array[1].vendor_package_name = "acme2";
	sbat_entry_array[1].vendor_version = "2";
	sbat_entry_array[1].vendor_url = "testURL2";
	test_sbat = create_mock_sbat_multiple_entries(sbat_entry_array,
						      sbat_entry_array_size);
	if (!test_sbat)
		return -1;

	list_t *test_sbat_entries;
	unsigned int sbat_var_array_size = 2;
	struct sbat_var sbat_var_array[sbat_var_array_size];
	sbat_var_array[0].component_name = "test1";
	sbat_var_array[0].component_generation = "1";
	sbat_var_array[1].component_name = "test2";
	sbat_var_array[1].component_generation = "2";
	test_sbat_entries = create_mock_sbat_entries_multiple_entries(sbat_var_array,
								      sbat_var_array_size);
	if (!test_sbat_entries)
		return -1;
	EFI_STATUS status;

	status = verify_sbat(test_sbat, test_sbat_entries);

	assert(status == EFI_SUCCESS);
	free_mock_sbat(test_sbat);
	free_mock_sbat_entries(test_sbat_entries);
	return 0;
}

int
test_verify_sbat_match_many_exact(void)
{
	struct sbat *test_sbat;
	unsigned int sbat_entry_array_size = 2;
	struct sbat_entry sbat_entry_array[sbat_entry_array_size];
	sbat_entry_array[0].component_name = "test1";
	sbat_entry_array[0].component_generation = "1";
	sbat_entry_array[0].vendor_name = "SBAT test1";
	sbat_entry_array[0].vendor_package_name = "acme";
	sbat_entry_array[0].vendor_version = "1";
	sbat_entry_array[0].vendor_url = "testURL";
	sbat_entry_array[1].component_name = "test2";
	sbat_entry_array[1].component_generation = "2";
	sbat_entry_array[1].vendor_name = "SBAT test2";
	sbat_entry_array[1].vendor_package_name = "acme2";
	sbat_entry_array[1].vendor_version = "2";
	sbat_entry_array[1].vendor_url = "testURL2";
	test_sbat = create_mock_sbat_multiple_entries(sbat_entry_array,
						      sbat_entry_array_size);
	if (!test_sbat)
		return -1;

	list_t *test_sbat_entries;
	unsigned int sbat_var_array_size = 2;
	struct sbat_var sbat_var_array[sbat_var_array_size];
	sbat_var_array[0].component_name = "test1";
	sbat_var_array[0].component_generation = "1";
	sbat_var_array[1].component_name = "test2";
	sbat_var_array[1].component_generation = "2";
	test_sbat_entries = create_mock_sbat_entries_multiple_entries(sbat_var_array,
								      sbat_var_array_size);
	if (!test_sbat_entries)
		return -1;
	EFI_STATUS status;

	status = verify_sbat(test_sbat, test_sbat_entries);

	assert(status == EFI_SUCCESS);
	free_mock_sbat(test_sbat);
	free_mock_sbat_entries(test_sbat_entries);
	return 0;
}

int
test_verify_sbat_reject_many_all(void)
{
	struct sbat *test_sbat;
	unsigned int sbat_entry_array_size = 2;
	struct sbat_entry sbat_entry_array[sbat_entry_array_size];
	sbat_entry_array[0].component_name = "test1";
	sbat_entry_array[0].component_generation = "1";
	sbat_entry_array[0].vendor_name = "SBAT test1";
	sbat_entry_array[0].vendor_package_name = "acme";
	sbat_entry_array[0].vendor_version = "1";
	sbat_entry_array[0].vendor_url = "testURL";
	sbat_entry_array[1].component_name = "test2";
	sbat_entry_array[1].component_generation = "2";
	sbat_entry_array[1].vendor_name = "SBAT test2";
	sbat_entry_array[1].vendor_package_name = "acme2";
	sbat_entry_array[1].vendor_version = "2";
	sbat_entry_array[1].vendor_url = "testURL2";
	test_sbat = create_mock_sbat_multiple_entries(sbat_entry_array,
						      sbat_entry_array_size);
	if (!test_sbat)
		return -1;

	list_t *test_sbat_entries;
	unsigned int sbat_var_array_size = 2;
	struct sbat_var sbat_var_array[sbat_var_array_size];
	sbat_var_array[0].component_name = "test1";
	sbat_var_array[0].component_generation = "3";
	sbat_var_array[1].component_name = "test2";
	sbat_var_array[1].component_generation = "5";
	test_sbat_entries = create_mock_sbat_entries_multiple_entries(sbat_var_array,
								      sbat_var_array_size);
	if (!test_sbat_entries)
		return -1;
	EFI_STATUS status;

	status = verify_sbat(test_sbat, test_sbat_entries);

	assert(status == EFI_SECURITY_VIOLATION);
	free_mock_sbat(test_sbat);
	free_mock_sbat_entries(test_sbat_entries);
	return 0;
}

int
test_verify_sbat_match_diff_name(void)
{
	struct sbat *test_sbat;
	unsigned int sbat_entry_array_size = 2;
	struct sbat_entry sbat_entry_array[sbat_entry_array_size];
	sbat_entry_array[0].component_name = "test1";
	sbat_entry_array[0].component_generation = "1";
	sbat_entry_array[0].vendor_name = "SBAT test1";
	sbat_entry_array[0].vendor_package_name = "acme";
	sbat_entry_array[0].vendor_version = "1";
	sbat_entry_array[0].vendor_url = "testURL";
	sbat_entry_array[1].component_name = "test2";
	sbat_entry_array[1].component_generation = "2";
	sbat_entry_array[1].vendor_name = "SBAT test2";
	sbat_entry_array[1].vendor_package_name = "acme2";
	sbat_entry_array[1].vendor_version = "2";
	sbat_entry_array[1].vendor_url = "testURL2";
	test_sbat = create_mock_sbat_multiple_entries(sbat_entry_array,
						      sbat_entry_array_size);
	if (!test_sbat)
		return -1;

	list_t *test_sbat_entries;
	unsigned int sbat_var_array_size = 2;
	struct sbat_var sbat_var_array[sbat_var_array_size];
	sbat_var_array[0].component_name = "foo";
	sbat_var_array[0].component_generation = "5";
	sbat_var_array[1].component_name = "bar";
	sbat_var_array[1].component_generation = "2";
	test_sbat_entries = create_mock_sbat_entries_multiple_entries(sbat_var_array,
								      sbat_var_array_size);
	if (!test_sbat_entries)
		return -1;
	EFI_STATUS status;

	status = verify_sbat(test_sbat, test_sbat_entries);

	assert(status == EFI_SUCCESS);
	free_mock_sbat(test_sbat);
	free_mock_sbat_entries(test_sbat_entries);
	return 0;
}

int
test_verify_sbat_match_diff_name_mixed(void)
{
	struct sbat *test_sbat;
	unsigned int sbat_entry_array_size = 2;
	struct sbat_entry sbat_entry_array[sbat_entry_array_size];
	sbat_entry_array[0].component_name = "test1";
	sbat_entry_array[0].component_generation = "1";
	sbat_entry_array[0].vendor_name = "SBAT test1";
	sbat_entry_array[0].vendor_package_name = "acme";
	sbat_entry_array[0].vendor_version = "1";
	sbat_entry_array[0].vendor_url = "testURL";
	sbat_entry_array[1].component_name = "test2";
	sbat_entry_array[1].component_generation = "2";
	sbat_entry_array[1].vendor_name = "SBAT test2";
	sbat_entry_array[1].vendor_package_name = "acme2";
	sbat_entry_array[1].vendor_version = "2";
	sbat_entry_array[1].vendor_url = "testURL2";
	test_sbat = create_mock_sbat_multiple_entries(sbat_entry_array,
						      sbat_entry_array_size);
	if (!test_sbat)
		return -1;

	list_t *test_sbat_entries;
	unsigned int sbat_var_array_size = 2;
	struct sbat_var sbat_var_array[sbat_var_array_size];
	sbat_var_array[0].component_name = "test1";
	sbat_var_array[0].component_generation = "1";
	sbat_var_array[1].component_name = "bar";
	sbat_var_array[1].component_generation = "2";
	test_sbat_entries = create_mock_sbat_entries_multiple_entries(sbat_var_array,
								      sbat_var_array_size);
	if (!test_sbat_entries)
		return -1;
	EFI_STATUS status;

	status = verify_sbat(test_sbat, test_sbat_entries);

	assert(status == EFI_SUCCESS);
	free_mock_sbat(test_sbat);
	free_mock_sbat_entries(test_sbat_entries);
	return 0;
}

int
test_verify_sbat_reject_diff_name_mixed(void)
{
	struct sbat *test_sbat;
	unsigned int sbat_entry_array_size = 2;
	struct sbat_entry sbat_entry_array[sbat_entry_array_size];
	sbat_entry_array[0].component_name = "test1";
	sbat_entry_array[0].component_generation = "1";
	sbat_entry_array[0].vendor_name = "SBAT test1";
	sbat_entry_array[0].vendor_package_name = "acme";
	sbat_entry_array[0].vendor_version = "1";
	sbat_entry_array[0].vendor_url = "testURL";
	sbat_entry_array[1].component_name = "test2";
	sbat_entry_array[1].component_generation = "2";
	sbat_entry_array[1].vendor_name = "SBAT test2";
	sbat_entry_array[1].vendor_package_name = "acme2";
	sbat_entry_array[1].vendor_version = "2";
	sbat_entry_array[1].vendor_url = "testURL2";
	test_sbat = create_mock_sbat_multiple_entries(sbat_entry_array,
						      sbat_entry_array_size);
	if (!test_sbat)
		return -1;

	list_t *test_sbat_entries;
	unsigned int sbat_var_array_size = 2;
	struct sbat_var sbat_var_array[sbat_var_array_size];
	sbat_var_array[0].component_name = "test1";
	sbat_var_array[0].component_generation = "5";
	sbat_var_array[1].component_name = "bar";
	sbat_var_array[1].component_generation = "2";
	test_sbat_entries = create_mock_sbat_entries_multiple_entries(sbat_var_array,
								      sbat_var_array_size);
	if (!test_sbat_entries)
		return -1;
	EFI_STATUS status;

	status = verify_sbat(test_sbat, test_sbat_entries);

	assert(status == EFI_SECURITY_VIOLATION);
	free_mock_sbat(test_sbat);
	free_mock_sbat_entries(test_sbat_entries);
	return 0;
}

int
main(void)
{
	// parse_sbat tests
	test_parse_sbat_null_sbat_base();
	test_parse_sbat_zero_sbat_size();
	test_parse_sbat_null_sbat();
	//test_parse_sbat_no_newline();
	test_parse_sbat_too_few_elem();
	test_parse_sbat_too_many_elem();
	//test_parse_sbat_no_newline_multiple_entries();
	test_parse_sbat_too_few_elem_multiple_entries();
	test_parse_sbat_too_many_elem_multiple_entries();
	test_parse_sbat_single_entry();
	test_parse_sbat_multiple_entries();

	// parse_sbat_var tests
	test_parse_sbat_var_invalid_list();

	// verify_sbat tests
	//test_verify_sbat_null_sbat();
	test_verify_sbat_null_sbat_entries();
	test_verify_sbat_match_one_exact();
	test_verify_sbat_match_one_higher();
	test_verify_sbat_reject_one();
	test_verify_sbat_reject_many();
	test_verify_sbat_match_many_higher();
	test_verify_sbat_match_many_exact();
	test_verify_sbat_reject_many_all();
	test_verify_sbat_match_diff_name();
	test_verify_sbat_match_diff_name_mixed();
	test_verify_sbat_reject_diff_name_mixed();

	return 0;
}

// vim:fenc=utf-8:tw=75:noet
