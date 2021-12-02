#ifndef _SONAR_H_
#define _SONAR_H_

struct sonar;

struct sonar *sonar_new(const char *filename);
void sonar_destroy(struct sonar *sonar);

void sonar_process(struct sonar *sonar);
void sonar_print(struct sonar *sonar);

#endif
