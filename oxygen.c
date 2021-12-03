#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "oxygen.h"

struct oxygen {
	FILE	*fd;

	int	nb_entries;
	int	*ox_entries;
	int	*co2_entries;
	int	wordlen;

	int	ox;
	int	co2;
	int	life_rating;
};

struct oxygen *oxygen_new(const char *filename)
{
	struct oxygen *ox = NULL;

	ox = calloc(1, sizeof(*ox));
	if (!ox)
		goto exit;

	ox->fd = fopen(filename, "r");
	if (!ox->fd) {
		fprintf(stderr, "Failed to open file '%s'\n", filename);
		goto exit;
	}

	ox->wordlen = -1;

	return ox;
exit:
	oxygen_destroy(ox);
	return NULL;
}

void oxygen_destroy(struct oxygen *ox)
{
	if (!ox)
		return;

	free(ox->ox_entries);
	fclose(ox->fd);
	free(ox);
}

#if 0
static void oxygen_print_ox_entries(struct oxygen *ox)
{
	int i, j;

	printf(" >>> --------------------------- \n");
	for (i = 0; i < ox->nb_entries; i++) {
		if (ox->ox_entries[i] < 0)
			continue;
		printf(" >>> ");
		for (j = ox->wordlen - 1; j >= 0; j--)
			printf(" %d ", (ox->ox_entries[i] >> j) & 0x1);
		printf("\n");
	}
	printf(" >>> --------------------------- \n");
}
#endif

#if 0
static void oxygen_print_co2_entries(struct oxygen *ox)
{
	int i, j;

	printf(" >>> --------------------------- \n");
	for (i = 0; i < ox->nb_entries; i++) {
		if (ox->co2_entries[i] < 0)
			continue;
		printf(" >>> ");
		for (j = ox->wordlen - 1; j >= 0; j--)
			printf(" %d ", (ox->co2_entries[i] >> j) & 0x1);
		printf("\n");
	}
	printf(" >>> --------------------------- \n");
}
#endif

static int oxygen_count_valid_entries(int *table, int size)
{
	int count = 0, i;

	for (i = 0; i < size; i++) {
		if (table[i] < 0)
			continue;
		count++;
	}
	return count;
}

#define LINE_SIZE	32
#define NUM_SIZE	16
static int oxygen_count_lines(struct oxygen *ox)
{
	int nblines = 0;
	char bufline[LINE_SIZE];

	do {
		(void)fgets(bufline, sizeof(bufline), ox->fd);
		nblines++;
	} while (!feof(ox->fd));

	rewind(ox->fd);

	return nblines - 1;
}

static int oxygen_fill_entries(struct oxygen *ox)
{
	int ret = 0;
	char bufline[LINE_SIZE];
	char *s;
	char n[NUM_SIZE];
	int idx = 0;

	do {
		s = fgets(bufline, sizeof(bufline), ox->fd);
		if (!s) {
			ret = -1;
			goto exit;
		}

		ret = sscanf(s, "%s", n);
		if (ret != 1)
			continue;

		if (ox->wordlen < 0) {
			ox->wordlen = strlen(n);
			fprintf(stdout, "Number length is %d\n", ox->wordlen);
		} else {
			if (ox->wordlen != strlen(n)) {
				fprintf(stdout,
					"Mismatching word length: %ld, expecting %d\n",
					strlen(n), ox->wordlen);
				ret = -1;
				goto exit;
			}
		}

		ox->ox_entries[idx] = strtol(n, NULL, 2);
		ox->co2_entries[idx] = strtol(n, NULL, 2);
		idx++;
	} while (!feof(ox->fd) && idx < ox->nb_entries);

exit:
	return ret;
}

static void oxygen_filter_ox_entries(struct oxygen *ox, int bitpos)
{
	int ones = 0, zeros = 0;
	int i;
	bool ref;

	/* count ones and zeros */
	for (i = 0; i < ox->nb_entries; i++) {
		if (ox->ox_entries[i] < 0)
			continue;
		ox->ox_entries[i] & (1 << bitpos) ? ones++ : zeros++;
	}

	ref = ones >= zeros;

	if (oxygen_count_valid_entries(ox->ox_entries, ox->nb_entries) <= 1)
		return;

	/* filter ox_entries */
	for (i = 0; i < ox->nb_entries; i++) {
		if (ox->ox_entries[i] < 0)
			continue;
		if ((ref << bitpos) != (ox->ox_entries[i] & (1 << bitpos)))
			/* invalidate entry */
			ox->ox_entries[i] = -1;
	}
}

static void oxygen_filter_co2_entries(struct oxygen *ox, int bitpos)
{
	int ones = 0, zeros = 0;
	int i;
	bool ref;

	/* count ones and zeros */
	for (i = 0; i < ox->nb_entries; i++) {
		if (ox->co2_entries[i] < 0)
			continue;
		ox->co2_entries[i] & (1 << bitpos) ? ones++ : zeros++;
	}

	ref = ones < zeros;

	if (oxygen_count_valid_entries(ox->co2_entries, ox->nb_entries) <= 1)
		return;

	/* filter co2_entries */
	for (i = 0; i < ox->nb_entries; i++) {
		if (ox->co2_entries[i] < 0)
			continue;
		if ((ref << bitpos) != (ox->co2_entries[i] & (1 << bitpos)))
			/* invalidate entry */
			ox->co2_entries[i] = -1;
	}
}

static int oxygen_extract_oxygen(struct oxygen *ox)
{
	int i;

	for (i = 0; i < ox->nb_entries; i++) {
		if (ox->ox_entries[i] < 0)
			continue;
		return ox->ox_entries[i];
	}

	fprintf(stderr, "Error: no entry found\n");
	return -1;
}

static int oxygen_extract_co2(struct oxygen *ox)
{
	int i;

	for (i = 0; i < ox->nb_entries; i++) {
		if (ox->co2_entries[i] < 0)
			continue;
		return ox->co2_entries[i];
	}

	fprintf(stderr, "Error: no entry found\n");
	return -1;
}

void oxygen_process(struct oxygen *ox)
{
	int ret;
	int i = 0;

	ox->nb_entries = oxygen_count_lines(ox);

	ox->ox_entries = calloc(ox->nb_entries, sizeof(int));
	if (!ox->ox_entries) {
		fprintf(stderr, "Failed to allocate entries table\n");
		return;
	}

	ox->co2_entries = calloc(ox->nb_entries, sizeof(int));
	if (!ox->co2_entries) {
		fprintf(stderr, "Failed to allocate entries table\n");
		return;
	}

	ret = oxygen_fill_entries(ox);
	if (ret < 0)
		return;

	for (i = 0; i < ox->wordlen; i++) {
		/*
		 * convert index i to bit position beginning with MSB first
		 */
		oxygen_filter_ox_entries(ox, ox->wordlen - 1 - i);
		oxygen_filter_co2_entries(ox, ox->wordlen - 1 - i);
	}

	ox->ox = oxygen_extract_oxygen(ox);
	ox->co2 = oxygen_extract_co2(ox);

	ox->life_rating = ox->ox * ox->co2;
}

void oxygen_print(struct oxygen *ox)
{
	fprintf(stdout, "------------- Oxygen -------------\n");
	fprintf(stdout, " oxygen = %d\n", ox->ox);
	fprintf(stdout, " co2 = %d\n", ox->co2);
	fprintf(stdout, " life support rating = %d\n", ox->life_rating);
	fprintf(stdout, "-----------------------------------\n");
}
