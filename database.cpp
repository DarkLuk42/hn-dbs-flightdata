//
// Created by lukas on 6/2/16.
//


#include "database.h"

ConnectionException::ConnectionException(std::string message)
{
    this->message = message;
}


ResultException::ResultException(std::string message, std::string status)
{
    this->message = message;
    this->status = status;
}
ResultException::ResultException(std::string message)
{
    this->message = message;
}


ExecStatusType DatabaseResult::resultStatus(){
    return PQresultStatus(this->result);
}
std::string DatabaseResult::resultStatusMessage(ExecStatusType status){
    return std::string(PQresStatus(status));
}
std::string DatabaseResult::resultErrorMessage(){
    return std::string(PQresultErrorMessage(this->result)) + this->db->errorMessage();
}

DatabaseResult::DatabaseResult(DatabaseConnection* db, PGresult* result){
    this->db = db;
    this->result = result;
}

void DatabaseResult::check(){
    ExecStatusType status = this->resultStatus();
    std::string message;
    switch(status)
    {
        case PGRES_EMPTY_QUERY:
        case PGRES_COMMAND_OK:
        case PGRES_TUPLES_OK:
        case PGRES_COPY_OUT:
        case PGRES_COPY_IN:
        case PGRES_COPY_BOTH:
            break;
        case PGRES_NONFATAL_ERROR:
            std::cerr << this->resultErrorMessage() << std::endl;
            break;
        case PGRES_BAD_RESPONSE:
        case PGRES_FATAL_ERROR:
            throw ResultException(this->resultErrorMessage(), resultStatusMessage(status));
        default:
            throw ResultException("Unsupported status", resultStatusMessage(status));
    }
};

void DatabaseResult::clear(){
    PQclear(this->result);
}
int DatabaseResult::nTuples(){
    return PQntuples(this->result);
}
int DatabaseResult::nFields(){
    return PQnfields(this->result);
}
char * DatabaseResult::getValue(int row_number, int column_number){
    return PQgetvalue(this->result, row_number, column_number);
}
int DatabaseResult::getLength(int row_number, int column_number){
    return PQgetlength(this->result, row_number, column_number);
}
bool DatabaseResult::isNull(int row_number, int column_number){
    return PQgetisnull(this->result, row_number, column_number);
}
Oid DatabaseResult::getFieldType(int column_number)
{
    return PQftype(this->result, column_number);
}
std::string DatabaseResult::fieldName(int column_number){
    return PQfname(this->result, column_number);
}
int DatabaseResult::fieldNumber(std::string column_name){
    return PQfnumber(this->result, column_name.c_str());
}
int DatabaseResult::fieldSize(int column_number){
    return PQfsize(this->result, column_number);
}


DatabaseConnection::DatabaseConnection(std::string database, std::string user, std::string password, std::string host){
    std::stringstream ss;
    ss << "dbname=" << database << " user=" << user << " password=" << password << " hostaddr=" << host;
    std::string connectionInfo = ss.str();
    this->conn = PQconnectdb(connectionInfo.c_str());
    this->check();
}
DatabaseConnection::DatabaseConnection(PGconn* conn){
    this->conn = conn;
    this->check();
}
void DatabaseConnection::finish(){
    PQfinish(this->conn);
}

PGconn* DatabaseConnection::getRaw(){
    return this->conn;
}

ConnStatusType DatabaseConnection::status(){
    return PQstatus(this->conn);
}

char* DatabaseConnection::errorMessage(){
    return PQerrorMessage(this->conn);
}

void DatabaseConnection::check(){
    if (this->status() != CONNECTION_OK) {
        throw ConnectionException(this->errorMessage());
    }
}

DatabaseResult DatabaseConnection::prepare(const char *stmtName, const char *query, int nParams, const Oid *paramTypes){
    //std::cout << "prepare(" << stmtName << ", " << query << ", " << nParams << ")" << std::endl;
    DatabaseResult result = DatabaseResult(this, PQprepare(this->conn, stmtName, query, nParams, paramTypes));
    result.check();
    return result;
}
DatabaseResult DatabaseConnection::prepare(const char *stmtName, const char *query, int nParams){
    return prepare(stmtName, query, nParams, NULL);
}

DatabaseResult DatabaseConnection::execute(const char *command){
    //std::cout << "execute(" << command << ")" << std::endl;
    DatabaseResult result = DatabaseResult(this, PQexec(this->conn, command));
    result.check();
    return result;
}

DatabaseResult DatabaseConnection::executeParams(const char *command,
                             int nParams,
                             const char * const *paramValues,
                             const int *paramLengths,
                             const int *paramFormats,
                             int resultFormat){
    //std::cout << "executeParams(" << command << ", " << nParams << ", ..., ...)" << std::endl;
    DatabaseResult result = DatabaseResult(this, PQexecParams(this->conn,
                                                              command,
                                                              nParams,
                                                              NULL,
                                                              paramValues,
                                                              paramLengths,
                                                              paramFormats,
                                                              resultFormat));
    result.check();
    return result;
}
DatabaseResult DatabaseConnection::executeParams(const char *command,
                             int nParams,
                             const char * const *paramValues,
                             const int *paramLengths){
    return executeParams(command, nParams, paramValues, paramLengths, NULL, 0);
}

DatabaseResult DatabaseConnection::executePrepared(const char *stmtName,
                               int nParams,
                               const char * const *paramValues,
                               const int *paramLengths,
                               const int *paramFormats,
                               int resultFormat){
    //std::cout << "executePrepared(" << stmtName << ", " << nParams << ", ..., ...)" << std::endl;
    DatabaseResult result = DatabaseResult(this, PQexecPrepared(this->conn,
                                                                stmtName,
                                                                nParams,
                                                                paramValues,
                                                                paramLengths,
                                                                paramFormats,
                                                                resultFormat));
    result.check();
    return result;
}
DatabaseResult DatabaseConnection::executePrepared(const char *stmtName,
                               int nParams,
                               const char * const *paramValues,
                               const int *paramLengths){
    return executePrepared(stmtName, nParams, paramValues, paramLengths, NULL, 0);
}