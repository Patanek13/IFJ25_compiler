/**
 * @file symtable.c
 * @brief Symtable implementation
 * @author Patrik Lošťák (xlostap00)
 * @details This file contains implementation of symtable operations and helper
 * functions
 *
 */

#include "symtable.h"
#include "strutils.h"


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
      hash ^= (high >> (8 * sizeof(size_t) - 8)); // Clear high bits
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

  size_t hash = symtable_hash(key);
  for (size_t idx = 0; idx < table->size; idx++) {
    size_t pos = (hash + idx) % table->size;
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


ErrorCode symtable_insert(SymTable *table, const char *key, SymbolData data) {
  if (table == NULL || key == NULL) {
    return ERR_INTERNAL;
  }

  size_t hash = symtable_hash(key);

  for (size_t idx = 0; idx < table->size; idx++) {
    size_t pos = (hash + idx) % table->size;
    SymItem *item = &table->items[pos];

    if (item->state == SLOT_EMPTY || item->state == SLOT_DELETED) {
      item->key = str_dup(key); // Duplicate the key
      if (item->key == NULL) {
        return ERR_INTERNAL; // Memory allocation failure
      }

      item->data = data;
      item->state = SLOT_OCCUPIED;
      table->count++;
      return ERR_OK; // Successfully inserted
    }

    if (item->state == SLOT_OCCUPIED && strcmp(item->key, key) == 0) {
      return ERR_SEMANTIC_REDEFINITION; // Symbol already exists
    }
  }

  return ERR_INTERNAL; // Table full
}


ErrorCode symtable_delete(SymTable *table, const char *key) {
  if (table == NULL || key == NULL) {
    return ERR_INTERNAL;
  }

  size_t hash = symtable_hash(key);

  for (size_t idx = 0; idx < table->size; idx++) {
    size_t pos = (hash + idx) % table->size;
    SymItem *item = &table->items[pos];

    if (item->state == SLOT_EMPTY) {
      return ERR_SEMANTIC_UNDEFINED; // Not found
    }

    if (item->state == SLOT_OCCUPIED && strcmp(item->key, key) == 0) {
      free(item->key);
      // Free SymbolData if it contains dynamically allocated memory
      if (item->data.kind == SYM_FUNCTION && item->data.info.function.param_types) {
        free(item->data.info.function.param_types);
      }

      item->state = SLOT_DELETED;
      table->count--;
      return ERR_OK; // Successfully deleted
    }
  }

  return ERR_SEMANTIC_UNDEFINED; // Not found
}


SymbolData create_variable_symbol(DataType type) {
  SymbolData data;
  data.kind = SYM_VARIABLE;
  data.type = type;
  data.defined = true;
  return data;
}

SymbolData create_function_symbol(DataType return_type, size_t param_count) {
  SymbolData data;
  data.kind = SYM_FUNCTION;
  data.type = return_type;
  data.defined = false; // Functions are initially declared but not defined
  data.info.function.param_count = param_count;

  if (param_count > 0) {
    data.info.function.param_types = malloc(param_count * sizeof(DataType));
    if (data.info.function.param_types == NULL) {
      // Handle memory allocation failure
      data.info.function.param_count = 0;
    }

    for (size_t idx = 0; idx < param_count; idx++) {
      data.info.function.param_types[idx] = TYPE_UNKNOWN; // Initialize parameter types
    }

  } else {
    data.info.function.param_types = NULL;
  }
  return data;
}


SymbolData create_getter_symbol(DataType type) {
  SymbolData data = create_function_symbol(type, 0);
  data.kind = SYM_GETTER;
  data.defined = true;
  return data;
}

SymbolData create_setter_symbol(DataType param_type) {
  SymbolData data;
  data.kind = SYM_SETTER;
  data.type = TYPE_NULL; // Setters do not return a value
  data.defined = true;

  // Set parameter type directly in setter info
  data.info.setter.param_type = param_type;

  return data;
}


char *make_function_key(const char *name, size_t param_count) {
  if (name == NULL) return NULL;

  size_t name_len = strlen(name);

  // Calculate number of digits in param_count
  size_t digits;
  if (param_count == 0) {
    digits = 1;
  } else {
    digits = 0;
    size_t tmp = param_count;
    while (tmp > 0) {
      digits++;
      tmp /= 10;
    }
  }

  size_t key_len = name_len + 1 + digits + 1; /* name + '#' + digits + '\0' */
  char *key = malloc(key_len);
  if (key == NULL) return NULL;

  snprintf(key, key_len, "%s#%zu", name, param_count);
  // Have to free the key after use
  return key;
}

