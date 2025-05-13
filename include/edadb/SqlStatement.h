/***
 * @file SqlStatement.hpp
 * @brief SqlStatement.hpp generates the SQL statement for the given type.
 */

#pragma once

#include <assert.h>
#include <vector>   
#include <iostream>
#include <sstream>

#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/algorithm.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/include/vector.hpp>

#include "Shadow.h"
#include "DbBackendType.h"
#include "Cpp2SqlType.h"
#include "TypeMetaData.h"

namespace edadb {

//////// Table Relations //////////////////////////////////////////////////////
/**
 * @brief Foreign keys constraint.
 */
struct ForeignKey {
public:
    std::string table; // referenced table name
    std::vector<std::string> column; // column name as foreign key
    std::vector<std::string> type; // column type for each column

public:
    ~ForeignKey() = default;
    ForeignKey () = default;
    ForeignKey (const ForeignKey &fkc) = default;
    ForeignKey (std::string tb, std::vector<std::string> c, std::vector<std::string> t)
        : table(tb), column(c), type(t) {}
    ForeignKey& operator=(const ForeignKey &fkc) = default;

public:
    bool valid() const {
        return !table.empty() && column.size() > 0 && type.size() > 0;
    }

public:
    void print() const {
        std::cout << "ForeignKey: table=" << table << std::endl;
        for (size_t i = 0; i < column.size(); ++i) {
            std::cout << "  column[" << i << "]=" << column[i] << ", type=" << type[i] << std::endl;
        }
    }
}; // ForeignKey



/**
 * @struct SqlStatementBase
 * @brief This struct is the base class for generating SQL statements.
 * @details This class provides the basic functionality for generating SQL statements.
 */
struct SqlStatementBase {

protected:  // some utility functions

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
     * @brief Convert string value to string.
     * @param val The binary value.
     * @return The string value.
     */
    static std::string binary2String(std::string &val) {
        return std::string("\'") + val + "\'";
    }


//////// Column Infos //////////////////////////////////////////////////////
    /**
     * @brief Appender for column names to the vector.
     */
    template <typename T>
    struct ColumnNameType {
    private:
        std::vector<std::string>& name;
        std::vector<std::string>& type;
        std::string prefix;
        uint32_t index = 0;

    public:
        ColumnNameType(
            std::vector<std::string>& n, std::vector<std::string>& t, std::string p = "")
            : name(n), type(t), prefix(p), index(0) {}

        /**
         * @brief Appender for column names to the vector.
         * @param x is TypeMetaData::tuple_type_pair() element:
         *     x.first  - pointer to the member variable
         *     x.second - the member name
         */
        template <typename TuplePair>
        void operator()(TuplePair const& x) {
            // use the type of the first element as cppType
            using ElemType = typename TuplePair::first_type;
            using CppType  = typename std::remove_const<typename std::remove_pointer<ElemType>::type>::type;

            // use variable's sql type decides the column type
            const std::vector<std::string>& cnames = TypeMetaData<T>::column_names();
            std::string column_name = cnames[index++];

            if constexpr (edadb::Cpp2SqlType<CppType>::sqlType == edadb::SqlType::Composite) {
                assert ((index > 0) &&
                    "SqlStatementBase::ColumnNameType::operator(): composite type should not be the first element");

                const auto vecs = TypeMetaData<CppType>::tuple_type_pair();
                const std::string next_pref = prefix + "_" + column_name + "_";
                boost::fusion::for_each(vecs,
                    ColumnNameType<CppType>(name, type, next_pref));
            }
            else if constexpr (edadb::Cpp2SqlType<CppType>::sqlType == edadb::SqlType::External) {
                assert ((index > 0) &&
                    "SqlStatementBase::ColumnNameType::operator(): external type should not be the first element");

                const auto vecs = TypeMetaData<edadb::Shadow<CppType>>::tuple_type_pair();
                const std::string next_pref = prefix + "_" + column_name + "_";
                boost::fusion::for_each(vecs,
                    ColumnNameType<edadb::Shadow<CppType>>(name, type, next_pref));
            }
            else {
                std::string sqlTypeString = edadb::getSqlTypeString<CppType> ();
                type.push_back(sqlTypeString);

                // use the user defined column name
                name.push_back(prefix + column_name);
//                name.push_back(x.second); // x.second is the name of the member variable
            }
        }
    };

    /**
     * @brief Appender for column names to the vector.
     */
    struct ColumnValues {
    private:
        std::vector<std::string>& values;
    
    public:
        ColumnValues(std::vector<std::string>& v) : values(v) {}

        /**
         * @brief Appender for column values to the vector.
         * @param v is TypeMetaData::TupType element:
         *     v  - pointer to the member variable, such as obj->name
         */
        template <typename ValueType>
        void operator()(ValueType& v) {
            // extract the CppType from the ValueType pointer
            using CppType = typename std::remove_const<typename std::remove_pointer<ValueType>::type>::type;

            if constexpr (edadb::Cpp2SqlType<CppType>::sqlType == edadb::SqlType::Composite) {
                // transform the object of composite type to string:
                //   expand the composite type's member variables to the vector
                boost::fusion::for_each(TypeMetaData<CppType>::getVal(v), ColumnValues(values));
            }
            else if constexpr (edadb::Cpp2SqlType<CppType>::sqlType == edadb::SqlType::External) {
                // transform the object of external type to string:
                //   expand the external type's member variables to the vector
                boost::fusion::for_each(TypeMetaData<edadb::Shadow<CppType>>::getVal(v), ColumnValues(values));
            }
            else {
                // transform the value of basic type to string:
                //   append the value to the vector
                values.push_back(binary2String(*v));
            }
        }
    };
}; // SqlStatementBase



/**
 * @struct SqlStatementImpl
 * @brief This struct is the implementation of SqlStatementBase.
 * @details This struct is the implementation of SqlStatementBase for different database backends, which is assigned by DbBackendType.
*/
template <DbBackendType DBType, typename T>
struct SqlStatementImpl : public SqlStatementBase {
    static_assert(DBType != DBType, "DbBackendType is not supported");
};


/**
 * SqlStatement is defined in backend, such as backend/sqlite/SqlStatement4Sqlite.h
 *  using SqlStatement = std::enable_if_t< Config::backend_type == DbBackendType::SQLITE,
 *      SqlStatementImpl<DbBackendType::SQLITE>>;
 */


} // namespace edadb