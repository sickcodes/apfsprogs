/*
 *  apfsprogs/apfsck/htable.c
 *
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include "apfsck.h"
#include "htable.h"
#include "super.h"
#include "types.h"

/**
 * alloc_htable - Allocates and returns an empty hash table
 */
union htable_entry **alloc_htable(void)
{
	union htable_entry **table;

	table = calloc(HTABLE_BUCKETS, sizeof(*table));
	if (!table) {
		perror(NULL);
		exit(1);
	}
	return table;
}

/**
 * free_htable - Free a hash table and all its entries
 * @table:	the catalog table to free
 * @free_entry:	function that checks and frees an entry
 */
void free_htable(union htable_entry **table,
		 void (*free_entry)(union htable_entry *))
{
	union htable_entry *current;
	union htable_entry *next;
	int i;

	for (i = 0; i < INODE_TABLE_BUCKETS; ++i) {
		current = table[i];
		while (current) {
			next = current->header.h_next;
			free_entry(current);
			current = next;
		}
	}
	free(table);
}

/**
 * get_htable_entry - Find or create an entry in a hash table
 * @id:		id of the entry
 * @size:	size of the entry
 * @table:	the hash table
 *
 * Returns the entry, after creating it if necessary.
 */
union htable_entry *get_htable_entry(u64 id, int size,
				     union htable_entry **table)
{
	int index = id % INODE_TABLE_BUCKETS; /* Trivial hash function */
	union htable_entry **entry_p = table + index;
	union htable_entry *entry = *entry_p;
	union htable_entry *new;

	/* In each linked list, entries are ordered by id */
	while (entry) {
		if (id == entry->header.h_id)
			return entry;
		if (id < entry->header.h_id)
			break;

		entry_p = &entry->header.h_next;
		entry = *entry_p;
	}

	new = calloc(1, size);
	if (!new) {
		perror(NULL);
		exit(1);
	}

	new->header.h_id = id;
	new->header.h_next = entry;
	*entry_p = new;
	return new;
}

/**
 * free_cnid_table - Free the cnid hash table and all its entries
 * @table: table to free
 */
void free_cnid_table(union htable_entry **table)
{
	/* No checks needed here, just call free() on each entry */
	free_htable(table, (void (*)(union htable_entry *))free);
}

/**
 * get_listed_cnid - Find or create a cnid structure in the cnid hash table
 * @id: the cnid
 *
 * Returns the cnid structure, after creating it if necessary.
 */
struct listed_cnid *get_listed_cnid(u64 id)
{
	union htable_entry *entry;

	entry = get_htable_entry(id, sizeof(struct listed_cnid),
				 vsb->v_cnid_table);
	return &entry->listed_cnid;
}
