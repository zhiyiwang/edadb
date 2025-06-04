/**
 * @file DbMap.hpp
 * @brief DbMap.hpp provides a way to map objects to relations.
 */

#pragma once

#include <fmt/format.h>
#include <string>
#include <stdint.h>
#include <string>

#include "MacroHelper.h"
#include "TypeMetaData.h"
#include "VecMetaData.h"
#include "Table4Class.h"
#include "DbBackendType.h"
#include "SqlStatement.h"
#include "SqlStatement4Sqlite.h"
#include "DbStatement.h"
#include "DbStatement4Sqlite.h"
#include "DbManager.h"
#include "DbManager4Sqlite.h"

namespace edadb {


/**
 * @brief DbMapBase class is the base class for all DbMap classes and manages the database connection.
 */
class DbMapBase : public Singleton<DbMapBase> {
private:
    /**
     * @brief friend class for Singleton pattern.
     */
    friend class Singleton<DbMapBase>;

protected:
    // Singleton DbManager ins to connect to database
    inline static DbManager &manager = DbManager::i();

protected:
    DbMapBase() = default;

public:
    virtual ~DbMapBase() = default;

public: 
    DbManager &getManager() { return manager; }

public:
    /**
     * @brief Initialize the backend database connection.
     * @param c The database connection string.
     * @return true if success; otherwise, false.
     */
    bool init(const std::string &c) {
        // no need to check inited, singleton manager will check it
        return manager.connect(c);
    }

    /**
     * @brief Execute the SQL statement directly.
     * @param sql The SQL statement.
     * @return true if executed; otherwise, false.
     */
    bool executeSql(const std::string &sql) {
        return manager.exec(sql);
    }

    /**
     * @brief Begin a transaction.
     * @return true if success; otherwise, false.
     */
    bool beginTransaction() {
        return manager.exec("BEGIN TRANSACTION;");
    }

    /**
     * @brief Commit a transaction.
     * @return true if success; otherwise, false.
     */
    bool commitTransaction() {
        return manager.exec("COMMIT;");
    }
}; // DbMapBase



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
        const std::string sql = fmt::format(
            SqlStatement<T>::createTableStatement(foreign_key), table_name);
        if (!manager.exec(sql)) {
            std::cerr << "DbMap::createTable: create table failed" << std::endl;
            return false;
        }

