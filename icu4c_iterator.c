#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_icu4c.h"

// Object handlers
static zend_object_handlers icu4c_iterator_handlers;

// Object creation function
zend_object *icu4c_iterator_create_object(zend_class_entry *ce)
{
    icu4c_iterator_obj *obj = zend_object_alloc(sizeof(icu4c_iterator_obj), ce);
    
    zend_object_std_init(&obj->std, ce);
    object_properties_init(&obj->std, ce);
    
    obj->std.handlers = &icu4c_iterator_handlers;
    
    // Initialize fields
    obj->text = NULL;
    obj->break_iter = NULL;
    obj->utext = NULL;
    obj->current_pos = 0;
    obj->total_clusters = 0;
    obj->cluster_boundaries = NULL;
    
    return &obj->std;
}

// Object destructor
static void icu4c_iterator_free_object(zend_object *object)
{
    icu4c_iterator_obj *obj = icu4c_iterator_from_obj(object);
    
    if (obj->text) {
        zend_string_release(obj->text);
    }
    
    if (obj->cluster_boundaries) {
        efree(obj->cluster_boundaries);
    }
    
    zend_object_std_dtor(&obj->std);
}

// Count elements handler for Countable interface
static zend_result icu4c_iterator_count_elements(zend_object *object, zend_long *count)
{
    icu4c_iterator_obj *obj = icu4c_iterator_from_obj(object);
    *count = obj->total_clusters;
    return SUCCESS;
}

// Internal iterator functions for IteratorAggregate
static void icu4c_internal_iterator_dtor(zend_object_iterator *iter)
{
    icu4c_internal_iterator *iterator = (icu4c_internal_iterator*)iter;
    if (Z_TYPE(iterator->current_value) != IS_UNDEF) {
        zval_ptr_dtor(&iterator->current_value);
    }
    zval_ptr_dtor(&iter->data);
}

static void icu4c_internal_iterator_rewind(zend_object_iterator *iter)
{
    ((icu4c_internal_iterator*)iter)->current_pos = 0;
}

static zend_result icu4c_internal_iterator_valid(zend_object_iterator *iter)
{
    icu4c_internal_iterator *iterator = (icu4c_internal_iterator*)iter;
    icu4c_iterator_obj *object = icu4c_iterator_from_obj(Z_OBJ(iter->data));

    if (iterator->current_pos < object->total_clusters) {
        return SUCCESS;
    }
    return FAILURE;
}

static zval *icu4c_internal_iterator_get_current(zend_object_iterator *iter)
{
    icu4c_internal_iterator *iterator = (icu4c_internal_iterator*)iter;
    icu4c_iterator_obj *object = icu4c_iterator_from_obj(Z_OBJ(iter->data));

    if (iterator->current_pos >= object->total_clusters) {
        return &EG(uninitialized_zval);
    }

#ifdef HAVE_ICU4C
    zend_string *cluster_str = icu4c_get_cluster_at_position(
        ZSTR_VAL(object->text), 
        ZSTR_LEN(object->text), 
        object->cluster_boundaries, 
        iterator->current_pos
    );
#else
    // Fallback: return single UTF-8 character
    const char *text = ZSTR_VAL(object->text);
    size_t text_len = ZSTR_LEN(object->text);
    size_t current_char = 0;
    size_t pos = 0;
    
    while (pos < text_len && current_char < iterator->current_pos) {
        if ((text[pos] & 0xC0) != 0x80) {
            current_char++;
        }
        pos++;
    }
    
    if (pos < text_len) {
        size_t char_len = 1;
        while (pos + char_len < text_len && (text[pos + char_len] & 0xC0) == 0x80) {
            char_len++;
        }
        zend_string *cluster_str = zend_string_init(text + pos, char_len, 0);
    } else {
        zend_string *cluster_str = NULL;
    }
#endif

    if (cluster_str) {
        // Store the current value in the iterator structure
        if (Z_TYPE(iterator->current_value) != IS_UNDEF) {
            zval_ptr_dtor(&iterator->current_value);
        }
        ZVAL_STR(&iterator->current_value, cluster_str);
        return &iterator->current_value;
    } else {
        return &EG(uninitialized_zval);
    }
}

static void icu4c_internal_iterator_get_key(zend_object_iterator *iter, zval *key)
{
    icu4c_internal_iterator *iterator = (icu4c_internal_iterator*)iter;
    ZVAL_LONG(key, iterator->current_pos);
}

static void icu4c_internal_iterator_move_forward(zend_object_iterator *iter)
{
    ((icu4c_internal_iterator*)iter)->current_pos++;
}

// Iterator function table
static const zend_object_iterator_funcs icu4c_internal_iterator_funcs = {
    icu4c_internal_iterator_dtor,
    icu4c_internal_iterator_valid,
    icu4c_internal_iterator_get_current,
    icu4c_internal_iterator_get_key,
    icu4c_internal_iterator_move_forward,
    icu4c_internal_iterator_rewind,
    NULL,
    NULL,
};

// Get iterator handler for IteratorAggregate
static zend_object_iterator *icu4c_iterator_get_iterator(zend_class_entry *ce, zval *object, int by_ref)
{
    if (by_ref) {
        zend_throw_error(NULL, "An iterator cannot be used with foreach by reference");
        return NULL;
    }

    // Create internal iterator that wraps our object and implements Iterator interface
    icu4c_internal_iterator *iterator = emalloc(sizeof(icu4c_internal_iterator));
    zend_iterator_init((zend_object_iterator*)iterator);

    ZVAL_OBJ_COPY(&iterator->intern.data, Z_OBJ_P(object));
    iterator->intern.funcs = &icu4c_internal_iterator_funcs;
    iterator->current_pos = 0;
    ZVAL_UNDEF(&iterator->current_value);

    return &iterator->intern;
}

