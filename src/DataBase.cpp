#include "DataBase.h"
#include <json/reader.h>
#include <sstream>
#include <json/json.h>
#include <boost/algorithm/string.hpp>
#include <fstream>

DataBase::DataBase()
{
}

DataBasePtr DataBase::instance;

DataBasePtr DataBase::getInstance()
{
    if(instance == nullptr){
        instance.reset( new DataBase() );
    }
    return instance;
}

void DataBase::checkScheme(const string& filename)
{
    ifstream file(filename);
    if(file.good()){
        conn << file.rdbuf();
    }
}

void DataBase::init(const string& connectStringDataBase)
{
    conn.open(connectStringDataBase);
}

soci::rowset<soci::row> DataBase::executeSql(const string &sql)
{
    soci::rowset<soci::row> rs = conn.prepare << sql;
    return rs;
}

inline void jsonValueToCollumnValue(const Json::Value &value, string& collumns, string& values)
{
    for(auto& member: value.getMemberNames()){
        if( ! collumns.empty()){
            collumns += ", ";
            values += ", ";
        }
        collumns += member;
        Json::Value val = value.get(member, Json::Value());
        if(val.isObject())
            values += '\''+ boost::algorithm::replace_all_copy(val.toStyledString(), "'", "\"")+'\'';
        else
            values += '\''+ boost::algorithm::replace_all_copy(val.asString(), "'", "\"")+'\'';
    }
}

size_t DataBase::create(const string& table, const Json::Value &value)
{
    long id = 0;
    string collumns;
    string values;
    jsonValueToCollumnValue(value, collumns, values);
    conn << "insert into "<<table<<" ("+collumns+") values("+values+") RETURNING id", soci::into(id);
    if(id == 0)
        conn.get_last_insert_id(table, id);
    return id;
}

void DataBase::update(const string& table, const string& search, const Json::Value &value)
{    
    string sets;
    for(auto& member: value.getMemberNames()){
        if(sets.size())
            sets += ", ";
        const string& val = value.get(member, Json::Value()).asString();
        sets += member + "='"+boost::algorithm::replace_all_copy( val, "'", "\"" ) + '\'';
    }
    conn << "update "<<table<<" set "+sets+" where id="<<search;
}

inline string condictionsToString(const vector<Condition>& conditions, const string& logicalOperator)
{
    string where;
    for(auto& condition: conditions){
        if(where.size())
            where += ' '+logicalOperator+' ';
        where += condition.col;
        switch (condition.oper) {
        case LIKE:
            where+=" like '%"+condition.val+"%'";
            break;
        case ARRAY_JSON:
            where+=" @> "+condition.val;
            break;
        case EQUAL:
            where+="='"+condition.val+"'";
            break;
        case DIFFERENT:
            where+="<>'"+condition.val+"'";
            break;                
        case BIGGER_THEN:
            where+=">'"+condition.val+"'";
            break;
        case LESS_THAN:
            where+="<'"+condition.val+"'";
            break;
        }
    }
    return where;
}

inline time_t tmToTimeT(const tm& s)
{
    const time_t& time = mktime((tm*)&s);
    return time;
}

inline void getValueInCollumn(const soci::row& r, const size_t& i, Json::Value& json)
{
    const string& name = r.get_properties(i).get_name();
    switch (r.get_properties(i).get_data_type()) {
    case soci::dt_string:
        json[name] = r.get<string>(i);
        break;
    case soci::dt_date:
        json[name] = (Json::Int64)tmToTimeT(r.get<tm>(i));
        break;
    case soci::dt_double:
        json[name] = r.get<double>(i);
        break;
    case soci::dt_integer:
        json[name] = (Json::Int)r.get<int>(i);
        break;
    case soci::dt_long_long:
        json[name] = (Json::Int64)r.get<long long>(i);
        break;
    case soci::dt_unsigned_long_long:
        json[name] = (Json::UInt64)r.get<unsigned long long>(i);
        break;
    default:
        break;
    }
}

Json::Value DataBase::search(const string& table, const vector<Condition>& conditions, const string& logicalOperator)
{
    Json::Value result;
    const string where = condictionsToString(conditions, logicalOperator);
    soci::rowset<soci::row> rs = conn.prepare << "SELECT *  FROM "<<table<< (where.size()?" WHERE "+where:"");
    for(soci::row& r: rs)
    {
        Json::Value entity;
        for(int i=0; i<r.size(); i++){
            if(r.get_indicator(i) != soci::i_null){
                getValueInCollumn(r, i, entity);
            }
        }
        result.append(entity);
    }
    return result;
}

void DataBase::remove(const string& table, const string& search)
{
}


