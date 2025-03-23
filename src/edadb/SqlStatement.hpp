/***
 * @file SqlStatement.hpp
 * @brief SqlStatement.hpp generates the SQL statement for the given type.
 */

#pragma once

#include <sstream>

namespace edadb {

template<class T>
struct SqlStatement {

//////// Trans Binary Value to String ////////////////////////////////////
    /**
     * @brief Convert binary value to string template.
     * @param val The binary value.
     * @return The string value.
     */
    template <typename ValueType>
    static std::string binary2String(ValueType &val) {
        return std::to_string(val);
    }
    
    /**
     * @brief Convert binary value to string template specialization.
     * @param val The binary value.
     * @return The string value.
     */
    static std::string binary2String(std::string &val) {
        return std::string("\'") + val + "\'";
    }


//////// Appenders //////////////////////////////////////////////////////
private: 
    /**
     * @brief Appender for name and type.
     * @param sql The SQL statement to append.
     * @param idx The index of the element.
     */
    struct Appender4NameType {
    private:
        std::string& sql;
        int idx;

    public:
        Appender4NameType(std::string& sql) : sql(sql), idx(0) {}

        template <typename ValueType>
        void operator()(ValueType const& x) 
        {
            // ValueType:
            //   defined using boost::fusion::make_pair<int*>(std::string("name"))
            //   the type is boost::fusion::pair<int*, std::string>
            using ElemType = typename ValueType::first_type;
            using CppType = typename std::remove_const<typename std::remove_pointer<ElemType>::type>::type;
            std::string dbType = edadb::cppTypeToDbTypeString<CppType> ();
            if(idx++ == 0) {
//                sql += x.second + " " + dbType + " PRIMARY KEY";  
                sql += x.second + " " + dbType;  
            } else {
                sql += ", " + x.second + " " + dbType;
            }
        }
    };


    struct Appender4Name {
    private:
        std::stringstream& ss;  
        int idx;

    public:
        Appender4Name(std::stringstream& s) : ss(s), idx(0) {}  

        template <typename ValueType>
        void operator()(ValueType const& x) {
            ss << (idx++ > 0 ? ", " : "") << x.second;
        }
    };


    struct Appender4Value {
    private:
        std::stringstream& ss;  
        int idx;

    public:
        Appender4Value(std::stringstream& s) : ss(s), idx(0) {}  

        template<typename ValueType>
        void operator()(ValueType val) {
            ss << (idx++ > 0 ? ", " : "") << *val;
        }

        void operator()(std::string* val) {
            ss << (idx++ > 0 ? ", " : "") << "\'" << *val << "\'";
        }
    };


    struct Appender4PlaceHolder {
    private:
        std::stringstream& ss;  
        int idx;

    public:
        Appender4PlaceHolder(std::stringstream& s) : ss(s), idx(0) {}  

        template<typename ValueType>
        void operator()(ValueType val) {
            ss << (idx++ > 0 ? ", " : "") << "?";
        }
    };



//////// Sql Statement /////////////////////////////////////////////////////////

public: // create table
    static std::string const& createTableStatement() {
        static std::string sql;
        if (sql.empty()) {
            sql = "CREATE TABLE IF NOT EXISTS \"{}\" (";
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            boost::fusion::for_each(vecs,
                    SqlStatement<T>::Appender4NameType(sql));
            sql += ")";
        }
        return sql;
    }


public: // insert
    /**
     * @brief Generate the insert statement with place holders.
     * @return The insert statement.
     */
    static std::string insertPlaceHolderStatement() {
        static std::string sql;
        if (sql.empty()) {
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            std::stringstream ss;
            ss << "INSERT INTO \"{}\" (";
            boost::fusion::for_each(vecs, Appender4Name(ss));
            ss << ") VALUES (";
            boost::fusion::for_each(vecs, Appender4PlaceHolder(ss));
            ss << ");";
            sql.assign(ss.str());
        }
        return sql;
    }

