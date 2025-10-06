#ifndef CONFIG_H
#define CONFIG_H

// Getter APIs for configurable texts (env-backed or defaults)
const char* get_username(void);

const char* get_first_title(void);
const char* get_first_desc1(void);
const char* get_first_desc2(void);

const char* get_second_title(void);
const char* get_second_desc1(void);
const char* get_second_desc2(void);

#endif