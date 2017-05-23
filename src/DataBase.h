#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <vector>
#include <soci/soci.h>
#include <json/json.h>

using namespace std;

class DataBase;
typedef shared_ptr<DataBase> DataBasePtr;

enum Operator {
    EQUAL,
    DIFFERENT,
    BIGGER_THEN,
    LESS_THAN,
    LIKE,
    ARRAY_JSON,
};

struct Condition {
    string col;
    Operator oper;
    string val;
};

class DataBase
{
public:
    static DataBasePtr getInstance();
    void checkScheme(const string &filename);
    void init(const string& connectStringDataBase);
    soci::rowset<soci::row> executeSql(const string& sql);
    
    /**
     * @param table, name of table
     * @param value, json values with datas in rows and columns
     */
    size_t create(const string& table, const Json::Value& value);
    void update(const string& table, const string& search, const Json::Value &value);
    Json::Value search(const string& table, const vector<Condition>& search, const string &logicalOperator="OR");
    void remove(const string& table, const string& search);

private:
    DataBase();
    static DataBasePtr instance;
    soci::session conn;
};

#endif // DATABASE_H
