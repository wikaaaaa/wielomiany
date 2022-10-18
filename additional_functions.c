#include <stdlib.h>
#include "additional_functions.h"
#include "poly.h"
#include <ctype.h>

void lengthenIfNecessary(char **text, size_t *length, size_t i) {
    if (i == *length) {
        *length = more(*length);
        *text = realloc(*text, (size_t) (*length) * sizeof *(*text));
        CheckReallocOutcome(text);
    }
}

void CheckReallocOutcome(const void *a) {
    if (a == NULL) {
        exit(1);
    }
}

void LengthenArrayIfNecessary(Mono **arr, size_t *length, size_t i) {
    if (*length == i) {
        *length = more(*length);
        *arr = realloc(*arr, *length * sizeof *(*arr));
        CheckReallocOutcome(*arr);
    }
}

size_t more(size_t length) {
    return 1 + 2 * length;
}