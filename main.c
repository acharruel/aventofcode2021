#include <stdlib.h>
#include <stdio.h>

#include "oxygen.h"
#include "power.h"
#include "sonar.h"
#include "submarine.h"

struct context {
	struct oxygen		*ox;
	struct power		*pow;
	struct sonar		*sonar;
	struct submarine	*submarine;
};

static void context_destroy(struct context *ctx)
{
	if (!ctx)
		return;

	sonar_destroy(ctx->sonar);
	submarine_destroy(ctx->submarine);
	power_destroy(ctx->pow);
	oxygen_destroy(ctx->ox);
	free(ctx);
}

static struct context *context_new(const char *depth_input_filename,
				   const char *sub_input_filename,
				   const char *pow_input_filename)
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

	ctx->pow = power_new(pow_input_filename);
	if (!ctx->pow)
		goto exit;

	ctx->ox = oxygen_new(pow_input_filename);
	if (!ctx->ox)
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

	if (argc != 4) {
		fprintf(stderr, "Wrong arguments...\n");
		ret = EXIT_FAILURE;
		goto exit;
	}

	ctx = context_new(argv[1], argv[2], argv[3]);
	if (!ctx) {
		fprintf(stderr, "Failed to create context\n");
		ret = EXIT_FAILURE;
		goto exit;
	}

#if 0
	sonar_process(ctx->sonar);
	sonar_print(ctx->sonar);
#endif

#if 0
	submarine_proces(ctx->submarine);
	submarine_print(ctx->submarine);
#endif

#if 1
	power_process(ctx->pow);
	power_print(ctx->pow);
#endif

#if 1
	oxygen_process(ctx->ox);
	oxygen_print(ctx->ox);
#endif

exit:
	context_destroy(ctx);
	return ret;
}
