//
// DBS-Praktikum Aufgabe 2
//    Header Funktionen zum Datenkopieren
//
// Christoph Dalitz
// Hochschule Niederrhein
// 2016/03/10
//

#ifndef copydata_H
#define copydata_H

#define MAX_QUERY_PARAMS 65535

#define INSERT_MODE_EACH 1
#define INSERT_MODE_MANY 2
#define INSERT_MODE_COPY 3
extern int INSERT_MODE;
extern bool INSERT_BINARY;

#include <libpq-fe.h>
#include <string.h>
#include <string>
#include <iostream>
#include "database.h"

using namespace std;

void showResults(DBr result);

// Kopieren der Daten aus dbfrom nach dbto
// rc: Anzahl kopierter Datensätze, -1 bei Fehler
int flightcopy(PGconn *dbfrom, PGconn *dbto);
int flightCopy(DB dbFrom, DB dbTo);

// Hinzufügen einer Fluggesellschaft
// ist die Flugesellschaft schon vorhanden, wird sie ncih hinzugefügt
// rc: -1 = Fehler, 0 = schon vorhanden, 1 = hinzugefügt
int addairline(PGconn *dbto, const string &code, const string &name);

void insertViaEachRow(DatabaseConnection dbTo, DatabaseResult result, std::string table, int nColumns, std::string insertColumns[], int &count);
void insertViaManyRows(DatabaseConnection dbTo, DatabaseResult result, std::string table, int nColumns, std::string insertColumns[], int &count);
void insertViaCopy(DatabaseConnection dbFrom, DatabaseConnection dbTo, std::string querySelect, std::string lastUpdateStr, std::string table, int nColumns, std::string insertColumns[], int &count);

std::string buildInsertQuery(std::string table, int nColumns, std::string columnFields[], int nRows);
std::string buildInsertQuery(std::string table, int nColumns, std::string columnFields[]);

int addAirline(DB db, const string &code, const string &name);
int addAirlines(DB dbFrom, DB dbTo);

#endif
