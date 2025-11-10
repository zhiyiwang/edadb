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
#include "TypeInfoTrait.h"
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
        return (!table.empty()) && (column.size() > 0) && (type.size() > 0);
    }

public:
    void print(const std::string& pref = "") const {
        std::cout << pref << "ForeignKey: table=" << table << std::endl;
        for (size_t i = 0; i < column.size(); ++i) {
            std::cout << pref
                << "  column[" << i << "]=" << column[i] << ", type=" << type[i]
                << std::endl;
        } // for 
    } // print
}; // ForeignKey



/**
 * @struct SqlStatementBase
 * @brief This struct is the base class for generating SQL statements.
 * @details This class provides the basic functionality for generating SQL statements.
 */
struct SqlStatementBase {
protected:  // some utility functions

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
        ColumnNameType(std::vector<std::string>& n, std::vector<std::string>& t,
            std::string p = "") : name(n), type(t), prefix(p), index(0) {}

        /**
         * @brief Appender for column names to the vector.
         * @param x is TypeMetaData::tuple_type_pair() element:
         *     x.first  - pointer to the member variable
         *     x.second - the member name
         */
        template <typename TuplePair>
        void operator()(TuplePair const& x) {
            // ElemType is the pointer type pointing to DefType defined in class T
            using ElemType= typename TuplePair::first_type;
            using DefType = typename remove_const_and_pointer<ElemType>::type;
            using CppType = typename TypeInfoTrait<DefType>::CppType;
            static_assert(TypeInfoTrait<DefType>::is_vector == false,
                "SqlStatementBase::ColumnNameType: ElemType cannot be a vector type");

            // use variable's sql type decides the column type
            const std::vector<std::string>& cnames = TypeMetaData<T>::column_names();
            std::string column_name = cnames[index++];

            static constexpr SqlType sqlType = TypeInfoTrait<DefType>::sqlType;
            /**
             * Check if the SQL type is composite or composite vector.
             * Both types need to be expanded to get the nested column names.
             * For SqlType::CompositeVector type,
             * edadb::DbMap<T> needs to create its child table as primary-key-referenced table.
             */
            if constexpr ((sqlType == SqlType::Composite) || (sqlType == SqlType::CompositeVector)) {
                assert((index > 0) &&
                    "SqlStatementBase::ColumnNameType::operator(): composite type should not be the first element");

                const auto vecs = TypeMetaData<CppType>::tuple_type_pair();
                const std::string next_pref = prefix + "_" + column_name + "_";
                boost::fusion::for_each(vecs, ColumnNameType<CppType>(name, type, next_pref));
            }
            else if constexpr (sqlType == SqlType::External) {
                assert ((index > 0) &&
                    "SqlStatementBase::ColumnNameType::operator(): external type should not be the first element");

                const auto vecs = TypeMetaData<Shadow<CppType>>::tuple_type_pair();
                const std::string next_pref = prefix + "_" + column_name + "_";
                boost::fusion::for_each(
                    vecs, ColumnNameType<Shadow<CppType>>(name, type, next_pref));
            }
            else if constexpr ((!std::is_enum<CppType>::value /* SqlType::Unknown */) &&
                ((sqlType == SqlType::CompositeVector) || (sqlType == SqlType::Unknown))) {
                assert(false &&
                    "SqlStatementBase::ColumnNameType::operator(): SqlType::CompositeVector or SqlType::Unknown type should not be expanded");
            }  
            else {
                std::string sqlTypeString = getSqlTypeString<CppType>();
                type.push_back(sqlTypeString);

                // use the user defined column name
                name.push_back(prefix + column_name);
//                name.push_back(x.second); // x.second is the name of the member variable
            } // if constexpr sqlType
        } // operator()
    }; // ColumnNameType
}; // SqlStatementBase



/**
 * @struct SqlStatementImpl
 * @brief This struct is the implementation of SqlStatementBase.
 * @details This struct is the implementation of SqlStatementBase for different database backends, which is assigned by DbBackendType.
*/
template <DbBackendType DBType, typename T>
struct SqlStatementImpl : public SqlStatementBase {
    static_assert(always_false<T>::value, "DbBackendType is not supported");
};


/**
 * SqlStatement is defined in backend, such as backend/sqlite/SqlStatement4Sqlite.h
 *  using SqlStatement = std::enable_if_t< Config::backend_type == DbBackendType::SQLITE,
 *      SqlStatementImpl<DbBackendType::SQLITE>>;
 */


} // namespace edadb