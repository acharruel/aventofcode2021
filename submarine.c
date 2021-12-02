#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "submarine.h"

struct submarine {
	FILE	*fd;

	int	hpos;
	int	depth;
	int	aim;
};

struct submarine *submarine_new(const char *filename)
{
	struct submarine *sub = NULL;

	sub = calloc(1, sizeof(*sub));
	if (!sub)
		goto exit;

	sub->fd = fopen(filename, "r");
	if (!sub->fd) {
		fprintf(stderr, "Failed to open file '%s'\n", filename);
		goto exit;
	}

	return sub;
exit:
	submarine_destroy(sub);
	return NULL;
}

void submarine_destroy(struct submarine *sub)
{
	if (!sub)
		return;

	fclose(sub->fd);
	free(sub);
}

#define LINE_SIZE	64
#define CMD_SIZE	16
static int submarine_parse_command(struct submarine *sub, struct sub_cmd *command)
{
	char cmd[CMD_SIZE];
	int val;
	int ret = 0;
	char bufline[LINE_SIZE];
	char *s;

	s = fgets(bufline, sizeof(bufline), sub->fd);
	if (!s) {
		ret = -1;
		goto exit;
	}

	ret = sscanf(s, "%s %d", cmd, &val);
	if (ret != 2) {
		ret = -1;
		goto exit;
	}

	ret = 0;

	if (!strcmp(cmd, "forward")) {
		command->cmd = FORWARD;
	} else if (!strcmp(cmd, "down")) {
		command->cmd = DOWN;
	} else if (!strcmp(cmd, "up")) {
		command->cmd = UP;
	} else {
		fprintf(stderr, "Unrecognized command: '%s'\n", cmd);
		ret = -1;
		goto exit;
	}

	command->val = val;

exit:
	return ret;
}

static void submarine_process_command(struct submarine *sub, struct sub_cmd *cmd)
{
	switch (cmd->cmd) {

	case FORWARD:
		sub->hpos += cmd->val;
		sub->depth += cmd->val * sub->aim;
		break;

	case DOWN:
		sub->aim += cmd->val;
		break;

	case UP:
		sub->aim -= cmd->val;
		break;

	default:
		fprintf(stderr, "Unsupported command: %d\n", cmd->cmd);
		return;
	}
}

void submarine_proces(struct submarine *sub)
{
	struct sub_cmd cmd;
	int ret;

	do {
		ret = submarine_parse_command(sub, &cmd);
		if (ret < 0)
			continue;
		submarine_process_command(sub, &cmd);
	} while (!feof(sub->fd));
}

void submarine_print(struct submarine *sub)
{
	fprintf(stdout, "------------ Submarine ------------\n");
	fprintf(stdout, " sub hpos = %d\n", sub->hpos);
	fprintf(stdout, " sub depth = %d\n", sub->depth);
	fprintf(stdout, " sub aim = %d\n", sub->aim);
	fprintf(stdout, " sub mult = %d\n", sub->depth * sub->hpos);
	fprintf(stdout, "-----------------------------------\n");
}
