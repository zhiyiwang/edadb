/**
 * @file ObjectRelationMapper.hpp
 * @brief ObjectRelationMapper.hpp provides a way to map objects to relations.
 */

#pragma once

#include <fmt/format.h>
#include <string>

#include "SqlStatement.hpp"
#include "Sqlite3Manager.hpp"

namespace edadb {


template<typename T>
struct ValueBinder {
public:
    Sqlite3Manager &manager;

public:
    ValueBinder(Sqlite3Manager &m) : manager(m) {}

public:
    using TupType = typename TypeMetaData<T>::TupType;

    template <typename Pair>
    void operator()(const Pair& pair) const {
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
            std::cout << "ValueBinder: Unknown Type" << std::endl;
            std::cout << "ValueBinder: " << typeid(value).name() << "value: " << *value << std::endl;
            return;
        }

        #if DEBUG_SQLITE3_INSERT
            std::cout << "ValueBinder: index: " << index << " type: " << type_str;
            std::cout << " value: " << *value << " address: " << value << std::endl;
        #endif

        manager.bind2SQL(index, type, bind_value);
    }

    void bind(T* obj) {
        constexpr std::size_t N = boost::fusion::result_of::size<TupType>::type::value;
        using Indices = boost::mpl::range_c<int, 0, N>;
        auto indices = boost::fusion::as_vector(Indices{});
        auto indexed_values = boost::fusion::zip(indices, TypeMetaData<T>::getVal(obj));
        boost::fusion::for_each(indexed_values, *this);
    }
};


template<typename T>
struct ValueColumn {
public:
    Sqlite3Manager &manager;

public:
    ValueColumn(Sqlite3Manager &m) : manager(m) {}

public:
    using TupType = typename TypeMetaData<T>::TupType;

    template <typename Pair>
    void operator()(const Pair& pair) const {
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
            manager.columnPointerInSQL(index, type, got, size);

            std::string *str = (std::string*)value;
            str->assign((const char*)got, size); 
            #if DEBUG_SQLITE3_SCAN
                type_str = cppTypeEnumToDbTypeString<DbTypes::kText>();
            #endif
        } else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, int*>) {
            type = DbTypes::kInteger;
            manager.columnValueInSQL(index, type, (void*)value);
            #if DEBUG_SQLITE3_SCAN
                type_str = cppTypeEnumToDbTypeString<DbTypes::kInteger>();
            #endif
        } else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, double*>) {
            type = DbTypes::kReal;
            manager.columnValueInSQL(index, type, (void*)value);
            #if DEBUG_SQLITE3_SCAN
                type_str = cppTypeEnumToDbTypeString<DbTypes::kReal>();
            #endif
        } else {
            std::cout << "Unknown Type" << std::endl;
            return;
        }

        #if DEBUG_SQLITE3_SCAN
            std::cout << "ValueColumn: index: " << index << " type: " << type_str;
            std::cout << " value: " << *value << " address: " << value << std::endl;
        #endif
    }

    void column(T* obj) {
        constexpr std::size_t N = boost::fusion::result_of::size<TupType>::type::value;
        using Indices = boost::mpl::range_c<int, 0, N>;
        auto indices = boost::fusion::as_vector(Indices{});
        auto indexed_values = boost::fusion::zip(indices, TypeMetaData<T>::getVal(obj));
        boost::fusion::for_each(indexed_values, *this);
    }
};




/**
 * @brief ObjectRelationMapper provides a way to map objects to relations.
 * @tparam T The class type.
 */
template<typename T>
class ObjectRelationMapper {
public:
    Sqlite3Manager manager;
    std::string table_name;


public:
    ObjectRelationMapper(const std::string& c, const std::string& t) : manager(), table_name(t) {
        manager.connect(c);
    }
    ~ObjectRelationMapper() = default;

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

        ValueBinder<T> binder(manager);
        binder.bind(obj);

        if (!manager.stepInsertSQL()) {
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
        if (!manager.stepColumnSQL()) {
            // no more row
            return false;
        }

        ValueColumn<T> cols(manager);
        cols.column(obj);

        return true;
    }


public:
    bool finalizeSQL() {
        return manager.finalizeSQL();
    }
};


} // namespace edadb