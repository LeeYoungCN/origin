#ifndef COMMON_TYPES_TYPE_TRAITS_H
#define COMMON_TYPES_TYPE_TRAITS_H

// 必要的标准库头文件
#include <string>
#include <string_view>
#include <type_traits>

/**
 * @brief  自定义类型特性：判断类型是否可转换为字符串（字符串类 + 数字类）
 * @note   兼容 C++17 及以上，支持：
 *         1. 字符串类：const char*、char*、std::string、std::string_view
 *         2. 数字类：int、long、long long、unsigned 系列、float、double、long double
 * @author 你的名字
 * @date   当前日期
 */

namespace origin::type_traits {  // 放入项目公共命名空间，避免命名冲突

// -------------------------- 内部辅助：移除 const/volatile/引用 修饰（C++17
// 及以上）--------------------------
template <typename T>
using remove_cvref_t = std::remove_cvref_t<T>;

// -------------------------- 子特性 1：判断是否为直接字符串类类型（可直接转换为
// std::string）--------------------------
template <typename T>
struct is_direct_string_type
    : std::integral_constant<bool, std::is_same_v<remove_cvref_t<T>, const char *> ||
                                       std::is_same_v<remove_cvref_t<T>, char *> ||
                                       std::is_same_v<remove_cvref_t<T>, std::string_view> ||
                                       std::is_same_v<remove_cvref_t<T>, std::string> > {};

/**
 * @brief  简化别名：直接字符串类型判断结果（true/false）
 * @tparam T  待判断的类型
 */
template <typename T>
constexpr bool is_direct_string_type_v = is_direct_string_type<T>::value;

// -------------------------- 子特性 2：判断是否为支持 std::to_string 的数字类型
// --------------------------
template <typename T>
struct is_numeric_type
    : std::integral_constant<bool, std::is_same_v<remove_cvref_t<T>, int> ||
                                       std::is_same_v<remove_cvref_t<T>, long> ||
                                       std::is_same_v<remove_cvref_t<T>, long long> ||
                                       std::is_same_v<remove_cvref_t<T>, unsigned int> ||
                                       std::is_same_v<remove_cvref_t<T>, unsigned long> ||
                                       std::is_same_v<remove_cvref_t<T>, unsigned long long> ||
                                       std::is_same_v<remove_cvref_t<T>, float> ||
                                       std::is_same_v<remove_cvref_t<T>, double> ||
                                       std::is_same_v<remove_cvref_t<T>, long double> > {};

/**
 * @brief  简化别名：数字类型判断结果（true/false）
 * @tparam T  待判断的类型
 */
template <typename T>
constexpr bool is_numeric_type_v = is_numeric_type<T>::value;

// -------------------------- 组合特性：判断是否为可转换为字符串的类型（字符串类 OR
// 数字类）--------------------------
template <typename T>
struct is_convertible_to_string
    : std::integral_constant<bool, is_direct_string_type_v<T> || is_numeric_type_v<T> > {};

/**
 * @brief  简化别名：可字符串化类型判断结果（true/false）【日常开发优先使用该别名】
 * @tparam T  待判断的类型
 */
template <typename T>
constexpr bool is_convertible_to_string_v = is_convertible_to_string<T>::value;

}  // namespace origin::type_traits

#endif  // COMMON_TYPES_TYPE_TRAITS_H