        // create child table for composite vector type 
        bool crt_tab = true;
        if constexpr (Cpp2SqlType<T>::sqlType == SqlType::CompositeVector) {
            // vector< vector<ElemT1>*, vector<ElemT2>*, ... > 
            //   each vector<ElemT>* is for a vector<ElemT> member variable 
            using VecElem = typename VecMetaData<T>::VecElem;
            VecElem seq{}; // only need type, ignore value
            boost::fusion::for_each(seq, [&](auto ptr){ 
                using PtrT = decltype(ptr);
                using Trait = VecTypeTrait<PtrT>;
                using OrgType = typename Trait::OrgType;
                if (crt_tab)
                    crt_tab = createChildTable<OrgType>();
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

        const std::string sql =
            fmt::format("DROP TABLE IF EXISTS \"{}\";", table_name);
        return manager.exec(sql);
    }


private:
    template <typename OrgT>
    bool createChildTable(void) {
        // assert OrgT is not pointer type
        static_assert(!std::is_pointer<OrgT>::value,
            "createChildTable: OrgT must not be a pointer type");

        // "this table name" + "_" + "child defined table name"
        std::string child_table_name = 
            table_name + "_" + TypeMetaData<OrgT>::table_name();

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
        DbMap<OrgT> *child_dbmap = new DbMap<OrgT>(child_table_name, fk);
        child_dbmap_vec.push_back(child_dbmap);
        return child_dbmap->createTable();
    } // createChildTable
}; // class DbMap



////////////////////////////////////////////////////////////////////////////////



// enum for DbMap operation
enum class DbMapOperation {
    NONE,
    INSERT,
    UPDATE,
    DELETE,
    SCAN,
    QUERY_PREDICATE,
    QUERY_PRIMARY_KEY,
    QUEYR_FOREIGN_KEY, 
    MAX
};


/**
 * Operation traits for DbMap operation.
 */
template <typename T, DbMapOperation OP>
struct OpTraits {
    static_assert(OP != OP, "DbMapOperation is not implemented for this operation.");
};


template <typename T>
struct OpTraits<T, DbMapOperation::INSERT> {
    static constexpr const char *name() {
        return "Insert";
    }
    static std::string getSQL(DbMap<T> &dbmap) {
        return SqlStatement<T>::insertPlaceHolderStatement(dbmap.getForeignKey());
    }
    static DbMapOperation op() {
        return DbMapOperation::INSERT;
    }
};


template <typename T>
struct OpTraits<T, DbMapOperation::UPDATE> {
    static constexpr const char *name() {
        return "Update";
    }
    static std::string getSQL(DbMap<T> &dbmap) {
        return SqlStatement<T>::updatePlaceHolderStatement(dbmap.getForeignKey());
    }
    static DbMapOperation op() {
        return DbMapOperation::UPDATE;
    }
};


template <typename T>
struct OpTraits<T, DbMapOperation::DELETE> {
    static constexpr const char *name() {
        return "Delete";
    }
    static std::string getSQL(DbMap<T> &dbmap) {
        return SqlStatement<T>::deletePlaceHolderStatement();
    }
    static DbMapOperation op() {
        return DbMapOperation::DELETE;
    }
};


template <typename T>
struct OpTraits<T, DbMapOperation::SCAN> {
    static constexpr const char *name() {
        return "Scan";
    }
    static std::string getSQL(DbMap<T> &dbmap) {
        return SqlStatement<T>::scanStatement(dbmap.getForeignKey());
    }
    static DbMapOperation op() {
        return DbMapOperation::SCAN;
    }
};


template <typename T>
struct OpTraits<T, DbMapOperation::QUERY_PREDICATE> {
    static constexpr const char *name() {
        return "QueryPredicate";
    }
    static std::string getSQL(DbMap<T> &dbmap, const std::string &pred) {
        return SqlStatement<T>::queryPredicateStatement(dbmap.getForeignKey(), pred);
    }
    static DbMapOperation op() {
        return DbMapOperation::QUERY_PREDICATE;
    }
};


template <typename T>
struct OpTraits<T, DbMapOperation::QUERY_PRIMARY_KEY> {
    static constexpr const char *name() {
        return "QueryPrimaryKey";
    }
    static std::string getSQL(DbMap<T> &dbmap) {
        return SqlStatement<T>::queryPrimaryKeyStatement(dbmap.getForeignKey());
    }
    static DbMapOperation op() {
        return DbMapOperation::QUERY_PRIMARY_KEY;
    }
};


template <typename T>
struct OpTraits<T, DbMapOperation::QUEYR_FOREIGN_KEY> {
    static constexpr const char *name() {
        return "QueryForeignKey";
    }
    static std::string getSQL(DbMap<T> &dbmap) {
        return SqlStatement<T>::queryForeignKeyStatement(dbmap.getForeignKey());
    }
    static DbMapOperation op() {
        return DbMapOperation::QUEYR_FOREIGN_KEY;
    }
};



////////////////////////////////////////////////////////////////////////////////



/**
 * @brief DbMap::Writer class is used to write objects to the database.
 * @tparam T The class type.
 */
template <typename T>
class DbMap<T>::DbStmtOp {
protected:
    /**
     * Related DbMap and DbManager to access the database:
     * DbMap contains the table name and DbManager contains the database connection.
     */
    DbMap     &dbmap;
    DbManager &manager;

    /**
     * The writer needs to insert the objects into different tables,
     * otherwise maybe use inline static thread_local variables:
     *   inline: no need to declare static
     *   static thread_local: var is shared by all instances of the thread
     */
    DbStatement dbstmt;
    uint32_t  bind_idx;

    DbMapOperation op = DbMapOperation::NONE;


protected:
    virtual ~DbStmtOp(void) = default;
    DbStmtOp(DbMap &m) : dbmap(m), manager(m.getManager()),
        bind_idx(manager.s_bind_column_begin_index)
    {
        resetBindIndex();

        if (!manager.isConnected()) {
            std::cerr << "DbMap::DbStmtOp: not inited" << std::endl;
            return;
        }
    } // DbStmtOp


public: // utility
    /**
     * @brief prepareImpl for the operation.
     * @tparam OP The operation type.
     * @return true if success, false otherwise.
     */
    template <DbMapOperation OP>
    bool prepareImpl(void) {
        return prepareImpl<OP>([&] {
            return fmt::format(OpTraits<T, OP>::getSQL(dbmap), dbmap.getTableName());
        });
    } // prepareImpl


    /**
     * @brief prepareImpl for the operation.
     * @tparam OP The operation type.
     * @tparam SqlBuilder The text sql statement builder lambda function.
     * @return true if success, false otherwise.
     */
    template <DbMapOperation OP, typename SqlBuilder>
    bool prepareImpl(SqlBuilder buildSql) {
        if (op != DbMapOperation::NONE) {
            std::cerr << "DbMap::DbStmtOp::prepareImpl ["
                << OpTraits<T, OP>::name() << "]: already prepared" << std::endl;
            return false;
        }

        if (!manager.isConnected()) {
            std::cerr << "DbMap::DbStmtOp::prepareImpl ["
                << OpTraits<T, OP>::name() << "]: not inited" << std::endl;
            return false;
        }

        if (!manager.initStatement(dbstmt)) {
            std::cerr << "DbMap::DbStmtOp::prepareImpl ["
                << OpTraits<T, OP>::name() << "]: init statement failed" << std::endl;
            return false;
        }

        const std::string sql = buildSql();
        if (!dbstmt.prepare(sql)) {
            std::cerr << "DbMap::DbStmtOp::prepareImpl ["
                << OpTraits<T, OP>::name() << "]: prepare failed" << std::endl;
            return false;
        }

        op = OpTraits<T, OP>::op();
        return true;
    } // prepareImpl


    /**
     * @brief execute the operation.
     * @param OP The operation type.
     * @param Func The function to execute, which is a lambda function.
     * @return true if success, false otherwise.
     */
    template <DbMapOperation OP, typename Func>
    bool executeImpl(Func func) {
        // check if the operation is prepared
        if (op != OP) {
            std::cerr << "DbMap::" << OpTraits<T, OP>::name() << "::executeImpl: not prepared" << std::endl;
            return false;
        }

        if (!manager.isConnected()) {
            std::cerr << "DbMap::" << OpTraits<T, OP>::name() << "::executeImpl: not inited" << std::endl;
            return false;
        }

        /**
         * lamda function to bind the object and communicate with the database 
         */
        if (!func()) {
            std::cerr << "DbMap::" << OpTraits<T, OP>::name() << "::executeImpl: bind failed" << std::endl;
            return false;
        }

        if (!dbstmt.reset()) {
            return false;
        }

        return true;
    } // executeImpl


    /**
     * @brief finalize dbstmt and reset the bind_idx
     */
    bool finalize() {
        if (!manager.isConnected()) {
            std::cerr << "DbMap::insertFinalize: not inited" << std::endl;
            return false;
        }

        op = DbMapOperation::NONE;
        return dbstmt.finalize();
    } // finalize


protected:
    /**
     * @brief reset the bind index to the begin index.
     */
    void resetBindIndex() {
        bind_idx = manager.s_bind_column_begin_index;
    } // resetBindIndex


protected:
    /**
     * @brief bind the element to the database.
     * @param elem The element pointer to bind, which is defined as a cpp type pointer.
     * @return true if success; otherwise, false.
     */
    template <typename ElemType>
    bool bindToColumn(const ElemType &elem) {
        bool ok = true;

        // extract the CppType from the ElemType pointer
        using CppType = typename
            std::remove_const<typename std::remove_pointer<ElemType>::type>::type;

        if constexpr (edadb::Cpp2SqlType<CppType>::sqlType == edadb::SqlType::Composite) {
            assert((bind_idx > 0) &&
                "DbMap<T>::Writer::bindToColumn: composite type should not be the first element");

            auto values = TypeMetaData<CppType>::getVal(elem);
            boost::fusion::for_each(
                /** param 1: Fusion Sequence, a tuple of values */
                values,

                /**
                 * param 2: Lambda function (closure) to bind the element
                 *   [this](auto const& elem): use the current class instance and the element
                 *   this->bindToColumn(elem): call bindToColumn func using current instance
                 *   hence, the bindToColumn function will be called recursively and
                 *       always use this->bind_idx to bind the element
                 */
                [this, &ok](auto const &ne) { ok = ok && this->bindToColumn(ne); });
        }
        else if constexpr (edadb::Cpp2SqlType<CppType>::sqlType == edadb::SqlType::External) {
            assert((bind_idx > 0) &&
                   "DbMap<T>::Writer::bindToColumn: external type should not be the first element");

            // transform the object of external type to Shadow
            Shadow<CppType> shadow;
            shadow.toShadow(elem);

            auto values = TypeMetaData<edadb::Shadow<CppType>>::getVal(&shadow);
            boost::fusion::for_each(
                values,
                [this, &ok](auto const &ne) { ok = ok && this->bindToColumn(ne); });
        }
        else if constexpr (std::is_enum_v<CppType>) {
            // bind the enum member to underlying type:
            //   Default underlying type is int, also can be user defined
            using U = std::underlying_type_t<CppType>;
            U tmp = static_cast<U>(*elem); // type safe cast during compile time
            ok = ok && dbstmt.bindColumn(bind_idx++, &tmp);
        }
        else {
            // bind the element to the database
            // only base type needs to be bound
            ok = ok && dbstmt.bindColumn(bind_idx++, elem);
        } // if constexpr SQLType::Composite

        return ok;
    } // bindToColumn


    /**
     * @brief bind the object to the database.
     * @tparam ParentType The parent type, default is void.
     * @param obj The object to bind.
     * @param p The parent object, if any, to bind the foreign key value.
     * @return true if success; otherwise, false.
     */
    template <typename ParentType = void>
    bool bindObject(T *obj, ParentType *p = nullptr, bool autoStep = true) {
        bool ok = true;

        // reset bind_idx to begin to bind
        resetBindIndex();

        // iterate through the non-vector members and bind them
        // @see DbMap<T>::Writer::bindToColumn for the recursive calling
        auto values = TypeMetaData<T>::getVal(obj);
        boost::fusion::for_each(
            values,
            [this, &ok](auto const &ne) { ok = ok && this->bindToColumn(ne); }
        );

        // ignore no ParentType (= void) during compile time
        // Otherwise, bind DbMap<T> foreign key value from ParentType p
        if constexpr (!std::is_same_v<ParentType, void>) {
            assert(this->dbmap.getForeignKey().valid());

            // bind the foreign key value (1st column in parent)
            auto fk_val_ptr = boost::fusion::at_c<Config::fk_ref_pk_col_index>
                    (TypeMetaData<ParentType>::getVal(p));
            ok = ok && dbstmt.bindColumn(bind_idx++, fk_val_ptr);
        } // if 


        // autoStep: run backend db step statement automatically
        // bind the this tuple before bind the child tuple referencing this primary key
        if (autoStep && !(ok = dbstmt.bindStep())) {
            std::cerr << "DbMap::Writer::bindObject: bind step failed" << std::endl;
            return ok;
        }

        // CompositeVector type: use obj as primary key to bind the child
        // constexpr to avoid compile time error
        if constexpr (Cpp2SqlType<T>::sqlType == SqlType::CompositeVector) {
            auto ve = VecMetaData<T>::getVecElem(obj);
            std::size_t vidx = 0;
            boost::fusion::for_each(
                ve,
                [&](auto ptr) {
                    ok = ok && this->bindChildVector(obj, vidx, ptr);
                } // lambda function
            ); // boost::fusion::for_each
        } // if constexpr SqlType::CompositeVector

        return ok;
    } // bindObject


    template <typename VecPtr>
    bool bindChildVector(T *obj, size_t &vidx, VecPtr ptr) {
        using PtrT = decltype(ptr);
        using Trait = VecTypeTrait<PtrT>;
        using ElemType = typename Trait::ElemType;
        using OrgType  = typename Trait::OrgType;

        auto child_dbmap_vec = this->dbmap.getChildDbMap();
        DbMap<OrgType> *child_dbmap = 
            static_cast<DbMap<OrgType> *>(child_dbmap_vec.at(vidx++));
        assert(!child_dbmap_vec.empty());
        assert(child_dbmap != nullptr);

        bool ok = true;
        typename DbMap<OrgType>::Writer child_writer(*child_dbmap);
        if constexpr (std::is_pointer_v<ElemType>) {
            // ptr is pointer to vector<ElemT*>, use it directly
            ok = child_writer.insertVector(*ptr, obj);
        } else {
            // trans vector<OrgType>* to vector<OrgType*> to call insertVector
            std::vector<OrgType *> vec_elem;
            for (auto &v : *ptr) 
                vec_elem.push_back(&v);
            ok = child_writer.insertVector(vec_elem, obj);
        } // if constexpr

        return ok;
    } // bindChildVector
}; // DbStmtOp



////////////////////////////////////////////////////////////////////////////////



// DbMap Writer: write database, include insert, update, delete
template <typename T>
class DbMap<T>::Writer : public DbStmtOp {
public:
    ~Writer() = default;
    Writer(DbMap &m) : DbStmtOp(m) {} 


public: // insert API
    template <typename ParentType = void>
    bool insertOne(T *obj, ParentType *p = nullptr) {
        return this->template prepareImpl<DbMapOperation::INSERT>()
            && this->insert(obj, p)
            && this->finalize();
    }

private:
    template <typename ParentType = void>
    bool insert(T *obj, ParentType *p = nullptr) {
        return this->template executeImpl<DbMapOperation::INSERT>(
            [&]() { return this->bindObject(obj, p); }
        );
    } // insert


public:
    template <typename ParentType = void>
    bool insertVector(std::vector<T *> &objs, ParentType *p = nullptr) {
        if (objs.empty()) {
            std::cerr << "DbMap::insertVector: empty vector" << std::endl;
            return false;
        }

        return processVector<DbMapOperation::INSERT>("DbMap::insertVector", [&]() {
            for (auto obj : objs) {
                if (!insert(obj, p)) {
                    std::cerr << "DbMap::insertVector: insert failed" << std::endl;
                    return false;
                }
            }
            return true; 
        });
    } // insertVector

private:
    /**
     * @brief process the vector of objects.
     * @tparam OP The operation type.
     * @tparam Func The function to execute, which is a lambda function.
     * @param errPrefix The error prefix.
     * @param func The function to execute, which is a lambda function.
     * @return true if success, false otherwise.
     */
    template <DbMapOperation OP, typename Func>
    bool processVector(const std::string &errPrefix, Func &&func) {
        if (!this->template prepareImpl<OP>()) {
            std::cerr << errPrefix << ": prepare failed" << std::endl;
            return false;
        }

        // lambda function to bind the object
        if (!func())
            return false;

        return this->finalize();
    } // processVector



public: // delete API: using place holder delete statement
    bool deleteOne(T *obj) {
        return this->template prepareImpl<DbMapOperation::DELETE>()
            && this->deleteOp(obj)
            && this->template finalize();
    }

    bool deleteVector(std::vector<T *> &objs) {
        if (objs.empty()) {
            std::cerr << "DbMap::deleteVector: empty vector" << std::endl;
            return false;
        }

        return processVector<DbMapOperation::DELETE>("DbMap::deleteVector", [&]() {
            for (auto obj : objs) {
                if (!deleteOp(obj)) {
                    std::cerr << "DbMap::deleteVector: delete failed" << std::endl;
                    return false;
                }
            }
            return true; 
        });
    } // deleteVector

private:
    bool deleteOp(T *obj) {
        return this->template executeImpl<DbMapOperation::DELETE>(
            [&]() {
                // bind the primary key value in the where clause using obj
                auto pk_val_ptr = boost::fusion::at_c<0>(TypeMetaData<T>::getVal(obj));
                this->dbstmt.bindColumn(this->bind_idx++, pk_val_ptr);

                return this->dbstmt.bindStep();
            } // lambda function
        ); // executeImpl
    } // deleteOp



public: // update API: using place holder update statement
    /**
     * @brief update a single object in the database.
     * @param obj The object to update.
     * @param updateChild Whether to update child objects if the object is a CompositeVector type.
     * @return true if success; otherwise, false.
     */
    bool updateOne(T *obj, bool updateChild = true) {
        // compile time check for the object type
        if constexpr (Cpp2SqlType<T>::sqlType == SqlType::CompositeVector) {
            // update composite vector type vector children
            if (updateChild) {
                // Update MULTIPLE objects in MULTIPLE tables:
                //   delete the object's original related tuples in multiple tables,
                //   then insert the object's new related tuples in multiple tables
                return deleteOne(obj) && insertOne(obj);
            }
        } // if 

        // update SINGLE object in ONE table:
        //   directly update the object
        return this->template prepareImpl<DbMapOperation::UPDATE>()
            && this->update(obj)
            && this->template finalize();
    } // updateOne


    bool updateVector(std::vector<T *> &objs, bool updateChild = true) {
        if (objs.empty()) {
            std::cerr << "DbMap::updateVector: empty vector" << std::endl;
            return false;
        }

        if constexpr (Cpp2SqlType<T>::sqlType == SqlType::CompositeVector) {
            if (updateChild) {
                return deleteVector(objs) && insertVector(objs); 
            }
        }

        return processVector<DbMapOperation::UPDATE>("DbMap::updateVector", [&]() {
            for (size_t i = 0; i < objs.size(); ++i) {
                if (!update(objs[i])) {
                    std::cerr << "DbMap::updateVector: update failed" << std::endl;
                    return false;
                }
            }
            return true;
        });
    } // updateVector

private:
    bool update(T *obj) {
        return this->template executeImpl<DbMapOperation::UPDATE>([&]() {
////            // Discard this ASSERT: Only update SqlType::CompositeVector tuple 
////            // T should not be CompositeVector type,
////            //   which means there is only one tuple to update
////            static_assert(Cpp2SqlType<T>::sqlType != SqlType::CompositeVector,
////                "T is CompositeVector, use deleteOne and insertOne instead");

            // bind obj to the database, params are:
            //   1. bindObject using obj primary key value
            //   2. ParentType is void, no foreign key value need to bind
            //   3. no auto step, because we will bind the primary key value in the where clause
            this->bindObject(obj, static_cast<void*>(nullptr), false);

            // bind the primary key value in the where clause using obj first column
            auto pk_val_ptr = boost::fusion::at_c<0>(TypeMetaData<T>::getVal(obj));
            this->dbstmt.bindColumn(this->bind_idx++, pk_val_ptr);

            return this->dbstmt.bindStep();
        });
    } // update
}; // DbMap::Writer





// DbMap Reader: read database
template <typename T>
class DbMap<T>::Reader : public DbStmtOp {
protected:
    uint32_t read_idx = 0;

public:
    ~Reader() = default;
    Reader(DbMap &m) : DbStmtOp(m) {
        resetReadIndex();
    }

public:
    /**
     * @brief prepare to read the object from the database.
     * @return true if prepared; otherwise, false.
     */
    bool prepare2Scan(void) {
        return this->template prepareImpl<DbMapOperation::SCAN>();
    } // prepare2Scan

    /**
     * @brief prepare to read the object from the database W/WO predicate.
     * @param pred The predicate to filter the object.
     * @return true if prepared; otherwise, false.
     */
    bool prepareByPredicate(const std::string &pred) {
        // need predicate to build the sql statement,
        // call prepareImpl with lambda function
        return this->template prepareImpl<DbMapOperation::QUERY_PREDICATE>(
            [&]() {
                return fmt::format(
                    SqlStatement<T>::queryPredicateStatement(this->dbmap.getForeignKey(), pred),
                    this->dbmap.getTableName()
                );
            }
        );
    } // prepareByPredicate

    /**
     * @brief prepare to read the object from the database by primary key
     * @param obj The object to read.
     * @return true if prepared; otherwise, false.
     */
    bool prepareByPrimaryKey(void) {
        return this->template prepareImpl<DbMapOperation::QUERY_PRIMARY_KEY>();
    } // prepareByPrimaryKey

    /**
     * @brief prepare to read the object from the database by foreign key
     * @param obj The object to read.
     * @return true if prepared; otherwise, false.
     */
    template <typename ParentType>
    bool prepareByForeignKey(ParentType *p) {
        bool ok =  this->template prepareImpl<DbMapOperation::QUEYR_FOREIGN_KEY>();
        if (!ok) {
            std::cerr << "DbMap::Reader::prepareByForeignKey: prepare failed" << std::endl;
            return false;
        }

        // get the foreign key value from the parent object to query as foreign key
        // read DbMap<T> foreign key value from ParentType p
        assert(this->dbmap.getForeignKey().valid());

        // get the foreign key from 1st column in parent
        auto fk_val_ptr = boost::fusion::at_c<Config::fk_ref_pk_col_index>
            (TypeMetaData<ParentType>::getVal(p)
        );
        ok = ok && this->dbstmt.bindColumn(this->bind_idx++, fk_val_ptr);

        return ok;
    } // prepareByForeignKey


public:
    /**
     * @brief read the object from the database.
     * @param obj The object to read.
     * @return true if read successfully; otherwise, false.
     */
    bool read(T *obj) {
        if (!manager.isConnected()) {
            std::cerr << "DbMap<" << typeid(T).name() << ">::Reader::"
                      << "read: not inited" << std::endl;
            return false;
        }

        return readObject(obj);
    } // read

protected:
    /** reset read_idx to begin to read */
    void resetReadIndex() {
        read_idx = manager.s_read_column_begin_index;
    }

    /**
     * @brief read the object from the database.
     * @param obj The object to read.
     * @return true if read successfully; otherwise, false.
     */
    bool readObject(T *obj) {
        // 1. reset read index to read  
        this->resetReadIndex();

        // 2. fetch the tuple using prepared statement
        if (!this->dbstmt.fetchStep()) {
            return false; // no more row
        }

        // 3. iterate to fetch the columns and read to members
        // @see DbMap<T>::Writer::fetchFromColumn for the recursive calling
        auto values = TypeMetaData<T>::getVal(obj);
        boost::fusion::for_each(
            values,
            [this](auto const &ne) { this->fetchFromColumn(ne); }
        );

        // 4. CompositeVector type: use this obj as primary key tuple to read the child
        // constexpr to avoid compile time error
        if constexpr (Cpp2SqlType<T>::sqlType == SqlType::CompositeVector) {
            std::size_t vidx = 0;
            bool ok = true;
            auto ve = VecMetaData<T>::getVecElem(obj);
            boost::fusion::for_each(
                ve,
                [&](auto ptr) {
                    // fetch the child vector when ok
                    ok = ok && fetchChildVector(obj, vidx, ptr);
                } // lambda function
            ); // for_each
        } // if 

        return true;
    } // readObject


    /**
     * @brief read the element from the database.
     * @param elem The element pointer to read, which is defined as a cpp type pointer.
     */
    template <typename ElemType>
    void fetchFromColumn(ElemType &elem) {
        // extract the CppType from the ElemType pointer
        using CppType = typename
            std::remove_const<typename std::remove_pointer<ElemType>::type>::type;

        if constexpr (edadb::Cpp2SqlType<CppType>::sqlType == edadb::SqlType::Composite) {
            assert((read_idx > 0) &&
                   "DbMap<T>::Reader::fetchFromColumn: composite type should not be the first element");

            // @see DbMap<T>::Writer::fetchFromColumn for the recursive calling
            auto values = TypeMetaData<CppType>::getVal(elem);
            boost::fusion::for_each(values,
                                    [this](auto const &ne)
                                    { this->fetchFromColumn(ne); });
        }
        else if constexpr (edadb::Cpp2SqlType<CppType>::sqlType == edadb::SqlType::External) {
            assert((read_idx > 0) &&
                   "DbMap<T>::Reader::fetchFromColumn: external type should not be the first element");

            Shadow<CppType> shadow;
            auto values = TypeMetaData<edadb::Shadow<CppType>>::getVal(&shadow);
            boost::fusion::for_each(values,
                                    [this](auto const &ne)
                                    { this->fetchFromColumn(ne); });

            // transform the object of Shadow type to original type
            shadow.fromShadow(elem);
        }
        else if constexpr (std::is_enum_v<CppType>) {
            using U = std::underlying_type_t<CppType>; // underlying type of enum
            U tmp;
            this->dbstmt.fetchColumn(read_idx++, &tmp);
            *elem = static_cast<CppType>(tmp);
        }
        else {
            // read the element from the database
            // only base type needs to be read
            this->dbstmt.fetchColumn(read_idx++, elem);
        }
    } // fetchFromColumn


    template <typename VecPtr>
    bool fetchChildVector(T *obj, size_t &vidx, VecPtr ptr) {
        using PtrT = decltype(ptr);
        using Trait = VecTypeTrait<PtrT>;
        using ElemType = typename Trait::ElemType;
        using OrgType  = typename Trait::OrgType;

        auto child_dbmap_vec = this->dbmap.getChildDbMap();
        DbMap<OrgType> *child_dbmap =
            static_cast<DbMap<OrgType> *>(child_dbmap_vec.at(vidx++));
        assert(!child_dbmap_vec.empty());
        assert(child_dbmap != nullptr);

        // create reader to read the child object
        typename DbMap<OrgType>::Reader child_reader(*child_dbmap);
        if (!child_reader.prepareByForeignKey(obj)) {
            std::cerr << "DbMap::Reader::fetchChildVector: prepareByForeignKey failed" << std::endl;
            return false;
        }

        OrgType child_obj;
        while (child_reader.read(&child_obj)) {
            if constexpr (std::is_pointer_v<ElemType>)
                // ptr point to vector<ElemT*>
                ptr->push_back(new OrgType(child_obj));
            else
                // ptr point to vector<ElemT>
                ptr->push_back(child_obj); 
        } // while

        if (!child_reader.finalize()) {
            std::cerr << "DbMap::Reader::fetchChildVector: finalize failed" << std::endl;
            return false;
        }

        return true;
    } // fetchChildVector
}; // DbMap::Reader


} // namespace edadb