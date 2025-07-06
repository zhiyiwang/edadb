/**
 * @file DbMapWriter.h
 * @brief DbMapWriter.h defines the DbMap::Writer class for writing objects to the database.
 * @note This file is part of the edadb project, which provides a way to map objects to relations in the database.
 */

#pragma once

#include <iostream>
#include <string>

#include "DbMap.h"
#include "DbMapOperation.h"
#include "DbMapDbStmtOp.h"


namespace edadb {


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
//            std::cerr << "DbMap::insertVector: empty vector" << std::endl;
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
    int deleteOp(T *obj) {
        return this->template executeImpl<DbMapOperation::DELETE>(
            [&]() {
                // bind the primary key value in the where clause using obj
                auto pk_def_ptr = boost::fusion::at_c<0>(TypeMetaData<T>::getVal(obj));
                using DefTypePtr = decltype(pk_def_ptr);
                using DefType = typename remove_const_and_pointer<DefTypePtr>::type;
                using TypeTrait = TypeInfoTrait<DefType>;
                using CppType = typename TypeTrait::CppType;
                CppType *pk_val_ptr = TypeTrait::getCppPtr2Bind(pk_def_ptr);
                // primary key value pointer should not be null
                // if it is null, then the object is not valid for delete 
                if (pk_val_ptr == nullptr) { return 0; }  

                this->dbstmt.bindColumn(this->bind_idx++, pk_val_ptr);

                return this->dbstmt.bindStep() ? 
                    1 : -1; // return 1 if bind step success, otherwise -1
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
        if constexpr (TypeInfoTrait<T>::sqlType == SqlType::CompositeVector) {
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

        if constexpr (TypeInfoTrait<T>::sqlType == SqlType::CompositeVector) {
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
////            static_assert(TypeInfoTrait<T>::sqlType != SqlType::CompositeVector,
////                "T is CompositeVector, use deleteOne and insertOne instead");

            // bind obj to the database, params are:
            //   1. bindObject using obj primary key value
            //   2. ParentType is void, no foreign key value need to bind
            //   3. no auto step, because we will bind the primary key value in the where clause
            this->bindObject(obj, static_cast<void*>(nullptr), false);

            // bind the primary key value in the where clause using obj first column
            // point to the primary key value (defined as DefType) pointer 
            auto pk_def_ptr = boost::fusion::at_c<0>(TypeMetaData<T>::getVal(obj));
            using DefTypePtr = decltype(pk_def_ptr);
            using DefType = typename remove_const_and_pointer<DefTypePtr>::type;
            using TypeTrait = TypeInfoTrait<DefType>;
            using CppType = typename TypeTrait::CppType;
            CppType *pk_val_ptr = TypeTrait::getCppPtr2Bind(pk_def_ptr);
            // primary key value pointer should not be null
            // if it is null, then the object is not valid for update
            if (pk_val_ptr == nullptr) { return 0; }  

            this->dbstmt.bindColumn(this->bind_idx++, pk_val_ptr);
            return this->dbstmt.bindStep() ? 
                1 : -1; // return 1 if bind step success, otherwise -1
        });
    } // update
}; // DbMap::Writer


} // namespace edadb