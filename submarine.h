#ifndef _SUBMARINE_H_
#define _SUBMARINE_H_

enum sub_cmd_desc {
	FORWARD = 1,
	DOWN,
	UP,
};

struct sub_cmd {
	enum sub_cmd_desc	cmd;
	int			val;
};

struct submarine;

struct submarine *submarine_new(const char *filename);
void submarine_destroy(struct submarine *sub);

void submarine_proces(struct submarine *sub);
void submarine_print(struct submarine *sub);

#endif
