//
// Created by lukas on 6/2/16.
//


#pragma once

#include <libpq-fe.h>
#include <string>
#include <sstream>
#include <iostream>

class ConnectionException;
class ResultException;
class DatabaseResult;
class DatabaseConnection;

class ConnectionException{
public:
    std::string message;
    ConnectionException(std::string message);
};

class ResultException{
public:
    std::string message;
    std::string status;
    ResultException(std::string message, std::string status);
    ResultException(std::string message);
};

class DatabaseResult {
private:
    PGresult* result;
    DatabaseConnection* db;

    ExecStatusType resultStatus();
    static std::string resultStatusMessage(ExecStatusType status);
    std::string resultErrorMessage();
public:
    DatabaseResult();
    DatabaseResult(DatabaseConnection* db, PGresult* result);

    void check();

    void clear();
    void operator~();
    int nTuples();
    int nFields();
    char * getValue(int row_number, int column_number);
    int getLength(int row_number, int column_number);
    bool isNull(int row_number, int column_number);
    Oid getFieldType(int column_number);
    std::string fieldName(int column_number);
    int fieldNumber(std::string column_name);
    int fieldSize(int column_number);
};

class DatabaseConnection {
private:
    PGconn* conn;
public:
    DatabaseConnection(std::string database, std::string user, std::string password, std::string host);
    DatabaseConnection(PGconn* conn);
    void finish();

    PGconn* getRaw();

    ConnStatusType status();

    char* errorMessage();

    void check();

    DatabaseResult prepare(const char *stmtName, const char *query, int nParams, const Oid *paramTypes);
    DatabaseResult prepare(const char *stmtName, const char *query, int nParams);

    DatabaseResult execute(const char *command);

    DatabaseResult executeParams(const char *command,
                                 int nParams,
                                 const char * const *paramValues,
                                 const int *paramLengths,
                                 const int *paramFormats,
                                 const Oid *paramTypes,
                                 int resultFormat);
    DatabaseResult executeParams(const char *command,
                                 int nParams,
                                 const char * const *paramValues,
                                 const int *paramLengths);

    DatabaseResult executePrepared(const char *stmtName,
                                   int nParams,
                                   const char * const *paramValues,
                                   const int *paramLengths,
                                   const int *paramFormats,
                                   int resultFormat);
    DatabaseResult executePrepared(const char *stmtName,
                                   int nParams,
                                   const char * const *paramValues,
                                   const int *paramLengths);
};

typedef DatabaseConnection DB;
typedef DatabaseResult DBr;