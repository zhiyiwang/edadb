/**
 * @file DbMap.h
 * @brief DbMap.h provides a way to map objects to relations in the database.0
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <boost/fusion/include/pair.hpp> 
#include <boost/fusion/include/at_c.hpp> 
#include <boost/fusion/include/for_each.hpp>

#include "TraitUtils.h"
#include "DbMapBase.h"
#include "SqlStatement.h"
#include "backend/sqlite/SqlStatement4Sqlite.h"
#include "TypeMetaData.h"
#include "VecMetaData.h"
#include "Table4Class.h"


namespace edadb {

/**
 * @brief DbMap class maps objects to relations.
 * @tparam T The class type.
 */
template <typename T>
class DbMap : public DbMapBase {
public:
    class DbStmtOp; // DB statement operation 
    class Writer; // write object to database
    class Reader; // read object from database

protected:
    const std::string table_name; // by TypeMetaData<T> using TABLE4CLASS
    ForeignKey       foreign_key; // foreign key constraint
    std::vector<DbMapBase *> child_dbmap_vec; // vector of child DbMap

public:
    DbMap(const std::string &n = TypeMetaData<T>::table_name(),
        const ForeignKey &fk = ForeignKey()) : table_name(n), foreign_key(fk) 
    {}

    ~DbMap() {
        for (auto &dbmap : child_dbmap_vec) {
            delete dbmap; dbmap = nullptr;
        } // for 
    }

public:
    const std::string       &getTableName () { return table_name;  }
    ForeignKey              &getForeignKey() { return foreign_key; }
    std::vector<DbMapBase*> &getChildDbMap() { return child_dbmap_vec; }
    DbMapBase*getChild(size_t i) { return child_dbmap_vec.at(i); }

public:
    /**
     * @brief Create the table for the class.
     * @param crt_tab If true, the function will begin a transaction and commit it after the creation.
     * @return true if success; otherwise, false.
     */
    bool createTable(bool crt_tab = true) {
        if (!manager.isConnected()) {
            std::cerr << "DbMap::createTable: not inited" << std::endl;
            return false;
        }

        // create this table by SqlStatement<T> using this table_name
        if (crt_tab) {
            const std::string sql = 
                SqlStatement<T>::createTableStatement(table_name, foreign_key);
            if (!manager.exec(sql)) {
                std::cerr << "DbMap::createTable: create table failed" << std::endl;
                return false;
            } // if
        } 
        else if (!child_dbmap_vec.empty()) {
            // child dbmap already created,
            // no need to create child table again
            return true; 
        }


        // create child table for composite vector type 
        bool status = true;
        if constexpr (TypeInfoTrait<T>::sqlType == SqlType::CompositeVector) {
            // seq: TupTypePairType=boost::fusion::vector<BoostFusionPair1, BoostFusionPair2,...>
            // BoostFusionPairX = boost::fusion::pair<ElemType, ElemName>
            // ElemType = T** or T* (defined T* or T in class)
            // ElemName = string, the name of the element
            auto seq = VecMetaData<T>::tuple_type_pair(); 
            boost::fusion::for_each(seq, [&](auto elem){ // elem is BoostFusionPair
                using DefVecPtr = typename decltype(elem)::first_type;

                // DefVec is vector<VecElemType> or vector<VecElemType>* defined in class
                using DefVec = typename remove_const_and_pointer<DefVecPtr>::type;
                static_assert(
                    TypeInfoTrait<DefVec>::is_vector,
                    "DbMap::createTable: DefVecPtr must be a vector type"
                );

                // elem.second is the name of the member variable
                const std::string &colName = elem.second; 

                // VecElemType: VecCppType or VecCppType* as defined in Class 
                // VecCppType is non-pointer type
                using VecCppType = typename TypeInfoTrait<DefVec>::VecCppType;
                status = status && createChildTable<VecCppType>(colName, crt_tab);
            }); // for_each
        } // if 

        return status;
    } // createTable

    /**
     * @brief Initialize the DbMap, create child DbMap if necessary.
     * @return true if success; otherwise, false.
     */
    bool init(void) {
        return createTable(false);
    }
        


    /**
     * @brief Drop the table for the class.
     * @return true if success; otherwise, false.
     */
    bool dropTable() {
        if (!manager.isConnected()) {
            std::cerr << "DbMap::dropTable: not inited" << std::endl;
            return false;
        }

        const std::string sql = "DROP TABLE IF EXISTS \"" + table_name + "\";";
        return manager.exec(sql);
    }


private:
    template <typename CppType>
    bool createChildTable(const std::string &colName, bool crt_tab = true) {
        // assert CppType is not pointer type
        static_assert(!std::is_pointer_v<CppType>,
            "DbMap::createChildTable: CppType cannot be a pointer type");

        // "this table name" + "_" + "child defined table name"
        std::string child_table_name = 
            table_name + "_" + TypeMetaData<CppType>::table_name() + "_" + colName;

        // create constraint for foreign key
        ForeignKey fk;
        fk.table = table_name;

        // use the first element as the foreign key
        const uint32_t primary_key_index = Config::fk_ref_pk_col_index;

        // get primary key name and type from this class
        const auto &cols = TypeMetaData<T>::column_names();
        fk.column.push_back( cols[primary_key_index] );

        using PrimKeyType = typename boost::fusion::result_of::value_at_c<
                typename TypeMetaData<T>::TupType, primary_key_index>::type;
        fk.type.push_back( getSqlTypeString<PrimKeyType>() );


        // create child dbmap and table
        DbMap<CppType> *child_dbmap = new DbMap<CppType>(child_table_name, fk);
        child_dbmap_vec.push_back(child_dbmap);
        return child_dbmap->createTable(crt_tab);
    } // createChildTable
}; // class DbMap


} // namespace edadb