// ICU4CIterator::__construct method
PHP_METHOD(ICU4CIterator, __construct)
{
    zend_string *text;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();
    
    icu4c_iterator_obj *obj = icu4c_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    // Initialize the iterator
    obj->text = zend_string_copy(text);
    obj->current_pos = 0;
    obj->cluster_boundaries = NULL;
    
#ifdef HAVE_ICU4C
    // Count grapheme clusters and build boundary array
    obj->total_clusters = icu4c_count_grapheme_clusters(ZSTR_VAL(text), ZSTR_LEN(text), &obj->cluster_boundaries);
#else
    // Fallback: count UTF-8 characters
    obj->total_clusters = 0;
    const char *ptr = ZSTR_VAL(text);
    const char *end = ptr + ZSTR_LEN(text);
    
    while (ptr < end) {
        if ((*ptr & 0xC0) != 0x80) {
            obj->total_clusters++;
        }
        ptr++;
    }
#endif
}

// ICU4CIterator::current method
PHP_METHOD(ICU4CIterator, current)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    icu4c_iterator_obj *obj = icu4c_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    if (!obj->text || obj->current_pos >= obj->total_clusters) {
        RETURN_NULL();
    }
    
#ifdef HAVE_ICU4C
    zend_string *cluster_str = icu4c_get_cluster_at_position(
        ZSTR_VAL(obj->text), 
        ZSTR_LEN(obj->text), 
        obj->cluster_boundaries, 
        obj->current_pos
    );
#else
    // Fallback: return single UTF-8 character
    const char *text = ZSTR_VAL(obj->text);
    size_t text_len = ZSTR_LEN(obj->text);
    size_t current_char = 0;
    size_t pos = 0;
    
    while (pos < text_len && current_char < obj->current_pos) {
        if ((text[pos] & 0xC0) != 0x80) {
            current_char++;
        }
        pos++;
    }
    
    if (pos < text_len) {
        size_t char_len = 1;
        while (pos + char_len < text_len && (text[pos + char_len] & 0xC0) == 0x80) {
            char_len++;
        }
        zend_string *cluster_str = zend_string_init(text + pos, char_len, 0);
    } else {
        zend_string *cluster_str = NULL;
    }
#endif
    
    if (cluster_str) {
        RETURN_STR(cluster_str);
    } else {
        RETURN_NULL();
    }
}

// ICU4CIterator::key method
PHP_METHOD(ICU4CIterator, key)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    icu4c_iterator_obj *obj = icu4c_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    RETURN_LONG(obj->current_pos);
}

// ICU4CIterator::next method
PHP_METHOD(ICU4CIterator, next)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    icu4c_iterator_obj *obj = icu4c_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    if (obj->current_pos < obj->total_clusters) {
        obj->current_pos++;
    }
}

// ICU4CIterator::rewind method
PHP_METHOD(ICU4CIterator, rewind)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    icu4c_iterator_obj *obj = icu4c_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    obj->current_pos = 0;
}

// ICU4CIterator::valid method
PHP_METHOD(ICU4CIterator, valid)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    icu4c_iterator_obj *obj = icu4c_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    RETURN_BOOL(obj->text && obj->current_pos < obj->total_clusters);
}

// ICU4CIterator::getIterator method  
PHP_METHOD(ICU4CIterator, getIterator)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    // Return self since this class implements Iterator
    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

// ICU4CIterator::count method
PHP_METHOD(ICU4CIterator, count)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    icu4c_iterator_obj *obj = icu4c_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    RETURN_LONG(obj->total_clusters);
}

// Method entries for ICU4CIterator class
static const zend_function_entry icu4c_iterator_methods[] = {
    PHP_ME(ICU4CIterator, __construct, arginfo_icu4c_iterator_construct, ZEND_ACC_PUBLIC)
    PHP_ME(ICU4CIterator, current, arginfo_icu4c_iterator_current, ZEND_ACC_PUBLIC)
    PHP_ME(ICU4CIterator, key, arginfo_icu4c_iterator_key, ZEND_ACC_PUBLIC)
    PHP_ME(ICU4CIterator, next, arginfo_icu4c_iterator_next, ZEND_ACC_PUBLIC)
    PHP_ME(ICU4CIterator, rewind, arginfo_icu4c_iterator_rewind, ZEND_ACC_PUBLIC)
    PHP_ME(ICU4CIterator, valid, arginfo_icu4c_iterator_valid, ZEND_ACC_PUBLIC)
    PHP_ME(ICU4CIterator, getIterator, arginfo_icu4c_iterator_getiterator, ZEND_ACC_PUBLIC)
    PHP_ME(ICU4CIterator, count, arginfo_icu4c_iterator_count, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

// Initialize ICU4CIterator class
void icu4c_iterator_init(void)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "ICU4CIterator", icu4c_iterator_methods);
    icu4c_iterator_ce = zend_register_internal_class(&ce);
    icu4c_iterator_ce->create_object = icu4c_iterator_create_object;
    
    // Set up object handlers
    memcpy(&icu4c_iterator_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    icu4c_iterator_handlers.free_obj = icu4c_iterator_free_object;
    icu4c_iterator_handlers.offset = XtOffsetOf(icu4c_iterator_obj, std);
    icu4c_iterator_handlers.count_elements = icu4c_iterator_count_elements;
    
    // Set get_iterator handler for IteratorAggregate
    icu4c_iterator_ce->get_iterator = icu4c_iterator_get_iterator;
    
    // Implement IteratorAggregate and Countable interfaces
    zend_class_implements(icu4c_iterator_ce, 2, zend_ce_aggregate, zend_ce_countable);
}