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

#include <libpq-fe.h>
#include <string>

using namespace std;

// Kopieren der Daten aus dbfrom nach dbto
// rc: Anzahl kopierter Datensätze, -1 bei Fehler
int flightcopy(PGconn *dbfrom, PGconn *dbto);

// Hinzufügen einer Fluggesellschaft
// ist die Flugesellschaft schon vorhanden, wird sie ncih hinzugefügt
// rc: -1 = Fehler, 0 = schon vorhanden, 1 = hinzugefügt
int addairline(PGconn *dbto, const string &code, const string &name);

#endif
