///**
// * @file ConvertType.hpp
// * @brief ConvertType.hpp provides a function to convert a C++ type to a supported type.
// */
//
//#pragma once
//
//#include <string>
//#include <cstdint>
//
//namespace edadb {
//
////template<typename T>
////struct ConvertCPPTypeToSupportType {
////    using type = T;
////};
////
////template<typename T>
////struct ConvertCPPTypeToSupportType<T*> {
////    using type = T;
////};
////
////template<>
////struct ConvertCPPTypeToSupportType<float> {
////    using type = double;
////};
////
////template<>
////struct ConvertCPPTypeToSupportType<std::uint64_t> {
////    using type = unsigned long long;
////};
////
//
/////// @brief Works for all stuff where the default type conversion operator is overloaded.
////template<typename T>
////typename ConvertCPPTypeToSupportType<T>::type convertToSupportedType(T const& val) {
////    const auto ret = static_cast<typename ConvertCPPTypeToSupportType<T>::type>(val);
////    return ret;
////}
////
////std::string convertToSupportedType(char const* val) {
////    const std::string ret = val;
////    return ret;
////}
//
//} // namespace edadb