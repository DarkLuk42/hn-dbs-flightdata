//
// DBS-Praktikum Aufgabe 2
//    Funktionen zum Datenkopieren
//
// Autoren:
//   Lukas Quast
//   Lukas Kropp
//

#define LUKAS_SLOW false
#define LUKAS_BINARY true
#define MAX_QUERY_PARAMS = 65535

#include "copydata.h"

void showResults(DBr result) {
    int r, c;

    for (r = 0; r < result.nTuples(); r++) {
        for(c = 0; c < result.nFields(); c++){
            char *ptr;

            ptr = result.getValue(r, c);

            std::cout << ptr << " ";
        }

        std::cout << std::endl;
    }
}

// Kopieren der Daten aus dbfrom nach dbto
// rc: Anzahl kopierter Datensätze, -1 bei Fehler
int flightcopy(PGconn *connFrom, PGconn *connTo)
{
    clock_t begin = clock();
    const char *preparedInsertName = "PREPARE_DATA_PLUS_INSERT";
    DB dbFrom = connFrom;
    DB dbTo = connTo;

    try {
        const char *querySelect = "SELECT "
                "id, icao, altitude, "
                "latitude, longitude, heading, "
                "velocity, vert_rate, timestamp, "
                "submitter, flight, airlinecode"
                " FROM flightdata LIMIT 1000";
        DBr result = dbFrom.executeParams(querySelect, 0, NULL, NULL, NULL, LUKAS_BINARY ? 1 : 0);
        std::cout << "SELECT: " << double(clock() - begin) / CLOCKS_PER_SEC << "s" << std::endl;

        int numFields = result.nFields();
        int numRows = result.nTuples();
        int count = 0;

        if(LUKAS_SLOW) {
            const char *paramValues[numFields];
            int paramLengths[numFields];
            int paramFormats[numFields];
            Oid paramTypes[numFields];
            for (int c = 0; c < numFields; c++) {
                paramTypes[c] = result.getFieldType(c);
                paramFormats[c] = LUKAS_BINARY ? 1 : 0;
            }

            const char *queryInsert = "INSERT INTO flugdaten ("
                    "id, icao, flughoehe, "
                    "breite, laenge, richtung, "
                    "geschwindigkeit, vert_rate, zeit, "
                    "sender, flugnr, airlinecode"
                    ") VALUES ("
                    "$1, $2, $3, "
                    "$4, $5, $6, "
                    "$7, $8, $9, "
                    "$10, $11, $12)";
            DBr preparedInsert = dbTo.prepare(preparedInsertName, queryInsert, numFields, paramTypes);
            preparedInsert.clear();

            for (int r = 0; r < numRows; r++) {
                for (int c = 0; c < numFields; c++) {
                    if (!result.isNull(r, c)) {
                        paramValues[c] = result.getValue(r, c);
                        paramLengths[c] = result.getLength(r, c);
                    }
                    else {
                        paramValues[c] = NULL;
                        paramLengths[c] = 0;
                    }
                }

                DBr tmpResult = dbTo.executePrepared(preparedInsertName, numFields, paramValues, paramLengths,
                                                     paramFormats, 0);
                tmpResult.clear();
                count++;
            }
        }else {
            int numTotal = numRows * numFields;
            const char *paramValues[numTotal];
            int paramLengths[numTotal];
            int paramFormats[numTotal];
            Oid paramTypes[numTotal];

            std::stringstream allInsert;
            allInsert << "INSERT INTO flugdaten ("
                    "id, icao, flughoehe, "
                    "breite, laenge, richtung, "
                    "geschwindigkeit, vert_rate, zeit, "
                    "sender, flugnr, airlinecode"
                    ") VALUES ";
            for (int r = 0; r < numRows; r++) {
                allInsert << "(";
                for (int c = 0; c < numFields; c++) {
                    int i = r * numFields + c;
                    allInsert << "$" << i + 1;
                    if (c < numFields - 1) {
                        allInsert << ",";
                    }
                    paramValues[i] = result.getValue(r, c);
                    paramLengths[i] = result.getLength(r, c);
                    paramTypes[i] = result.getFieldType(c);
                    paramFormats[i] = LUKAS_BINARY ? 1 : 0;
                }
                allInsert << ")";
                if (r < numRows - 1) {
                    allInsert << ",";
                }
                count++;
            }

            std::string insertQuery = allInsert.str();

            DBr preparedInsert = dbTo.prepare(preparedInsertName, insertQuery.c_str(), numFields, paramTypes);
            preparedInsert.clear();

            DBr tmpResult = dbTo.executePrepared(preparedInsertName, numTotal, paramValues, paramLengths, paramFormats, 0);
            tmpResult.clear();
        }
        std::cout << "BINARY: " << (LUKAS_BINARY ? 1 : 0) << std::endl;
        std::cout << "SLOW: " << (LUKAS_SLOW ? 1 : 0) << std::endl;
        std::cout << "INSERT: " << double(clock() - begin) / CLOCKS_PER_SEC << "s" << std::endl;
        result.clear();
        return count;
    }catch (ResultException e)
    {
        std::cerr << e.message << e.status << std::endl;
        return -1;
    }
}

// Hinzufügen einer Fluggesellschaft
// ist die Flugesellschaft schon vorhanden, wird sie nicht hinzugefügt
// rc: -1 = Fehler, 0 = schon vorhanden, 1 = hinzugefügt
int addairline(PGconn *dbto, const string &code, const string &name)
{
    DB db = dbto;
    const char* paramValues[2];
    int paramLengths[2];
    paramValues[0] = code.c_str();
    paramValues[1] = name.c_str();
    paramLengths[0] = code.length();
    paramLengths[1] = name.length();

    try{
        DBr test = db.executeParams("SELECT count(1) FROM airline WHERE code=$1 AND name=$2", 2, paramValues, paramLengths);
        if(boost::lexical_cast<int>(test.getValue(0, 0)) == 1)
        {
            test.clear();
            return 0;
        }
        test.clear();
        db.executeParams("INSERT INTO airline (code, name) VALUES ($1, $2)", 2, paramValues, paramLengths);
        return 1;
    }catch (ResultException e)
    {
        std::cerr << e.message << std::endl;
        return -1;
    }
}
