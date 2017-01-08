#ifndef DT_INI_H
#define DT_INI_H

int dt_ini_open(char *file);
int dt_ini_get_entry(char *section, char *key, char *val);
int dt_ini_release();

#endif
