#ifndef STORAGE_H
#define STORAGE_H
#include <stdbool.h>

void storage_init(void);
bool storage_check_uid(const char *uid_str, char *out_name);
int  storage_user_count(void);
bool storage_user_get(int index, char *out_name, char *out_uid);
bool storage_add_user(const char *uid_str, const char *name);
bool storage_remove_uid(const char *uid_str);

#endif /* STORAGE_H */
