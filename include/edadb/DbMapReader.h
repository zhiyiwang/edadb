/**
 * @file DbMapReader.h
 * @brief DbMapReader.h defines the DbMap Reader class for reading objects from the database.
 * @note This file is part of the edadb project, which provides a way to map objects to relations in the database.
 */

#pragma once

#include <iostream>
#include <string>

#include "DbMap.h"
#include "DbMapOperation.h"
#include "DbMapDbStmtOp.h"
#include "DbMapWriter.h"


namespace edadb {


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
                return SqlStatement<T>::queryPredicateStatement(
                    this->dbmap.getTableName(),
                    this->dbmap.getForeignKey(),
                    pred
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
        if constexpr (TypeInfoTrait<T>::sqlType == SqlType::CompositeVector) {
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
        using DefType = typename remove_const_and_pointer<ElemType>::type;
        using CppType = typename TypeInfoTrait<DefType>::CppType;
        CppType *cpp_val_ptr = TypeInfoTrait<DefType>::getCppPtr2Fetch(elem);
        if constexpr (TypeInfoTrait<DefType>::is_pointer) {
            // if elem is a pointer, we need to let elem point to the cpp_val_ptr
            // ElemType &elem is DefType* or CppType**
            *elem = cpp_val_ptr; // set object pointer member to the cpp_val_ptr
        } // if 

        if constexpr (TypeInfoTrait<DefType>::sqlType == SqlType::Composite) {
            assert((read_idx > 0) &&
                   "DbMap<T>::Reader::fetchFromColumn: composite type should not be the first element");

            // @see DbMap<T>::Writer::fetchFromColumn for the recursive calling
            auto values = TypeMetaData<CppType>::getVal(cpp_val_ptr);
            boost::fusion::for_each(values,
                                    [this](auto const &ne)
                                    { this->fetchFromColumn(ne); });
        }
        else if constexpr (TypeInfoTrait<DefType>::sqlType == SqlType::External) {
            assert((read_idx > 0) &&
                   "DbMap<T>::Reader::fetchFromColumn: external type should not be the first element");

            Shadow<CppType> shadow;
            auto values = TypeMetaData<Shadow<CppType>>::getVal(&shadow);
            boost::fusion::for_each(values,
                                    [this](auto const &ne)
                                    { this->fetchFromColumn(ne); });

            // transform the object of Shadow type to original type
            shadow.fromShadow(cpp_val_ptr);
        }
        else if constexpr (std::is_enum_v<CppType>) {
            using U = std::underlying_type_t<CppType>; // underlying type of enum
            U tmp;
            this->dbstmt.fetchColumn(read_idx++, &tmp);
            *cpp_val_ptr = static_cast<CppType>(tmp);
        }
        else {
            // read the element from the database
            // only base type needs to be read
            this->dbstmt.fetchColumn(read_idx++, cpp_val_ptr);
        }
    } // fetchFromColumn


    template <typename DefVecPtr>
    bool fetchChildVector(T *obj, size_t &vidx, DefVecPtr ptr) {
//        using PtrT = decltype(ptr);
//        using Trait = VecTypeTrait<PtrT>;
//        using ElemType = typename Trait::ElemType;
//        using CppType  = typename Trait::CppType;

        using DefType = typename remove_const_and_pointer<DefVecPtr>::type; 
        using TypeTrait = TypeInfoTrait<DefType>;
        using CppType = typename TypeTrait::CppType; // always be vector<ElemT>
        static_assert(is_vector<CppType>::value,
            "DbMap::Reader::fetchChildVector: DefVecPtr must be a vector type");
        
        // always be vector<ElemT>*
        CppType *vec_ptr = TypeTrait::getCppPtr2Bind(ptr);
        using VecCppType = typename TypeTrait::VecCppType;
        auto child_dbmap_vec = this->dbmap.getChildDbMap();
        DbMap<VecCppType> *child_dbmap =
            static_cast<DbMap<VecCppType> *>(child_dbmap_vec.at(vidx++));
        assert(!child_dbmap_vec.empty());
        assert(child_dbmap != nullptr);

        // create reader to read the child object
        typename DbMap<VecCppType>::Reader child_reader(*child_dbmap);
        if (!child_reader.prepareByForeignKey(obj)) {
            std::cerr << "DbMap::Reader::fetchChildVector: prepareByForeignKey failed" << std::endl;
            return false;
        }

        VecCppType child_obj;
        while (child_reader.read(&child_obj)) {
            if constexpr (TypeTrait::elemIsPointer) 
                // ptr point to vector<ElemT*>
                ptr->push_back(new VecCppType(child_obj));
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