/**
 * @file DbMap.hpp
 * @brief DbMap.hpp provides a way to map objects to relations.
 */

#pragma once

#include <fmt/format.h>
#include <string>
#include <stdint.h>

#include "DbBackendType.h"
#include "SqlStatement.h"
#include "DbStatement.h"
#include "DbStatement4Sqlite.h"
#include "DbManager.h"
#include "DbManager4Sqlite.h"

namespace edadb {


/**
 * @brief DbMap class maps objects to relations.
 * @tparam T The class type.
 */
template<typename T>
class DbMap : public Singleton< DbMap<T> > {
private:
    /**
     * @brief friend class for Singleton pattern.
     */
    friend class Singleton< DbMap<T> >;

public:
    class Writer;  // write object to database
    class Reader;  // read object from database

protected:
    const std::string table_name; // defined in TypeMetaData<T> by TABLE4CLASS macro
    DbManager&        manager; // Singleton DbManager ins to connect to database
    bool   mgr_inited = false; // DbManager is inited

protected:
    /**
     * @brief protected ctor to avoid direct instantiation, use Singleton instead.
     */
    DbMap () : table_name(TypeMetaData<T>::table_name()), manager(DbManager::i()) {}

    /**
     * @brief protected dtor to avoid direct instantiation, use Singleton instead.
     */
    ~DbMap() = default;

public:
    const std::string& getTableName() { return table_name; }
    DbManager&         getManager  () { return manager   ; }
    bool               inited      () { return mgr_inited; }

public:
    bool init(const std::string& c) {
        if (mgr_inited) {
            std::cerr << "DbMap::init: already inited" << std::endl;
            return false;
        }
        
        mgr_inited = manager.connect(c);
        if (!mgr_inited) 
            std::cerr << "DbMap::init: connect failed" << std::endl;
        return mgr_inited;
    }

public:
    bool createTable() {
        if (!mgr_inited) {
            std::cerr << "DbMap::createTable: not inited" << std::endl;
            return false;
        }

        const std::string sql =
            fmt::format(SqlStatement<T>::createTableStatement(), table_name);
        return manager.exec(sql);
    }

    bool dropTable() {
        if (!mgr_inited) {
            std::cerr << "DbMap::dropTable: not inited" << std::endl;
            return false;
        }

        const std::string sql = fmt::format("DROP TABLE IF EXISTS \"{}\";", table_name);
        return manager.exec(sql);
    }

public:
    bool beginTransaction() {
        return manager.exec("BEGIN TRANSACTION;");
    }

    bool commitTransaction() {
        return manager.exec("COMMIT;");
    }
}; // class DbMap





// enum for DbMap operation
enum class DbMapOperation {
    NONE,
    INSERT,
    UPDATE,
    DELETE,
    MAX
};

// DbMap Writer: write database, include insert, update, delete
template<typename T>
class DbMap<T>::Writer {
protected:
    // reference is const pointer to DbMap
    // inline static:
    //   inline: no need to declare static
    //   static: var is shared by all instances of the class
    inline static DbMap     &dbmap = DbMap<T>::i();
    inline static DbManager &manager = dbmap.getManager();

    // inline thread_local:
    //   inline: no need to declare static
    //   static thread_local: var is shared by all instances of the thread
    inline static thread_local DbStatement dbstmt; 
    inline static thread_local uint32_t bind_idx = 0;

    inline static thread_local DbMapOperation op = DbMapOperation::NONE;

public:
    Writer () { resetBindIndex(); }
    ~Writer() = default;

public:
    /**
     * @brief finalize dbstmt and reset the bind_idx
    */
    bool finalize() {
        if (!dbmap.inited()) {
            std::cerr << "DbMap::insertFinalize: not inited" << std::endl;
            return false;
        }

        op = DbMapOperation::NONE;
        return dbstmt.finalize();
    }

public: // insert
    bool insertOne(T* obj) {
        return prepare2Insert() && insert(obj) && finalize();
    }

    bool insertVector(std::vector<T*> &objs) {
        if (objs.empty()) {
            std::cerr << "DbMap::insertVector: empty vector" << std::endl;
            return false;
        }

        if (!prepare2Insert()) {
            std::cerr << "DbMap::insertVector: prepare failed" << std::endl;
            return false;
        }

        for (auto obj : objs) {
            if (!insert(obj)) {
                std::cerr << "DbMap::insertVector: insert failed" << std::endl;
                return false;
            }
        }

        return finalize();  
    } 


public: // insert utility
    bool prepare2Insert() {
        if (op != DbMapOperation::NONE) {
            std::cerr << "DbMap::Writer::prepare2Insert: already prepared" << std::endl;
            return false;
        }

        if (!dbmap.inited()) {
            std::cerr << "DbMap::Inserter::prepare: not inited" << std::endl;
            return false;
        }

        if (!manager.initStatement(dbstmt)) {
            std::cerr << "DbMap::Inserter::prepare: init statement failed" << std::endl;
            return false;
        }

        const std::string sql =
            fmt::format(SqlStatement<T>::insertPlaceHolderStatement(), dbmap.getTableName());
        if (!dbstmt.prepare(sql)) {
            std::cerr << "DbMap::Inserter::prepare: prepare statement failed" << std::endl;
            return false;
        }

        op = DbMapOperation::INSERT;
        return true;
    }

