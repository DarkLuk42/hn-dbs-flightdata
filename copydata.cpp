//
// DBS-Praktikum Aufgabe 2
//    Funktionen zum Datenkopieren
//
// Autoren:
//   Lukas Quast
//   Lukas Kropp
//

#include "copydata.h"
int INSERT_MODE = INSERT_MODE_MANY;
bool INSERT_BINARY = true;

void showResults(DBr result) {
    int r, c;

    std::cout << std::endl;

    std::cout << result.nTuples() << " rows, " << result.nFields() << " fields" << std::endl;

    for(c = 0; c < result.nFields(); c++){
        std::cout << result.fieldName(c) << " ";
    }
    std::cout << std::endl;

    for (r = 0; r < result.nTuples(); r++) {
        for(c = 0; c < result.nFields(); c++){
            if(result.isNull(r, c))
            {
                std::cout << "null" << " ";
            }
            else
            {
                std::cout << "'" << result.getValue(r, c) << "'" << " ";
            }
        }

        std::cout << std::endl;
    }
}
int flightcopy(PGconn *connFrom, PGconn *connTo){
    return flightCopy(DB(connFrom), DB(connTo));
}
int flightCopy(DB dbFrom, DB dbTo)
{
    try {
        DBr lastUpdate = dbTo.execute("SELECT max(zeit) FROM flugdaten");
        std::string querySelect;
        std::string lastUpdateStr = "";
        DBr result;
        if(lastUpdate.nTuples() > 0 && !lastUpdate.isNull(0, 0))
        {
            querySelect = "SELECT "
                    "id, icao, altitude, "
                    "latitude, longitude, heading, "
                    "velocity, vert_rate, timestamp, "
                    "submitter, flight, airlinecode"
                    " FROM flightdata WHERE timestamp > $1";
            const char * paramValues[1];
            lastUpdateStr = lastUpdate.getValue(0, 0);
            paramValues[0] = lastUpdateStr.c_str();
            const int *paramLengths = NULL;
            const int *paramFormats = NULL;
            const Oid *paramTypes = NULL;
            result = dbFrom.executeParams(querySelect.c_str(), 1, paramValues, paramLengths, paramFormats, paramTypes, INSERT_BINARY ? 1 : 0);
        }
        else
        {
            querySelect = "SELECT "
                    "id, icao, altitude, "
                    "latitude, longitude, heading, "
                    "velocity, vert_rate, timestamp, "
                    "submitter, flight, airlinecode"
                    " FROM flightdata";
            result = dbFrom.executeParams(querySelect.c_str(), 0, NULL, NULL, NULL, NULL, INSERT_BINARY ? 1 : 0);
        }

        std::string table = "flugdaten";
        int count = 0;
        std::string insertColumns[] = {
                "id",
                "icao",
                "flughoehe",
                "breite",
                "laenge",
                "richtung",
                "geschwindigkeit",
                "vert_rate",
                "zeit",
                "sender",
                "flugnr",
                "airlinecode"
        };

        std::cout << "insert-mode: " <<
        (INSERT_MODE == INSERT_MODE_EACH ? "each" :
         (INSERT_MODE == INSERT_MODE_MANY ? "many" : "copy")) <<
        " " << (INSERT_BINARY ? "binary" : "string") << std::endl;

        switch(INSERT_MODE)
        {
            default:
            case INSERT_MODE_EACH:
                insertViaEachRow(dbTo, result, table, 12, insertColumns, count);
                break;
            case INSERT_MODE_MANY:
                insertViaManyRows(dbTo, result, table, 12, insertColumns, count);
                break;
            case INSERT_MODE_COPY:
                insertViaCopy(dbFrom, dbTo, querySelect, lastUpdateStr, table, 12, insertColumns, count);
                break;
        }
        return count;
    }catch (ResultException e)
    {
        std::cerr << e.message << e.status << std::endl;
        return -1;
    }
}

void insertViaManyRows(DatabaseConnection dbTo, DatabaseResult result, std::string table, int nColumns, std::string insertColumns[], int &count){
    int numRows = result.nTuples();
    int numFields = result.nFields();

    int numTotal = numRows * numFields;
    const char *paramValues[numTotal];
    int paramLengths[numTotal];
    int paramFormats[numTotal];
    Oid paramTypes[numTotal];

    int chunkMaxRows = MAX_QUERY_PARAMS / numFields;
    for(int chunkRow = 0; chunkRow < numRows; chunkRow += chunkMaxRows) {
        int numCurrentRows = numRows % chunkMaxRows;
        int numCurrentTotal = numCurrentRows * numFields;
        for (int r = 0; r < numCurrentRows; r++) {
            for (int c = 0; c < numFields; c++) {
                int i = r * numFields + c;
                paramValues[i] = result.getValue(chunkRow+r, c);
                paramLengths[i] = result.getLength(chunkRow+r, c);
                paramTypes[i] = result.getFieldType(c);
                paramFormats[i] = INSERT_BINARY ? 1 : 0;
            }
            ++count;
        }

        std::string insertQuery = buildInsertQuery(std::string("flugdaten"), 12, insertColumns, numCurrentRows);

        dbTo.executeParams(insertQuery.c_str(), numCurrentTotal, paramValues, paramLengths, paramFormats, paramTypes, 0);
    }
}

