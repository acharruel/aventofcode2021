#ifndef _OXYGEN_H_
#define _OXYGEN_H_

struct oxygen;

struct oxygen *oxygen_new(const char *filename);
void oxygen_destroy(struct oxygen *ox);

void oxygen_process(struct oxygen *ox);
void oxygen_print(struct oxygen *ox);

#endif
