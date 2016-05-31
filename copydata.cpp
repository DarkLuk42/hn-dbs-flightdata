//
// DBS-Praktikum Aufgabe 2
//    Funktionen zum Datenkopieren
//
// Autoren:
//   Lukas Quast
//   Lukas Kropp
//

#include "copydata.h"
//#include "helper.h"

void show_results(PGresult *res) {
    int r, c;

    for (r = 0; r < PQntuples(res); r++) {
        for(c = 0; c < PQnfields(res); c++){
            char *ptr;

            ptr = PQgetvalue(res, r, c);

            printf("'%s', ", ptr);
        }

        printf("\n");
    }
}

// Kopieren der Daten aus dbfrom nach dbto
// rc: Anzahl kopierter Datensätze, -1 bei Fehler
int flightcopy(PGconn *connFrom, PGconn *connTo)
{
    const char* stmtName = "PREPARE_DATA_PLUS_INSERT";
    PGresult* resFrom = PQexec(connFrom, "SELECT * FROM flightdata WHERE latitude != 0 LIMIT 10");

    if (PQresultStatus(resFrom) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "SELECT failed: %s", PQerrorMessage(connFrom));
        PQclear(resFrom);
        return -1;
    }
    show_results(resFrom);
    PQclear(resFrom);
    return 0;
    PGresult* resTo = PQprepare(connTo, stmtName, "INSERT INTO flugdaten (id) VALUES ($1)", 1, NULL);
    if (PQresultStatus(resTo) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "Prepare INSERT failed: %s", PQerrorMessage(connTo));
        PQclear(resTo);
        return -1;
    }
    PQclear(resTo);

    int numColumns = PQnfields(resFrom);
    const char* paramValues[numColumns];
    int paramLengths[numColumns];

    for (int r = 0; r < PQntuples(resFrom); r++) {
        for(int c = 0; c < numColumns; c++){
            paramValues[c] = PQgetvalue(resFrom, r, c);
            paramLengths[c] = strlen(paramValues[c]);

            PGresult* resTo = PQexecPrepared(connTo,
                                             stmtName,
                                             numColumns,
                                             paramValues,
                                             paramLengths,
                                             NULL,
                                             0);
            if (PQresultStatus(resTo) != PGRES_COMMAND_OK)
            {
                fprintf(stderr, "Prepare INSERT failed: %s", PQerrorMessage(connTo));
                PQclear(resTo);
                return -1;
            }
            PQclear(resTo);
        }
    }
    show_results(resFrom);
    PQclear(resFrom);
    return 0;
}

// Hinzufügen einer Fluggesellschaft
// ist die Flugesellschaft schon vorhanden, wird sie nicht hinzugefügt
// rc: -1 = Fehler, 0 = schon vorhanden, 1 = hinzugefügt
int addairline(PGconn *dbto, const string &code, const string &name)
{
  return 0;
}
