#include <stdlib.h>
#include <stdio.h>

#include "sonar.h"
#include "submarine.h"

struct context {
	struct sonar		*sonar;
	struct submarine	*submarine;
};

static void context_destroy(struct context *ctx)
{
	if (!ctx)
		return;

	sonar_destroy(ctx->sonar);
	submarine_destroy(ctx->submarine);
	free(ctx);
}

static struct context *context_new(const char *depth_input_filename,
				   const char *sub_input_filename)
{
	struct context *ctx = NULL;

	ctx = calloc(1, sizeof(*ctx));
	if (!ctx)
		goto exit;

	ctx->sonar = sonar_new(depth_input_filename);
	if (!ctx->sonar)
		goto exit;

	ctx->submarine = submarine_new(sub_input_filename);
	if (!ctx->submarine)
		goto exit;

	return ctx;
exit:
	context_destroy(ctx);
	return NULL;
}

int main(int argc, char *argv[])
{
	int ret = EXIT_SUCCESS;
	struct context *ctx = NULL;

	if (argc != 3) {
		fprintf(stderr, "Wrong arguments...\n");
		ret = EXIT_FAILURE;
		goto exit;
	}

	ctx = context_new(argv[1], argv[2]);
	if (!ctx) {
		fprintf(stderr, "Failed to create context\n");
		ret = EXIT_FAILURE;
		goto exit;
	}

	sonar_process(ctx->sonar);
	sonar_print(ctx->sonar);

	submarine_proces(ctx->submarine);
	submarine_print(ctx->submarine);

exit:
	context_destroy(ctx);
	return ret;
}
