// config.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"

enum { FIELD_COUNT = 7, VALUE_SIZE = 1024 };

static const char *const env_keys[FIELD_COUNT] = {
    "BRAND_NAME", "FIRST_TITLE", "FIRST_DESC1", "FIRST_DESC2",
    "SECOND_TITLE", "SECOND_DESC1", "SECOND_DESC2"
};
static const char *const defaults[FIELD_COUNT] = {
    "Aotumuri", "Programming Languages", "TypeScript / Python / C#",
    "JavaScript / McFunction / C", "Spoken Languages", "Japanese (Native)",
    "English, German (Learning)"
};
static char file_values[FIELD_COUNT][VALUE_SIZE];
static int initialized;

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)end[-1])) *--end = '\0';
    return s;
}

static void load_profile(void) {
    if (initialized) return;
    initialized = 1;
    const char *path = getenv("SVG4README_CONFIG");
    FILE *fp = fopen((path && *path) ? path : "profile.conf", "r");
    if (!fp) return; /* profile.conf is optional */

    char line[2048];
    while (fgets(line, sizeof(line), fp)) {
        char *key = trim(line);
        if (*key == '#' || !*key) continue;
        char *eq = strchr(key, '=');
        if (!eq) continue;
        *eq = '\0';
        key = trim(key);
        char *value = trim(eq + 1);
        size_t len = strlen(value);
        if (len >= 2 && ((value[0] == '"' && value[len - 1] == '"') ||
                         (value[0] == '\'' && value[len - 1] == '\''))) {
            value[len - 1] = '\0';
            value++;
        }
        for (int i = 0; i < FIELD_COUNT; i++) {
            if (strcmp(key, env_keys[i]) == 0) {
                snprintf(file_values[i], VALUE_SIZE, "%s", value);
                break;
            }
        }
    }
    fclose(fp);
}

static const char *value_for(int field) {
    load_profile();
    const char *env = getenv(env_keys[field]);
    if (env && *env) return env; /* environment variables can still override a profile */
    return file_values[field][0] ? file_values[field] : defaults[field];
}

const char* get_username(void) { return value_for(0); }
const char* get_first_title(void) { return value_for(1); }
const char* get_first_desc1(void) { return value_for(2); }
const char* get_first_desc2(void) { return value_for(3); }
const char* get_second_title(void) {
    return value_for(4);
}
const char* get_second_desc1(void) { return value_for(5); }
const char* get_second_desc2(void) { return value_for(6); }

static size_t utf8_width(const char *s) {
    unsigned char c = (unsigned char)*s;
    if ((c & 0x80) == 0) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

static void print_escaped_range(FILE *fp, const char *start, const char *end) {
    for (const char *p = start; p < end; p++) {
        switch (*p) {
            case '&': fputs("&amp;", fp); break;
            case '<': fputs("&lt;", fp); break;
            case '>': fputs("&gt;", fp); break;
            case '\"': fputs("&quot;", fp); break;
            case '\'': fputs("&apos;", fp); break;
            default: fputc(*p, fp);
        }
    }
}

void print_svg_escaped(FILE *fp, const char *value) {
    const char *text = value ? value : "";
    print_escaped_range(fp, text, text + strlen(text));
}

double brand_font_size(const char *value) {
    int chars = 0;
    for (const char *p = value ? value : ""; *p; p += utf8_width(p)) chars++;
    if (chars <= 20) return 57.0;
    double size = 1140.0 / chars; /* leave room for the moon on the right */
    return size < 30.0 ? 30.0 : size;
}

int print_wrapped_svg_text(FILE *fp, const char *value, double x, double y,
                           int max_chars, int max_lines, double font_size,
                           double line_height, const char *fill) {
    int lines = 0;
    const char *p = value ? value : "";
    while (*p && lines < max_lines) {
        const char *start = p, *end = p;
        int chars = 0;
        while (*end && *end != '\n' && chars < max_chars) {
            end += utf8_width(end);
            chars++;
        }
        const char *next = end;
        int truncated = *end && *end != '\n';
        if (*end == '\n') next++;
        while (end > start && isspace((unsigned char)end[-1])) end--;

        fprintf(fp, "<text x='%.1f' y='%.1f' font-size='%.1f' font-family='Inter,Roboto,Segoe UI,Helvetica,Arial,sans-serif' fill='%s'>",
                x, y + lines * line_height, font_size, fill);
        print_escaped_range(fp, start, end);
        if (truncated && lines == max_lines - 1) fputs("…", fp);
        fputs("</text>\n", fp);
        lines++;
        if (truncated && lines == max_lines) break;
        p = next;
    }
    return lines ? lines : 1;
}

static int wrapped_line_count(const char *value, int max_chars, int max_lines) {
    int lines = 0, chars = 0;
    const char *p = value ? value : "";
    do {
        if (*p == '\0' || *p == '\n' || chars == max_chars) {
            lines++;
            chars = 0;
            if (lines == max_lines || *p == '\0') break;
            if (*p == '\n') p++;
            continue;
        }
        p += utf8_width(p);
        chars++;
    } while (1);
    return lines;
}

int info_column_height(const char *title, const char *desc1, const char *desc2) {
    return wrapped_line_count(title, 23, 2) * 42
         + wrapped_line_count(desc1, 28, 3) * 40
         + wrapped_line_count(desc2, 28, 3) * 40;
}

void print_info_column(FILE *fp, double x, double y, const char *title,
                       const char *desc1, const char *desc2,
                       const char *title_fill, const char *desc_fill) {
    int title_lines = print_wrapped_svg_text(fp, title, x, y, 23, 2, 36, 42, title_fill);
    double desc_y = y + title_lines * 42;
    int first_lines = print_wrapped_svg_text(fp, desc1, x + 15, desc_y, 28, 3, 30, 40, desc_fill);
    print_wrapped_svg_text(fp, desc2, x + 15, desc_y + first_lines * 40, 28, 3, 30, 40, desc_fill);
}
