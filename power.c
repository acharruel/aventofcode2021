#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "power.h"

struct power {
	FILE	*fd;

	int	wordlen;
	int	*ones;
	int	*zeros;

	int	gamma;
	int	epsilon;
	int	power;
};

struct power *power_new(const char *filename)
{
	struct power *pow = NULL;

	pow = calloc(1, sizeof(*pow));
	if (!pow)
		goto exit;

	pow->fd = fopen(filename, "r");
	if (!pow->fd) {
		fprintf(stderr, "Failed to open file '%s'\n", filename);
		goto exit;
	}

	pow->wordlen = -1;

	return pow;
exit:
	power_destroy(pow);
	return NULL;
}

void power_destroy(struct power *pow)
{
	if (!pow)
		return;


	free(pow->zeros);
	free(pow->ones);

	fclose(pow->fd);
	free(pow);
}

#define LINE_SIZE	32
#define NUM_SIZE	16
static int power_parse_line(struct power *pow)
{
	int ret = 0;
	char bufline[LINE_SIZE];
	char *s;
	char n[NUM_SIZE];
	int b;
	int i;

	s = fgets(bufline, sizeof(bufline), pow->fd);
	if (!s) {
		ret = -1;
		goto exit;
	}

	ret = sscanf(s, "%s", n);
	if (ret != 1) {
		ret = -1;
		goto exit;
	}

	if (pow->wordlen < 0) {
		pow->wordlen = strlen(n);
		fprintf(stdout, "Number length is %d\n", pow->wordlen);
	} else {
		if (pow->wordlen != strlen(n)) {
			fprintf(stdout,
				"Mismatching word length: %ld, expecting %d\n",
				strlen(n), pow->wordlen);
			ret = -1;
			goto exit;
		}
	}

	if (!pow->zeros)
		pow->zeros = calloc(pow->wordlen, sizeof(int));
	if (!pow->ones)
		pow->ones = calloc(pow->wordlen, sizeof(int));
	if (!pow->zeros) {
		fprintf(stderr, "Failed to allocate zeros array\n");
		ret = -1;
		goto exit;
	}
	if (!pow->ones) {
		fprintf(stderr, "Failed to allocate ones array\n");
		ret = -1;
		goto exit;
	}

	b = strtol(n, NULL, 2);

	for (i = 0; i < pow->wordlen; i++) {
		if (b & 0x1)
			pow->ones[i]++;
		else
			pow->zeros[i]++;
		b >>= 1;
	}

exit:
	return ret;
}

static void power_process_numbers(struct power *pow)
{
	int i;

	for (i = 0; i < pow->wordlen; i++) {
		pow->gamma |=
			pow->ones[pow->wordlen - 1 - i] > pow->zeros[pow->wordlen - 1 - i];
		pow->epsilon |=
			pow->ones[pow->wordlen - 1 - i] < pow->zeros[pow->wordlen - 1 - i];

		if (i != pow->wordlen - 1) {
			pow->gamma <<= 1;
			pow->epsilon <<= 1;
		}
	}

	pow->power = pow->gamma * pow->epsilon;
}

void power_process(struct power *pow)
{
	int ret;

	do {
		ret = power_parse_line(pow);
		if (ret < 0)
			continue;
	} while (!feof(pow->fd));

	power_process_numbers(pow);
}

void power_print(struct power *pow)
{
	fprintf(stdout, "-------------- Power --------------\n");
	fprintf(stdout, " gamma = %d\n", pow->gamma);
	fprintf(stdout, " epsilon = %d\n", pow->epsilon);
	fprintf(stdout, " power = %d\n", pow->power);
	fprintf(stdout, "-----------------------------------\n");
}
