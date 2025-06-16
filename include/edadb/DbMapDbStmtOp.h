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
        if (!func()) {
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
     * @return true if success; otherwise, false.
     */
    template <typename ElemType>
    bool bindToColumn(const ElemType &elem) {
        bool ok = true;

        // extract the CppType from the ElemType:
        //   ElemType is a pointer type pointing to the member variable of T;
        //   DefType is the defined type of the member variable in T;
        //   CppType is the C++ type of the member variable;
        //   sqlType is the SQL type of the member variable according to Cpp2SqlType<DefType>
        using DefType = typename remove_const_and_pointer<ElemType>::type;
        using TypeTrait = TypeInfoTrait<DefType>;
        using CppType = typename TypeTrait::CppType;
        constexpr SqlType sqlType = TypeTrait::sqlType;
        CppType *cpp_val_ptr = TypeTrait::getCppPtr2Bind(elem);
        if constexpr (sqlType == SqlType::Composite) {
            assert((bind_idx > 0) &&
                "DbMakkk>::Writer::bindToColumn: composite type should not be the first element");

            auto values = TypeMetaData<CppType>::getVal(cpp_val_ptr);
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
                [this, &ok](auto const &ne) {
                    ok = ok && this->bindToColumn(ne);
                }
            ); // boost::fusion::for_each
        }
        else if constexpr (sqlType == SqlType::External) {
            assert((bind_idx > 0) &&
                   "DbMap<T>::Writer::bindToColumn: external type should not be the first element");

            // transform the object of external type to Shadow
            Shadow<CppType> shadow;
            shadow.toShadow(cpp_val_ptr);

            auto values = TypeMetaData<edadb::Shadow<CppType>>::getVal(&shadow);
            boost::fusion::for_each(
                values,
                [this, &ok](auto const &ne) {
                    ok = ok && this->bindToColumn(ne);
                }
            ); // boost::fusion::for_each
        }
        else if constexpr (std::is_enum_v<CppType>) {
            // bind the enum member to underlying type:
            //   Default underlying type is int, also can be user defined
            using U = std::underlying_type_t<CppType>;
            U tmp = static_cast<U>(*cpp_val_ptr); // type safe cast during compile time
            ok = ok && dbstmt.bindColumn(bind_idx++, &tmp);
        }
        else {
            // bind the element to the database
            // only base type needs to be bound
            ok = ok && dbstmt.bindColumn(bind_idx++, cpp_val_ptr);
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
            [this, &ok](auto const &ne) {
                ok = ok && this->bindToColumn(ne);
            }
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
        if constexpr (TypeInfoTrait<T>::sqlType == SqlType::CompositeVector) {
            auto ve = VecMetaData<T>::getVecElem(obj); // boost::fusion::vector<...>
            std::size_t vidx = 0;
            boost::fusion::for_each(
                ve,
                [&](auto ptr) { // boost::fusion::vector<ElemT>* pointer
                    ok = ok && this->bindChildVector(obj, vidx, ptr);
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

        // always be vector<ElemT>* 
        CppType *vec_ptr = TypeTrait::getCppPtr2Bind(ptr);
        using VecCppType = typename TypeTrait::VecCppType;
        auto child_dbmap_vec = this->dbmap.getChildDbMap();
        DbMap<VecCppType> *child_dbmap = 
            static_cast<DbMap<VecCppType> *>(child_dbmap_vec.at(vidx++));
        assert(!child_dbmap_vec.empty());
        assert(child_dbmap != nullptr);

        bool ok = true;
        typename DbMap<VecCppType>::Writer child_writer(*child_dbmap);
        if constexpr (TypeTrait::elemIsPointer) {
            // vec_ptr is pointer to vector<ElemT*>, use it directly
            ok = child_writer.insertVector(*vec_ptr, obj);
        } else {
            // trans vector<VecCppType>* to vector<VecCppType*> to call insertVector
            std::vector<VecCppType *> vec_elem;
            for (auto &v : *vec_ptr) 
                vec_elem.push_back(&v);
            ok = child_writer.insertVector(vec_elem, obj);
        } // if constexpr

        return ok;
    } // bindChildVector
}; // DbStmtOp


} // namespace edadb