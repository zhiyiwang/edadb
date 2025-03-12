/**
 * @file DbMap.hpp
 * @brief DbMap.hpp provides a way to map objects to relations.
 */

#pragma once

#include <fmt/format.h>
#include <string>
#include <stdint.h>

#include "SqlStatement.hpp"
#include "Sqlite3Manager.hpp"

namespace edadb {

/**
 * @brief DbMap class maps objects to relations.
 * @tparam T The class type.
 */
template<typename T>
class DbMap {

// inner class Binder to bind values to insert
struct Binder {
private:
    Sqlite3Manager &manager;
    uint32_t index{0}; 

public:
    Binder(Sqlite3Manager &m) : manager(m), index(manager.s_bind_column_begin_index) {} 

    void resetIndex() {
        index = manager.s_bind_column_begin_index;
    }


public:
    using TupType = typename TypeMetaData<T>::TupType;

    template <typename Pair>
    void operator()(const Pair& pair) const {
//        manager.bind2SQL(index++, type, bind_value);
//
        // TODO: move to sqlite3manager
        auto index = boost::fusion::at_c<0>(pair);
        auto value = boost::fusion::at_c<1>(pair);
        DbTypes type = DbTypes::kUnknown;
        void *bind_value = nullptr;
        #if DEBUG_SQLITE3_INSERT
            std::string type_str; 
        #endif
        if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::string*>) {
            type = DbTypes::kText;
            bind_value = (void*)value->c_str();
            #if DEBUG_SQLITE3_INSERT
                type_str = cppTypeEnumToDbTypeString<DbTypes::kText>();
            #endif
        } else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, int*>) {
            type = DbTypes::kInteger;
            bind_value = (void*)value;
            #if DEBUG_SQLITE3_INSERT
                type_str = cppTypeEnumToDbTypeString<DbTypes::kInteger>();
            #endif
        } else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, double*>) {
            type = DbTypes::kReal;
            bind_value = (void*)value;
            #if DEBUG_SQLITE3_INSERT
                type_str = cppTypeEnumToDbTypeString<DbTypes::kReal>();
            #endif
        } else {
            std::cout << "Binder: Unknown Type" << std::endl;
            std::cout << "Binder: " << typeid(value).name() << "value: " << *value << std::endl;
            return;
        }

        #if DEBUG_SQLITE3_INSERT
            std::cout << "Binder: index: " << index << " type: " << type_str;
            std::cout << " value: " << *value << " address: " << value << std::endl;
        #endif

        manager.bind2SQL(index, type, bind_value);
    }

    void bind(T* obj) {
//        resetIndex();
//        boost::fusion::for_each(TypeMetaData<T>::getVal(obj), *this);
//
        constexpr std::size_t N = boost::fusion::result_of::size<TupType>::type::value;
        using Indices = boost::mpl::range_c<int, 0, N>;
        auto indices = boost::fusion::as_vector(Indices{});
        auto indexed_values = boost::fusion::zip(indices, TypeMetaData<T>::getVal(obj));
        boost::fusion::for_each(indexed_values, *this);
    }
};


// inner class Fetcher to fetch values from scan
struct Fetcher {
public:
    Sqlite3Manager &manager;
    uint32_t index{0};

public:
    Fetcher(Sqlite3Manager &m) : manager(m), index(manager.s_fetch_column_begin_index) {}

    void resetIndex() {
        index = manager.s_fetch_column_begin_index;
    }


public:
    using TupType = typename TypeMetaData<T>::TupType;