    std::string insertStatement(T* obj) {
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        const auto vals = TypeMetaData<T>::getVal(obj);
        std::stringstream ss;
        ss << "INSERT INTO \"{}\" (";
        boost::fusion::for_each(vecs, Appender4Name(ss));
        ss << ") VALUES (";
        boost::fusion::for_each(vals, Appender4Value(ss));
        ss << ");";
        return ss.str();
    }


public: // scan
    static std::string const &scanStatement() {
        // select col1, col2, ... from table_name;
        static std::string sql;
        if (sql.empty()) {
            std::stringstream ss;
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            boost::fusion::for_each(vecs, Appender4Name(ss));
            sql = "SELECT " + ss.str() + " FROM \"{}\";";
        }
        return sql;
    }


public: // lookup by primary key
    static std::string const lookupStatement(T* obj) {
        static std::string sql;
        if (sql.empty()) {
            std::stringstream ss;
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            boost::fusion::for_each(vecs, Appender4Name(ss));
            sql = "SELECT " + ss.str() + " FROM \"{}\" WHERE ";

            auto pk_name_pair = boost::fusion::at_c<0>(vecs);
            sql += pk_name_pair.second + " = ";
        }

        const auto vals = TypeMetaData<T>::getVal(obj);
        auto pk_val_ptr = boost::fusion::at_c<0>(vals);
        auto pk_val_str = binary2String(*pk_val_ptr);
        return sql + pk_val_str + ";";
    }


public: // delete by primary key
    static std::string const deleteStatement(T* obj) {
        static std::string sql_prefix;
        if (sql_prefix.empty()) {
            // primary key is the first element in the tuple
            static auto vecs = TypeMetaData<T>::tuple_type_pair();
            auto pk_name_pair = boost::fusion::at_c<0>(vecs);
            sql_prefix = "DELETE FROM \"{}\" WHERE ";
            sql_prefix += pk_name_pair.second + " = ";
        }

        const auto vals = TypeMetaData<T>::getVal(obj);
        auto pk_val_ptr = boost::fusion::at_c<0>(vals);
        auto pk_val_str = binary2String(*pk_val_ptr);
        return sql_prefix + pk_val_str + ";";
    }


private: // update by primary key
    struct UpdateNames {
    private:
        std::vector<std::string>& names;

    public:
        UpdateNames(std::vector<std::string>& n) : names(n) {}

        template <typename ValueType>
        void operator()(ValueType const& x) {
            names.push_back(x.second);
        }
    };

    struct UpdateValues {
    private:
        std::vector<std::string>& values;
    
    public:
        UpdateValues(std::vector<std::string>& v) : values(v) {}

        template <typename ValueType>
        void operator()(ValueType& x) {
            values.push_back(binary2String(*x));
        }
    };


public: 
    static std::string const updateStatement(T* org_obj, T* new_obj) {
        std::string sql = "UPDATE \"{}\" SET ";

        // get names and values
        std::vector<std::string> names, values;
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(vecs, UpdateNames(names));
        boost::fusion::for_each(TypeMetaData<T>::getVal(new_obj), UpdateValues(values));

        // generate sql
        const uint32_t num = names.size();
        for (uint32_t i = 0; i < num; ++i) {
            sql += names[i] + " = " + values[i] + (i + 1 < num ? ", " : "");
        }

        // get primary key name and value
        auto pk_val_ptr = boost::fusion::at_c<0>(TypeMetaData<T>::getVal(org_obj));
        auto pk_val_str = binary2String(*pk_val_ptr);
        sql += " WHERE " + names[0] + " = " + pk_val_str + ";";
        return sql;
    }


public: // debug
    void print(T* obj1, T* obj2) {
        std::cout << "SqlStatement<" << typeid(T).name() << ">" << " SQLs:" << std::endl;
        std::cout << "Create Table SQL: " << std::endl << "\t" 
            << createTableStatement() << std::endl;
        std::cout << "Insert Place Holder SQL: " << std::endl << "\t" 
            << insertPlaceHolderStatement() << std::endl;
        std::cout << "Insert SQL: " << std::endl << "\t" 
            << insertStatement(obj1) << std::endl;
        std::cout << "Insert SQL: " << std::endl << "\t" 
            << insertStatement(obj2) << std::endl;
        std::cout << "Scan SQL: " << std::endl << "\t" 
            << scanStatement() << std::endl;
        std::cout << "Lookup SQL: " << std::endl << "\t"
            << lookupStatement(obj1) << std::endl;
        std::cout << "Delete SQL: " << std::endl << "\t"
            << deleteStatement(obj1) << std::endl;
        std::cout << "Delete SQL: " << std::endl << "\t"
            << deleteStatement(obj2) << std::endl;
        std::cout << "Update SQL: " << std::endl << "\t"
            << updateStatement(obj1, obj2) << std::endl;
        std::cout << std::endl << std::endl;
    }
};


} // namespace edadb