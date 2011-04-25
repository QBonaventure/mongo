/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2008-2011 WiredTiger, Inc.
 *	All rights reserved.
 */

#include "wt_internal.h"
#include "util.h"

const char *progname;

int	usage(void);

int
main(int argc, char *argv[])
{
	WT_CONNECTION *conn;
	WT_SESSION *session;
	WT_CURSOR *cursor;
	WT_ITEM key, value;
	const char *home, *srcname;
	char cursor_config[100], datasrc[100];
	int ch, debug, ret, tret, verbose;

	WT_UTILITY_INTRO(progname, argv);

	conn = NULL;
	home = NULL;
	debug = verbose = 0;

	while ((ch = getopt(argc, argv, "dh:Vv")) != EOF)
		switch (ch) {
		case 'd':
			debug = 1;
			break;
		case 'h':			/* home directory */
			home = optarg;
			break;
		case 'V':			/* version */
			printf("%s\n", wiredtiger_version(NULL, NULL, NULL));
			return (EXIT_SUCCESS);
		case 'v':			/* version */
			verbose = 1;
			break;
		case '?':
		default:
			return (usage());
		}
	argc -= optind;
	argv += optind;

	/* The remaining argument is the file name. */
	if (argc != 1)
		return (usage());
	srcname = *argv;

	if ((ret = wiredtiger_open(home, verbose ?
	    __wt_event_handler_verbose : NULL, NULL, &conn)) != 0 ||
	    (ret = conn->open_session(conn, NULL, NULL, &session)) != 0)
		goto err;

	snprintf(datasrc, sizeof(datasrc), "stat:%s", srcname);
	snprintf(cursor_config, sizeof(cursor_config), "dump=print%s",
	    debug ? ",debug" : "");


	if ((ret = session->open_cursor(session, datasrc, NULL,
	    cursor_config, &cursor)) != 0) {
		fprintf(stderr, "%s: cursor open(%s) failed: %s\n",
		    progname, datasrc, wiredtiger_strerror(ret));
		goto err;
	}

	while ((ret = cursor->next(cursor)) == 0) {
		cursor->get_key(cursor, &key);
		fwrite(key.data, key.size, 1, stdout);
		fwrite("\n", 1, 1, stdout);
		cursor->get_value(cursor, &value);
		fwrite(value.data, value.size, 1, stdout);
		fwrite("\n", 1, 1, stdout);
	}

	if (ret != WT_NOTFOUND) {
		fprintf(stderr, "%s: cursor get(%s) failed: %s\n",
		    progname, datasrc, wiredtiger_strerror(ret));
		goto err;
	}

	if (0) {
err:		ret = 1;
	}
	if (conn != NULL && (tret = conn->close(conn, NULL)) != 0 && ret == 0)
		ret = tret;
	return (ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

int
usage(void)
{
	(void)fprintf(stderr, "usage: %s [-V] file\n", progname);
	return (EXIT_FAILURE);
}
