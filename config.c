// config.c
#include <stdlib.h>
#include "config.h"

static const char* fallback(const char* v, const char* def) {
    return (v && *v) ? v : def;
}

const char* get_username(void) {
    return fallback(getenv("BRAND_NAME"), "Aotumuri");
}

const char* get_first_title(void) {
    return fallback(getenv("FIRST_TITLE"), "Programming Languages");
}
const char* get_first_desc1(void) {
    return fallback(getenv("FIRST_DESC1"), "TypeScript / Python / C#");
}
const char* get_first_desc2(void) {
    return fallback(getenv("FIRST_DESC2"), "JavaScript / McFunction / C");
}

const char* get_second_title(void) {
    return fallback(getenv("SECOND_TITLE"), "Spoken Languages");
}
const char* get_second_desc1(void) {
    return fallback(getenv("SECOND_DESC1"), "Japanese (Native)");
}
const char* get_second_desc2(void) {
    return fallback(getenv("SECOND_DESC2"), "English, German (Learning)");
}