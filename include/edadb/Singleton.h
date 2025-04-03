/**
 * @file Singleton.h
 * @brief Singleton.h provides a way to implement singleton pattern.
 */

 #pragma once

#include <boost/core/noncopyable.hpp>

namespace edadb {

/**
 * @class Singleton Template
 * @brief This class is a singleton pattern.
 */
template<class T>
class Singleton : public boost::noncopyable {
public:
    /**
     * get global single instance
     * @return T& The single instance.
     */
    static T &i() {
        static T _me; // local static, guarantee only initialized once
        return _me;
    }

protected: // protected ctor and dtor can be called by derived class only
    Singleton() = default;
    ~Singleton() = default;
}; 



} // namespace edadb