//
// DBS-Praktikum Aufgabe 2
//    Hauptprogramm importflights
//
// Autoren:
//   Student1
//   Student2
//

#include "database.h"
#include "copydata.h"
#include <string>
#include <sstream>
#include <iostream>
#include <ctime>

void insertFlights(bool optCreateBeforeImport, bool optDeleteBeforeImport, DB connFrom, DB connTo, bool optShowResults)
{
    const char *statsQuery = "SELECT (SELECT count(1) FROM flugdaten) AS fluege, (SELECT count(1) FROM airline) AS airlines";

    if(optCreateBeforeImport)
    {
        try {
            if (optDeleteBeforeImport) {
                connTo.execute("DROP TABLE flugdaten, airline");
            }

            connTo.execute("CREATE TABLE airline (\n"
                           "code character varying(3) PRIMARY KEY,\n"
                           "name character varying(100)\n"
                           ");");
            connTo.execute("CREATE TABLE flugdaten (\n"
                           "id          integer PRIMARY KEY,\n"
                           "icao        character(6),\n"
                           "flugnr      character varying(7),\n"
                           "airlinecode character varying(3) references airline(code),\n"
                           "flughoehe   integer,\n"
                           "breite      double precision,\n"
                           "laenge      double precision ,\n"
                           "richtung    integer,\n"
                           "geschwindigkeit integer,\n"
                           "vert_rate   integer,\n"
                           "zeit        timestamp,\n"
                           "sender      character varying(50)\n"
                           ");");
            std::cout << "Created tables." << std::endl;
        } catch (ResultException e) {
            throw ResultException("Creating tables failed!");
        }
    }else if (optDeleteBeforeImport) {
        try {
            connTo.execute("TRUNCATE TABLE flugdaten, airline");
            std::cout << "Truncated tables." << std::endl;
        } catch (ResultException e) {
            throw ResultException("Truncating tables failed!");
        }
    }

    DBr statsResult = connTo.execute(statsQuery);

    int countAirlines = addAirlines(connFrom, connTo);
    if (countAirlines == -1) {
        throw ResultException("Adding airlines failed!");
    }
    std::cout << "Added " << countAirlines << " airlines." << std::endl;

    int result = flightcopy(connFrom.getRaw(), connTo.getRaw());
    if (result == -1) {
        throw ResultException("Flights could not be added.");
    }
    std::cout << "Added " << result << " flights." << std::endl;

    if(optShowResults)
    {
        showResults(statsResult);
        showResults(connTo.execute(statsQuery));
    }
}

double testInsertFlights(bool optCreateBeforeImport, bool optDeleteBeforeImport, DB dbFrom, DB dbTo, bool insertBinary, int insertMode)
{
    dbTo.execute("BEGIN TRANSACTION");
    std::cout << "Began transaction." << std::endl;

    INSERT_BINARY = insertBinary;
    INSERT_MODE = insertMode;
    clock_t begin = std::clock();

    insertFlights(optCreateBeforeImport, optDeleteBeforeImport, dbFrom, dbTo, false);

    double seconds = (double(std::clock() - begin) / CLOCKS_PER_SEC);
    std::cout << seconds << " seconds" << std::endl;

    dbTo.execute("ROLLBACK TRANSACTION");
    std::cout << "Rolled transaction back." << std::endl;

    std::cout << std::endl;

    return seconds;
}

