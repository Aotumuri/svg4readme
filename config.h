#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

// Getter APIs for profile.conf, environment variables, or defaults.
const char* get_username(void);

const char* get_first_title(void);
const char* get_first_desc1(void);
const char* get_first_desc2(void);

const char* get_second_title(void);
const char* get_second_desc1(void);
const char* get_second_desc2(void);

/* Emit escaped SVG text, wrapping long values without letting them escape a panel. */
int print_wrapped_svg_text(FILE *fp, const char *value, double x, double y,
                           int max_chars, int max_lines, double font_size,
                           double line_height, const char *fill);
int info_column_height(const char *title, const char *desc1, const char *desc2);
void print_info_column(FILE *fp, double x, double y, const char *title,
                       const char *desc1, const char *desc2,
                       const char *title_fill, const char *desc_fill);
void print_svg_escaped(FILE *fp, const char *value);
double brand_font_size(const char *value);

#endif
