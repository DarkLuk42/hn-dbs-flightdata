//
// DBS-Praktikum Aufgabe 2
//    Funktionen zum Datenkopieren
//
// Autoren:
//   Student1
//   Student2
//

#include "copydata.h"


// Kopieren der Daten aus dbfrom nach dbto
// rc: Anzahl kopierter Datensätze, -1 bei Fehler
int flightcopy(PGconn *dbfrom, PGconn *dbto)
{
  return 0;
}

// Hinzufügen einer Fluggesellschaft
// ist die Flugesellschaft schon vorhanden, wird sie nicht hinzugefügt
// rc: -1 = Fehler, 0 = schon vorhanden, 1 = hinzugefügt
int addairline(PGconn *dbto, const string &code, const string &name)
{
  return 0;
}
