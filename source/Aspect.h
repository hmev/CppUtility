#pragma once

#include <type_traits>
//
//#define HAS_MEMBER(member) \
//template <typename T, typename... Ts> \
//struct has_member_##member \
//{\
//private:\
//	template <typename U> \
//	static auto Check(int) -> decltype(std::declval<U>().member(std::declval<Args>()...),\
//		std::true_type());\
//	template <typename U>\
//	static std::false_type Check(...);\
//	\
//public:\
//	enum {value = std::is_same<decltype(Check<T>(0)), std::true_type>::value }; \
//}; \
//
//
//template <typename T, typename... Ts>
//struct Aspect
//{
//	Aspect(Func& f) : m_func(std::forwawrd<Func>(f))
//	{
//	}
//
//	template <typename T>
//	typename std::enable_if<has_member_Before<T, 
//
//};