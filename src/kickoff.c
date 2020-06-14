/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * (c) 2020- MonetDB Solutions B.V.
 */

#include "monetdbe.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#define error(msg) {fprintf(stderr, "Failure: %s\n", msg); return -1;}

int
main(void)
{
	char* err = NULL;
	monetdbe_connection conn = NULL;
	monetdbe_result* result = NULL;

	// first argument is a string for the db directory or NULL for in-memory mode
	if ((err = monetdbe_startup(NULL, 0)) != NULL)
		error(err)
	if ((err = monetdbe_connect(&conn)) != NULL)
		error(err)
	if ((err = monetdbe_query(conn, "CREATE TABLE test (x integer, y string)", NULL, NULL)) != NULL)
		error(err)
	if ((err = monetdbe_query(conn, "INSERT INTO test VALUES (42, 'Hello'), (NULL, 'World')", NULL, NULL)) != NULL)
		error(err)
	if ((err = monetdbe_query(conn, "SELECT x, y FROM test; ", &result, NULL)) != NULL)
		error(err)

	fprintf(stdout, "Query result with %zu cols and %"PRId64" rows\n", result->ncols, result->nrows);
	for (int64_t r = 0; r < result->nrows; r++) {
		for (size_t c = 0; c < result->ncols; c++) {
			monetdbe_column* rcol;
			if ((err = monetdbe_result_fetch(conn, result, &rcol, c)) != NULL)
				error(err)
			switch (rcol->type) {
				case monetdbe_int32_t: {
					monetdbe_column_int32_t * col = (monetdbe_column_int32_t *) rcol;
					if (col->data[r] == col->null_value) {
						printf("NULL");
					} else {
						printf("%d", col->data[r]);
					}
					break;
				}
				case monetdbe_str: {
					monetdbe_column_str * col = (monetdbe_column_str *) rcol;
					if (col->is_null(col->data[r])) {
						printf("NULL");
					} else {
						printf("%s", (char*) col->data[r]);
					}
					break;
				}
				default: {
					printf("UNKNOWN");
				}
			}

			if (c + 1 < result->ncols) {
				printf(", ");
			}
		}
		printf("\n");
	}

	if ((err = monetdbe_cleanup_result(conn, result)) != NULL)
		error(err)
	if ((err = monetdbe_disconnect(conn)) != NULL)
		error(err)
	if ((err = monetdbe_shutdown()) != NULL)
		error(err)
	return 0;
}
