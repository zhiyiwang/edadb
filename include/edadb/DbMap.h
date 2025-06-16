/**
 * @file DbMap.h
 * @brief DbMap.h provides a way to map objects to relations in the database.0
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "TraitUtils.h"
#include "DbMapBase.h"
#include "SqlStatement.h"
#include "SqlStatement4Sqlite.h"
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
    DbMapBase* getChild(size_t i) {
        return (i < child_dbmap_vec.size()) ? child_dbmap_vec[i] : nullptr;
    }

public:
    /**
     * @brief Create the table for the class.
     * @return true if success; otherwise, false.
     */
    bool createTable(void) {
        if (!manager.isConnected()) {
            std::cerr << "DbMap::createTable: not inited" << std::endl;
            return false;
        }

        // create this table by SqlStatement<T> using this table_name
        const std::string sql = 
            SqlStatement<T>::createTableStatement(table_name, foreign_key);
        if (!manager.exec(sql)) {
            std::cerr << "DbMap::createTable: create table failed" << std::endl;
            return false;
        }

        // create child table for composite vector type 
        bool crt_tab = true;
        if constexpr (TypeInfoTrait<T>::sqlType == SqlType::CompositeVector) {
            // VecElem = boost::fusion::vector<DefVecPtr1, DefVecPtr2, ...>
            // DefVecPtrX = vector<ElemT>* or vector<ElemT>**, pointing to DefVec
            //   DefVec is defined as vector<T> or vector<T>* in class
            // ElemT =  T or T*
            using VecElem = typename VecMetaData<T>::VecElem;
            VecElem seq{}; // only need type, ignore value
            boost::fusion::for_each(seq, [&](auto ptr){ 
                using DefVecPtr = decltype(ptr);

                // DefType is is a vector<VecElemType> or vector<VecElemType>*
                using DefVec = typename remove_const_and_pointer<DefVecPtr>::type;
                static_assert(
                    TypeInfoTrait<DefVec>::is_vector,
                    "DbMap::createTable: DefVecPtr must be a vector type"
                );

                // VecElemType: VecCppType or VecCppType* as defined in Class 
                // VecCppType is non-pointer type
                using VecCppType = typename TypeInfoTrait<DefVec>::VecCppType;
                if (crt_tab) {
                    crt_tab = createChildTable<VecCppType>();
                } // if
            }); // for_each
        } // if 

        return crt_tab;
    } // createTable


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
    bool createChildTable(void) {
        // assert CppType is not pointer type
        static_assert(!std::is_pointer_v<CppType>,
            "DbMap::createChildTable: CppType cannot be a pointer type");

        // "this table name" + "_" + "child defined table name"
        std::string child_table_name = 
            table_name + "_" + TypeMetaData<CppType>::table_name();

        // create constraint for foreign key
        ForeignKey fk;
        fk.table = table_name;

        // use the first element as the foreign key
        const uint32_t primary_key_index = Config::fk_ref_pk_col_index;

        // get primary key name and type from this class
        const auto &cols = TypeMetaData<T>::column_names();
        fk.column.push_back( cols[primary_key_index] );

        using PrimKeyType =
            typename boost::fusion::result_of::value_at_c<
                typename TypeMetaData<T>::TupType, primary_key_index>::type;
        fk.type.push_back( getSqlTypeString<PrimKeyType>() );


        // create child dbmap and table
        DbMap<CppType> *child_dbmap = new DbMap<CppType>(child_table_name, fk);
        child_dbmap_vec.push_back(child_dbmap);
        return child_dbmap->createTable();
    } // createChildTable
}; // class DbMap


} // namespace edadb