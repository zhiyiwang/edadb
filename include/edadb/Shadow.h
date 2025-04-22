/**
 * @file Shadow.h
 * @brief Shadow.h provides a way to define a shadow class for the private member of a class.
 * @details The shadow class is used to define a class that is not defined in the database.
 */

#pragma once

namespace edadb {

/**
 * @brief Shadow class is used to define a shadow class for the private member of a class.
 * @details The shadow class is used to define a class that is not defined in the database.
 * 
 * @example An Example of Shadow class
 * class ExternalClass {
 * private:
 *     int private_field;
 * public:
 *     int get() const { return private_field; }
 *     void set(int val) { private_field = val; }
 * };
 * 
 * template<>
 * class Shadow<ExternalClass>{
 * public:
 *    int shadow_field;
 * public:
 *    void fromShadow(ExternalClass* obj){
 *        obj->set(shadow_field);
 *    }
 *    void toShadow(ExternalClass* obj){
 *        shadow_field = obj->get();
 *    }
 * };
 * 
 * ExternalClass ec;
 * 
 * // Define a shadow class for private member of ExternalClass, 
 * // which will define edadb::shadow<CLASSNAME> using macro TABLE4CLASS for TypeMetaData.
 * Table4ExternalClass(ExternalClass, (shadow_field));
 */
template<typename T>
class Shadow {
public:
    virtual void fromShadow(T* obj);
    virtual void toShadow(T* obj);
};

} // namespace edadb