void insertViaEachRow(DatabaseConnection dbTo, DatabaseResult result, std::string table, int nColumns, std::string insertColumns[], int &count){
    int numRows = result.nTuples();
    int numFields = result.nFields();

    const char *preparedInsertName;
    if(INSERT_BINARY) {
        preparedInsertName = "PREPARE_DATA_PLUS_INSERT_BINARY";
    }else{
        preparedInsertName = "PREPARE_DATA_PLUS_INSERT";
    }
    const char *paramValues[numFields];
    int paramLengths[numFields];
    int paramFormats[numFields];
    Oid paramTypes[numFields];

    for (int c = 0; c < numFields; c++) {
        paramTypes[c] = result.getFieldType(c);
        paramFormats[c] = INSERT_BINARY ? 1 : 0;
    }

    std::string insertQuery = buildInsertQuery(table, nColumns, insertColumns);
    dbTo.prepare(preparedInsertName, insertQuery.c_str(), numFields, paramTypes);

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

        dbTo.executePrepared(preparedInsertName, numFields, paramValues, paramLengths, paramFormats, 0);
        ++count;
    }
}

void insertViaCopy(DatabaseConnection dbFrom, DatabaseConnection dbTo, std::string querySelect, std::string lastUpdateStr, std::string table, int nColumns, std::string insertColumns[], int &count){
    std::stringstream copyFrom;
    copyFrom << "COPY (" << querySelect << ") TO STDOUT";
    std::stringstream copyTo;
    copyTo << "COPY " << table << " (";
    for(int c = 0; c < nColumns; c++)
    {
        copyTo << insertColumns[c];
        if(c<nColumns-1)
        {
            copyTo << ",";
        }
    }
    copyTo << ") FROM STDIN";
    if(lastUpdateStr.compare("") == 0) {
        dbFrom.execute(copyFrom.str().c_str());
    }else {
        const char *paramValues[1];
        paramValues[0] = lastUpdateStr.c_str();
        try{
            std::cout << "try 0" << std::endl;
            dbFrom.executeParams(copyFrom.str().c_str(), 0, NULL, NULL);
        }catch(ResultException e)
        {
            std::cout << e.message << std::endl;
            std::cout << "try 1" << std::endl;
            dbFrom.executeParams(copyFrom.str().c_str(), 1, paramValues, NULL);
            std::cout << ":(" << std::endl;
        }
    }
    dbTo.execute(copyTo.str().c_str());
    char** buffer = new char*;
    int bufferLen;
    while((bufferLen = dbFrom.getCopyData(buffer, 0)) != -1){
        while(dbTo.putCopyData(*buffer, bufferLen) == 0);
        count++;
        dbFrom.freeMem(*buffer);
    }
    dbTo.putCopyEnd(NULL);
}

std::string buildInsertQuery(std::string table, int nColumns, std::string columnFields[], int nRows)
{
    std::stringstream allInsert;
    allInsert << "INSERT INTO " << table << " (";
    for (int c = 0; c < nColumns; c++) {
        allInsert << columnFields[c];
        if (c < nColumns - 1) {
            allInsert << ",";
        }
    }
    allInsert << ") VALUES ";
    for (int r = 0; r < nRows; r++) {
        allInsert << "(";
        for (int c = 0; c < nColumns; c++) {
            int i = r * nColumns + c;
            allInsert << "$" << i + 1;
            if (c < nColumns - 1) {
                allInsert << ",";
            }
        }
        allInsert << ")";
        if (r < nRows - 1) {
            allInsert << ",";
        }
    }
    return allInsert.str();
}
std::string buildInsertQuery(std::string table, int nColumns, std::string columnFields[]){
    return buildInsertQuery(table, nColumns, columnFields, 1);
}

int addairline(PGconn *dbto, const string &code, const string &name){
    return addAirline(DB(dbto), code, name);
}
int addAirline(DB db, const string &code, const string &name)
{
    const char* paramValues[2];
    int paramLengths[2];
    paramValues[0] = code.c_str();
    paramValues[1] = name.c_str();
    paramLengths[0] = code.length();
    paramLengths[1] = name.length();

    try{
        DBr test = db.executeParams("SELECT count(1) FROM airline WHERE code=$1 AND name=$2", 2, paramValues, paramLengths);
        char* count = test.getValue(0, 0);
        if(count[0] != '0')
        {
            return 0;
        }
        db.executeParams("INSERT INTO airline (code, name) VALUES ($1, $2)", 2, paramValues, paramLengths);
        return 1;
    }catch (ResultException e)
    {
        std::cerr << e.message << std::endl;
        return -1;
    }
}
int addAirlines(DB dbFrom, DB dbTo)
{
    int countAirlines = 0;
    DBr airlinesResult = dbFrom.execute(
            "SELECT airlinecode, airline FROM flightdata GROUP BY airline, airlinecode");
    for (int r = 0; r < airlinesResult.nTuples(); r++) {
        int airlineResult = addairline(dbTo.getRaw(), std::string(airlinesResult.getValue(r, 0)),
                                       std::string(airlinesResult.getValue(r, 1)));
        if(airlineResult == -1)
        {
            throw ResultException("Airline could not be added.");
        }
        countAirlines += airlineResult;
    }
    return countAirlines;
}
