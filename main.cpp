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
    bool optCreateTables = false;
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
        }else if(input.compare("-create") == 0)
        {
            optCreateTables = true;
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

    printf("create: %s\n", optCreateTables ? "true" : "false");
    printf("delete: %s\n", optDeleteBeforeImport ? "true" : "false");
    printf("user: %s\n", optUser.c_str());
    printf("password: %s\n", optPassword.c_str());
    printf("host: %s\n", optHost.c_str());
    printf("from: %s\n", optFromDatabase.c_str());
    printf("to: %s\n", optToDatabase.c_str());

    try {
        DB connFrom = DB(optFromDatabase, optUser, optPassword, optHost);
        DB connTo = DB(optToDatabase, optUser, optPassword, optHost);

        if (optDeleteBeforeImport) {
            try{
                connTo.execute("TRUNCATE TABLE flugdaten, airline");
                std::cout << "Truncated tables." << std::endl;
            }catch (ResultException e)
            {
                std::cerr << e.message << std::endl;
                return 1;
            }
        }

        int countAirlines = 0;
        DBr airlinesResult = connFrom.execute(
                "SELECT airlinecode, airline FROM flightdata GROUP BY airline, airlinecode");
        for (int r = 0; r < airlinesResult.nTuples(); r++) {
            int airlineResult = addairline(connTo.getRaw(), std::string(airlinesResult.getValue(r, 0)),
                       std::string(airlinesResult.getValue(r, 1)));
            if(airlineResult == -1)
            {
                throw ResultException("Airline could not be added.");
            }
            countAirlines += airlineResult;
        }
        std::cout << "Added " << countAirlines << " airlines." << std::endl;

        int result = flightcopy(connFrom.getRaw(), connTo.getRaw());
        if(result == -1)
        {
            throw ResultException("Flights could not be added.");
        }
        std::cout << "Added " << result << " flights." << std::endl;

        connFrom.finish();
        connTo.finish();
    }catch (ResultException e)
    {
        std::cerr << e.message << std::endl;
        return 1;
    }catch (ConnectionException e)
    {
        std::cerr << e.message << std::endl;
        return 1;
    }

    return 0;
}