    bool insert(T* obj) {
        if ((op == DbMapOperation::NONE) && (!prepare2Insert())) {
            std::cerr << "DbMap::Inserter::insert: prepare failed" << std::endl;
            return false;
        }
        else if (op != DbMapOperation::INSERT) {
            std::cerr << "DbMap::Inserter::insert: not prepared" << std::endl;
            return false;
        }

        if (!dbmap.inited()) {
            std::cerr << "DbMap::Inserter::insert: not inited" << std::endl;
            return false;
        }

        bindObject(obj);

        if (!dbstmt.bindStep()) {
            return false;
        }

        if (!dbstmt.reset()) {
            return false;
        }

        return true;
    }

    /**
     * @brief Operator to bind each element in obj 
     * @param elem The element pointer to bind, which is defined as a cpp type pointer.
     */
    template <typename ElemType>
    void operator()(const ElemType &elem) {
        dbstmt.bindColumn(bind_idx++, elem);
    }

private: // insert
    /** reset bind_idx to begin to bind */      
    void resetBindIndex() {
        bind_idx = manager.s_bind_column_begin_index;
    }

    /**
     * @brief bind the object to the database.
     * @param obj The object to bind.
     */
    void bindObject(T* obj) {
        // reset bind_idx to begin to bind 
        resetBindIndex();

        // iterate through the values and bind them
        auto values = TypeMetaData<T>::getVal(obj);
        boost::fusion::for_each(values, *this);
    }

public: 
    bool deleteByPrimaryKeys(T* obj) {
        if (op != DbMapOperation::NONE) {
            std::cerr << "DbMap::Deleter::deleteByPrimaryKeys: already prepared" << std::endl;
            return false;
        }

        if (!dbmap.inited()) {
            std::cerr << "DbMap::Deleter::deleteOne: not inited" << std::endl;
            return false;
        }

        const std::string sql =
            fmt::format(SqlStatement<T>::deleteStatement(obj), dbmap.getTableName());
        return manager.exec(sql);
    }

public:
    bool update(T* org_obj, T* new_obj) {
        if (op != DbMapOperation::NONE) {
            std::cerr << "DbMap::Deleter::deleteByPrimaryKeys: already prepared" << std::endl;
            return false;
        }

        if (!dbmap.inited()) {
            std::cerr << "DbMap::Updater::update: not inited" << std::endl;
            return false;
        }

        const std::string sql =
            fmt::format(SqlStatement<T>::updateStatement(org_obj, new_obj), dbmap.getTableName());
        return manager.exec(sql);
    }
}; // class Writer



// DbMap Reader: read database
template<typename T>
class DbMap<T>::Reader {
protected:
    DbMap     &dbmap;
    DbManager &manager;

    // thread local variables  
    DbStatement dbstmt; 
    uint32_t read_idx = 0;

public:
    Reader (DbMap &m) : dbmap(m), manager(m.getManager()) { resetReadIndex(); }
    ~Reader() = default;

public:
    /**
     * @brief prepare to read the object from the database W/WO predicate.
     * @param pred The predicate to filter the object.
     * @return true if prepared; otherwise, false.
     */
    bool prepareByPredicate(const std::string &pred = "") {
        if (!dbmap.inited()) {
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
    bool prepareByPrimaryKey(T* obj) {
        if (!dbmap.inited()) {
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


    bool read(T* obj) {
        if (!dbmap.inited()) {
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
        if (!dbmap.inited()) {
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
    void readObject(T* obj) {
        // reset read_idx to begin to read 
        resetReadIndex();

        // iterate through the values and read them
        auto values = TypeMetaData<T>::getVal(obj);
        boost::fusion::for_each(values, *this);
    }

public:
    /**
     * @brief Operator to read each element in obj
     *   invoked by boost::fusion::for_each @ readObject
     * @param elem The element to read from column in db row.
     */
    template <typename ElemType>
    void operator()(ElemType &elem) {
        dbstmt.fetchColumn(read_idx++, elem);
    }
};




} // namespace edadb