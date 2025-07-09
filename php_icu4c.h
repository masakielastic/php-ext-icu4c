#ifndef PHP_ICU4C_H
#define PHP_ICU4C_H

#include "php.h"
#include "zend_interfaces.h"

#ifdef HAVE_ICU4C
#include <unicode/ubrk.h>
#include <unicode/utext.h>
#include <unicode/uloc.h>
#include <unicode/ustring.h>
#endif

#define PHP_ICU4C_VERSION "1.0.0"
#define PHP_ICU4C_EXTNAME "icu4c"

extern zend_module_entry icu4c_module_entry;
#define phpext_icu4c_ptr &icu4c_module_entry

// ICU4CIterator class entry
extern zend_class_entry *icu4c_iterator_ce;

// ICU4CIterator object structure
typedef struct _icu4c_iterator_obj {
    zend_string *text;           // Original text string
    UBreakIterator *break_iter;  // ICU4C BreakIterator
    UText *utext;               // ICU4C UText
    size_t current_pos;         // Current position (cluster index)
    size_t total_clusters;      // Total number of grapheme clusters
    int32_t *cluster_boundaries; // Array of cluster boundary positions
    zend_object std;            // Standard object
} icu4c_iterator_obj;

// Object accessor macro
static inline icu4c_iterator_obj *icu4c_iterator_from_obj(zend_object *obj) {
    return (icu4c_iterator_obj*)((char*)(obj) - XtOffsetOf(icu4c_iterator_obj, std));
}

// Internal iterator structure for IteratorAggregate
typedef struct _icu4c_internal_iterator {
    zend_object_iterator intern;
    size_t current_pos;
    zval current_value;
} icu4c_internal_iterator;

// Function declarations
PHP_FUNCTION(icu4c_iter);

// ArgInfo declarations
ZEND_BEGIN_ARG_INFO_EX(arginfo_icu4c_iter, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_icu4c_iterator_construct, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_icu4c_iterator_current, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_icu4c_iterator_key, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_icu4c_iterator_next, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_icu4c_iterator_rewind, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_icu4c_iterator_valid, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_icu4c_iterator_getiterator, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_icu4c_iterator_count, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_MINIT_FUNCTION(icu4c);
PHP_MSHUTDOWN_FUNCTION(icu4c);
PHP_MINFO_FUNCTION(icu4c);

// ICU4CIterator class method declarations
PHP_METHOD(ICU4CIterator, __construct);
PHP_METHOD(ICU4CIterator, current);
PHP_METHOD(ICU4CIterator, key);
PHP_METHOD(ICU4CIterator, next);
PHP_METHOD(ICU4CIterator, rewind);
PHP_METHOD(ICU4CIterator, valid);
PHP_METHOD(ICU4CIterator, getIterator);
PHP_METHOD(ICU4CIterator, count);

// Internal utility functions
#ifdef HAVE_ICU4C
size_t icu4c_count_grapheme_clusters(const char *text, size_t text_len, int32_t **boundaries);
zend_string *icu4c_get_cluster_at_position(const char *text, size_t text_len, const int32_t *boundaries, size_t cluster_index);
#endif

// ICU4CIterator class initialization
void icu4c_iterator_init(void);
zend_object *icu4c_iterator_create_object(zend_class_entry *ce);

#endif /* PHP_ICU4C_H */