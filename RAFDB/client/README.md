# RAFDB client boundary

Public C API: `../banco/rafdb.h`; bounded SQL-like gateway: `../gateway/rafdb_sql.h`.

No MySQL binary protocol, TCP server, authentication, wire-compatible mysql client library or prepared statements exist yet. Do **not** advertise this as a drop-in MySQL replacement. Client adapters are a future hosted boundary. No secrets in manifests.
