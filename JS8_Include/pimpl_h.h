/**
 * @file pimpl.h
 * @brief An implementation-hiding utility class template leveraging the Pimpl idiom.
 *
 * This file provides a generic wrapper around a modern pointer structure to abstract 
 * compilation dependencies via a unique opaque object representation.
 *
 * @note Out-of-line member definitions and explicit destructor declarations must 
 * accompany compilation firewall boundaries to prevent template parsing of incomplete types.
 *
 * @see Herb Sutter's GotW #101 (http://herbsutter.com/gotw/_101/) for design mechanics.
 */
#ifndef PIMPL_H_HPP_
#define PIMPL_H_HPP_

#include <memory>

/**
 * @class pimpl
 * @brief Opaque implementation manager utilizing perfect forwarding of constructors.
 * 
 * Encapsulates the boilerplate required to separate an interface from its hidden, 
 * underlying implementation properties. Avoids cascading compilation unit rebuilds 
 * when modifying hidden data structures.
 *
 * @tparam T The encapsulated, incomplete internal implementation type (e.g., `widget::impl`).
 */
template <typename T> class pimpl {
  private:
    /**
     * @brief Internal smart pointer housing the lifecycle of the incomplete implementation object.
     */
    std::unique_ptr<T> m_;

  public:
    /**
     * @brief Constructs an instance of the opaque implementation object.
     * 
     * Default constructor dynamically initializes target internal block context out-of-line.
     */
    pimpl();

    /**
     * @brief Forwarding constructor that handles dynamic parameter passing.
     * 
     * Utilizes variadic template arguments to safely perfect-forward initialization properties
     * into the hidden destination instance structure.
     *
     * @tparam Args Variadic parameter packs inferred at compilation runtime.
     * @param[in,out] args Deduced universal reference parameters passed to the underlying constructor.
     */
    template <typename... Args> pimpl(Args &&... args);

    /**
     * @brief Destroys the managed implementation block.
     * 
     * Must be defined out-of-line in an implementation context where type @p T is complete,
     * otherwise compilation will fail with standard library template restrictions.
     */
    ~pimpl();

    /**
     * @brief Overloaded structure dereference arrow operator for mutable operations.
     * @return A mutable raw pointer to the underlying implementation class object @p T.
     */
    T *operator->();

    /**
     * @brief Overloaded structure dereference arrow operator for constant operations.
     * @return A constant raw pointer to the underlying implementation class object @p T.
     */
    T const *operator->() const;

    /**
     * @brief Overloaded indirection dereference operator for mutable access.
     * @return A mutable reference to the underlying implementation object.
     */
    T &operator*();

    /**
     * @brief Overloaded indirection dereference operator for constant access.
     * @return A constant reference to the underlying implementation object.
     */
    T const &operator*() const;
};

#endif // PIMPL_H_HPP_
