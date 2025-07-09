#include <stdio.h>
#include <stdlib.h>
#include <unicode/ubrk.h>
#include <unicode/utext.h>
#include <unicode/uloc.h>
#include <unicode/ustring.h>

void print_each_grapheme(const char *str);

int main(void)
{
    const char* str = "葛\U000E0101飾区";
    print_each_grapheme(str);
    return 0;
}

void print_each_grapheme(const char *str)
{
    UErrorCode status = U_ZERO_ERROR;
    UText *ut = utext_openUTF8(NULL, str, -1, &status);
    if (U_FAILURE(status)) {
        fprintf(stderr, "utext_openUTF8 error: %s\n", u_errorName(status));
        return;
    }

    UBreakIterator *bi = ubrk_open(UBRK_CHARACTER, uloc_getDefault(), NULL, 0, &status);
    if (U_FAILURE(status)) {
        fprintf(stderr, "ubrk_open error: %s\n", u_errorName(status));
        utext_close(ut);
        return;
    }

    ubrk_setUText(bi, ut, &status);
    if (U_FAILURE(status)) {
        fprintf(stderr, "ubrk_setUText error: %s\n", u_errorName(status));
        ubrk_close(bi);
        utext_close(ut);
        return;
    }

    int32_t previous = ubrk_first(bi);
    if (previous == UBRK_DONE) {
        fprintf(stderr, "ubrk_first returned UBRK_DONE (empty string?)\n");
        ubrk_close(bi);
        utext_close(ut);
        return;
    }

    int32_t current;
    while ((current = ubrk_next(bi)) != UBRK_DONE) {
        int32_t size = current - previous;
        // UTF-8 の範囲外を保護
        if (size > 0) {
            printf("%.*s\n", size, str + previous);
        } else {
            fprintf(stderr, "Detected non-positive cluster size: %d (previous=%d, current=%d)\n", size, previous, current);
        }
        previous = current;
    }

    ubrk_close(bi);
    utext_close(ut);
}
