/**
 * @file pimpl_impl.h
 * @brief Out-of-line template definitions for the pimpl compilation firewall helper.
 *
 * This file contains the implementation details for the pimpl class template. It handles 
 * the allocation, perfect-forwarding initialization, and deallocation of the opaque type.
 *
 * @note This file must be included in the implementation file (.cpp) of the visible class 
 * where the type @p T is completely defined, rather than in the visible class's header.
 *
 * @see Herb Sutter's GotW #101 (http://herbsutter.com)
 * @author Your Name / Organization
 * @date 2026-09-13
 * @version 1.16
 */
#ifndef PIMPL_IMPL_HPP_
#define PIMPL_IMPL_HPP_

#include <utility>

/**
 * @brief Default constructor allocating a default-initialized implementation object.
 * @tparam T The encapsulated, now-complete internal implementation type.
 */
template <typename T> 
pimpl<T>::pimpl() : m_{new T{}} {}

/**
 * @brief Variadic forwarding constructor passing all parameters to the implementation object.
 * 
 * Demonstrates double-template declaration syntax: one for the class template parameters 
 * and one for the member function template parameters.
 *
 * @tparam T The encapsulated internal implementation type.
 * @tparam Args Variadic parameter types inferred at call site.
 * @param[in,out] args Universal reference parameters to forward to @p T's constructor.
 */
template <typename T>
template <typename... Args>
pimpl<T>::pimpl(Args &&...args) : m_{new T{std::forward<Args>(args)...}} {}

/**
 * @brief Destructor.
 * 
 * Safely deletes the internal opaque instance pointer. Because this definition runs in 
 * the implementation context, the full definition of @p T is available, which avoids 
 * compilation errors caused by deleting an incomplete type.
 *
 * @tparam T The encapsulated internal implementation type.
 */
template <typename T> 
pimpl<T>::~pimpl() {}

/**
 * @brief Mutable structure dereference arrow operator.
 * @tparam T The encapsulated internal implementation type.
 * @return A mutable raw pointer to the underlying implementation class object.
 */
template <typename T> 
T *pimpl<T>::operator->() { return m_.get(); }

/**
 * @brief Constant structure dereference arrow operator.
 * @tparam T The encapsulated internal implementation type.
 * @return A constant raw pointer to the underlying implementation class object.
 */
template <typename T> 
T const *pimpl<T>::operator->() const { return m_.get(); }

/**
 * @brief Mutable indirection dereference operator.
 * @tparam T The encapsulated internal implementation type.
 * @return A mutable reference to the underlying implementation object.
 */
template <typename T> 
T &pimpl<T>::operator*() { return *m_.get(); }

/**
 * @brief Constant indirection dereference operator.
 * @tparam T The encapsulated internal implementation type.
 * @return A constant reference to the underlying implementation object.
 */
template <typename T> 
T const &pimpl<T>::operator*() const { return *m_.get(); }

#endif // PIMPL_IMPL_HPP_
