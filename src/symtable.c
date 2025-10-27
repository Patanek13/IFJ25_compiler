/**
 * @file symtable.c
 * @brief Symtable implementation
 * @author Patrik Lošťák (xlostap00)
 * @details This file contains implementation of symtable operations and helper
 * functions
 *
 */

#include "symtable.h"

/* PJW Hash Function
 * @link https://ssojet.com/compare-hashing-algorithms/bernsteins-hash-djb2-vs-pjw-hash--elf-hash
 *
 */
size_t symtable_hash(const char *key) {
  if (key == NULL) {
    return 0;
  }

  size_t hash = 0, high;
  const size_t high_mask = ((size_t)0xF) << (8 * sizeof(size_t) - 4); // Mask for high bits
  while (*key) {
    hash = (hash << 4) + (unsigned char)(*key++);
    if ((high = hash & high_mask) != 0) {
      hash ^= (high >> (8 * (sizeof(size_t) - 8))); // Clear high bits
    }
    hash &= ~high; // Clear high bits
  }
  return hash % SYMTABLE_DEFAULT_SIZE;
}

ErrorCode symtable_init(SymTable *table) {
  if (table == NULL) {
    return ERR_INTERNAL;
  }

  table->size = SYMTABLE_DEFAULT_SIZE;
  table->count = 0;
  table->items = (SymItem *)calloc(table->size, sizeof(SymItem));

  if (table->items == NULL) {
    return ERR_INTERNAL;
  }

  return ERR_OK;
}

void symtable_free(SymTable *table) {
  if (table == NULL || table->items == NULL) {
    return;
  }

  for (size_t idx = 0; idx < table->size; ++idx) {
    if (table->items[idx].state == SLOT_OCCUPIED) {
      free(table->items[idx].key);
      // Free SymbolData if it contains dynamically allocated memory
      if (table->items[idx].data.kind == SYM_FUNCTION &&
          table->items[idx].data.info.function.param_types) {
        free(table->items[idx].data.info.function.param_types);
      }
    }
  }

  free(table->items);
  table->items = NULL;
  table->size = 0;
  table->count = 0;
}

SymbolData *symtable_lookup(SymTable *table, const char *key) {
  if (table == NULL || key == NULL) {
    return NULL;
  }

  unsigned index = symtable_hash(key);
  for (unsigned idx = 0; idx < table->size; idx++) {
    unsigned pos = (index + idx) % table->size;
    SymItem *item = &table->items[pos];

    if (item->state == SLOT_EMPTY) {
      return NULL; // Not found
    }

    if (item->state == SLOT_OCCUPIED && strcmp(item->key, key) == 0) {
      return &item->data; // Found
    }
  }
  return NULL; // Not found
}
