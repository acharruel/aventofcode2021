#include <stdlib.h>
#include <stdio.h>

#include "sonar.h"

#define WINDOW_LEN	3

struct sonar {
	/* input file */
	FILE	*fd;

	/* measurements */
	int	mes[WINDOW_LEN];
	int	idx;
	int	sum_prev;

	/* results */
	int inc;
	int dec;
};

struct sonar *sonar_new(const char *filename)
{
	struct sonar *sonar = NULL;
	int i;

	sonar = calloc(1, sizeof(*sonar));
	if  (!sonar)
		goto exit;

	sonar->fd = fopen(filename, "r");
	if (!sonar->fd) {
		fprintf(stderr, "Failed to open file '%s'\n", filename);
		goto exit;
	}

	for (i = 0; i < WINDOW_LEN; i++)
		sonar->mes[i] = -1;

	sonar->sum_prev = -1;

	return sonar;
exit:
	sonar_destroy(sonar);
	return NULL;
}

void sonar_destroy(struct sonar *sonar)
{
	if (!sonar)
		return;

	fclose(sonar->fd);
	free(sonar);
}

#define LINE_SIZE	32
static int sonar_parse_depth(struct sonar *sonar)
{
	int depth = -1;
	int ret;
	char bufline[LINE_SIZE];
	char *s;

	s = fgets(bufline, sizeof(bufline), sonar->fd);
	if (!s)
		goto exit;

	ret = sscanf(s, "%d", &depth);
	if (ret != 1) {
		depth = -1;
		goto exit;
	}

exit:
	return depth;
}

static void sonar_process_depth(struct sonar *sonar, int depth)
{
	int sum = 0;
	int i;

	/* store new depth */
	sonar->mes[sonar->idx++] = depth;
	if (sonar->idx >= WINDOW_LEN)
		sonar->idx = 0;

	/* compute current sum */
	for (i = 0; i < WINDOW_LEN; i++) {
		if (sonar->mes[i] < 0) {
			fprintf(stdout,
				"Incomplete buffer, cannot process sum\n");
			return;
		}
		sum += sonar->mes[i];
	}

	if (sonar->sum_prev < 0) {
		fprintf(stdout,
			"sum = %d, N/A no previous sum\n", sum);
	} else if (sum > sonar->sum_prev) {
		fprintf(stdout, "sum = %d, increased\n", sum);
		sonar->inc++;
	} else if (sum < sonar->sum_prev) {
		fprintf(stdout, "sum = %d, decreased\n", sum);
		sonar->dec++;
	} else {
		fprintf(stdout, "sum = %d, no change\n", sum);
	}

	sonar->sum_prev = sum;
}

void sonar_process(struct sonar *sonar)
{
	int depth;

	do {
		depth = sonar_parse_depth(sonar);
		if (depth < 0)
			continue;
		sonar_process_depth(sonar, depth);
	} while (!feof(sonar->fd));
}

void sonar_print(struct sonar *sonar)
{
	fprintf(stdout, "------------ Sonar ------------\n");
	fprintf(stdout, "increased = %d\n", sonar->inc);
	fprintf(stdout, "decreased = %d\n", sonar->dec);
	fprintf(stdout, "-----------------------------------\n");
}
