//
// DBS-Praktikum Aufgabe 2
//    Hauptprogramm importflights
//
// Autoren:
//   Student1
//   Student2
//

#include <stdio.h>
#include <cstdlib>
#include "copydata.h"
#include <libpq-fe.h>
#include <string>
#include <sstream>

static void exit_nicely(PGconn *conn)
{
    PQfinish(conn);
    exit(1);
}

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

    printf("create: %s\n", optCreateTables ? "true" : "false");
    printf("delete: %s\n", optDeleteBeforeImport ? "true" : "false");
    printf("user: %s\n", optUser.c_str());
    printf("password: %s\n", optPassword.c_str());
    printf("host: %s\n", optHost.c_str());
    printf("from: %s\n", optFromDatabase.c_str());
    printf("to: %s\n", optToDatabase.c_str());

    PGconn* connFrom;
    std::stringstream ssFrom;
    ssFrom << "dbname=" << optFromDatabase << " user=" << optUser << " password=" << optPassword << " hostaddr=" << optHost;
    string connInfoFrom = ssFrom.str();
    connFrom = PQconnectdb(connInfoFrom.c_str());
    if (PQstatus(connFrom) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection to database %s failed: %s",
                optFromDatabase.c_str(),
                PQerrorMessage(connFrom));
        exit_nicely(connFrom);
    }

    PGconn* connTo;
    std::stringstream ssTo;
    ssTo << "dbname=" << optToDatabase << " user=" << optUser << " password=" << optPassword << " hostaddr=" << optHost;
    string connInfoTo = ssTo.str();
    connTo = PQconnectdb(connInfoTo.c_str());
    if (PQstatus(connTo) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection to database %s failed: %s",
                optFromDatabase.c_str(),
                PQerrorMessage(connTo));
        PQfinish(connFrom);
        exit_nicely(connTo);
    }

    int result = flightcopy(connFrom, connTo);

    PQfinish(connFrom);
    PQfinish(connTo);

    printf("done\n");

    return 0;
}
