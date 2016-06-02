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

// Hauptprogramm
//========================================================
int main (int argc, char* argv[])
{
    bool optDeleteBeforeImport = false;
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
        DB connFrom = DB(optFromDatabase, optUser, optPassword, optHost);
        DB connTo = DB(optToDatabase, optUser, optPassword, optHost);

        try {
            connTo.execute("BEGIN TRANSACTION");
            std::cout << "Began transaction." << std::endl;

            const char *statsQuery = "SELECT (SELECT count(1) FROM flugdaten) AS fluege, (SELECT count(1) FROM airline) AS airlines";

            if (optDeleteBeforeImport) {
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

            connTo.execute("COMMIT TRANSACTION");
            std::cout << "Commit transaction." << std::endl;

            showResults(statsResult);
            showResults(connTo.execute(statsQuery));

            connFrom.finish();
            connTo.finish();
        }catch (ResultException e)
        {
            std::cerr << e.message << std::endl;
            connFrom.finish();
            connTo.execute("ROLLBACK TRANSACTION");
            std::cout << "Rolled transaction back." << std::endl;
            connTo.finish();
            return 1;
        }
    }catch (ConnectionException e)
    {
        std::cerr << e.message << std::endl;
        return 1;
    }

    return 0;
}
