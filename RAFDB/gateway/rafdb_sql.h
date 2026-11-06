#ifndef RAFDB_SQL_H
#define RAFDB_SQL_H
#include "../banco/rafdb.h"
/* Exact, bounded SQL-like grammar ONLY; NOT MySQL protocol compatibility.
 * INSERT INTO rafdb VALUES ('k','v');
 * UPDATE rafdb SET value='v' WHERE key='k';
 * SELECT value FROM rafdb WHERE key='k';
 * DELETE FROM rafdb WHERE key='k';
 * No escaping, transactions, schemas, sockets, authentication or SQL parser.
 */
int rf_sql(struct rf_db *db, const char *sql, unsigned size,
           rf_u8 *result, unsigned *inout_size);
#endif
