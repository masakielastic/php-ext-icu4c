#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_icu4c.h"

// Global class entry
zend_class_entry *icu4c_iterator_ce;

// icu4c_iter function implementation
PHP_FUNCTION(icu4c_iter)
{
    zend_string *text;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();
    
    // Create new ICU4CIterator object
    object_init_ex(return_value, icu4c_iterator_ce);
    
    // Get the object structure
    icu4c_iterator_obj *obj = icu4c_iterator_from_obj(Z_OBJ_P(return_value));
    
    // Initialize the iterator
    obj->text = zend_string_copy(text);
    obj->current_pos = 0;
    obj->break_iter = NULL;
    obj->utext = NULL;
    obj->cluster_boundaries = NULL;
    
#ifdef HAVE_ICU4C
    // Count grapheme clusters and build boundary array
    obj->total_clusters = icu4c_count_grapheme_clusters(ZSTR_VAL(text), ZSTR_LEN(text), &obj->cluster_boundaries);
    
    if (obj->total_clusters == 0) {
        // Handle error case
        obj->total_clusters = 0;
    }
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

#ifdef HAVE_ICU4C
// Count grapheme clusters and build boundary array
size_t icu4c_count_grapheme_clusters(const char *text, size_t text_len, int32_t **boundaries)
{
    if (text_len == 0) {
        *boundaries = NULL;
        return 0;
    }
    
    UErrorCode status = U_ZERO_ERROR;
    UText *ut = utext_openUTF8(NULL, text, text_len, &status);
    if (U_FAILURE(status)) {
        *boundaries = NULL;
        return 0;
    }
    
    UBreakIterator *bi = ubrk_open(UBRK_CHARACTER, uloc_getDefault(), NULL, 0, &status);
    if (U_FAILURE(status)) {
        utext_close(ut);
        *boundaries = NULL;
        return 0;
    }
    
    ubrk_setUText(bi, ut, &status);
    if (U_FAILURE(status)) {
        ubrk_close(bi);
        utext_close(ut);
        *boundaries = NULL;
        return 0;
    }
    
    // Count clusters and collect boundaries
    size_t cluster_count = 0;
    size_t boundary_capacity = 16;
    int32_t *boundary_array = emalloc(boundary_capacity * sizeof(int32_t));
    
    int32_t previous = ubrk_first(bi);
    if (previous == UBRK_DONE) {
        ubrk_close(bi);
        utext_close(ut);
        efree(boundary_array);
        *boundaries = NULL;
        return 0;
    }
    
    boundary_array[cluster_count++] = previous;
    
    int32_t current;
    while ((current = ubrk_next(bi)) != UBRK_DONE) {
        if (cluster_count >= boundary_capacity) {
            boundary_capacity *= 2;
            boundary_array = erealloc(boundary_array, boundary_capacity * sizeof(int32_t));
        }
        boundary_array[cluster_count++] = current;
        previous = current;
    }
    
    ubrk_close(bi);
    utext_close(ut);
    
    // Resize to actual size
    boundary_array = erealloc(boundary_array, cluster_count * sizeof(int32_t));
    *boundaries = boundary_array;
    
    return cluster_count - 1; // Number of clusters is boundaries - 1
}

// Get grapheme cluster at specific position
zend_string *icu4c_get_cluster_at_position(const char *text, size_t text_len, const int32_t *boundaries, size_t cluster_index)
{
    if (boundaries == NULL || cluster_index >= text_len) {
        return NULL;
    }
    
    int32_t start = boundaries[cluster_index];
    int32_t end = boundaries[cluster_index + 1];
    
    if (start >= 0 && end > start && end <= (int32_t)text_len) {
        return zend_string_init(text + start, end - start, 0);
    }
    
    return NULL;
}
#endif

// Function entries
const zend_function_entry icu4c_functions[] = {
    PHP_FE(icu4c_iter, arginfo_icu4c_iter)
    PHP_FE_END
};

// Module entry
zend_module_entry icu4c_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_ICU4C_EXTNAME,
    icu4c_functions,
    PHP_MINIT(icu4c),
    PHP_MSHUTDOWN(icu4c),
    NULL,
    NULL,
    PHP_MINFO(icu4c),
    PHP_ICU4C_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_ICU4C
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(icu4c)
#endif

// Module initialization
PHP_MINIT_FUNCTION(icu4c)
{
    // Initialize ICU4CIterator class
    icu4c_iterator_init();
    
    return SUCCESS;
}

// Module shutdown
PHP_MSHUTDOWN_FUNCTION(icu4c)
{
    return SUCCESS;
}

// Module info
PHP_MINFO_FUNCTION(icu4c)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "ICU4C support", "enabled");
    php_info_print_table_row(2, "Version", PHP_ICU4C_VERSION);
#ifdef HAVE_ICU4C
    php_info_print_table_row(2, "ICU4C support", "enabled");
    
    // Get ICU version
    UVersionInfo version;
    u_getVersion(version);
    char version_str[U_MAX_VERSION_STRING_LENGTH];
    u_versionToString(version, version_str);
    php_info_print_table_row(2, "ICU Version", version_str);
#else
    php_info_print_table_row(2, "ICU4C support", "disabled");
#endif
    php_info_print_table_end();
}