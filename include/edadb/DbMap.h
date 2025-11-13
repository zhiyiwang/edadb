/**
 * @file DbMap.h
 * @brief DbMap.h provides a way to map objects to relations in the database.0
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <typeindex>

#include <boost/fusion/include/pair.hpp> 
#include <boost/fusion/include/at_c.hpp> 
#include <boost/fusion/include/for_each.hpp>

#include "TypeStack.h"
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
    FKC this_fkc; // FKC for this table, this is the child table containing foreign key
    FKC work_fkc; // FKC for child table, this is the parent table containing primary key
    std::vector<DbMapBase *> child_dbmap_vec; // vector of child DbMap

public:
    DbMap(const ForeignKeyConstraint& fkc = ForeignKeyConstraint()) : this_fkc(fkc), work_fkc() {
        // call by edadb api
        if (this_fkc.prim_tab_name.empty()) {
            this_fkc.prim_tab_name = TypeMetaData<T>::table_name();
            this_fkc.fore_tab_name = TypeMetaData<T>::table_name();
            this_fkc.fore_tab_pref = TypeMetaData<T>::table_name();
        }

        // set fkc as my foreign key constraint(this_fkc)
        std::string tab_name = this_fkc.valid() ?
                this_fkc.fore_tab_name : TypeMetaData<T>::table_name();
        work_fkc.prim_tab_name = tab_name;
        work_fkc.fore_tab_name = tab_name;
        work_fkc.fore_tab_pref = tab_name;
    } // DbMap

    ~DbMap() {
        for (auto &dbmap : child_dbmap_vec) {
            delete dbmap; dbmap = nullptr;
        } // for 
    }

public:
    const std::string       &getTableName () { return work_fkc.prim_tab_name;  }
    ForeignKeyConstraint    &getThisForeignKey() { return this_fkc; }
    ForeignKeyConstraint    &getWorkForeignKey() { return work_fkc; }
    std::vector<DbMapBase*> &getChildDbMap() { return child_dbmap_vec; }
    DbMapBase*getChild(size_t i) { return child_dbmap_vec.at(i); }

public:
    /**
     * @brief Create the table for the class.
     * @param run_sql If true, the function will begin a transaction and commit it after the creation.
     * @return true if success; otherwise, false.
     */
    bool createTable(bool run_sql = true) {
        if (!manager.isConnected()) {
            std::cerr << "DbMap::createTable: not inited" << std::endl;
            return false;
        }

        // create this table by SqlStatement<T> using this table_name
        if (run_sql) {
            const std::string sql = 
                SqlStatement<T>::createTableStatement(this_fkc, work_fkc);
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


        bool status = true;
        // table and column name sequence to build foreign key constraint
        if constexpr (TypeInfoTrait<T>::sqlType == SqlType::CompositeVector) {
            /**
             * T is a composite vector type,
             * create child table for each element type in the vector
             */
            status = createCompositeVectorTable<T>(work_fkc, run_sql);
            if (!status) {
                std::cerr << "DbMap::createTable: createCompositeVectorTable failed" << std::endl;
                return status;
            }
        }
        else if constexpr (TypeInfoTrait<T>::sqlType == SqlType::Composite) {
            /**
             * T is a composite type,
             * recursively create child table for each nested composite vector/member type
             */
            status = createCompositeTable<T>(work_fkc, run_sql);
            if (!status) {
                std::cerr << "DbMap::createTable: createCompositeTable failed" << std::endl;
                return status;
            }
        }

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

        assert(this_fkc.fore_tab_name == work_fkc.prim_tab_name);
        const std::string sql = "DROP TABLE IF EXISTS \"" + this_fkc.fore_tab_name + "\";";
        return manager.exec(sql);
    }


private:
    /**
     * @brief Create the child table for the vector member variable.
     * @tparam ParentType The parent type in the parent-child relationship.
     * @param fkc The foreign key constraint.
     * @param run_sql If true, execute the create table SQL statement.
     * @return true if success; otherwise, false.
     */
    template <typename ParentType>
    bool createCompositeVectorTable(ForeignKeyConstraint &fkc, bool run_sql) {
        const std::string &pk_col_name = TypeMetaData<ParentType>::column_names().at(0);
        fkc.prim_col_name = fkc.getPrimaryColumnFullName(pk_col_name);

        using PkDefType = typename boost::fusion::result_of::value_at_c<
                typename TypeMetaData<ParentType>::TupType, 0 >::type;
        fkc.key_type = getSqlTypeString< typename TypeInfoTrait<PkDefType>::CppType>();

        bool status = true;
        auto seq = VecMetaData<ParentType>::tuple_type_pair();
        boost::fusion::for_each(seq, [&](auto elem){ 
            if (!status) return;

            using ChildVecPtrType = typename decltype(elem)::first_type;
            using ChildVecType = typename remove_const_and_pointer<ChildVecPtrType>::type;
            static_assert(TypeInfoTrait<ChildVecType>::is_vector,
                "DbMap::createTable4CompositeVectorMember: ChildVecPtr must be a vector type"
            );
            using ChildVecElemCppType = typename TypeInfoTrait<ChildVecType>::VecCppType;

            const std::string& pmem = elem.second; // member name

            ForeignKeyConstraint next_fkc(fkc);
            next_fkc.updatePrefix(pmem, TypeMetaData<ChildVecElemCppType>::table_name());
            next_fkc.sealForeignTableFullName();
            
            status = createChildTable<ParentType, ChildVecElemCppType>(next_fkc, run_sql);
        }); // for_each

        return status;
    } // createCompositeVectorTable


    template <typename ParentType, typename ChildType>
    bool createChildTable(ForeignKeyConstraint &fkc, bool run_sql) {
        static_assert(!std::is_pointer_v<ChildType>,
            "DbMap::createChildTable: ChildType cannot be a pointer type");

        // foreign key name, referencing primary key column name from parent table
        const uint32_t primary_key_index = Config::fk_ref_pk_col_index;
        const auto &cols = TypeMetaData<ParentType>::column_names();
        fkc.sealForeignColumnFullName(cols[primary_key_index]);
//        fkc.fore_col_name = fkc.prim_col_pref + "_" + cols[primary_key_index];

        using PKType = typename boost::fusion::result_of::value_at_c<
                typename TypeMetaData<ParentType>::TupType, primary_key_index >::type;
        fkc.key_type = getSqlTypeString< typename TypeInfoTrait<PKType>::CppType >();

        // create child dbmap and table
        // NOTE: child dbmap for vector element points to root of Composite/CompositeVector type
        DbMap<ChildType> *child_dbmap = new DbMap<ChildType>(fkc);
        child_dbmap_vec.push_back(child_dbmap);
        return child_dbmap->createTable(run_sql);
    } // createChildTable


private:
    /**
     * @brief Create the child table for the member variable.
     * @tparam ParentType The parent type in the parent-child relationship.
     * @param fkc The foreign key constraint.
     * @param run_sql If true, execute the create table SQL statement.
     * @return true if success; otherwise, false.
    */
    template <typename ParentType>
    bool createCompositeTable(ForeignKeyConstraint &fkc, bool run_sql) {
        const std::string &pk_col_name = TypeMetaData<ParentType>::column_names().at(0);
        fkc.prim_col_name = fkc.getPrimaryColumnFullName(pk_col_name);

        using PkDefType = typename boost::fusion::result_of::value_at_c<
                typename TypeMetaData<ParentType>::TupType, 0 >::type;
        fkc.key_type = getSqlTypeString< typename TypeInfoTrait<PkDefType>::CppType>();

        bool status = true;
        auto seq = TypeMetaData<ParentType>::tuple_type_pair();
        boost::fusion::for_each(seq, [&](auto elem) { 
            using DefPtrType = typename decltype(elem)::first_type;
            using DefType = typename remove_const_and_pointer<DefPtrType>::type; 
            using DefCppType = typename TypeInfoTrait<DefType>::CppType;

            /**
             * For each composite/composite vector member variable,
             * recursively create child table
             */
            static constexpr SqlType sqlType = TypeInfoTrait<DefCppType>::sqlType;
            if constexpr (sqlType == SqlType::Composite) {
                FKC next_fkc(fkc);
                next_fkc.updatePrefix(elem.second, TypeMetaData<DefCppType>::table_name());
                status = createCompositeTable<DefCppType>(next_fkc, run_sql);
            }
            else if constexpr (sqlType == SqlType::CompositeVector) {
                FKC next_fkc(fkc);
                next_fkc.updatePrefix(elem.second, TypeMetaData<DefCppType>::table_name());
                status = createCompositeTable<DefCppType>(next_fkc, run_sql);
                if (!status) return;

                status = createCompositeVectorTable<DefCppType>(next_fkc, run_sql);
            } // if-else
        }); // for_each

        return status;
    } // createCompositeTable
}; // class DbMap


} // namespace edadb