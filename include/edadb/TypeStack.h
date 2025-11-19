/**
 * @file TypeStack.h
 * @brief TypeStack.h defines a stack data structure to hold types.
 */

#pragma once

#include <iostream>
#include <typeinfo>

#include <vector>
#include <string>

#include "TypeMetaData.h"


namespace edadb {

/**
 * @struct TypeStack
 * @brief TypeStack is a stack data structure to hold types.
 * @tparam Ts The types to be held in the stack.
 */
template <typename... Ts>
struct TypeStack {};


/**
 * @struct PushBack
 * @brief PushBack adds a new type to the back of the TypeStack.
 * @tparam Stack The original TypeStack.
 * @tparam New The new type to be added.
 */
template <typename Stack, typename New>
struct PushBack;

template <typename... Ts, typename New>
struct PushBack<TypeStack<Ts...>, New> {
    using type = TypeStack<Ts..., New>;
};


/**
 * @struct PushFront
 * @brief PushFront adds a new type to the front of the TypeStack.
 * @tparam Stack The original TypeStack.
 * @tparam New The new type to be added.
 */
template <typename New, typename Stack>
struct PushFront;

template <typename New, typename... Ts>
struct PushFront<New, TypeStack<Ts...>> {
    using type = TypeStack<New, Ts...>;
};

// type alias for PushBack
template<class Stack, class T>
using PushBack_t = typename PushBack<Stack, T>::type;


/**
 * @struct PopBack
 * @brief PopBack removes the last type from the TypeStack.
 * @tparam Stack The original TypeStack.
 */
template <typename Stack>
struct PopBack;

template <>
struct PopBack<TypeStack<>> {
    using type = TypeStack<>;
};

template <typename T>
struct PopBack<TypeStack<T>> {
    using type = TypeStack<>;
};

// at least two types in the stack
// recursively pop the last type
template <typename T, typename U, typename... Ts>
struct PopBack<TypeStack<T, U, Ts...>> {
private:
  using TailPopped = typename PopBack<TypeStack<U, Ts...>>::type;
public:
  using type = typename PushFront<T, TailPopped>::type;
};


template <typename Stack>
struct Last;  

template <>
struct Last<TypeStack<>> {
    using type = void;
};

template <typename T>
struct Last<TypeStack<T>> {
    using type = T;
};

template <typename T, typename U, typename... Ts>
struct Last<TypeStack<T, U, Ts...>> {
  using type = typename Last<TypeStack<U, Ts...>>::type;
};


template <typename Stack>
struct LastTwo {
  static_assert(always_false<Stack>::value, "LastTwo expects Stack = TypeStack<...>");
};

template <>
struct LastTwo<TypeStack<>> {
  using second_last = void;
  using last        = void;
};

template <typename T>
struct LastTwo<TypeStack<T>> {
  using second_last = void;
  using last        = T;
};

template <typename T1, typename T2>
struct LastTwo<TypeStack<T1, T2>> {
  using second_last = T1;
  using last        = T2;
};

template <typename T1, typename T2, typename T3, typename... Rest>
struct LastTwo<TypeStack<T1, T2, T3, Rest...>>
    : LastTwo<TypeStack<T2, T3, Rest...>> {};





/**
 * @struct PrintStack
 * @brief PrintStack prints the types in the TypeStack.
 * @tparam Stack The TypeStack to be printed.
 */
template <typename Stack>
struct PrintStack;

template <typename... Ts>
struct PrintStack<TypeStack<Ts...>> {
    static void print() {
        // print each type in the stack
        ((std::cout << typeid(Ts).name() << " -> "), ...);
        std::cout << "END\n";
    }
};


} // namespace edadb