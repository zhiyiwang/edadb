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
     *
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
        class Writer; // write object to database
        class Reader; // read object from database

    protected:
        const std::string table_name; // def in TypeMetaData<T> by TABLE4CLASS 
        ForeignKey foreign_key; // foreign key constraint of this table
        std::vector<DbMapBase *> child_dbmap_vec; // vector of child DbMap

    public:
        DbMap(const std::string &n = TypeMetaData<T>::table_name(),
            const ForeignKey &fk = ForeignKey()) : table_name(n), foreign_key(fk) 
        {}

        ~DbMap() {
            // delete all child DbMap
            for (auto &dbmap : child_dbmap_vec) {
                delete dbmap; dbmap = nullptr;
            }
        }

    public:
        const std::string &getTableName () { return table_name;  }
        ForeignKey        &getForeignKey() { return foreign_key; }

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

            // create table by SqlStatement<T> using this table_name
            const std::string sql = fmt::format(SqlStatement<T>().createTableStatement(foreign_key), table_name);
            if (!manager.exec(sql)) {
                std::cerr << "DbMap::createTable: create table failed" << std::endl;
                return false;
            }

            // create table for composite vector
            bool crt_tab = true;
            if (Cpp2SqlType<T>::sqlType == SqlType::CompositeVector) {
                // VecMetaData elements: vector<ElemT>* 
                using VecElem = typename VecMetaData<T>::VecElem;
                VecElem seq{}; // only need type, ignore value
                boost::fusion::for_each(seq, [&](auto ptr){ // lambda func
                  using PtrT = decltype(ptr);
                  using VecT  = std::remove_const_t<std::remove_pointer_t<PtrT>>;
                  using ElemT = typename VecT::value_type;
                  crt_tab = (crt_tab && createChildTable<ElemT>());
                });
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
        template <typename ElemT>
        bool createChildTable(void) {
            // create child DbMap for each element
            std::string child_table_name =
                table_name + "_" + TypeMetaData<ElemT>::table_name();
            DbMap<ElemT> *child_dbmap = new DbMap<ElemT>(child_table_name);
            child_dbmap_vec.push_back(child_dbmap);

            // create constraint for foreign key
            ForeignKey &fk = child_dbmap->getForeignKey();
            fk.table = table_name;

            // get the first element as the foreign key
            const uint32_t primary_key_index = 0;

            // get primary key name and type from this class
            const auto &cols = TypeMetaData<T>::column_names();
            fk.column.push_back( cols[primary_key_index] );

            using PrimKeyType = typename boost::fusion::result_of
                ::value_at_c<typename TypeMetaData<T>::TupType, 
                    primary_key_index>::type;
            std::string t = getSqlTypeString<PrimKeyType>();
            fk.type.push_back(t);

            // use foreign key constraint to create child table
            return child_dbmap->createTable();
        } // createChildTable
    }; // class DbMap


    // enum for DbMap operation
    enum class DbMapOperation {
        NONE,
        INSERT,
        UPDATE,
        DELETE,
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
        static constexpr const char *name() { return "Inserter"; }
        static std::string getSQL(DbMap<T> &dbmap) {
            return SqlStatement<T>::insertPlaceHolderStatement();
        }
        static DbMapOperation op() { return DbMapOperation::INSERT; }
    };

    template <typename T>
    struct OpTraits<T, DbMapOperation::UPDATE> {
        static constexpr const char *name() {
            return "Updater";
        }
        static std::string getSQL(DbMap<T> &dbmap) {
            return SqlStatement<T>::updatePlaceHolderStatement();
        }
        static DbMapOperation op() {
            return DbMapOperation::UPDATE;
        }
    };

    template <typename T>
    struct OpTraits<T, DbMapOperation::DELETE> {
        static constexpr const char *name() {
            return "Deleter";
        }
        static std::string getSQL(DbMap<T> &dbmap) {
            return SqlStatement<T>::deletePlaceHolderStatement();
        }
        static DbMapOperation op() {
            return DbMapOperation::DELETE;
        }
    };


    // DbMap Writer: write database, include insert, update, delete
    template <typename T>
    class DbMap<T>::Writer {
    protected:
        /**
         * Writer related DbMap and DbManager to access the database:
         * DbMap contains the table name and DbManager contains the database connection.
         */
        DbMap &dbmap;
        DbManager &manager;

        /**
         * The writer needs to insert the objects into different tables,
         * otherwise maybe use inline static thread_local variables:
         *   inline: no need to declare static
         *   static thread_local: var is shared by all instances of the thread
         */
        DbStatement dbstmt;
        uint32_t bind_idx;

        DbMapOperation op = DbMapOperation::NONE;

    public:
        ~Writer() = default;
        Writer(DbMap &m) : dbmap(m), manager(m.getManager()),
                    bind_idx(manager.s_bind_column_begin_index) {
            resetBindIndex();

            if (!manager.isConnected()) {
                std::cerr << "DbMap::Writer: not inited" << std::endl;
                return;
            }
        }

    public: // utility
        /**
         * @brief prepare2 for the operation.
         * @tparam OP The operation type.
         * @return true if success, false otherwise.
         */
        template <DbMapOperation OP>
        bool prepare2() {
            if (op != DbMapOperation::NONE) {
                std::cerr << "DbMap::Writer::prepare2: already prepared" << std::endl;
                return false;
            }

            if (!manager.isConnected()) {
                std::cerr << "DbMap::Writer::prepare2: not inited" << std::endl;
                return false;
            }

            if (!manager.initStatement(dbstmt)) {
                std::cerr << "DbMap::Writer::prepare2: init statement failed" << std::endl;
                return false;
            }

            const std::string sql =
                fmt::format(OpTraits<T, OP>::getSQL(dbmap), dbmap.getTableName());
            if (!dbstmt.prepare(sql)) {
                std::cerr << "DbMap::Writer::prepare2: prepare statement failed" << std::endl;
                return false;
            }

            op = OpTraits<T, OP>::op();
            return true;
        } // prepare2

        /**
         * @brief execute the operation.
         * @param OP The operation type.
         * @param Func The function to execute, which is a lambda function.
         * @return true if success, false otherwise.
         */
        template <DbMapOperation OP, typename Func>
        bool executeOp(Func func) {
            // check if the operation is prepared
            if ((op == DbMapOperation::NONE) && (!prepare2<OP>())) {
                std::cerr << "DbMap::" << OpTraits<T, OP>::name() << "::executeOp: prepare failed" << std::endl;
                return false;
            }
            else if (op != OP) {
                std::cerr << "DbMap::" << OpTraits<T, OP>::name() << "::executeOp: not prepared" << std::endl;
                return false;
            }

            if (!manager.isConnected()) {
                std::cerr << "DbMap::" << OpTraits<T, OP>::name() << "::executeOp: not inited" << std::endl;
                return false;
            }

            // lamda function to bind the object
            func();

            if (!dbstmt.bindStep()) {
                return false;
            }

            if (!dbstmt.reset()) {
                return false;
            }

            return true;
        } // executeOp

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
            if (!prepare2<OP>()) {
                std::cerr << errPrefix << ": prepare failed" << std::endl;
                return false;
            }

            // lambda function to bind the object
            if (!func())
                return false;

            return finalize();
        } // processVector

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

    public: // insert API
        bool insertOne(T *obj) {
            return prepare2<DbMapOperation::INSERT>() && insert(obj) && finalize();
        }

        bool insertVector(std::vector<T *> &objs) {
            if (objs.empty()) {
                std::cerr << "DbMap::insertVector: empty vector" << std::endl;
                return false;
            }

            return processVector<DbMapOperation::INSERT>("DbMap::insertVector", [&]() {
                for (auto obj : objs) {
                    if (!insert(obj)) {
                        std::cerr << "DbMap::insertVector: insert failed" << std::endl;
                        return false;
                    }
                }
                return true; 
            });
        } // insertVector

        bool insert(T *obj) {
            return executeOp<DbMapOperation::INSERT>([&]() { bindObject(obj); });
        } // insert

        /**
         * @brief Use operator() to bind each element in obj
         * @param elem The element pointer to bind, which is defined as a cpp type pointer.
         */
        template <typename ElemType>
        void operator()(const ElemType &elem) {
            bindToColumn(elem);
        }

        /**
         * @brief bind the element to the database.
         * @param elem The element pointer to bind, which is defined as a cpp type pointer.
         */
        template <typename ElemType>
        void bindToColumn(const ElemType &elem) {
            // extract the CppType from the ElemType pointer
            using CppType = typename std::remove_const<typename std::remove_pointer<ElemType>::type>::type;

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
                    [this](auto const &ne)
                    { this->bindToColumn(ne); });
            }
            else if constexpr (edadb::Cpp2SqlType<CppType>::sqlType == edadb::SqlType::External) {
                assert((bind_idx > 0) &&
                       "DbMap<T>::Writer::bindToColumn: external type should not be the first element");

                Shadow<CppType> shadow;
                // transform the object of external type to Shadow
                shadow.toShadow(elem);

                auto values = TypeMetaData<edadb::Shadow<CppType>>::getVal(&shadow);
                boost::fusion::for_each(
                    values,
                    [this](auto const &ne)
                    { this->bindToColumn(ne); });
            }
            else if constexpr (std::is_enum_v<CppType>) {
                // bind the enum member to underlying type:
                //   Default underlying type is int, also can be user defined
                using U = std::underlying_type_t<CppType>;
                U tmp = static_cast<U>(*elem); // type safe cast during compile time
                dbstmt.bindColumn(bind_idx++, &tmp);
            }
            else {
                // bind the element to the database
                // only base type needs to be bound
                dbstmt.bindColumn(bind_idx++, elem);
            }
        } // bindToColumn

    private: // utility
        /** reset bind_idx to begin to bind */
        void resetBindIndex() {
            bind_idx = manager.s_bind_column_begin_index;
        }

        /**
         * @brief bind the object to the database.
         * @param obj The object to bind.
         */
        void bindObject(T *obj) {
            // reset bind_idx to begin to bind
            resetBindIndex();

            // iterate through the values and bind them
            // @see DbMap<T>::Writer::bindToColumn for the recursive calling
            auto values = TypeMetaData<T>::getVal(obj);
            boost::fusion::for_each(values,
                                    [this](auto const &ne)
                                    { this->bindToColumn(ne); });
        } // bindObject

    public: // Delete API: using text delete statement
        bool deleteByPrimaryKeys(T *obj) {
            if (op != DbMapOperation::NONE) {
                std::cerr << "DbMap::Deleter::deleteByPrimaryKeys: already prepared" << std::endl;
                return false;
            }

            if (!manager.isConnected()) {
                std::cerr << "DbMap::Deleter::deleteOne: not inited" << std::endl;
                return false;
            }

            const std::string sql =
                fmt::format(SqlStatement<T>::deleteStatement(obj), dbmap.getTableName());
            return manager.exec(sql);
        } // deleteByPrimaryKeys

    public: // delete API: using place holder delete statement
        bool deleteOne(T *obj) {
            return prepare2<DbMapOperation::DELETE>() && deleteOp(obj) && finalize();
        }

        bool deleteOp(T *obj) {
            return executeOp<DbMapOperation::DELETE>([&]()
                                                     {
            // bind the primary key value in the where clause using obj
            auto pk_val_ptr = boost::fusion::at_c<0>(TypeMetaData<T>::getVal(obj));
            dbstmt.bindColumn(bind_idx++, pk_val_ptr); });
        } // deleteOne

    public: // update API: using text update statement
        bool updateBySqlStmt(T *org_obj, T *new_obj) {
            if (op != DbMapOperation::NONE) {
                std::cerr << "DbMap::Updater::update: already prepared" << std::endl;
                return false;
            }

            if (!manager.isConnected()) {
                std::cerr << "DbMap::Updater::update: not inited" << std::endl;
                return false;
            }

            const std::string sql =
                fmt::format(SqlStatement<T>::updateStatement(org_obj, new_obj), dbmap.getTableName());
            return manager.exec(sql);
        } // updateBySqlStmt

    public: // update API: using place holder update statement
        bool updateOne(T *org_obj, T *new_obj) {
            return prepare2<DbMapOperation::UPDATE>() && update(org_obj, new_obj) && finalize();
        }

        bool updateVector(std::vector<T *> &org_objs, std::vector<T *> &new_objs) {
            if (org_objs.empty() || new_objs.empty()) {
                std::cerr << "DbMap::updateVector: empty vector" << std::endl;
                return false;
            }
            if (org_objs.size() != new_objs.size()) {
                std::cerr << "DbMap::updateVector: size mismatch" << std::endl;
                return false;
            }

            return processVector<DbMapOperation::UPDATE>("DbMap::updateVector", [&]() {
                for (size_t i = 0; i < org_objs.size(); ++i) {
                    if (!update(org_objs[i], new_objs[i])) {
                        std::cerr << "DbMap::updateVector: update failed" << std::endl;
                        return false;
                    }
                }
                return true;
            });
        } // updateVector

    public:
        bool update(T *org_obj, T *new_obj) {
            return executeOp<DbMapOperation::UPDATE>([&]() {
                // bind new_obj to the database
                bindObject(new_obj);

                // bind the primary key value in the where clause using org_obj
                auto pk_val_ptr = boost::fusion::at_c<0>(TypeMetaData<T>::getVal(org_obj));
                dbstmt.bindColumn(bind_idx++, pk_val_ptr);
            });
        } // update
    }; // class Writer


    // DbMap Reader: read database
    template <typename T>
    class DbMap<T>::Reader {
    protected:
        DbMap &dbmap;
        DbManager &manager;

        // thread local variables
        DbStatement dbstmt;
        uint32_t read_idx = 0;

    public:
        Reader(DbMap &m) : dbmap(m), manager(m.getManager()) { resetReadIndex(); }
        ~Reader() = default;

    public:
        /**
         * @brief prepare to read the object from the database W/WO predicate.
         * @param pred The predicate to filter the object.
         * @return true if prepared; otherwise, false.
         */
        bool prepareByPredicate(const std::string &pred = "") {
            if (!manager.isConnected()) {
                std::cerr << "DbMap<" << typeid(T).name() << ">::Reader::"
                          << "prepare2Scan: not inited" << std::endl;
                return false;
            }

            if (!manager.initStatement(dbstmt)) {
                std::cerr << "DbMap<" << typeid(T).name() << ">::Reader::"
                          << "prepare2Scan: init statement failed" << std::endl;
                return false;
            }

            std::string sql =
                fmt::format(SqlStatement<T>::scanStatement(), dbmap.getTableName());
            sql += (pred.empty() ? "" : " WHERE " + pred + ";");

            if (!dbstmt.prepare(sql)) {
                std::cerr << "DbMap<" << typeid(T).name() << ">::Reader::"
                          << "prepare2Scan: prepare statement failed" << std::endl;
                return false;
            }

            return true;
        }

        /**
         * @brief prepare to read the object from the database by primary key
         * @param obj The object to read.
         * @return true if prepared; otherwise, false.
         */
        bool prepareByPrimaryKey(T *obj) {
            if (!manager.isConnected()) {
                std::cerr << "DbMap<" << typeid(T).name() << ">::Reader::"
                          << "prepare: not inited" << std::endl;
                return false;
            }

            if (!manager.initStatement(dbstmt)) {
                std::cerr << "DbMap<" << typeid(T).name() << ">::Reader::"
                          << "prepare: init statement failed" << std::endl;
                return false;
            }

            std::string sql =
                fmt::format(SqlStatement<T>::lookupStatement(obj), dbmap.getTableName());
            if (!dbstmt.prepare(sql)) {
                std::cerr << "DbMap<" << typeid(T).name() << ">::Reader::"
                          << "prepare: prepare statement failed" << std::endl;
                return false;
            }

            return true;
        }

    public:
        bool read(T *obj) {
            if (!manager.isConnected()) {
                std::cerr << "DbMap<" << typeid(T).name() << ">::Reader::"
                          << "read: not inited" << std::endl;
                return false;
            }

            if (!dbstmt.fetchStep()) {
                return false; // no more row
            }

            readObject(obj);
            return true;
        }

        bool finalize() {
            if (!manager.isConnected()) {
                std::cerr << "DbMap::scanFinalize: not inited" << std::endl;
                return false;
            }

            return dbstmt.finalize();
        }

    private:
        /** reset read_idx to begin to read */
        void resetReadIndex() {
            read_idx = manager.s_read_column_begin_index;
        }

        /**
         * @brief read the object from the database.
         * @param obj The object to read.
         */
        void readObject(T *obj) {
            // reset read_idx to begin to read
            resetReadIndex();

            // iterate through the values and read them
            // @see DbMap<T>::Writer::fetchFromColumn for the recursive calling
            auto values = TypeMetaData<T>::getVal(obj);
            boost::fusion::for_each(values,
                [this](auto const &ne) { this->fetchFromColumn(ne); }
            );
        }

    public:
        /**
         * @brief Using operator() to read each element in obj
         *   invoked by boost::fusion::for_each @ readObject
         * @param elem The element to read from column in db row.
         */
        template <typename ElemType>
        void operator()(ElemType &elem) {
            fetchFromColumn(elem);
        }

        /**
         * @brief read the element from the database.
         * @param elem The element pointer to read, which is defined as a cpp type pointer.
         */
        template <typename ElemType>
        void fetchFromColumn(ElemType &elem) {
            // extract the CppType from the ElemType pointer
            using CppType = typename std::remove_const<typename std::remove_pointer<ElemType>::type>::type;

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
                dbstmt.fetchColumn(read_idx++, &tmp);
                *elem = static_cast<CppType>(tmp);
            }
            else {
                // read the element from the database
                // only base type needs to be read
                dbstmt.fetchColumn(read_idx++, elem);
            }
        } // fetchFromColumn
    };
} // namespace edadb