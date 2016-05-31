//
// Created by lukas on 5/31/16.
//

#ifndef IMPORTFLIGHTS_HELPER_H
#define IMPORTFLIGHTS_HELPER_H

#endif //IMPORTFLIGHTS_HELPER_H

PGresult *PQexec(PGconn *conn, const char *command);
PGresult *PQexecParams(PGconn *conn,
                       const char *command,
                       int nParams,
                       const Oid *paramTypes,
                       const char * const *paramValues,
                       const int *paramLengths,
                       const int *paramFormats,
                       int resultFormat);
PGresult *PQprepare(PGconn *conn,
                    const char *stmtName,
                    const char *query,
                    int nParams,
                    const Oid *paramTypes);
PGresult *PQexecPrepared(PGconn *conn,
                         const char *stmtName,
                         int nParams,
                         const char * const *paramValues,
                         const int *paramLengths,
                         const int *paramFormats,
                         int resultFormat);
PGresult *PQdescribePrepared(PGconn *conn, const char *stmtName);
PGresult *PQdescribePortal(PGconn *conn, const char *portalName);
ExecStatusType PQresultStatus(const PGresult *res);
char *PQresStatus(ExecStatusType status);
char *PQresultErrorMessage(const PGresult *res);
char *PQresultErrorField(const PGresult *res, int fieldcode);
void PQclear(PGresult *res);
int PQntuples(const PGresult *res);
int PQnfields(const PGresult *res);
char *PQfname(const PGresult *res,
              int column_number);
int PQfnumber(const PGresult *res,
              const char *column_name);
Oid PQftable(const PGresult *res,
             int column_number);
int PQftablecol(const PGresult *res,
                int column_number);
int PQfformat(const PGresult *res,
              int column_number);
Oid PQftype(const PGresult *res,
            int column_number);
int PQfmod(const PGresult *res,
           int column_number);
int PQfsize(const PGresult *res,
            int column_number);
int PQbinaryTuples(const PGresult *res);
char *PQgetvalue(const PGresult *res,
                 int row_number,
                 int column_number);
int PQgetisnull(const PGresult *res,
                int row_number,
                int column_number);
int PQgetlength(const PGresult *res,
                int row_number,
                int column_number);
int PQnparams(const PGresult *res);
Oid PQparamtype(const PGresult *res, int param_number);
void PQprint(FILE *fout,      /* output stream */
             const PGresult *res,
             const PQprintOpt *po);
typedef struct
{
    pqbool  header;      /* print output field headings and row count */
    pqbool  align;       /* fill align the fields */
    pqbool  standard;    /* old brain dead format */
    pqbool  html3;       /* output HTML tables */
    pqbool  expanded;    /* expand tables */
    pqbool  pager;       /* use pager for output if needed */
    char    *fieldSep;   /* field separator */
    char    *tableOpt;   /* attributes for HTML table element */
    char    *caption;    /* HTML table caption */
    char    **fieldName; /* null-terminated array of replacement field names */
} PQprintOpt;
char *PQcmdStatus(PGresult *res);
char *PQcmdTuples(PGresult *res);
Oid PQoidValue(const PGresult *res);
char *PQoidStatus(const PGresult *res);
char *PQescapeLiteral(PGconn *conn, const char *str, size_t length);
char *PQescapeIdentifier(PGconn *conn, const char *str, size_t length);
size_t PQescapeStringConn(PGconn *conn,
                          char *to, const char *from, size_t length,
                          int *error);
size_t PQescapeString (char *to, const char *from, size_t length);
unsigned char *PQescapeByteaConn(PGconn *conn,
                                 const unsigned char *from,
                                 size_t from_length,
                                 size_t *to_length);
unsigned char *PQescapeBytea(const unsigned char *from,
                             size_t from_length,
                             size_t *to_length);
unsigned char *PQunescapeBytea(const unsigned char *from, size_t *to_length);