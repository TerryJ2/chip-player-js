/* Extended Module Player
 * Copyright (C) 1996-2012 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU General Public License. See doc/COPYING
 * for more information.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "driver.h"
#include "convert.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define DATA(x) (((struct data *)drv_file.data)->x)

struct data {
	int fd;
	int endian;
};

static int init(struct context_data *);
static void bufdump(struct context_data *, void *, int);
static void shutdown(struct context_data *);

static void dummy()
{
}

static char *help[] = {
	"big-endian", "Generate big-endian 16-bit samples",
	"little-endian", "Generate little-endian 16-bit samples",
	NULL
};

struct xmp_drv_info drv_file = {
	"file",			/* driver ID */
	"file",			/* driver description */
	help,			/* help */
	init,			/* init */
	shutdown,		/* shutdown */
	dummy,			/* starttimer */
	dummy,			/* stoptimer */
	bufdump,		/* bufdump */
};

static int init(struct context_data *ctx)
{
	struct xmp_options *o = &ctx->o;
	char *buf;
	int bsize;
	char *token, **parm;

	drv_file.data = malloc(sizeof (struct data));
	if (drv_file.data == NULL)
		return -1;

	DATA(endian) = 0;
	parm_init();
	chkparm0("big-endian", DATA(endian) = 1);
	chkparm0("little-endian", DATA(endian) = -1);
	parm_end();

	if (!o->outfile)
		o->outfile = "xmp.out";

	if (strcmp(o->outfile, "-")) {
		DATA(fd) = open(o->outfile, O_WRONLY | O_CREAT | O_TRUNC
							| O_BINARY, 0644);
		if (DATA(fd) < 0)
			return -1;
	} else {
		DATA(fd) = 1;
	}

	if (strcmp(o->outfile, "-")) {
		bsize = strlen(drv_file.description) + strlen(o->outfile) + 8;
		buf = malloc(bsize);
		snprintf(buf, bsize, "%s: %s", drv_file.description,
							 o->outfile);
		drv_file.description = buf;
	} else {
		drv_file.description = strdup("Output to stdout");
	}

	return 0;
}

static void bufdump(struct context_data *ctx, void *b, int i)
{
	struct xmp_options *o = &ctx->o;
	int j;

	if ((o->big_endian && DATA(endian) == -1) ||
				(!o->big_endian && DATA(endian) == 1)) {
		xmp_cvt_sex(i, b);
	}

	while (i) {
		if ((j = write(DATA(fd), b, i)) > 0) {
			i -= j;
			b = (char *)b + j;
		} else
			break;
	}
}

static void shutdown(struct context_data *ctx)
{
	if (DATA(fd) > 0)
		close(DATA(fd));

	free(drv_file.description);
	free(drv_file.data);
}