    template <typename Pair>
    void operator()(const Pair& pair) const {
//        manager.fetchValueInSQL(index++, type, value);

        auto index = boost::fusion::at_c<0>(pair);
        auto value = boost::fusion::at_c<1>(pair);
        DbTypes type = DbTypes::kUnknown;
        #if DEBUG_SQLITE3_SCAN
            std::string type_str;
        #endif
        if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::string*>) {
            type = DbTypes::kText;
            void *got = nullptr;
            int size = 0;
            manager.fetchPointerInSQL(index, type, got, size);

            std::string *str = (std::string*)value;
            str->assign((const char*)got, size); 
            #if DEBUG_SQLITE3_SCAN
                type_str = cppTypeEnumToDbTypeString<DbTypes::kText>();
            #endif
        } else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, int*>) {
            type = DbTypes::kInteger;
            manager.fetchValueInSQL(index, type, (void*)value);
            #if DEBUG_SQLITE3_SCAN
                type_str = cppTypeEnumToDbTypeString<DbTypes::kInteger>();
            #endif
        } else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, double*>) {
            type = DbTypes::kReal;
            manager.fetchValueInSQL(index, type, (void*)value);
            #if DEBUG_SQLITE3_SCAN
                type_str = cppTypeEnumToDbTypeString<DbTypes::kReal>();
            #endif
        } else {
            std::cout << "Unknown Type" << std::endl;
            return;
        }

        #if DEBUG_SQLITE3_SCAN
            std::cout << "Fetcher: index: " << index << " type: " << type_str;
            std::cout << " value: " << *value << " address: " << value << std::endl;
        #endif
    }

    void fetch(T* obj) {
//        resetIndex();
//        boost::fusion::for_each(TypeMetaData<T>::getVal(obj), *this);

        constexpr std::size_t N = boost::fusion::result_of::size<TupType>::type::value;
        using Indices = boost::mpl::range_c<int, 0, N>;
        auto indices = boost::fusion::as_vector(Indices{});
        auto indexed_values = boost::fusion::zip(indices, TypeMetaData<T>::getVal(obj));
        boost::fusion::for_each(indexed_values, *this);
    }
};


public:
    std::string table_name;
    Sqlite3Manager manager;

public:
    DbMap(const std::string& c, const std::string& t) : table_name(t) {
    }
    ~DbMap() = default;

public:
    bool init(const std::string& c, const std::string& t) {
        table_name = t;
        if (!manager.connect(c)) {
            std::cerr << "DbMap::init: connect failed" << std::endl;
            return false;
        }

        return true;
    }

public:
    bool createTable() {
        const std::string sql = fmt::format(SqlStatement<T>::createTableStatement(), table_name);
        #if DEBUG_SQLITE3_API
            std::cout << "Create Table SQL: " << sql << std::endl;
        #endif
        return manager.exec(sql);
    }

public:
    bool prepare2Insert() {
        const std::string sql = fmt::format(SqlStatement<T>::insertPlaceHolderStatement(), table_name);
        #if DEBUG_SQLITE3_INSERT
            std::cout << "Insert Place Holder SQL: " << sql << std::endl;
        #endif
        return manager.prepareSQL(sql);
    }

    bool insert(T* obj) {
        #if DEBUG_SQLITE3_INSERT
            std::cout << "Insert obj address: " << obj << std::endl;
            std::cout << "obj->name: " << obj->name << " address: " << &obj->name << std::endl;
            std::cout << "obj->width: " << obj->width << " address: " << &obj->width << std::endl;
            std::cout << "obj->height: " << obj->height << " address: " << &obj->height << std::endl;
        #endif

        Binder b(manager);
        b.bind(obj);

        if (!manager.bindStep()) {
            return false;
        }

        if (!manager.resetSQL()) {
            return false;
        }

        return true;
    }


public:
    bool prepare2Scan() {
        const std::string sql = fmt::format(SqlStatement<T>::scanStatement(), table_name);
        return manager.prepareSQL(sql);
    }

    bool scan(T* obj) {
        if (!manager.fetchStep()) {
            // no more row
            return false;
        }

        Fetcher f(manager);
        f.fetch(obj);

        return true;
    }


public:
    bool finalizeSQL() {
        return manager.finalizeSQL();
    }
}; // class DbMap


} // namespace edadb