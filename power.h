#ifndef _POWER_H_
#define _POWER_H_

struct power;

struct power *power_new(const char *filename);
void power_destroy(struct power *pow);

void power_process(struct power *pow);
void power_print(struct power *pow);

void power_ox_process(struct power *pow);

#endif
