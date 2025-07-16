/**
 * @file DbMapOperationDbStmtOp.h
 * @brief DbMapOperationDbStmtOp.h defines the operations for DbMap and provides the DbStmtOp class for database operations.
 */
#pragma once

#include <string>
#include <stdint.h>

#include "TraitUtils.h"
#include "DbManager.h"
#include "DbMap.h"
#include "DbMapOperation.h"
#include "SqlStatement.h"
#include "SqlStatement4Sqlite.h"


namespace edadb {


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
        return prepareImpl<OP>([&] { return DbMapOpTrait<T, OP>::getSQL(dbmap); });
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
                << DbMapOpTrait<T, OP>::name() << "]: already prepared" << std::endl;
            return false;
        }

        if (!manager.isConnected()) {
            std::cerr << "DbMap::DbStmtOp::prepareImpl ["
                << DbMapOpTrait<T, OP>::name() << "]: not inited" << std::endl;
            return false;
        }

        if (!manager.initStatement(dbstmt)) {
            std::cerr << "DbMap::DbStmtOp::prepareImpl ["
                << DbMapOpTrait<T, OP>::name() << "]: init statement failed" << std::endl;
            return false;
        }

        const std::string sql = buildSql();
        if (!dbstmt.prepare(sql)) {
            std::cerr << "DbMap::DbStmtOp::prepareImpl ["
                << DbMapOpTrait<T, OP>::name() << "]: prepare failed" << std::endl;
            return false;
        }

        op = DbMapOpTrait<T, OP>::op();
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
            std::cerr << "DbMap::" << DbMapOpTrait<T, OP>::name()
                << "::executeImpl: not prepared" << std::endl;
            return false;
        }

        if (!manager.isConnected()) {
            std::cerr << "DbMap::" << DbMapOpTrait<T, OP>::name()
                << "::executeImpl: not inited" << std::endl;
            return false;
        }

        /**
         * lamda function to bind the object and communicate with the database 
         */
        if (func() < 0) {
            std::cerr << "DbMap::" << DbMapOpTrait<T, OP>::name()
                << "::executeImpl: bind failed" << std::endl;
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
     *     elem = nullptr only if the member (or its parent) is a pointer type and nullptr
     * @return >0 if success; 0 if the element is nullptr; -1 if error.
     */
    template <typename ElemType>
    int bindToColumn(const ElemType &elem, bool* all_nullptr) {
        int ok = 0;

        // extract the CppType from the ElemType:
        //   ElemType is a pointer type pointing to the member variable of T;
        //   DefType is the defined type of the member variable in T;
        //   CppType is the C++ type of the member variable;
        //   sqlType is the SQL type of the member variable according to Cpp2SqlType<DefType>
        using DefType = typename remove_const_and_pointer<ElemType>::type;
        using TypeTrait = TypeInfoTrait<DefType>;
        using CppType = typename TypeTrait::CppType;

        // ElemType = &DefType, DefType = CppType* or CppType
        // No matter DefType is CppType* or CppType,
        //     we always use CppType* cpp_val_ptr bind the column
        CppType *cpp_val_ptr = TypeTrait::getCppPtr2Bind(elem);
        bool is_nullptr = (cpp_val_ptr == nullptr);
        if ((!is_nullptr) && (all_nullptr != nullptr)) {
            // at least one member is not nullptr
            *all_nullptr = false;
        }
        
        // check SqlType to determine how many columns to bind
        constexpr SqlType sqlType = TypeTrait::sqlType;
        if constexpr (sqlType == SqlType::Composite) {
            assert((bind_idx > 0) &&
                "DbMap<T>::Writer::bindToColumn: composite type should not be the first element");
            
            bool comp_all_nullptr = true;
            if (is_nullptr) {
                auto values = TypeMetaData<CppType>::getVal(cpp_val_ptr);
                boost::fusion::for_each(
                    values,
                    [this, &ok, &comp_all_nullptr](auto const &ne) {
                        // bind value pointer to nullptr using bindToColumn
                        // ne point to nullptr + offset, so we need to set to nullptr
                        using SubElemType = decltype(ne);
                        SubElemType set = static_cast<SubElemType>(nullptr);
                        static_assert(std::is_pointer_v<SubElemType>,
                            "DbMap::Writer::bindToColumn: Composite type must be a pointer type"
                        ); 

                        int got = 0;
                        if (ok >= 0)
                            got = this->bindToColumn(set, &comp_all_nullptr);
                        ok = got < 0 ? got : ok;
                    }
                ); // boost::fusion::for_each
            } else {
                auto values = TypeMetaData<CppType>::getVal(cpp_val_ptr);
                boost::fusion::for_each(
                    /** param 1: Fusion Sequence, a tuple of values */
                    values,
                
                    /**
                     * param 2: Lambda function (closure) to bind the element
                     *   [this](auto const& elem): use the current class instance and the element
                     *   this->bindToColumn(elem): call bindToColumn func using current instance
                     *   imence, the bindToColumn function will be called recursively and
                     *       always use this->bind_idx to bind the element
                     */
                    [this, &ok, &comp_all_nullptr](auto const &ne) {
                        int got = 0;
                        if (ok >= 0)
                            got = this->bindToColumn(ne, &comp_all_nullptr);

                        ok = got < 0 ? got : ok + got;
                    }
                ); // boost::fusion::for_each
            }
        }
        else if constexpr (sqlType == SqlType::External) {
            assert((bind_idx > 0) &&
                   "DbMap<T>::Writer::bindToColumn: external type should not be the first element");

            bool ext_all_nullptr = true;
            Shadow<CppType> shadow;
            if (is_nullptr) {
                auto values = TypeMetaData<Shadow<CppType>>::getVal(&shadow);
                boost::fusion::for_each(
                    values,
                    [this, &ok, &ext_all_nullptr](auto const &ne) {
                        using SubElemType = decltype(ne);
                        SubElemType set = static_cast<SubElemType>(nullptr);
                        static_assert(std::is_pointer_v<SubElemType>,
                            "DbMap::Writer::bindToColumn: External type must be a pointer type"
                        );
                        int got = 0;
                        if (ok >= 0)
                            got = this->bindToColumn(set, &ext_all_nullptr);
                        ok = got < 0 ? got : ok;
                    }
                ); // boost::fusion::for_each
            } else {
                // transform the object of external type to Shadow
                shadow.toShadow(cpp_val_ptr);

                auto values = TypeMetaData<Shadow<CppType>>::getVal(&shadow);
                boost::fusion::for_each(
                    values,
                    [this, &ok, &ext_all_nullptr](auto const &ne) {
                        int got = 0;
                        if (ok >= 0)
                            got = this->bindToColumn(ne, &ext_all_nullptr);
                        ok = got < 0 ? got : ok + got;
                    }
                ); // boost::fusion::for_each
            } // if is_nullptr 
        }
        else if constexpr (std::is_enum_v<CppType>) {
            if (is_nullptr) {
                // bind nullptr to the column
                int got = dbstmt.bindNull(bind_idx++);
                ok = got < 0 ? got : ok;
            } else {
                // bind the enum member to underlying type:
                //   Default underlying type is int, also can be user defined
                using U = std::underlying_type_t<CppType>;
            
                // type safe cast during compile time
                U tmp = static_cast<U>(*cpp_val_ptr); 
                int got = dbstmt.bindColumn(bind_idx++, &tmp);
                ok = got < 0 ? got : ok + got;
            }
        }
        else {
            // bind the element to the database
            // only base type needs to be bound
            if (is_nullptr) {
                // bind nullptr to the column
                int got = dbstmt.bindNull(bind_idx++);
                ok = got < 0 ? got : ok;
            } else {
                // bind the value to the column
                int got = dbstmt.bindColumn(bind_idx++, cpp_val_ptr);
                ok = got < 0 ? got : ok + got;
            }
        } // if constexpr SQLType::Composite

        return ok;
    } // bindToColumn


    /**
     * @brief bind the object to the database.
     * @tparam ParentType The parent type, default is void.
     * @param obj The object to bind.
     * @param p The parent object, if any, to bind the foreign key value.
     * @return > 0 if success; 0 if all members are nullptr;
     *         -1 if bind step failed.
     */
    template <typename ParentType = void>
    int bindObject(T *obj, ParentType *p = nullptr, bool autoStep = true) {
        int ok = 0; 
        bool all_nullptr = true;

        // reset bind_idx to begin to bind
        resetBindIndex();

        // iterate through the non-vector members and bind them
        // @see DbMap<T>::Writer::bindToColumn for the recursive calling
        auto values = TypeMetaData<T>::getVal(obj);
        boost::fusion::for_each(
            values,
            [this, &ok, &all_nullptr](auto const &ne) {
                int got = 0; 
                if (ok >= 0)
                    got = this->bindToColumn(ne, &all_nullptr);

                // got < 0 means bind failed, skip the rest binding
                // otherwise, accumulate the bind non-nullptr count
                ok = got < 0 ? got : ok + got; 
            }
        );

        // all members are nullptr, nothing to bind
        // vector<ElemT>* members are also skipped since no primary key available
        if (all_nullptr) {
            dbstmt.clearBindings();
            dbstmt.reset();
            return 0; 
        }


        // ignore no ParentType (= void) during compile time
        // Otherwise, bind DbMap<T> foreign key value from ParentType p
        if constexpr (!std::is_same_v<ParentType, void>) {
            assert(this->dbmap.getForeignKey().valid());

            // bind the foreign key value (1st column in parent)
            auto fk_def_ptr = boost::fusion::at_c<Config::fk_ref_pk_col_index>
                (TypeMetaData<ParentType>::getVal(p));
            using DefTypePtr = decltype(fk_def_ptr);
            using DefType = typename remove_const_and_pointer<DefTypePtr>::type;
            auto fk_val_ptr = TypeInfoTrait<DefType>::getCppPtr2Bind(fk_def_ptr);
            assert(fk_val_ptr != nullptr &&
                "DbMap::Writer::bindObject: foreign key value pointer is null");
            int got = dbstmt.bindColumn(bind_idx++, fk_val_ptr);
            ok = got < 0 ? got : ok + got;
        } // if 


        // autoStep: run backend db step statement automatically
        // bind the this tuple before bind the child tuple referencing this primary key
        if (autoStep && !(ok = dbstmt.bindStep())) {
            std::cerr << "DbMap::Writer::bindObject: bind step failed" << std::endl;
            return ok;
        }

        // CompositeVector type: use obj as primary key to bind the child
        // constexpr to avoid compile time error
        if constexpr (TypeInfoTrait<T>::sqlType == SqlType::CompositeVector) {
            auto ve = VecMetaData<T>::getVecElem(obj); // boost::fusion::vector<...>
            std::size_t vidx = 0;
            boost::fusion::for_each(
                ve,
                [&](auto ptr) { // boost::fusion::vector<ElemT>* pointer
                    // if ptr pointing to nullptr pointer, skip binding
                    int got = this->bindChildVector(obj, vidx, ptr);
                    got = got < 0 ? got : ok + got;
                } // lambda function
            ); // boost::fusion::for_each
        } // if constexpr SqlType::CompositeVector

        return ok;
    } // bindObject


    template <typename DefVecPtr>
    bool bindChildVector(T *obj, size_t &vidx, DefVecPtr ptr) {
        using DefType = typename remove_const_and_pointer<DefVecPtr>::type;
        using TypeTrait = TypeInfoTrait<DefType>;
        using CppType = typename TypeTrait::CppType; // always be vector<ElemT>
        static_assert(is_vector<CppType>::value,
            "DbMap::DbStmtOp::bindChildVector: DefVecPtr must be a vector type");

        bool ok = true;
        if (TypeTrait::getCppPtr2Bind(ptr) == nullptr) {
            // if TypeTrait::is_pointer == false, ptr can never be nullptr,
            //   since it is pointing to a vector<ElemT> member defined in class
            return ok;
        } // if 

        // always be vector<ElemT>* 
        CppType *vec_ptr = TypeTrait::getCppPtr2Bind(ptr);
        using VecCppType = typename TypeTrait::VecCppType;
        auto child_dbmap_vec = this->dbmap.getChildDbMap();
        DbMap<VecCppType> *child_dbmap = 
            static_cast<DbMap<VecCppType> *>(child_dbmap_vec.at(vidx++));
        assert(!child_dbmap_vec.empty());
        assert(child_dbmap != nullptr);

        typename DbMap<VecCppType>::Writer child_writer(*child_dbmap);
        if constexpr (TypeTrait::elemIsPointer) {
            // vec_ptr is pointer to vector<ElemT*>, use it directly
            ok = child_writer.insertVector(*vec_ptr, obj);
        } else {
            // trans vectorVec<CppType>* to vector<VecCppType*> to call insertVector
            std::vector<VecCppType *> vec_elem;
            for (auto &v : *vec_ptr) 
                vec_elem.push_back(&v);
            ok = child_writer.insertVector(vec_elem, obj);
        } // if constexpr

        return ok;
    } // bindChildVector
}; // DbStmtOp


} // namespace edadb