// Hauptprogramm
//========================================================
int main (int argc, char* argv[])
{
    INSERT_BINARY = false;
    INSERT_MODE = INSERT_MODE_MANY;

    bool optTest = false;
    bool optDeleteBeforeImport = false;
    bool optCreateBeforeImport = false;
    string optUser = "lukas";
    string optPassword = "test";
    string optHost = "127.0.0.1";
    string optFromDatabase = "dbs_import";
    string optToDatabase = "dbs_praktikum";

    for(int i = 0; i < argc; i++){
        string input = string(argv[i]);
        if(input.compare("-del") == 0)
        {
            optDeleteBeforeImport = true;
        }else if(input.compare("-create") == 0)
        {
            optCreateBeforeImport = true;
        }else if(input.compare("-test") == 0)
        {
            optTest = true;
        }else if(input.compare("-binary") == 0)
        {
            INSERT_BINARY = true;
        }else if(input.compare("-mode") == 0){
            if(++i < argc)
            {
                input = string(argv[i]);
                if(input.compare("copy") == 0)
                {
                    INSERT_MODE = INSERT_MODE_COPY;
                }else if(input.compare("each") == 0){
                    INSERT_MODE = INSERT_MODE_EACH;
                }else if(input.compare("many") == 0){
                    INSERT_MODE = INSERT_MODE_MANY;
                }
            }
        }else if(input.compare("-u") == 0){
            if(++i < argc)
            {
                input = string(argv[i]);
                optUser = input;
            }
        }else if(input.compare("-p") == 0){
            if(++i < argc)
            {
                input = string(argv[i]);
                optPassword = input;
            }
        }else if(input.compare("-h") == 0){
            if(++i < argc)
            {
                input = string(argv[i]);
                optHost = input;
            }
        }else if(input.compare("-from") == 0){
            if(++i < argc)
            {
                input = string(argv[i]);
                optFromDatabase = input;
            }
        }else if(input.compare("-to") == 0){
            if(++i < argc)
            {
                input = string(argv[i]);
                optToDatabase = input;
            }
        }
    }

    std::cout << std::endl;
    std::cout << "delete:   " << (optDeleteBeforeImport ? "yes" : "no") << std::endl;
    std::cout << "user:     " << optUser << std::endl;
    std::cout << "password: " << optPassword << std::endl;
    std::cout << "host:     " << optHost << std::endl;
    std::cout << "db-from:  " << optFromDatabase << std::endl;
    std::cout << "db-to:    " << optToDatabase << std::endl;
    std::cout << std::endl;

    try {
        DB dbFrom = DB(optFromDatabase, optUser, optPassword, optHost);
        DB dbTo = DB(optToDatabase, optUser, optPassword, optHost);

        try {
            if(optTest) {
                double timeSEach = testInsertFlights(optCreateBeforeImport, optDeleteBeforeImport, dbFrom, dbTo, false, INSERT_MODE_EACH);
                double timeBEach = testInsertFlights(optCreateBeforeImport, optDeleteBeforeImport, dbFrom, dbTo, true, INSERT_MODE_EACH);
                double timeSMany = testInsertFlights(optCreateBeforeImport, optDeleteBeforeImport, dbFrom, dbTo, false, INSERT_MODE_MANY);
                double timeBMany = testInsertFlights(optCreateBeforeImport, optDeleteBeforeImport, dbFrom, dbTo, true, INSERT_MODE_MANY);
                double timeCopy = testInsertFlights(optCreateBeforeImport, optDeleteBeforeImport, dbFrom, dbTo, false, INSERT_MODE_COPY);

                std::cout << "string each " << timeSEach << " seconds" << std::endl;
                std::cout << "binary each " << timeBEach << " seconds" << std::endl;
                std::cout << "string many " << timeSMany << " seconds" << std::endl;
                std::cout << "binary many " << timeBMany << " seconds" << std::endl;
                std::cout << "copy        " << timeCopy << " seconds" << std::endl;
            }else{
                dbTo.execute("BEGIN TRANSACTION");
                std::cout << "Began transaction." << std::endl;

                insertFlights(optCreateBeforeImport, optDeleteBeforeImport, dbFrom, dbTo, true);

                dbTo.execute("COMMIT TRANSACTION");
                std::cout << "Commit transaction." << std::endl;
            }

            dbFrom.finish();
            dbTo.finish();
        }catch (ResultException e)
        {
            std::cerr << e.message << std::endl;
            dbFrom.finish();
            dbTo.execute("ROLLBACK TRANSACTION");
            std::cout << "Rolled transacetion back." << std::endl;
            dbTo.finish();
            return 1;
        }
    }catch (ConnectionException e)
    {
        std::cerr << e.message << std::endl;
        return 1;
    }

    return 0;
}
