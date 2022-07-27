#ifndef CPPMETA_H
#define CPPMETA_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdexcept>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
#include <typeinfo>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning (disable: 4913)
#endif

#ifndef NULL
#define CPPMETA_NULL 0
#else
#define CPPMETA_NULL NULL
#endif

namespace cppmeta
{
    namespace detail
    {
        struct void_type {};
        
        template<int>
        class disabled {};
    }

    struct reflection
    {
        enum {
            compile_time,
            run_time
        };
    };

    template<class T = detail::void_type, int Reflection = reflection::run_time>
    struct reflect;

    template<class T>
    struct reflect<T, reflection::compile_time>
    {
        template<class meta>
        static void info() {}
    };

    namespace detail
    {
        template<class T>
        struct reflect_ct_helper
            : cppmeta::reflect<T, reflection::compile_time>
        {
            using cppmeta::reflect<T, reflection::compile_time>::info;
            template<class meta>
            static void call(meta&)
            {
                return cppmeta::reflect<T, reflection::compile_time>::template info<meta>();
            }
        };
    }

    template<class T>
    struct reflect_ct;

    template<class T>
    struct reflect_rt;

    template<class T = detail::void_type, int Reflection = reflection::run_time>
    struct resolve;

    namespace type_traits
    {
        template<class T>
        struct is_const 
        {
            static const bool value = false;
        };

        template<class T>
        struct is_const <const T>
        {
            static const bool value = true;
        };

        template<class T>
        struct add_const
        {
            typedef const T type;
        };

        template <bool, class IfTrueT, class IfFalseT>
        struct conditional
        {
            typedef IfTrueT type;
        };

        template <class IfTrueT, class IfFalseT>
        struct conditional<false, IfTrueT, IfFalseT>
        {
            typedef IfFalseT type;
        };

        template< class T >
        struct remove_reference
        {
            typedef T type;
        };

        template<class T>
        struct remove_const
        {    // remove top level const qualifier
            typedef T type;
        };

        template<class T>
        struct remove_const<const T>
        {    // remove top level const qualifier
            typedef T type;
        };

        template<class T>
        struct remove_const<const volatile T>
        {    // remove top level const qualifier
            typedef volatile T type;
        };

        // remove_volatile
        template<class T>
        struct remove_volatile
        {    // remove top level volatile qualifier
            typedef T type;
        };

        template<class T>
        struct remove_volatile<volatile T>
        {    // remove top level volatile qualifier
            typedef T type;
        };

        // remove_cv
        template<class T>
        struct remove_cv
        {    // remove top level const and volatile qualifiers
            typedef typename remove_const<typename remove_volatile<T>::type>::type
                type;
        };

        template< class T >
        struct remove_reference<T&>
        {
            typedef T type;
        };

        template<class T, class>
        struct remove_pointer_helper
        {
            typedef T     type;
        };

        template<class T, class U>
        struct remove_pointer_helper<T, U*>
        {
            typedef U     type;
        };

        // remove_pointer
        template<class T>
        struct remove_pointer
            : public remove_pointer_helper<T, typename remove_cv<T>::type>
        { };

        namespace detail
        {
            typedef char yes_type;
            struct no_type
            {
                char d[8];
            };
        }

        template<class T>
        T& declref();


        struct any { template<class T> any(const T&) {} };
        template<class>
        struct templated_any { template<class T> templated_any(const T&) {} };

        struct no_to_from_conversion {};
        struct has_to_from_conversion {};

        no_to_from_conversion operator,(no_to_from_conversion, has_to_from_conversion);

        template<class T>
        detail::yes_type has_conversion_tester(T);
        detail::no_type has_conversion_tester(no_to_from_conversion);

        template<class U>
        detail::yes_type is_simple_type_tester(void (U::*)());
        template<class U>
        detail::no_type is_simple_type_tester(...);

        template<class T>
        struct is_simple_type
        {
            static const bool value = 
                sizeof(is_simple_type_tester<T>(0)) == sizeof(detail::no_type);
        };

        template<>
        struct is_simple_type<void>
        {
            static const bool value = true;
        };

        template<class, class>
        struct is_same
        {
            static const bool value = false;
        };

        template<class T>
        struct is_same<T, T>
        {
            static const bool value = true;
        };

        template<class>
        struct is_reference
        {
            static const bool value = false;
        };

        template<class T>
        struct is_reference<T&>
        {
            static const bool value = true;
        };

        template<class>
        struct is_pointer
        {
            static const bool value = false;
        };

        template<class T>
        struct is_pointer<T*>
        {
            static const bool value = true;
        };

        template<unsigned N> struct priority_tag : priority_tag < N - 1 > {};
        template<> struct priority_tag<0> {};

        template<class T>
        detail::yes_type is_convertable_tester(T, priority_tag<1>);
        template<class T>
        detail::no_type is_convertable_tester(any, priority_tag<0>);

        template<class FromT, class ToT>
        struct is_convertable
        {
            static const bool value =
                sizeof(is_convertable_tester<ToT>(declref<FromT>(), priority_tag<1>())) == sizeof(detail::yes_type);
        };

        template<class T>
        struct is_function
        {
            static const bool value =
                is_convertable<typename remove_pointer<T>::type*, const void*>::value == bool(false);
        };

        template<class>
        struct is_void
        {
            static const bool value = false;
        };

        template<>
        struct is_void<void>
        {
            static const bool value = true;
        };

        template <bool, class T>
        struct enable_if
        {
        private:
            struct enable_if_dummy;
        public:
            typedef enable_if_dummy(&type)[1];
        };

        template <class T>
        struct enable_if<true, T>
        {
            typedef T type;
        };

        // add_pointer
        template<class T>
        struct add_pointer
        {
            typedef 
            typename
            conditional<
                is_reference<T>::value,
                typename remove_reference<T>::type,
                T
            >::type * type;
        };

        template<class T, template<class> class RemoverT>
        struct first_level
        {
            typedef
            typename
            conditional<
                is_reference<T>::value,
                typename 
                RemoverT<
                    typename remove_reference<T>::type
                >::type&,
                typename 
                conditional<
                    is_pointer<T>::value,
                    typename
                    add_pointer<
                        typename RemoverT<typename remove_pointer<T>::type>::type
                    >::type,
                    typename RemoverT<T>::type
                >::type
            >::type type;
        };
    }

    class any
    {
    public:
        /// Constructs an object of type any with an empty state.
        any()
            : vtable(CPPMETA_NULL)
            , size_()
        { }

        /// Constructs an object of type any with an equivalent state as other.
        any(const any& other)
            : vtable(other.vtable)
            , size_(other.size_)
        {
            if (!other.empty())
            {
                other.vtable->copy(other.storage, this->storage);
            }
        }

        /// Same effect as this->clear().
        ~any()
        {
            this->clear();
        }

        /// Constructs an object of type any that contains an object of type T direct-initialized with std::forward<ValueType>(value).
        template<class ValueType>
        any(const ValueType& value)
            : size_(sizeof(value))
        {
            this->construct(value);
        }

        /// Has the same effect as any(rhs).swap(*this). No effects if an exception is thrown.
        any& operator=(const any& rhs)
        {
            any(rhs).swap(*this);
            return *this;
        }

        /// Has the same effect as any(std::forward<ValueType>(value)).swap(*this). No effect if a exception is thrown.
        template<class ValueType>
        any& operator=(const ValueType& value)
        {
            any tmp(value); 
            tmp.swap(*this);
            return *this;
        }

        /// If not empty, destroys the contained object.
        void clear() throw()
        {
            if (!empty())
            {
                this->vtable->destroy(storage);
                this->vtable = CPPMETA_NULL;
            }
        }

        /// Returns true if *this has no contained object, otherwise false.
        bool empty() const throw()
        {
            return this->vtable == CPPMETA_NULL;
        }

        /// Exchange the states of *this and rhs.
        void swap(any& rhs) throw()
        {
            std::swap(size_, rhs.size_);

            if (this->vtable != rhs.vtable)
            {
                any tmp(rhs);

                // move from *this to rhs.
                rhs.vtable = this->vtable;
                if (this->vtable != CPPMETA_NULL)
                {
                    this->vtable->move(this->storage, rhs.storage);
                    //this->vtable = nullptr; -- unneeded, see below
                }

                // move from tmp (previously rhs) to *this.
                this->vtable = tmp.vtable;
                if (tmp.vtable != CPPMETA_NULL)
                {
                    tmp.vtable->move(tmp.storage, this->storage);
                    tmp.vtable = CPPMETA_NULL;
                }
            }
            else // same types
            {
                if (this->vtable != CPPMETA_NULL)
                    this->vtable->swap(this->storage, rhs.storage);
            }
        }

        /// non-standard
        std::size_t size() const
        {
            return this->size_;
        }

        const char* data() const
        {
            return cast<const char>();
        }

    private: // Storage and Virtual Method Table

        union storage_union
        {
            struct stack_storage_t
            {
                unsigned char data[sizeof(void*)];
            };
            stack_storage_t     stack;
            void* dynamic;
        };

        /// Base VTable specification.
        struct vtable_type
        {
            // Note: The caller is responssible for doing .vtable = nullptr after destructful operations
            // such as destroy() and/or move().

            /// Destroys the object in the union.
            /// The state of the union after this call is unspecified, caller must ensure not to use src anymore.
            void(*destroy)(storage_union&);

            /// Copies the **inner** content of the src union into the yet unitialized dest union.
            /// As such, both inner objects will have the same state, but on separate memory locations.
            void(*copy)(const storage_union& src, storage_union& dest);

            /// Moves the storage from src to the yet unitialized dest union.
            /// The state of src after this call is unspecified, caller must ensure not to use src anymore.
            void(*move)(storage_union& src, storage_union& dest);

            /// Exchanges the storage between lhs and rhs.
            void(*swap)(storage_union& lhs, storage_union& rhs);
        };

        /// VTable for dynamically allocated storage.
        template<class T>
        struct vtable_dynamic
        {
            static void destroy(storage_union& storage)
            {
                //assert(reinterpret_cast<T*>(storage.dynamic));
                delete reinterpret_cast<T*>(storage.dynamic);
            }

            static void copy(const storage_union& src, storage_union& dest)
            {
                const T& src_storage = *reinterpret_cast<const T*>(src.dynamic);
                T& dst_storage = *(new T(src_storage));
                dest.dynamic = &dst_storage;
            }

            static void move(storage_union& src, storage_union& dest)
            {
                dest.dynamic = src.dynamic;
                src.dynamic = 0;
            }

            static void swap(storage_union& lhs, storage_union& rhs)
            {
                // just exchage the storage pointers.
                std::swap(lhs.dynamic, rhs.dynamic);
            }
        };

        /// VTable for stack allocated storage.
        template<class T>
        struct vtable_stack
        {
            static void destroy(storage_union& storage) throw()
            {
                reinterpret_cast<T*>(&storage.stack)->~T();
            }

            static void copy(const storage_union& src, storage_union& dest)
            {
                const T& src_storage = reinterpret_cast<const T&>(src.stack);
                T& dst_storage = *(new (&dest.stack) T(src_storage));
            }

            static void move(storage_union& src, storage_union& dest)  throw()
            {
                copy(src, dest);
                destroy(src);
            }

            static void swap(storage_union& lhs, storage_union& rhs)  throw()
            {
                storage_union tmp_storage;
                move(rhs, tmp_storage);
                move(lhs, rhs);
                move(tmp_storage, lhs);
            }
        };

        /// Whether the type T must be dynamically allocated or can be stored on the stack.
        template<class T>
        struct requires_allocation 
        {
            static const bool value = 
                (sizeof(T) > sizeof(storage_union().stack))
                || type_traits::is_simple_type<T>::value == bool(false);
        };

        /// Returns the pointer to the vtable of the type T.
        template<class T>
        static vtable_type* vtable_for_type()
        {
            typedef typename type_traits::conditional<
                requires_allocation<T>::value, vtable_dynamic<T>, vtable_stack<T>
            >::type VTableType;

            static vtable_type table = {
                VTableType::destroy,
                VTableType::copy, VTableType::move,
                VTableType::swap,
            };
            return &table;
        }

    protected:

        /// Casts (with no type_meta checks) the storage pointer as const T*.
        template<class T>
        const T* cast() const
        {
            const T* result =
                requires_allocation<T>::value
                    ? reinterpret_cast<const T*>(storage.dynamic)
                    : reinterpret_cast<const T*>(&storage.stack);
            return result;
        }

        /// Casts (with no type_meta checks) the storage pointer as T*.
        template<class T>
        T* cast()
        {
            T* result = 
                requires_allocation<T>::value
                ? reinterpret_cast<T*>(storage.dynamic)
                : reinterpret_cast<T*>(&storage.stack);
            return result;
        }

    private:
        storage_union storage; // on offset(0) so no padding for align
        vtable_type* vtable;
        std::size_t size_;

        template<class ValueType, class T>
        void do_construct(
            typename 
            type_traits::conditional<
                requires_allocation<T>::value, 
                const ValueType, 
                detail::disabled<__LINE__>
            >::type &value)
        {
            storage.dynamic = new T(value);
        }

        template<class ValueType, class T>
        void do_construct(
            typename 
            type_traits::conditional<
                requires_allocation<T>::value, 
                detail::disabled<__LINE__>,
                const ValueType
            >::type &value)
        {
            new (&storage.stack) T(value);
        }

        /// Chooses between stack and dynamic allocation for the type decay_t<ValueType>,
        /// assigns the correct vtable, and constructs the object on our storage.
        template<class ValueType>
        void construct(const ValueType& value)
        {
            typedef 
            //typename 
            //type_traits::remove_cv<
                typename type_traits::remove_reference<ValueType>::type
            //>::type
                T;

            this->vtable = vtable_for_type<T>();

            do_construct<ValueType, T>(value);
        }

        template<class T>
        struct remove_const_from_ptr_or_value
        {
            typedef
            typename
            type_traits::conditional<
                type_traits::is_pointer<T>::value == bool(true) &&
                type_traits::is_function<T>::value == bool(false),
                typename type_traits::remove_const<typename type_traits::remove_pointer<T>::type>::type*,
                typename type_traits::remove_const<T>::type
            >::type type;
        };

        template<class T>
        struct remove_volatile_from_ptr_or_value
        {
            typedef
            typename
            type_traits::conditional<
                type_traits::is_pointer<T>::value == bool(true) &&
                type_traits::is_function<T>::value == bool(false),
                typename type_traits::remove_volatile<typename type_traits::remove_pointer<T>::type>::type*,
                typename type_traits::remove_volatile<T>::type
            >::type type;
        };

        template<class T>
        struct remove_cv_from_ptr_or_value
        {
            typedef
            typename
            type_traits::conditional<
                type_traits::is_pointer<T>::value == bool(true) &&
                type_traits::is_function<T>::value == bool(false),
                typename type_traits::remove_cv<typename type_traits::remove_pointer<T>::type>::type*,
                typename type_traits::remove_cv<T>::type
            >::type type;
        };

    public:

        template<class ValueType>
        static inline ValueType* any_cast(
                any* operand)
        {
            using type_traits::remove_reference;
            using type_traits::remove_pointer;
            using type_traits::remove_const;
            using type_traits::remove_volatile;
            using type_traits::remove_cv;
            
            typedef
            typename remove_reference<ValueType>::type type_no_ref;
            typedef
            typename remove_pointer<ValueType>::type type_no_ptr;
            typedef
            typename remove_const_from_ptr_or_value<type_no_ref>::type type_no_const;
            typedef
            typename remove_volatile_from_ptr_or_value<type_no_ref>::type type_no_volatile;
            typedef
            typename remove_cv_from_ptr_or_value<type_no_ref>::type type_no_cv;

            //void(*tmp1)(type_no_ref);
            //void(*tmp2)(type_no_const);
            //void(*tmp3)(type_no_volatile);
            //void(*tmp4)(type_no_cv);

            if (operand->vtable == vtable_for_type<type_no_ref>())
                return operand->cast<ValueType>();

            if (operand->vtable == vtable_for_type<type_no_const>())
                return operand->cast<ValueType>();

            if (operand->vtable == vtable_for_type<type_no_volatile>())
                return operand->cast<ValueType>();

            if (operand->vtable == vtable_for_type<type_no_cv>())
                return operand->cast<ValueType>();


            if (operand->vtable == vtable_for_type<type_no_ref*>())
                return *operand->cast<ValueType*>();

            if (operand->vtable == vtable_for_type<type_no_const*>())
                return *operand->cast<ValueType*>();

            if (operand->vtable == vtable_for_type<type_no_volatile*>())
                return *operand->cast<ValueType*>();

            if (operand->vtable == vtable_for_type<type_no_cv*>())
                return *operand->cast<ValueType*>();

            return CPPMETA_NULL;
        }

        template<class ValueType>
        static inline const ValueType* any_cast_const(
            const any* operand)
        {
            using type_traits::remove_reference;
            using type_traits::remove_pointer;
            using type_traits::remove_const;
            using type_traits::remove_volatile;
            using type_traits::remove_cv;
            
            typedef
            typename remove_reference<const ValueType>::type type_no_ref;
            typedef
            typename remove_const_from_ptr_or_value<type_no_ref>::type type_no_const;
            typedef
            typename remove_volatile_from_ptr_or_value<type_no_ref>::type type_no_volatile;
            typedef
            typename remove_cv_from_ptr_or_value<type_no_ref>::type type_no_cv;

            if (operand->vtable == vtable_for_type<type_no_ref>())
                return operand->cast<ValueType>();

            if (operand->vtable == vtable_for_type<type_no_const>())
                return operand->cast<ValueType>();

            if (operand->vtable == vtable_for_type<type_no_volatile>())
                return operand->cast<ValueType>();

            if (operand->vtable == vtable_for_type<type_no_cv>())
                return operand->cast<ValueType>();


            if (operand->vtable == vtable_for_type<type_no_ref*>())
                return *operand->cast<ValueType*>();

            if (operand->vtable == vtable_for_type<type_no_const*>())
                return *operand->cast<ValueType*>();

            if (operand->vtable == vtable_for_type<type_no_volatile*>())
                return *operand->cast<ValueType*>();

            if (operand->vtable == vtable_for_type<type_no_cv*>())
                return *operand->cast<ValueType*>();

            return CPPMETA_NULL;
        }
    };

    class bad_any_cast :
        public std::bad_cast
    {
        std::string what_;
    public:
        ~bad_any_cast() throw() {}
        bad_any_cast(const std::string &msg)
            : what_("cppmeta::bad_any_cast: " + msg) {}

        const char * what() const throw()
        {
            return what_.c_str();
        }
    };

    template<class ValueType>
    inline ValueType any_cast(
            any& operand)
    {
        typedef typename type_traits::remove_reference<ValueType>::type cast_type;
        cast_type* result = any::any_cast<cast_type>(&operand);

        if (!result) throw(std::bad_cast());

        return *result;
    }

    template<class ValueType>
    inline ValueType any_cast(
        const any &operand)
    {
        typedef typename type_traits::add_const<typename type_traits::remove_reference<ValueType>::type>::type cast_type;
        cast_type* result = any::any_cast_const<cast_type>(&operand);

        if (!result) throw(std::bad_cast());

        return *result;
    }

    template<class ValueType>
    inline ValueType* any_cast(
            any* operand)
    {
        if (!operand)
            return CPPMETA_NULL;

        typedef ValueType cast_type;

        cast_type* result = any::any_cast<cast_type>(operand);

        return result;
    }

    template<class ValueType>
    inline const ValueType* any_cast(
        const any* operand)
    {
        if (!operand)
            return CPPMETA_NULL;

        typedef ValueType cast_type;

        const cast_type* result = any::any_cast_const<cast_type>(operand);

        return result;
    }


    template <class T>
    class smart_ptr;

    template <class T>
    struct smart_ptrref_ { // proxy reference for smart_ptr copying
        explicit smart_ptrref_(T* right_) : ref_(right_) {} // construct from generic pointer to smart_ptr ptr

        T* ref_; // generic pointer to smart_ptr ptr
    };

    template <class T>
    class smart_ptr { // wrap an object pointer to ensure destruction
    public:
        typedef T element_type;

        explicit smart_ptr(T* ptr_ = CPPMETA_NULL) throw() : _ptr(ptr_) {} // construct from object pointer

        smart_ptr(smart_ptr& right_) throw() : _ptr(right_.release()) {
            // construct by assuming pointer from right_ smart_ptr
        }

        smart_ptr(smart_ptrref_<T> right_) throw() { // construct by assuming pointer from right_ smart_ptrref_
            T* ptr = right_.ref_;
            right_.ref_ = CPPMETA_NULL; // release old
            _ptr = ptr; // reset this
        }

        template <class OtherT>
        operator smart_ptr<OtherT>() throw() { // convert to compatible smart_ptr
            return smart_ptr<OtherT>(*this);
        }

        template <class OtherT>
        operator smart_ptrref_<OtherT>() throw() { // convert to compatible smart_ptrref_
            OtherT* cvtptr = _ptr; // test implicit conversion
            smart_ptrref_<OtherT> ans(cvtptr);
            _ptr = CPPMETA_NULL; // pass ownership to smart_ptrref_
            return ans;
        }

        template <class OtherT>
        smart_ptr& operator=(smart_ptr<OtherT>& right_) throw() { // assign compatible right_ (assume pointer)
            reset(right_.release());
            return *this;
        }

        template <class OtherT>
        smart_ptr(smart_ptr<OtherT>& right_) throw() : _ptr(right_.release()) {
            // construct by assuming pointer from right_
        }

        smart_ptr& operator=(smart_ptr& right_) throw() { // assign compatible right_ (assume pointer)
            reset(right_.release());
            return *this;
        }

        smart_ptr& operator=(smart_ptrref_<T> right_) throw() { // assign compatible right_.ref_ (assume pointer)

            T* ptr = right_.ref_;
            right_.ref_ = 0; // release old
            reset(ptr); // set new
            return *this;
        }

        ~smart_ptr() throw() {
            delete _ptr;
        }

        T& operator*() const throw() {
            return *get();
        }

        T* operator->() const throw() {
            return get();
        }

        T* get() const throw() { // return wrapped pointer
            return _ptr;
        }

        T* release() throw() { // return wrapped pointer and give up ownership
            T* tmp = _ptr;
            _ptr = CPPMETA_NULL;
            return tmp;
        }

        void reset(T* _Ptr = CPPMETA_NULL) { // destroy designated object and store new pointer
            if (_Ptr != _ptr) {
                delete _ptr;
            }

            _ptr = _Ptr;
        }

    private:
        T* _ptr; // the wrapped object pointer
    };

    template <>
    class smart_ptr<void> {
    public:
        typedef void element_type;
    };

    template<class ClassT, class T>
    struct entity_types
    {
        typedef T value_type;
        typedef value_type& reference;
        typedef const value_type& const_reference;

        typedef value_type ClassT::*member_ptr_type;

        typedef const_reference(ClassT::*ref_getter_member_func_ptr_type)() const;
        typedef void (ClassT::*ref_setter_member_func_ptr_type)(const_reference);
        typedef value_type(ClassT::*val_getter_member_func_ptr_type)() const;
        typedef void (ClassT::*val_setter_member_func_ptr_type)(value_type);
        typedef reference(ClassT::*nonconst_ref_getter_member_func_ptr_type)();

        typedef const_reference(*ref_getter_func_ptr_type)(const ClassT&);
        typedef void (*ref_setter_func_ptr_type)(ClassT&, const_reference);
        typedef value_type(*val_getter_func_ptr_type)(const ClassT&);
        typedef void (*val_setter_func_ptr_type)(ClassT&, value_type);
        typedef reference(*nonconst_ref_getter_func_ptr_type)(ClassT&);
    };

    template<class T>
    struct entity_types<T, T>
    {
        typedef T value_type;
        typedef value_type& reference;
        typedef const value_type& const_reference;

        typedef value_type* member_ptr_type;

        typedef const_reference(*ref_getter_func_ptr_type)();
        typedef void (*ref_setter_func_ptr_type)(const_reference);
        typedef value_type(*val_getter_func_ptr_type)();
        typedef void (*val_setter_func_ptr_type)(value_type);
        typedef reference(*nonconst_ref_getter_func_ptr_type)();
    };

    template<class ClassT, int Size, class T>
    struct entity_types<ClassT, T[Size]>
    {
        typedef T(value_type)[Size];
        typedef value_type& reference;
        typedef const value_type& const_reference;

        typedef value_type ClassT::*member_ptr_type;

        typedef const_reference(ClassT::*ref_getter_member_func_ptr_type)() const;
        typedef void (ClassT::*ref_setter_member_func_ptr_type)(const_reference);
        typedef ref_getter_member_func_ptr_type val_getter_member_func_ptr_type;
        typedef ref_setter_member_func_ptr_type val_setter_member_func_ptr_type;
        typedef reference(ClassT::*nonconst_ref_getter_member_func_ptr_type)();
        
        typedef const_reference(*ref_getter_func_ptr_type)(const ClassT&) ;
        typedef void (*ref_setter_func_ptr_type)(ClassT&, const_reference);
        typedef ref_getter_func_ptr_type val_getter_func_ptr_type;
        typedef ref_setter_func_ptr_type val_setter_func_ptr_type;
        typedef reference(*nonconst_ref_getter_func_ptr_type)(ClassT&);
    };

    namespace detail
    {
        template<class>
        struct member_resolver;

        template <
            class ResultT,
            class ClassT,
            class Arg0T = void_type,
            class Arg1T = void_type,
            class Arg2T = void_type,
            class Arg3T = void_type,
            class Arg4T = void_type,
            class Arg5T = void_type,
            class Arg6T = void_type,
            class Arg7T = void_type>
            class member_call;

        template <class ResultT, class ClassT>
        class member_call<ResultT, ClassT> {
            ResultT(ClassT::* d_member)();
        public:
            member_call(ResultT(ClassT::* member)()) : d_member(member) { }
            ResultT operator()(ClassT* object) const { return (object->*this->d_member)(); }
            ResultT operator()(ClassT& object) const { return (object.*this->d_member)(); }
            bool operator== (member_call const& other) const { return this->d_member == other.d_member; }
            bool operator!= (member_call const& other) const { return !(*this == other); }
        };

        template <class ResultT, class ClassT, class Arg0T>
        class member_call<ResultT, ClassT, Arg0T> {
            ResultT(ClassT::* d_member)(Arg0T);
        public:
            member_call(ResultT(ClassT::* member)(Arg0T)) : d_member(member) { }
            ResultT operator()(ClassT* object, Arg0T arg0) const { return (object->*this->d_member)(arg0); }
            ResultT operator()(ClassT& object, Arg0T arg0) const { return (object.*this->d_member)(arg0); }
            bool operator== (member_call const& other) const { return this->d_member == other.d_member; }
            bool operator!= (member_call const& other) const { return !(*this == other); }
        };
    }

    namespace detail
    {
        template <
            class ResultT,
            class Arg0T = void_type,
            class Arg1T = void_type,
            class Arg2T = void_type,
            class Arg3T = void_type,
            class Arg4T = void_type,
            class Arg5T = void_type,
            class Arg6T = void_type,
            class Arg7T = void_type>
        class function_call;

        template <class ResultT>
        class function_call<ResultT> {
            ResultT(*d_func)();
        public:
            function_call(ResultT(*func)()) : d_func(func) { }
            ResultT operator()() { return (this->d_func)(); }
            bool operator== (function_call const& other) const { return this->d_func == other.d_func; }
            bool operator!= (function_call const& other) const { return !(*this == other); }
        };

        template <class ResultT, class Arg0T>
        class function_call<ResultT, Arg0T> {
            ResultT(*d_func)(Arg0T);
        public:
            function_call(ResultT(*func)(Arg0T)) : d_func(func) { }
            ResultT operator()(Arg0T arg0) { return (this->d_func)(arg0); }
            bool operator== (function_call const& other) const { return this->d_func == other.d_func; }
            bool operator!= (function_call const& other) const { return !(*this == other); }
        };
    }

    namespace detail
    {
        template<int ArgCount>
        class any_functor_call;

        template<>
        class any_functor_call<0>
        {
            any d_functor;
            any(*d_functor_call)(const any&);

            template<class T>
            any functor_call(const any& functor)
            {
                T* functor_ptr = any_cast<T&>(functor);
                if (functor_ptr)
                    return (*functor_ptr)();
                return any();
            }

        public:

            template<class ResultT>
            any_functor_call(function_call<ResultT> functor)
                : d_functor(functor)
                , d_functor_call(&functor_call< function_call<ResultT>/**/>)
            { }

            any operator()() const
            {
                return d_functor_call(d_functor);
            }
        };

        template<>
        class any_functor_call<1>
        {
            any d_functor;
            any(*d_functor_call)(const any&, any&);

            template<class T>
            any functor_call(const any& functor, any& arg0)
            {
                T* functor_ptr = any_cast<T&>(functor);
                if (functor_ptr)
                    return (*functor_ptr)(arg0);
                return any();
            }

        public:

            template<class ResultT, class Arg0T>
            any_functor_call(function_call<ResultT, Arg0T> functor)
                : d_functor(functor)
                , d_functor_call(&functor_call< function_call<ResultT, Arg0T>/**/>)
            { }

            template<class ResultT, class ClassT>
            any_functor_call(member_call<ResultT, ClassT> functor)
                : d_functor(functor)
                , d_functor_call(&functor_call< member_call<ResultT, ClassT>/**/>)
            { }

            any operator()(any arg0) const
            {
                return d_functor_call(d_functor, arg0);
            }
        };

        template<>
        class any_functor_call<2>
        {
            any d_functor;
            any(*d_functor_call)(const any&, any&, any&);

            template<class T>
            any functor_call(const any& functor, any& arg0, any& arg1)
            {
                T* functor_ptr = any_cast<T&>(functor);
                if (functor_ptr)
                    return (*functor_ptr)(arg0, arg1);
                return any();
            }

        public:

            template<class ResultT, class Arg0T, class Arg1T>
            any_functor_call(function_call<ResultT, Arg0T, Arg1T> functor)
                : d_functor(functor)
                , d_functor_call(&functor_call< function_call<ResultT, Arg0T, Arg1T>/**/>)
            { }

            template<class ResultT, class ClassT>
            any_functor_call(member_call<ResultT, ClassT> functor)
                : d_functor(functor)
                , d_functor_call(&functor_call< member_call<ResultT, ClassT>/**/>)
            { }

            any operator()(any arg0, any arg1) const
            {
                return d_functor_call(d_functor, arg0, arg1);
            }
        };
    }

    namespace detail
    {
        template<class ParentT>
        struct entities_containter;
    }

    namespace detail
    {
        struct make_exception
        {
            static 
            std::string library_name()
            {
                return "cppmeta";
            }

            template<class T>
            static
            std::string make_message(const std::string &name, const std::string &category_name, const std::string &msg)
            {
                const std::string& resolved_name = resolve<T>::name;
                std::string parent_name_message = 
                    !resolved_name.empty() ?
                    (resolved_name + " ") :
                    " ";
                std::string name_message = 
                    !name.empty() ?
                    (" with name '" + name + "' ") : 
                    " ";
                return 
                    library_name() + " error: " + 
                    parent_name_message + 
                    category_name + 
                    name_message + 
                    msg;
            }

            template<class T>
            static 
            std::out_of_range is_not_registered(const std::string &name, const std::string &category_name)
            {
                return std::out_of_range(
                    make_message<T>(name, category_name, "is not registered"));
            }

            template<class T, class ValueT>
            static 
            std::out_of_range is_not_registered(const ValueT &value,
                typename
                type_traits::conditional<
                    type_traits::is_convertable<ValueT, std::string>::value,
                    disabled<__LINE__>,
                    const std::string
                 >::type &category_name)
            {
                return is_not_registered<T>(resolve<ValueT>::name, category_name + " entitiy");
            }

            template<class T>
            static 
            bad_any_cast can_not_cast(const std::string &name, const std::string &category_from_name, const std::string &category_to_name)
            {
                return bad_any_cast(
                    make_message<T>(name, category_from_name, "cannot cast to " + category_to_name));
            }
        };

    }


    struct EntityInfo
    {
        std::string name;
        bool is_optional;
        bool is_object, is_value, is_property, is_member, is_function;

        EntityInfo(const std::string& name_param)
            : name(name_param) 
            , is_optional(false)
            , is_object(false)
            , is_value(false)
            , is_property(false)
            , is_member(false)
            , is_function(false)
        {}
    };

    struct MemberInfo
        : EntityInfo
    {
        bool is_bitfield;

        MemberInfo(const std::string& name_param)
            : EntityInfo(name_param)
            , is_bitfield(false)
        {}
    };

    template<class>
    struct Entities;

    class Entity;

    template<class ParentT, class T>
    class MemberProxy;

    template <class ClassT, class T>
    class Member
        : public MemberInfo
    {
        any _value;

        typedef
        typename
        type_traits::conditional<
            type_traits::is_reference<T>::value == bool(true) ||
            type_traits::is_pointer<T>::value == bool(true),
            T,
            typename type_traits::add_const<T>::type
        >::type value_type;

    public:

        value_type &value;

        Member(const std::string& name_param, 
            typename
            type_traits::conditional<
                type_traits::is_reference<T>::value,
                detail::disabled<__LINE__>,
                T
            >::type value_param)
            : MemberInfo(name_param)
            , _value(value_param)
            , value(any_cast<T&>(_value))
        { 
            is_member = true;
        }

        Member(const std::string& name_param, 
            typename
            type_traits::conditional<
                type_traits::is_reference<T>::value,
                T,
                detail::disabled<__LINE__>
            >::type value_param)
            : MemberInfo(name_param)
            , _value(&value_param)
            , value(any_cast<T&>(_value))
        { 
            is_member = true;
        }

        Member(const Member& other)
            : MemberInfo(other)
            , _value(other._value)
            , value(any_cast<T&>(_value))
        { }

        template<class OtherT>
        Member(const Member<
            typename
            type_traits::conditional<
                type_traits::is_convertable<OtherT, T>::value,
                ClassT,
                void
            >::type, OtherT>& other)
            : MemberInfo(other)
            , _value(static_cast<T>(other.value))
            , value(any_cast<T&>(_value))
        { }

    private:

        template<class, class>
        friend class MemberProxy;
        friend class Entity;
    };

    template <class ClassT>
    class Member<ClassT, void>
        : public MemberInfo
    {
    public:

        const any& value;

        Member(const any& other_value, const MemberInfo &other_info)
            : MemberInfo(other_info)
            , value(other_value)
        { 
            is_member = true;
        }

    private:

        template<class, class>
        friend class MemberProxy;
        friend class Entity;
    };

    template<class T>
    class ValueProxy;

    template <class T>
    class Value
        : public EntityInfo
    {
        any _value;

    public:

        typedef
        typename
        type_traits::conditional<
            type_traits::is_pointer<T>::value == bool(true),
            typename type_traits::remove_reference<T>::type,
            typename type_traits::first_level<T, type_traits::add_const>::type
        >::type value_type;

        value_type &value;

        Value(const std::string& name_param, T value_param)
            : EntityInfo(name_param)
            , _value(value_param)
            , value(any_cast<T&>(_value))
        { 
            is_value = true;
        }

        Value(const Value<T>& other)
            : EntityInfo(other)
            , _value(other._value)
            , value(any_cast<T&>(_value))
        { }

        operator const value_type& () const
        {
            return value;
        }

    private:

        template<class>
        friend class ValueProxy;
        friend class Entity;
    };

    template<class T>
    class ObjectProxy;

    template <class T>
    class Object
        : public EntityInfo
    {
        any _value;

    public:

        typedef
        typename
        type_traits::conditional<
            type_traits::is_reference<T>::value == bool(true) ||
            type_traits::is_pointer<T>::value == bool(true),
            typename type_traits::remove_reference<T>::type,
            typename type_traits::add_const<T>::type
        >::type value_type;

        value_type& value;

        Object(const std::string& name_param, 
            typename
            type_traits::conditional<
                type_traits::is_reference<T>::value,
                detail::disabled<__LINE__>,
                T
            >::type value_param)
            : EntityInfo(name_param)
            , _value(value_param)
            , value(any_cast<T&>(_value))
        {
            is_object = true;
        }

        Object(const std::string& name_param, 
            typename
            type_traits::conditional<
                type_traits::is_reference<T>::value,
                T,
                detail::disabled<__LINE__>
            >::type value_param)
            : EntityInfo(name_param)
            , _value(&value_param)
            , value(any_cast<T&>(_value))
        {
            is_object = true;
        }

        Object(const Object<T>& other)
            : EntityInfo(other)
            , _value(other._value)
            , value(any_cast<T&>(_value))
        { }

    private:

        template<class>
        friend class ObjectProxy;
        friend class Entity;
    };

    template<class ParentT, class T>
    class PropertyProxy;

    template <class ClassT, class T>
    class Property
        : public EntityInfo
    {
        any _value;

    public:

        T& value;
        bool is_bitfield;

        Property(const std::string& name_param, T value_param)
            : EntityInfo(name_param)
            , _value(value_param)
            , value(any_cast<T&>(_value))
            , is_bitfield(false)
        { 
            is_property = true;
        }

        Property(const Property &other)
            : EntityInfo(other)
            , _value(other.value)
            , value(any_cast<T&>(_value))
            , is_bitfield(false)
        { }

        template<class OtherT>
        Property(const Property<
            typename
            type_traits::conditional<
                type_traits::is_convertable<OtherT, T>::value,
                ClassT,
                void
            >::type, OtherT>& other)
            : EntityInfo(other)
            , _value(static_cast<T>(other.value))
            , value(any_cast<T&>(_value))
            , is_bitfield(false)
        { }

    private:

        template<class, class>
        friend class PropertyProxy;
        friend class Entity;
    };

    template<class T>
    class Function
        : public EntityInfo
    {
        typedef Entity base_type;
        any _function;

    public:

        T& function;

        Function(const std::string& name_param, T value_param)
            : EntityInfo(name_param)
            , _function(value_param)
            , function(any_cast<T&>(_function))
        { 
            is_function = true;
        }

        Function(const Function& other)
            : EntityInfo(other)
            , _function(static_cast<T>(other.function))
            , function(any_cast<T&>(_function))
        { }

        any invoke() const
        {
            typedef detail::any_functor_call<0> call_0;
            return any_cast<call_0>(function)();
        }

        any invoke(any arg0) const
        {
            typedef detail::any_functor_call<1> call_1;
            return any_cast<call_1>(function)(arg0);
        }

        any invoke(any arg0, any arg1) const
        {
            typedef detail::any_functor_call<2> call_2;
            return any_cast<call_2>(function)(arg0, arg1);
        }
    };


    template<class ParentT>
    class EntityProxy;

    class Entity // any entity
        : public EntityInfo
    {
        any _entity;
        struct vtable_type {};
        
        struct type_id_type{
            typedef void(*id_type)();
            id_type id;
            type_id_type(id_type id_ = NULL): id(id_) {}
            bool operator==(const type_id_type &other) const {return id == other.id;}
            bool operator!=(const type_id_type &other) const {return id != other.id;}
        };
        template<class T>
        static type_id_type _type_id()
        {
            struct lambdas
            {
                static void id() {}
            };
            return &lambdas::id;
        }


        typedef any& (*get_underlying_entity_value_t)(any&);
        typedef EntityInfo& (*get_underlying_entity_info_t)(any&);

        template<class T>
        static any& get_underlying_entity_value_impl(any& entity)
        {
            return any_cast<T&>(entity)._value;
        }

        template<class T>
        static EntityInfo& get_underlying_entity_info_impl(any& entity)
        {
            return any_cast<T&>(entity);
        }


        get_underlying_entity_value_t get_underlying_entity_value;
        get_underlying_entity_info_t get_underlying_entity_info;

        type_id_type stored_member_type_id;
        type_id_type stored_class_type_id;

        template<class> friend struct detail::member_resolver;

    public:
        const any& value;

        template <class ClassT, class T>
        Entity(const Member<ClassT, T ClassT::*>& member)
            : EntityInfo(member)
            , _entity(member)
            , get_underlying_entity_value(static_cast<get_underlying_entity_value_t>(&get_underlying_entity_value_impl<Member<ClassT, T ClassT::*>/**/>))
            , get_underlying_entity_info(static_cast<get_underlying_entity_info_t>(&get_underlying_entity_info_impl<Member<ClassT, T ClassT::*>/**/>))
            , stored_member_type_id(type_id<T>())
            , stored_class_type_id(type_id<ClassT>())
            , value(any_cast<Member<ClassT, T ClassT::*>&>(_entity)._value)
        { }

        template <class ClassT, class T>
        Entity(const Member<ClassT, T>& member)
            : EntityInfo(member)
            , _entity(member)
            , get_underlying_entity_value(static_cast<get_underlying_entity_value_t>(&get_underlying_entity_value_impl<Member<ClassT, T>/**/>))
            , get_underlying_entity_info(static_cast<get_underlying_entity_info_t>(&get_underlying_entity_info_impl<Member<ClassT, T>/**/>))
            , stored_member_type_id(type_id<T>())
            , stored_class_type_id(type_id<ClassT>())
            , value(any_cast<Member<ClassT, T>&>(_entity)._value)
        { }

        template <class T>
        Entity(const Value<T>& value)
            : EntityInfo(value)
            , _entity(value)
            , get_underlying_entity_value(static_cast<get_underlying_entity_value_t>(&get_underlying_entity_value_impl<Value<T>/**/>))
            , get_underlying_entity_info(static_cast<get_underlying_entity_info_t>(&get_underlying_entity_info_impl<Value<T>/**/>))
            , stored_member_type_id(type_id<T>())
            , stored_class_type_id(0)
            , value(any_cast<Value<T>&>(_entity)._value)
        { }

        template <class T>
        Entity(const Object<T>& object)
            : EntityInfo(object)
            , _entity(object)
            , get_underlying_entity_value(static_cast<get_underlying_entity_value_t>(&get_underlying_entity_value_impl<Object<T>/**/>))
            , get_underlying_entity_info(static_cast<get_underlying_entity_info_t>(&get_underlying_entity_info_impl<Object<T>/**/>))
            , stored_member_type_id(type_id<T>())
            , stored_class_type_id(0)
            , value(any_cast<Object<T>&>(_entity)._value)
        { }
        
        Entity(const Entity &other)
            : EntityInfo(other)
            , _entity(other._entity)
            , get_underlying_entity_value(other.get_underlying_entity_value)
            , get_underlying_entity_info(other.get_underlying_entity_info)
            , stored_member_type_id(other.stored_member_type_id)
            , stored_class_type_id(other.stored_class_type_id)
            , value(get_underlying_entity_value(_entity))
        { }

        Entity& operator=(const Entity& other)
        {
            static_cast<EntityInfo&>(*this) = other;
            _entity = other._entity;
            get_underlying_entity_value = other.get_underlying_entity_value;
            get_underlying_entity_info = other.get_underlying_entity_info;
            stored_member_type_id = other.stored_member_type_id;
            stored_class_type_id = other.stored_class_type_id;
            return *this;
        }

        template<class T>
        static type_id_type type_id()
        {
            typedef typename type_traits::remove_cv<T>::type type;
            static type_id_type result = _type_id<type>();
            return result;
        }

        template <class ClassT, class T>
        operator Member<ClassT, T> ()
        {
            if (is_member)
            {
                T* result = any_cast<T>(&get_underlying_entity_value(_entity));
                if(result)
                    return Member<ClassT, T>(name, *result);
            }
            throw(
                detail::make_exception::can_not_cast<T>(
                    name, "entitiy", "member"
                )
            );
        }

        template <class ClassT>
        operator Member<ClassT, void> ()
        {
            if (is_member)
            {
                return Member<ClassT, void>(
                    value,
                    reinterpret_cast<MemberInfo&>(
                        get_underlying_entity_info(_entity)
                        )
                    );
            }
            throw(
                detail::make_exception::can_not_cast<ClassT>(
                    name, "entitiy", "member"
                )
            );
        }

        template <class T>
        operator Value<T> ()
        {
            if(is_value)
            {
                typedef typename type_traits::add_pointer<typename Value<T>::value_type>::type cast_type_res;
                cast_type_res result =
                    any_cast<typename type_traits::remove_pointer<cast_type_res>::type>(&get_underlying_entity_value(_entity));
                if (result)
                    return Value<T>(name, *result);
            }
            throw(
                detail::make_exception::can_not_cast<T>(
                    name, "entitiy", "value"
                )
            );
        }

        template <class T>
        operator Object<T> ()
        {
            if(is_object)
            {
                typedef typename Object<T>::value_type cast_type;
                cast_type* result = any_cast<cast_type>(&get_underlying_entity_value(_entity));
                if (result)
                    return Object<T>(name, *result);
            }
            throw(
                detail::make_exception::can_not_cast<T>(
                    name, "entitiy", "object"
                )
            );
        }
    };

    namespace detail
    {
        template<class>
        struct EntitiesStorage;
    }

    template<class>
    struct Entities;

    template<class ParentT>
    class EntityProxy
    {
    public:
        typedef Entity entity_type;

        entity_type value;

        EntityProxy(const entity_type& member)
            : value(member) {}

        EntityProxy& optional() {
            value.is_optional = true;

            return *this;
        }

        detail::entities_containter<ParentT> operator,(const EntityProxy<ParentT>& rhs) const
        {
            typedef detail::entities_containter<ParentT> container_type;

            container_type container;

            container.members.reserve(2);

            container.members.push_back(value);
            container.members.push_back(rhs.value);
            return container;
        }

        ~EntityProxy()
        {
            detail::EntitiesStorage<ParentT>::internal::value().push_back(value);
        }
    };

    template<class>
    struct Members;

    template<class ParentT, class T>
    class MemberProxy
    {
    public:
        typedef Member<ParentT, T> entity_type;

        entity_type value;

        MemberProxy(const entity_type& member)
            : value(member) {}

        MemberProxy& optional() {
            value.is_optional = true;

            return *this;
        }

        MemberProxy& bitfield() {
            value.is_bitfield = true;

            return *this;
        }

        template<class T2>
        detail::entities_containter<ParentT> operator,(const MemberProxy<ParentT, T2>& rhs) const
        {
            typedef detail::entities_containter<ParentT> container_type;

            container_type container;

            container.members.reserve(2);

            container.members.push_back(value);
            container.members.push_back(rhs.value);
            return container;
        }

        ~MemberProxy()
        {
            detail::EntitiesStorage<ParentT>::internal::value().push_back(value);
        }
    };

    template<class>
    struct Values;

    template<class T>
    class ValueProxy
    {
    public:
        typedef Value<T> entity_type;

        entity_type value;

        ValueProxy(const entity_type& member)
            : value(member) {}

        ValueProxy& optional() {
            value.is_optional = true;

            return *this;
        }

        detail::entities_containter<T> operator,(const ValueProxy<T>& rhs) const
        {
            typedef detail::entities_containter<T> container_type;

            container_type container;

            container.members.reserve(2);

            container.members.push_back(value);
            container.members.push_back(rhs.value);
            return container;
        }

        ~ValueProxy()
        {
            detail::EntitiesStorage<T>::internal::value().push_back(value);
        }
    };

    template<class>
    struct Objects;

    template<class T>
    class ObjectProxy
    {
    public:
        typedef Object<T> entity_type;

        entity_type value;

        ObjectProxy(const entity_type& member)
            : value(member) {}

        ObjectProxy& optional() {
            value.is_optional = true;

            return *this;
        }

        detail::entities_containter<T>operator,(const ObjectProxy<T>& rhs) const
        {
            typedef detail::entities_containter<T> container_type;

            container_type container;

            container.members.reserve(2);

            container.members.push_back(value);
            container.members.push_back(rhs.value);
            return container;
        }

        ~ObjectProxy()
        {
            detail::EntitiesStorage<T>::internal::value().push_back(value);
        }
    };

    namespace detail
    {
        template<class ParentT>
        struct MembersProxy
        {
            typedef const Entity&(*get_member_t)(const std::string&);

            static std::vector<get_member_t>& grand_parent_entities()
            {
                static std::vector<get_member_t> val;
                return val;
            }

            template<class GrandParentT>
            MembersProxy(Members<GrandParentT>& members, 
                typename
                type_traits::conditional<
                    type_traits::is_convertable<
                        int GrandParentT::*,
                        int ParentT::*
                    >::value,
                    int,
                    void>::type disabler = 0)
            {
                struct lambdas
                {
                    typedef GrandParentT grand_parent_t;
                    static const Entity& getter(const std::string& name)
                    {
                        typename detail::EntitiesStorage<grand_parent_t>::type& entities = detail::EntitiesStorage<grand_parent_t>::value();
                        for(std::size_t i = 0; i < entities.size(); ++i)
                        {
                            if (entities[i].name == name)
                                return entities[i];
                        }

                        std::vector<get_member_t>& grand_parent_entities = MembersProxy<grand_parent_t>::grand_parent_entities();

                        for (std::size_t i = 0; i < grand_parent_entities.size(); ++i)
                        {
                            return grand_parent_entities[i](name);
                        }

                        throw(make_exception::is_not_registered<grand_parent_t>(name, "entity"));
                    }
                };
                grand_parent_entities().push_back(&lambdas::getter);
            }
        };

        template<class ParentT>
        struct entities_containter
        {
            typedef std::vector<Entity> type;

            entities_containter() {}
            entities_containter(const typename type::value_type& value) :
                entities(1, value)
            { }

            entities_containter(const EntityProxy<ParentT>& proxy)
            {
                entities.push_back(proxy.value);
            }

            entities_containter(const ValueProxy<ParentT>& proxy)
            {
                entities.push_back(proxy.value);
            }

            template<class T>
            entities_containter(const MemberProxy<ParentT, T>& proxy)
            {
                entities.push_back(proxy.value);
            }

            entities_containter(const ObjectProxy<ParentT>& proxy)
            {
                entities.push_back(proxy.value);
            }

            type entities;

            entities_containter& operator=(const entities_containter& value)
            {
                entities = value.entities;
                return *this;
            }

            detail::entities_containter<ParentT>& operator,(
                typename
                type_traits::conditional<
                    type_traits::is_simple_type<ParentT>::value,
                    disabled<__LINE__>,
                    const MembersProxy<ParentT>&
                >::type)
            {
                return *this;
            }
        };

    }


}

namespace cppmeta
{
    inline
        bool is_valid_utf8(std::string value)
    {
        const char * string = value.c_str();

        const unsigned char * bytes = (const unsigned char *) string;
        unsigned int cp;
        int num;

        while (*bytes != 0x00)
        {
            if ((*bytes & 0x80) == 0x00)
            {
                // U+0000 to U+007F 
                cp = (*bytes & 0x7F);
                num = 1;
            }
            else if ((*bytes & 0xE0) == 0xC0)
            {
                // U+0080 to U+07FF 
                cp = (*bytes & 0x1F);
                num = 2;
            }
            else if ((*bytes & 0xF0) == 0xE0)
            {
                // U+0800 to U+FFFF 
                cp = (*bytes & 0x0F);
                num = 3;
            }
            else if ((*bytes & 0xF8) == 0xF0)
            {
                // U+10000 to U+10FFFF 
                cp = (*bytes & 0x07);
                num = 4;
            }
            else
                return false;

            bytes += 1;
            for (int i = 1; i < num; ++i)
            {
                if ((*bytes & 0xC0) != 0x80)
                    return false;
                cp = (cp << 6) | (*bytes & 0x3F);
                bytes += 1;
            }

            if ((cp > 0x10FFFF) ||
                ((cp >= 0xD800) && (cp <= 0xDFFF)) ||
                ((cp <= 0x007F) && (num != 1)) ||
                ((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
                ((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
                ((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4)))
                return false;
        }

        return true;
    }

    inline
        std::string cp1251_to_utf8(std::string value)
    {
        smart_ptr<char> out_ptr(new char[value.size() * 4]);

        char *out = out_ptr.get();
        const char *in = value.c_str();

        static const int table[128] = {
            0x82D0,0x83D0,0x9A80E2,0x93D1,0x9E80E2,0xA680E2,0xA080E2,0xA180E2,
            0xAC82E2,0xB080E2,0x89D0,0xB980E2,0x8AD0,0x8CD0,0x8BD0,0x8FD0,
            0x92D1,0x9880E2,0x9980E2,0x9C80E2,0x9D80E2,0xA280E2,0x9380E2,0x9480E2,
            0,0xA284E2,0x99D1,0xBA80E2,0x9AD1,0x9CD1,0x9BD1,0x9FD1,
            0xA0C2,0x8ED0,0x9ED1,0x88D0,0xA4C2,0x90D2,0xA6C2,0xA7C2,
            0x81D0,0xA9C2,0x84D0,0xABC2,0xACC2,0xADC2,0xAEC2,0x87D0,
            0xB0C2,0xB1C2,0x86D0,0x96D1,0x91D2,0xB5C2,0xB6C2,0xB7C2,
            0x91D1,0x9684E2,0x94D1,0xBBC2,0x98D1,0x85D0,0x95D1,0x97D1,
            0x90D0,0x91D0,0x92D0,0x93D0,0x94D0,0x95D0,0x96D0,0x97D0,
            0x98D0,0x99D0,0x9AD0,0x9BD0,0x9CD0,0x9DD0,0x9ED0,0x9FD0,
            0xA0D0,0xA1D0,0xA2D0,0xA3D0,0xA4D0,0xA5D0,0xA6D0,0xA7D0,
            0xA8D0,0xA9D0,0xAAD0,0xABD0,0xACD0,0xADD0,0xAED0,0xAFD0,
            0xB0D0,0xB1D0,0xB2D0,0xB3D0,0xB4D0,0xB5D0,0xB6D0,0xB7D0,
            0xB8D0,0xB9D0,0xBAD0,0xBBD0,0xBCD0,0xBDD0,0xBED0,0xBFD0,
            0x80D1,0x81D1,0x82D1,0x83D1,0x84D1,0x85D1,0x86D1,0x87D1,
            0x88D1,0x89D1,0x8AD1,0x8BD1,0x8CD1,0x8DD1,0x8ED1,0x8FD1
        };
        while (*in)
            if (*in & 0x80) {
                int v = table[(int) (0x7f & *in++)];
                if (!v)
                    continue;
                *out++ = (char) v;
                *out++ = (char) (v >> 8);
                if (v >>= 16)
                    *out++ = (char) v;
            }
            else
                *out++ = *in++;
        *out = 0;

        return out_ptr.get();
    }

    template<class, int>
    struct caster
    {
        typedef void type;
    };

    template<int Value, class T>
    struct caster_index_next :
        caster_index_next<
        Value + 1,
        typename caster<T, Value + 1>::type
        >
    { };

    template<int Value>
    struct caster_index_next<Value, void>
    {
        static const int value = Value;
    };

    template<class T>
    struct caster_index :
        caster_index_next<0, T>
    { };

    



    template<class X>
    struct to
    {
        template<class T>
        static
        ::cppmeta::type_traits::no_to_from_conversion call(X&, const T&, ...);
    };

    template<class X>
    struct from
    {
        template<class T>
        static
        ::cppmeta::type_traits::no_to_from_conversion call(const X&, T&, ...);
    };

    
    namespace type_traits
    {

        template<class X, class T = int>
        struct has_conversion_to
        {
            static const bool value = 
                (sizeof(has_conversion_tester((::cppmeta::to<X>::call(::cppmeta::type_traits::declref<X>(), ::cppmeta::type_traits::declref<T>()), has_to_from_conversion()))) == sizeof(detail::yes_type));
        };

        template<class X, class T = int>
        struct has_conversion_from
        {
            static const bool value = 
                (sizeof(has_conversion_tester((::cppmeta::from<X>::call(::cppmeta::type_traits::declref<const X>(), ::cppmeta::type_traits::declref<T>()), has_to_from_conversion()))) == sizeof(detail::yes_type));
        };

        template<>
        struct has_conversion_to<void>
        {
            static const bool value = false;
        };

        template<>
        struct has_conversion_from<void>
        {
            static const bool value = false;
        };
    }

    template<class T>
    class ConverterPlaceholder{};

    enum ConverterType
    {
        to_converter,
        from_converter
    };

    namespace detail
    {
        template<class T1, int Size1, class T2, int Size2>
        void array_copy(T1(&dst)[Size1], const T2(&src)[Size2])
        {
            for (int i = 0; i < Size1; ++i)
                dst[i] = src[i];
        }

        template<class ClassT, class X, class T, ConverterType>
        struct ConverterHelperImpl;

        template<class ClassT, class X, class T>
        struct ConverterHelperImpl<ClassT, X, T, to_converter>
        {
            static void to_impl(X& x, const void* data, const ClassT& obj)
            {
                const Member<ClassT, T>& member = *reinterpret_cast<const Member<ClassT, T>*>(data);

                ::cppmeta::to<X>::call(x, obj.*member.value);
            }
        };

        template<class X, class T>
        struct ConverterHelperImpl<T, X, T, to_converter>
        {
            static void to_impl(X& x, const void* data, const T& obj)
            {
                const Member<T, T>& member = *reinterpret_cast<const Member<T, T>*>(data);

                ::cppmeta::to<X>::call(x, *member.value);
            }
        };

        template<class ClassT, class X, class T>
        struct ConverterHelperImpl<ClassT, X, T, from_converter>
        {
            static void from_impl(const X& x, void* data, ClassT& obj)
            {
                const Member<ClassT, T>& member = *reinterpret_cast<const Member<ClassT, T>*>(data);

                ::cppmeta::from<X>::call(x, obj.*member.value);
            }
        };

        template<class X, class T>
        struct ConverterHelperImpl<T, X, T, from_converter>
        {
            static void from_impl(const X& x, void* data, T& obj)
            {
                const Member<T, T>& member = *reinterpret_cast<const Member<T, T>*>(data);

                ::cppmeta::from<X>::call(x, *member.value);
            }
        };

        template<class ClassT, class X, class T, ConverterType>
        struct ConverterHelper;

        template<class ClassT, class X, class T>
        struct ConverterHelperImplDummy
        {
            static void to_impl(X& , const void*, const ClassT& )
            {
                throw std::runtime_error("Error: can't serialize member because it's write only");
            }

            static void from_impl(const X& , void*, ClassT& )
            {
                throw std::runtime_error("Error: can't deserialize member because it's read only");
            }
        };

        template<class ClassT, class X, class T>
        struct ConverterHelper<ClassT, X, T, to_converter>
            : type_traits::conditional<
                type_traits::has_conversion_to<X, T>::value,
                ConverterHelperImpl<ClassT, X, T, to_converter>,
                ConverterHelperImplDummy<ClassT, X, T>
            >::type
        { };

        template<class ClassT, class X, class T>
        struct ConverterHelper<ClassT, X, T, from_converter>
            : type_traits::conditional<
                type_traits::has_conversion_from<X, T>::value,
                ConverterHelperImpl<ClassT, X, T, from_converter>,
                ConverterHelperImplDummy<ClassT, X, T>
            >::type
        { };
    }

    template<class ClassT, class X, ConverterType>
    class Converter;

    template<class ClassT, class X>
    class Converter<ClassT, X, to_converter>
    {
        typedef void(&to_type)(X&, const void*, const ClassT&);
        typedef void(*to_ptr_type)(X&, const void*, const ClassT&);



    protected:
        const to_ptr_type to;

        to_ptr_type to_cast(to_ptr_type value)
        { return value; }

    public:
        template<class T>
        Converter(const ConverterPlaceholder<T>&):
            to(to_cast(&detail::ConverterHelper<ClassT, X, T, to_converter>::to_impl))
        { }

        Converter(const Converter &other) :
            to(other.to)
        { }

        Converter& operator=(const Converter& other)
        {
            to = other.to;
            return *this;
        }
    };
    
    template<class ClassT, class X>
    class Converter<ClassT, X, from_converter>
    {
        typedef void(&from_type)(const X&, void*, ClassT&);
        typedef void(*from_ptr_type)(const X&, void*, ClassT&);

    protected:
        const from_ptr_type from;

        from_ptr_type from_cast(from_ptr_type value)
        { return value; }

    public:
        template<class T>
        Converter(const ConverterPlaceholder<T>&):
            from(from_cast(&detail::ConverterHelper<ClassT, X, T, from_converter>::from_impl))
        { }

        Converter(const Converter &other) :
            from(other.from)
        { }

        Converter& operator=(const Converter& other)
        {
            from = other.from;
            return *this;
        }
    };

    template<class X, ConverterType>
    struct CasterNextBuilder;

    template<class Converter1T, class Converter2T>
    struct JoinConverters : public Converter1T, public Converter2T
    {
        template<class T>
        JoinConverters(const ConverterPlaceholder<T>& ph)
            : Converter1T(ph)
            , Converter2T(ph)

        { }
    };

    template<class ClassT, class ConverterFrom, class ConverterTo, class CasterT>
    struct ConverterNext;

    template<class ClassT, class ConverterFrom, class ConverterTo, class CasterT>
    struct ConverterNextBuilder
    {
        typedef
        typename type_traits::conditional<
            type_traits::has_conversion_to<CasterT>::value,
            JoinConverters<ConverterTo, Converter<ClassT, CasterT, to_converter>/**/>,
            ConverterTo
        >::type NextConverterTo;

        typedef
        typename type_traits::conditional<
            type_traits::has_conversion_from<CasterT>::value,
            JoinConverters<ConverterFrom, Converter<ClassT, CasterT, from_converter>/**/>,
            ConverterFrom
        >::type NextConverterFrom;

        typedef typename caster<ClassT, caster_index<CasterT>::value + 1>::type NextCasterT;

        typedef ConverterNext< ClassT, NextConverterFrom, NextConverterTo, NextCasterT> type;
    };

    template<class ClassT, class ConverterFrom, class ConverterTo, class CasterT>
    struct ConverterNext
        : public ConverterNextBuilder<ClassT, ConverterFrom, ConverterTo, CasterT>::type
    { };


    template<class ClassT, class ConverterFrom, class ConverterTo>
    struct ConverterNext<ClassT, ConverterFrom, ConverterTo, void>
    {
        typedef JoinConverters<ConverterFrom, ConverterTo> type;
    };

    template<ConverterType>
    struct DummyConverter
    {
        template<class T>
        DummyConverter(const ConverterPlaceholder<T>&) {}
    };

    template<class ClassT>
    struct ConverterBuilder
    { 
        typedef typename caster<ClassT, 1>::type caster_type;
        typedef typename ConverterNext<ClassT, DummyConverter<from_converter>, DummyConverter<to_converter>, caster_type>::type type;
    };


    namespace detail
    {
        template<class T, int>
        T& declstaticval()
        {
            static T value;
            return value;
        }

        template<class T, class>
        T& declstaticval()
        {
            static T value;
            return value;
        }
    }

    template<int Reflection>
    struct reflect<detail::void_type, Reflection>
        : reflect<void, Reflection>
    { };

    template<int Reflection>
    struct resolve<detail::void_type, Reflection>
        : resolve<void, Reflection>
    { };

    namespace detail
    {
        template<class ParentT>
        struct EntitiesStorage
        {

            typedef typename detail::entities_containter<ParentT>::type type;

            static type& value()
            {
                struct reflect_ct_local
                    : cppmeta::reflect_ct<ParentT>
                {
                    typedef type captured_type;
                    typedef ParentT captured_ParentT;
                    typedef cppmeta::detail::reflect_ct_helper<captured_ParentT> base;

                    static captured_type& info()
                    {
                        captured_type &result = 
                            cppmeta::detail::declstaticval<captured_type, captured_ParentT>();
                        base::call(result);
                        return result;
                    }
                };
                static type &result = 
                    reflect_ct_local::info();
                return result;
            }

            struct internal
            {
                typedef typename detail::entities_containter<ParentT>::type type;
                static type& value()
                {
                    return cppmeta::detail::declstaticval<type, ParentT>();
                }

                template<class, class>
                friend class MemberProxy;
                template<class>
                friend class ValueProxy;
                template<class>
                friend class ObjectProxy;
                template<class, class>
                friend class PropertyProxy;
            };
            
        };
    }

    template<class ParentT>
    struct Entities
    {
        detail::entities_containter<ParentT> operator=(const EntityProxy<ParentT>& value)
        {
            return value;
        }

        const detail::entities_containter<ParentT>& operator=(const detail::entities_containter<ParentT>& value)
        {
            return value;
        }

        const detail::entities_containter<ParentT> operator+=(const EntityProxy<ParentT>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<ParentT>& operator+=(const detail::entities_containter<ParentT>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<ParentT> operator[](const EntityProxy<ParentT>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<ParentT>& operator[](const detail::entities_containter<ParentT>& value)
        {
            return *this = value;
        }
    };

    template<>
    struct Entities<void>
    {
        template<class ParentT>
        detail::entities_containter<ParentT> operator=(const EntityProxy<ParentT>& value)
        {
            return value;
        }

        template<class ParentT>
        const detail::entities_containter<ParentT>& operator=(const detail::entities_containter<ParentT>& value)
        {
            return value;
        }

        template<class ParentT>
        const detail::entities_containter<ParentT> operator+=(const EntityProxy<ParentT>& value)
        {
            return *this = value;
        }

        template<class ParentT>
        const detail::entities_containter<ParentT>& operator+=(const detail::entities_containter<ParentT>& value)
        {
            return *this = value;
        }

        template<class ParentT>
        const detail::entities_containter<ParentT> operator[](const EntityProxy<ParentT>& value)
        {
            return *this = value;
        }

        template<class ParentT>
        const detail::entities_containter<ParentT>& operator[](const detail::entities_containter<ParentT>& value)
        {
            return *this = value;
        }
    };

    struct :
        public Entities<void>
    {
        using Entities<void>::operator+=;
        using Entities<void>::operator=;
        using Entities<void>::operator[];
    }  static entities;

    template<class ParentT>
    struct Members
    {
        template<class T>
        detail::entities_containter<ParentT> operator=(const MemberProxy<ParentT, T>& value)
        {
            return value;
        }

        const detail::entities_containter<ParentT>& operator=(const detail::entities_containter<ParentT>& value)
        {
            return value;
        }

        template<class T>
        const detail::entities_containter<ParentT> operator+=(const MemberProxy<ParentT, T>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<ParentT>& operator+=(const detail::entities_containter<ParentT>& value)
        {
            return *this = value;
        }

        template<class T>
        const detail::entities_containter<ParentT> operator[](const MemberProxy<ParentT, T>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<ParentT>& operator[](const detail::entities_containter<ParentT>& value)
        {
            return *this = value;
        }
    };

    template<>
    struct Members<void>
    {
        template<class ParentT, class T>
        detail::entities_containter<ParentT> operator=(const MemberProxy<ParentT, T>& value)
        {
            return value;
        }

        template<class ParentT>
        const detail::entities_containter<ParentT>& operator=(const detail::entities_containter<ParentT>& value)
        {
            return value;
        }

        template<class ParentT, class T>
        const detail::entities_containter<ParentT> operator+=(const MemberProxy<ParentT, T>& value)
        {
            return *this = value;
        }

        template<class ParentT>
        const detail::entities_containter<ParentT>& operator+=(const detail::entities_containter<ParentT>& value)
        {
            return *this = value;
        }

        template<class ParentT, class T>
        const detail::entities_containter<ParentT> operator[](const MemberProxy<ParentT, T>& value)
        {
            return *this = value;
        }

        template<class ParentT>
        const detail::entities_containter<ParentT>& operator[](const detail::entities_containter<ParentT>& value)
        {
            return *this = value;
        }
    };

    struct :
        public Members<void>
    {
        using Members<void>::operator+=;
        using Members<void>::operator=;
        using Members<void>::operator[];
    }  static members;

    template<class T>
    struct Values
    {
        detail::entities_containter<T> operator=(const ValueProxy<T>& value)
        {
            return value;
        }

        const detail::entities_containter<T>& operator=(const detail::entities_containter<T>& value)
        {
            return value;
        }

        const detail::entities_containter<T> operator+=(const ValueProxy<T>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<T>& operator+=(const detail::entities_containter<T>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<T> operator[](const ValueProxy<T>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<T>& operator[](const detail::entities_containter<T>& value)
        {
            return *this = value;
        }
    };

    template<>
    struct Values<void>
    {
        template<class T>
        detail::entities_containter<T> operator=(const ValueProxy<T>& value)
        {
            return value;
        }

        template<class T>
        const detail::entities_containter<T>& operator=(const detail::entities_containter<T>& value)
        {
            return value;
        }

        template<class T>
        const detail::entities_containter<T> operator+=(const ValueProxy<T>& value)
        {
            return *this = value;
        }

        template<class T>
        const detail::entities_containter<T>& operator+=(const detail::entities_containter<T>& value)
        {
            return *this = value;
        }

        template<class T>
        const detail::entities_containter<T> operator[](const ValueProxy<T>& value)
        {
            return *this = value;
        }

        template<class T>
        const detail::entities_containter<T>& operator[](const detail::entities_containter<T>& value)
        {
            return *this = value;
        }
    };

    struct :
        public Values<void>
    {
        using Values<void>::operator+=;
        using Values<void>::operator=;
        using Values<void>::operator[];
    }  static values;

    template<class T>
    struct Objects
    {
        detail::entities_containter<T> operator=(const ObjectProxy<T>& value)
        {
            return value;
        }

        const detail::entities_containter<T>& operator=(const detail::entities_containter<T>& value)
        {
            return value;
        }

        detail::entities_containter<T*> operator=(const ObjectProxy<T*>& value)
        {
            return value;
        }

        const detail::entities_containter<T*>& operator=(const detail::entities_containter<T*>& value)
        {
            return value;
        }

        detail::entities_containter<T&> operator=(const ObjectProxy<T&>& value)
        {
            return value;
        }

        const detail::entities_containter<T&>& operator=(const detail::entities_containter<T&>& value)
        {
            return value;
        }

        const detail::entities_containter<T> operator+=(const ObjectProxy<T>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<T>& operator+=(const detail::entities_containter<T>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<T*> operator+=(const ObjectProxy<T*>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<T*>& operator+=(const detail::entities_containter<T*>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<T&> operator+=(const ObjectProxy<T&>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<T&>& operator+=(const detail::entities_containter<T&>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<T> operator[](const ObjectProxy<T>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<T>& operator[](const detail::entities_containter<T>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<T*> operator[](const ObjectProxy<T*>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<T*>& operator[](const detail::entities_containter<T*>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<T&> operator[](const ObjectProxy<T&>& value)
        {
            return *this = value;
        }

        const detail::entities_containter<T&>& operator[](const detail::entities_containter<T&>& value)
        {
            return *this = value;
        }
    };

    template<>
    struct Objects<void>
    {
        template<class T>
        detail::entities_containter<T> operator=(const ObjectProxy<T>& value)
        {
            return value;
        }

        template<class T>
        const detail::entities_containter<T>& operator=(const detail::entities_containter<T>& value)
        {
            return value;
        }

        template<class T>
        detail::entities_containter<T*> operator=(const ObjectProxy<T*>& value)
        {
            return value;
        }

        template<class T>
        const detail::entities_containter<T*>& operator=(const detail::entities_containter<T*>& value)
        {
            return value;
        }

        template<class T>
        const detail::entities_containter<T> operator+=(const ObjectProxy<T>& value)
        {
            return *this = value;
        }

        template<class T>
        const detail::entities_containter<T>& operator+=(const detail::entities_containter<T>& value)
        {
            return *this = value;
        }

        template<class T>
        const detail::entities_containter<T*> operator+=(const ObjectProxy<T*>& value)
        {
            return *this = value;
        }

        template<class T>
        const detail::entities_containter<T*>& operator+=(const detail::entities_containter<T*>& value)
        {
            return *this = value;
        }

        template<class T>
        const detail::entities_containter<T> operator[](const ObjectProxy<T>& value)
        {
            return *this = value;
        }

        template<class T>
        const detail::entities_containter<T>& operator[](const detail::entities_containter<T>& value)
        {
            return *this = value;
        }
    };

    struct :
        public Objects<void>
    {
        using Objects<void>::operator+=;
        using Objects<void>::operator=;
        using Objects<void>::operator[];
    }  static objects;

    namespace detail
    {
        template<class T>
        struct type_meta
        {
            static std::string& name;
            static Entities<T>& entities;
            static Members<T>& members;
            static Values<T>& values;
            static Objects<T>& objects;
        protected:
            static std::string& get_name() { return detail::declstaticval<std::string, T>(); }
            static Entities<T>& get_entities() { return detail::declstaticval<Entities<T>, __LINE__>(); }
            static Members<T>&  get_members()  { return detail::declstaticval<Members<T>, __LINE__>(); }
            static Values<T>&   get_values()   { return detail::declstaticval<Values<T>, __LINE__>(); }
            static Objects<T>&  get_objects()  { return detail::declstaticval<Objects<T>, __LINE__>(); }
        };

        template<class T>
        std::string& type_meta<T>::name =
            type_meta<T>::get_name();

        template<class T>
        Entities<T>& type_meta<T>::entities =
            type_meta<T>::get_entities();

        template<class T>
        Members<T>& type_meta<T>::members =
            type_meta<T>::get_members();

        template<class T>
        Values<T>& type_meta<T>::values =
            type_meta<T>::get_values();

        template<class T>
        Objects<T>& type_meta<T>::objects =
            type_meta<T>::get_objects();
    }

    template<class T>
    struct reflect<T>
        : detail::type_meta<T>
    {
    };

    template<class T>
    struct reflect_ct
        : reflect<T, cppmeta::reflection::compile_time> {};

    template<class T>
    struct reflect_rt
        : reflect<T, cppmeta::reflection::run_time> {};

    namespace detail
    {
        template<class T, bool>
        struct entity_resolver;

        template<class T>
        struct entity_resolver<T, false>
        {
            template<class ArgT>
            static
            Entity* entity(const ArgT& pred_arg, bool(&pred)(const Entity&, const ArgT&))
            {
#ifdef _DEBUG
                struct lambdas { static void test() {} }; void(&test)() = lambdas::test;
#endif
                
                typename detail::EntitiesStorage<T>::type& entities =
                    detail::EntitiesStorage<T>::value();

                for (std::size_t i = 0; i < entities.size(); ++i)
                {
                    if (pred(entities[i], pred_arg))
                        return &entities[i];
                }

                return 0;
            }

        };

        template<class T>
        struct entity_resolver<T, true>
        {
            template<class ArgT>
            static 
            Entity* entity_impl(const ArgT& pred_arg, bool(&pred)(const Entity&, const ArgT&))
            {
                Entity *result = entity_resolver<T, false>::entity(pred_arg, pred);
                if (!result)
                {
                    typedef typename type_traits::remove_reference<T>::type type_no_ref;
                    typedef typename type_traits::remove_pointer<T>::type type_no_ptr;

                    if (!type_traits::is_same<T, type_no_ref*>::value)
                    {
                        result = entity_resolver<type_no_ref*, false>::entity(pred_arg, pred);
                        if (result) return result;
                    }
                    if ( !type_traits::is_same<type_no_ref, T>::value && 
                         !type_traits::is_same<type_no_ref*, type_no_ref>::value)
                    {
                        result = entity_resolver<type_no_ref, false>::entity(pred_arg, pred);
                        if (result) return result;
                    }
                    if (!type_traits::is_same<type_no_ptr, T>::value)
                    {
                        result = entity_resolver<type_no_ptr&, false>::entity(pred_arg, pred);
                        if (result) return result;

                    }
                    if ( !type_traits::is_same<type_no_ptr, T>::value && 
                         !type_traits::is_same<type_no_ptr&, type_no_ptr>::value)
                    {
                        result = entity_resolver<type_no_ptr, false>::entity(pred_arg, pred);
                        if (result) return result;
                    }
                }

                return result;
            }

            template<class ArgT>
            static 
            Entity* entity(const ArgT& pred_arg, bool(&pred)(const Entity&, const ArgT&))
            {
                Entity* result = entity_impl(pred_arg, pred);

                if (result) return result;

                typedef typename type_traits::first_level<T, type_traits::remove_const>::type type_no_const;
                typedef typename type_traits::first_level<T, type_traits::add_const>::type type_const;
                typedef typename type_traits::first_level<T, type_traits::remove_volatile>::type type_no_volatile;
                typedef typename type_traits::first_level<T, type_traits::remove_cv>::type type_no_cv;
                
                if ( !type_traits::is_same<type_no_const, T>::value )
                {
                    result = entity_resolver<type_no_const, true>::entity_impl(pred_arg, pred);
                    if (result) return result;
                }
                if( !type_traits::is_same<type_const, T>::value && 
                    !type_traits::is_same<type_const, type_no_const>::value)
                {
                    
                    result = entity_resolver<type_const, true>::entity_impl(pred_arg, pred);
                    if (result) return result;
                }
                if( !type_traits::is_same<type_no_volatile, T>::value &&
                    !type_traits::is_same<type_no_volatile, type_no_const>::value &&
                    !type_traits::is_same<type_no_volatile, type_const>::value)
                {
                    
                    result = entity_resolver<type_no_volatile, true>::entity_impl(pred_arg, pred);
                    if (result) return result;
                }
                if( !type_traits::is_same<type_no_cv, T>::value &&
                    !type_traits::is_same<type_no_cv, type_no_const>::value &&
                    !type_traits::is_same<type_no_cv, type_const>::value &&
                    !type_traits::is_same<type_no_cv, type_no_volatile>::value)
                {
                    
                    result = entity_resolver<type_no_cv, true>::entity_impl(pred_arg, pred);
                    if (result) return result;
                }
                return result;
            }

        };
    }

    namespace detail
    {
        template<class T>
        struct object_resolver;

        template<>
        struct object_resolver<void>
        {
            template<class T>
            static 
            const Object<T> object(const T& value)
            {
                return object_resolver<T>::object(value);
            }
        };

        template<class T>
        struct object_resolver
        {

            static 
            const Object<T> object(const T& value, bool extended_search = true)
            {
                struct lambdas
                {
                    static bool is_object(const Entity& entity, const T& value)
                    {
                        return entity.is_object && entity.value == value;
                    }
                };
                Entity* result = 0;
                if(!extended_search)
                    result = entity_resolver<T, false>::entity(value, lambdas::is_object);
                else
                    result = entity_resolver<T, true>::entity(value, lambdas::is_object);

                if (result) return *result;

                throw(make_exception::is_not_registered<T>(value, "object"));
            }

            static 
            const Object<T> object(const std::string& name, bool extended_search = true)
            {
                struct lambdas
                {
                    static bool is_object(const Entity& entity, const std::string& name)
                    {
                        return entity.is_object && entity.name == name;
                    }
                };
                
                Entity* result = 0;
                if (!extended_search)
                    result = entity_resolver<T, false>::entity(name, lambdas::is_object);
                else
                    result = entity_resolver<T, true>::entity(name, lambdas::is_object);

                if (result) return *result;

                throw(make_exception::is_not_registered<T>(name, "object"));
            }
        };
    }

    namespace detail
    {

        template<>
        struct member_resolver<void>
        {
            template<class ParentT, class T>
            static
            Member<ParentT, T ParentT::*> member(T ParentT::* value, bool extended_search = true)
            {
                typedef T ParentT::* type;
                struct lambdas
                {
                    typedef type captured_type;
                    static bool is_member(const Entity& entity, const captured_type& value)
                    {
                        if (!entity.is_member)
                            return false;

                        const captured_type* ptr = cppmeta::any_cast<captured_type>(&entity.value);
                        return  ptr && *ptr == value;
                    }
                };
                Entity* result = 0;
                if (!extended_search)
                    result = entity_resolver<ParentT, false>::entity(value, lambdas::is_member);
                else
                    result = entity_resolver<ParentT, true>::entity(value, lambdas::is_member);

                if (result) return *result;

                throw(make_exception::is_not_registered<T>(value, "member"));
            }
        };

        template<class ParentT>
        struct member_resolver
        {
            typedef 
            typename
            type_traits::conditional<
                type_traits::is_simple_type<ParentT>::value,
                disabled<__LINE__>,
                ParentT
            >::type parent_class;

            template<class T>
            static 
            Member<ParentT, T parent_class::*> member(T (parent_class::* value), bool extended_search = true)
            {
                return member_resolver<void>::member(value, extended_search);
            }

            template<class T>
            static 
            Member<ParentT, T parent_class::*> member(
                typename
                type_traits::conditional<
                    type_traits::is_same<parent_class, ParentT>::value,
                    const std::string&,
                    disabled<__LINE__>
                >::type name, bool extended_search = true)
            {
                typedef T parent_class::* member_type;

                struct lambdas
                {
                    typedef member_type captured_member_type;
                    static bool is_member(const Entity& entity, const std::string& name)
                    {
                        return entity.is_member && entity.name == name;
                    }
                };
                Entity* result = 0;
                if (!extended_search)
                    result = entity_resolver<ParentT, false>::entity(name, lambdas::is_member);
                else
                    result = entity_resolver<ParentT, true>::entity(name, lambdas::is_member);

                if (result) return *result;
                
                // looking for parent members if any
                for (std::size_t i = 0; i < detail::MembersProxy<ParentT>::grand_parent_entities().size(); ++i)
                {
                    const Entity &grand_parent_entity = 
                        detail::MembersProxy<ParentT>::grand_parent_entities()[i](name);
    
                    if (grand_parent_entity.stored_member_type_id != Entity::type_id<T>())
                        continue;

                    member_type value;
                    std::memcpy(&value, grand_parent_entity.value.data(), sizeof(member_type));

                    return Member<ParentT, member_type>(name, value);
                }

                throw(make_exception::is_not_registered<T>(name, "member"));
            }

            template<class T>
            static 
            Value<T> member(const std::string& name, 
                typename
                type_traits::conditional<
                    type_traits::is_const<T>::value,
                    const ParentT,
                    ParentT
                >::type *parent, bool extended_search = true)
            {
                typedef typename type_traits::remove_const<T>::type clear_type;
                typedef clear_type ParentT::*member_value;

                Member<ParentT, member_value> other = 
                    member<clear_type>(name, extended_search);
                T &value = parent->*other.value;

                return Value<T>(other.name, value);
            }

            template<class T>
            static 
            Value<T> member(const std::string& name, 
                typename
                type_traits::conditional<
                    type_traits::is_const<T>::value,
                    const ParentT,
                    ParentT
                >::type &parent, bool extended_search = true)
            {
                return member<T>(name, &parent, extended_search);
            }

            static 
            typename
            type_traits::conditional<
                type_traits::is_simple_type<ParentT>::value,
                const Entity&,
                Member<ParentT, void>
            >::type member1(const std::string& name)
            {
                typename detail::EntitiesStorage<ParentT>::type& entities = detail::EntitiesStorage<ParentT>::value();

                for (std::size_t i = 0; i < entities.size(); ++i)
                {
                    if (entities[i].name == name)
                    {
                        return entities[i];
                    }
                }

                throw(make_exception::is_not_registered<ParentT>(name, "member"));
            }
        };
        
        template<class ParentT, class T>
        struct member_resolver<T ParentT::*>
        {
            typedef T ParentT::*member_type;

            static 
            Member<ParentT, member_type> member(const member_type value, bool extended_search = true)
            {
                struct lambdas
                {
                    typedef member_type captured_member_type;
                    typedef T captured_T;
                    static bool is_member(const Entity& entity, const captured_member_type& value)
                    {
                        if (!entity.is_member) return false;

                        const captured_T* ptr = 
                            cppmeta::any_cast<captured_member_type>(&entity.value);
                        return ptr && *ptr == value;
                    }
                };
                Entity* result = 0;
                if (!extended_search)
                    result = entity_resolver<T, false>::entity(value, lambdas::is_member);
                else
                    result = entity_resolver<T, true>::entity(value, lambdas::is_member);

                if (result) return *result;

                throw(make_exception::is_not_registered<T>(value, "member"));
            }

            static 
            Member<ParentT, member_type> member(const std::string& name, bool extended_search = true)
            {
                struct lambdas
                {
                    typedef member_type captured_member_type;
                    static bool is_member(const Entity& entity, const captured_member_type& value)
                    {
                        return entity.is_member && entity.value == value;
                    }
                };
                Entity* result = 0;
                if (!extended_search)
                    result = entity_resolver<T, false>::entity(name, lambdas::is_member);
                else
                    result = entity_resolver<T, true>::entity(name, lambdas::is_member);

                if (result) return *result;

                // looking for parent members if any
                for (std::size_t i = 0; i < detail::MembersProxy<ParentT>::grand_parent_entities().size(); ++i)
                {
                    const Entity& grand_parent_entity =
                        detail::MembersProxy<ParentT>::grand_parent_entities()[i](name);

                    if (grand_parent_entity.stored_member_type_id != Entity::type_id<T>())
                        continue;

                    member_type value;
                    std::memcpy(&value, grand_parent_entity.value.data(), sizeof(member_type));

                    return Member<ParentT, member_type>(name, value);
                }

                throw(make_exception::is_not_registered<T>(name, "member"));
            }
        };


    }

    namespace detail
    {
        template<class>
        struct function_resolver;


        template<class FunctionT>
        struct function_resolver
        {

        };

        template<>
        struct function_resolver<void>
        {
        
        };

    }

    namespace detail
    {
        template<class>
        struct value_resolver;

        template<>
        struct value_resolver<void>
        {
            template<class T>
            static 
            typename
            type_traits::conditional<
                type_traits::is_simple_type<T>::value,
                Value<T>&,
                detail::disabled<__LINE__>
            >::type value(const T& value, bool extended_search = true)
            {
                struct lambdas
                {
                    typedef T captured_T;
                    static bool is_value(const Entity& entity, const T& value)
                    {
                        if (!entity.is_value)
                            return false;
                        const captured_T* ptr = cppmeta::any_cast<captured_T>(&entity.value);
                        return ptr && *ptr == value;
                    }
                };

                Entity* result = 0;
                if (!extended_search)
                    result = entity_resolver<T, false>::entity(value, lambdas::is_value);
                else
                    result = entity_resolver<T, true>::entity(value, lambdas::is_value);

                if (result) return *result;

                throw(make_exception::is_not_registered<T>(value, "value"));
            }       
        };

        template<class T>
        struct value_resolver
        {
            static 
            Value<T> value(
                typename
                type_traits::conditional<
                    type_traits::is_simple_type<T>::value,
                    const T&,
                    detail::disabled<__LINE__>
                >::type value)
            {
                return value_resolver<void>::value(value);
            }

            static 
            Value<T> value(
                typename
                type_traits::conditional<
                    type_traits::is_simple_type<T>::value,
                    const std::string&,
                    detail::disabled<__LINE__>
                >::type name, bool extended_search = true)
            {
                struct lambdas
                {
                    static bool is_value(const Entity& entity, const std::string& name)
                    {
                        return entity.is_value && entity.name == name;
                    }
                };

                Entity* result = 0;
                if (!extended_search)
                    result = entity_resolver<T, false>::entity(name, lambdas::is_value);
                else
                    result = entity_resolver<T, true>::entity(name, lambdas::is_value);

                if (result) return *result;

                throw(make_exception::is_not_registered<T>(name, "value"));
            }

            static 
            Value<T> constant(
                typename
                type_traits::conditional<
                    type_traits::is_simple_type<T>::value,
                    const std::string&,
                    detail::disabled<__LINE__>
                >::type name)
            {
                return value(name);
            }
        };


    }

    template<class T>
    struct resolve<T>
        : private detail::type_meta<T>
        , detail::object_resolver<T>
        , detail::member_resolver<T>
        , detail::function_resolver<T>
        , detail::value_resolver<T>
    {
        static const std::string& name;
    };

    template<class T>
    const std::string& resolve<T>::name =
        detail::type_meta<T>::get_name();


    template<class ClassT>
    struct class_
    {
        static void destructor(ClassT& obj) { delete &obj; }
        static ClassT constructor() { return ClassT(); }
        static ClassT default_constructor() { return ClassT(); }
        template<class Arg0T>
        static ClassT constructor(Arg0T arg0) { return ClassT(arg0); }
        template<class Arg0T, class Arg1T>
        static ClassT constructor(Arg0T arg0, Arg1T arg1) { return ClassT(arg0, arg1); }
        template<class Arg0T, class Arg1T, class Arg2T>
        static ClassT constructor(Arg0T arg0, Arg1T arg1, Arg2T arg2) { return ClassT(arg0, arg1, arg2); }

    };

    template<class StructT>
    struct struct_
        : class_<StructT> {};

    template<class T>
    detail::entities_containter<T> operator,(detail::entities_containter<T> container, const EntityProxy<T>& obj)
    {
        container.members.push_back(obj.value);
        return container;
    }

    template<class ClassT, class T>
    detail::entities_containter<ClassT> operator,(detail::entities_containter<ClassT> container, const MemberProxy<ClassT, T>& obj)
    {
        container.entities.push_back(obj.value);
        return container;
    }

    template<class T>
    detail::entities_containter<T> operator,(detail::entities_containter<T> container, const ValueProxy<T>& obj)
    {
        container.entities.push_back(obj.value);
        return container;
    }

    template<class T>
    detail::entities_containter<T> operator,(detail::entities_containter<T> container, const ObjectProxy<T>& obj)
    {
        container.entities.push_back(obj.value);
        return container;
    }

    template<class ParentT>
    detail::entities_containter<ParentT> operator,(const std::string&, const detail::entities_containter<ParentT>& container)
    {
        return container;
    }
}

namespace cppmeta
{
    template <class T>
    ValueProxy<T> value(const std::string &name, T& input)
    {
        return Value<T>(name, input);
    }

    template <class T>
    ValueProxy<T> constant(const std::string &name, const T& input)
    {
        return Value<T>(name, input);
    }
}

namespace cppmeta
{
    template <class T>
    ObjectProxy<T&> reference(const std::string &name, T& input)
    {
        return Object<T*>(name, &input);
    }

    template <class T>
    ObjectProxy<const T*> const_reference(const std::string &name, const T& input)
    {
        return Object<const T&>(name, &input);
    }
}

namespace cppmeta
{
    template <class T>
    ObjectProxy<T*> object(const std::string &name, T& input)
    {
        return Object<T*>(name, &input);
    }

    template <class T>
    ObjectProxy<T*> object(const std::string &name, T* input)
    {
        return Object<T*>(name, input);
    }

    template <class T>
    ObjectProxy<const T*> const_object(const std::string &name, const T* input)
    {
        return Object<const T*>(name, input);
    }
}

namespace cppmeta
{
    template <class ClassT, class T>
    MemberProxy<ClassT, T ClassT::*> member(const std::string &name, T ClassT::* ptr)
    {
        return Member<ClassT, T ClassT::*>(name, ptr);
    }

    template <class ClassT>
    MemberProxy<ClassT, ClassT(*)()> member(const std::string &name, ClassT(*constructor)())
    {
        return Member<ClassT, ClassT(*)() >(name, constructor);
    }

    template <class ClassT>
    MemberProxy<ClassT, void(*)(ClassT&)> member(const std::string &name, void(*destructor)(ClassT&))
    {
        return Member<ClassT, void(*)(ClassT&) >(name, destructor);
    }

    template <class ClassT, class Arg0T>
    MemberProxy<ClassT, ClassT(*)(Arg0T)> member(const std::string &name, ClassT(*constructor)(Arg0T))
    {
        return Member<ClassT, ClassT(*)(Arg0T) >(name, constructor);
    }

    template <class ClassT, class Arg0T, class Arg1T>
    MemberProxy<ClassT, ClassT(*)(Arg0T, Arg1T)> member(const std::string &name, ClassT(*constructor)(Arg0T, Arg1T))
    {
        return Member<ClassT, ClassT(*)(Arg0T, Arg1T) >(name, constructor);
    }
}

namespace cppmeta
{
    template<class FuncT>
    struct FunctorProxy
    {
        FuncT value;
        FunctorProxy(FuncT func) :
            value(func) {}
    };

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, FunctorProxy<typename entity_types<ClassT, T>::ref_getter_member_func_ptr_type> getterPtr, FunctorProxy<typename entity_types<ClassT, T>::ref_setter_member_func_ptr_type> setterPtr);

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, FunctorProxy<typename entity_types<ClassT, T>::ref_getter_func_ptr_type> getterPtr, FunctorProxy<typename entity_types<ClassT, T>::ref_setter_func_ptr_type> setterPtr)
    {
        return Property<ClassT, T>(name, getterPtr.value, setterPtr.value);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, FunctorProxy<typename entity_types<ClassT, T>::val_getter_member_func_ptr_type> getterPtr, FunctorProxy<typename entity_types<ClassT, T>::val_setter_member_func_ptr_type> setterPtr)
    {
        return Property<ClassT, T>(name, getterPtr.value, setterPtr.value);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, FunctorProxy<typename entity_types<ClassT, T>::val_getter_func_ptr_type> getterPtr, FunctorProxy<typename entity_types<ClassT, T>::val_member_func_ptr_type> setterPtr)
    {
        return Property<ClassT, T>(name, getterPtr.value, setterPtr.value);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, FunctorProxy<typename entity_types<ClassT, T>::ref_getter_member_func_ptr_type> getterPtr, FunctorProxy<typename entity_types<ClassT, T>::val_setter_member_func_ptr_type> setterPtr)
    {
        return Property<ClassT, T>(name, getterPtr.value, setterPtr.value);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, FunctorProxy<typename entity_types<ClassT, T>::ref_getter_func_ptr_type> getterPtr, FunctorProxy<typename entity_types<ClassT, T>::val_member_func_ptr_type> setterPtr)
    {
        return Property<ClassT, T>(name, getterPtr.value, setterPtr.value);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, FunctorProxy<typename entity_types<ClassT, T>::val_getter_member_func_ptr_type> getterPtr, FunctorProxy<typename entity_types<ClassT, T>::ref_setter_member_func_ptr_type> setterPtr)
    {
        return Property<ClassT, T>(name, getterPtr.value, setterPtr.value);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, FunctorProxy<typename entity_types<ClassT, T>::val_getter_func_ptr_type> getterPtr, FunctorProxy<typename entity_types<ClassT, T>::ref_setter_func_ptr_type> setterPtr)
    {
        return Property<ClassT, T>(name, getterPtr.value, setterPtr.value);
    }

    // read only
    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, FunctorProxy<typename entity_types<ClassT, T>::ref_getter_member_func_ptr_type> getterPtr)
    {
        typename entity_types<ClassT, T>::val_setter_member_func_ptr_type dummy(CPPMETA_NULL);
        return Property<ClassT, T>(name, getterPtr.value, dummy);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, FunctorProxy<typename entity_types<ClassT, T>::ref_getter_func_ptr_type> getterPtr)
    {
        typename entity_types<ClassT, T>::val_setter_func_ptr_type dummy(CPPMETA_NULL);
        return Property<ClassT, T>(name, getterPtr.value, dummy);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, FunctorProxy<typename entity_types<ClassT, T>::val_getter_member_func_ptr_type> getterPtr)
    {
        typename entity_types<ClassT, T>::val_setter_member_func_ptr_type dummy(CPPMETA_NULL);
        return Property<ClassT, T>(name, getterPtr.value, dummy);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, FunctorProxy<typename entity_types<ClassT, T>::val_getter_func_ptr_type> getterPtr)
    {
        typename entity_types<ClassT, T>::val_setter_func_ptr_type dummy(CPPMETA_NULL);
        return Property<ClassT, T>(name, getterPtr.value, dummy);
    }

    // set only 
    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, FunctorProxy<typename entity_types<ClassT, T>::ref_setter_member_func_ptr_type> setterPtr)
    {
        typename entity_types<ClassT, T>::val_getter_member_func_ptr_type dummy(CPPMETA_NULL);
        return Property<ClassT, T>(name, dummy, setterPtr.value);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, FunctorProxy<typename entity_types<ClassT, T>::ref_setter_func_ptr_type> setterPtr)
    {
        typename entity_types<ClassT, T>::val_getter_func_ptr_type dummy(CPPMETA_NULL);
        return Property<ClassT, T>(name, dummy, setterPtr.value);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, FunctorProxy<typename entity_types<ClassT, T>::val_setter_member_func_ptr_type> setterPtr)
    {
        typename entity_types<ClassT, T>::val_getter_member_func_ptr_type dummy(CPPMETA_NULL);
        return Property<ClassT, T>(name, dummy, setterPtr.value);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, FunctorProxy<typename entity_types<ClassT, T>::val_setter_func_ptr_type> setterPtr)
    {
        typename entity_types<ClassT, T>::val_getter_func_ptr_type dummy(CPPMETA_NULL);
        return Property<ClassT, T>(name, dummy, setterPtr.value);
    }


    // templated functors

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, const T& (ClassT::* getterPtr)() const, void (ClassT::* setterPtr)(const T&))
    {
        return Property<ClassT, T>(name, getterPtr, setterPtr);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, const T& (* getterPtr)(const ClassT&), void (* setterPtr)(ClassT&, const T&))
    {
        return Property<ClassT, T>(name, getterPtr, setterPtr);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, T(ClassT::* getterPtr)() const, void (ClassT::* setterPtr)(T))
    {
        return Property<ClassT, T>(name, getterPtr, setterPtr);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, T(*getterPtr)(const ClassT&), void (*setterPtr)(ClassT&, T))
    {
        return Property<ClassT, T>(name, getterPtr, setterPtr);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, const T& (ClassT::* getterPtr)() const, void (ClassT::* setterPtr)(T))
    {
        return Property<ClassT, T>(name, getterPtr, setterPtr);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, const T& (*getterPtr)(const ClassT&), void (*setterPtr)(ClassT&, T))
    {
        return Property<ClassT, T>(name, getterPtr, setterPtr);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, T(ClassT::* getterPtr)() const, void (ClassT::* setterPtr)(const T&))
    {
        return Property<ClassT, T>(name, getterPtr, setterPtr);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, T(*getterPtr)(const ClassT&), void (*setterPtr)(ClassT&, const T&))
    {
        return Property<ClassT, T>(name, getterPtr, setterPtr);
    }

    // read only
    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, const T& (ClassT::* getterPtr)() const)
    {
        typename entity_types<ClassT, T>::val_setter_member_func_ptr_type dummy(CPPMETA_NULL);
        return Property<ClassT, T>(name, getterPtr, dummy);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, const T& (*getterPtr)(const ClassT&))
    {
        typename entity_types<ClassT, T>::val_setter_func_ptr_type dummy(CPPMETA_NULL);
        return Property<ClassT, T>(name, getterPtr, dummy);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, T(ClassT::* getterPtr)() const)
    {
        typename entity_types<ClassT, T>::val_setter_member_func_ptr_type dummy(CPPMETA_NULL);
        return Property<ClassT, T>(name, getterPtr, dummy);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, T(*getterPtr)(const ClassT&))
    {
        typename entity_types<ClassT, T>::val_setter_func_ptr_type dummy(CPPMETA_NULL);
        return Property<ClassT, T>(name, getterPtr, dummy);
    }

    // set only 
    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, void (ClassT::* setterPtr)(const T&))
    {
        typename entity_types<ClassT, T>::val_getter_member_func_ptr_type dummy(CPPMETA_NULL);
        return Property<ClassT, T>(name, dummy, setterPtr);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, void (*setterPtr)(ClassT&, const T&))
    {
        typename entity_types<ClassT, T>::val_getter_func_ptr_type dummy(CPPMETA_NULL);
        return Property<ClassT, T>(name, dummy, setterPtr);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, void (ClassT::* setterPtr)(T))
    {
        typename entity_types<ClassT, T>::val_getter_member_func_ptr_type dummy(CPPMETA_NULL);
        return Property<ClassT, T>(name, dummy, setterPtr);
    }

    template <class ClassT, class T>
    PropertyProxy<ClassT, T> property(std::string name, void (*setterPtr)(ClassT&, T))
    {
        typename entity_types<ClassT, T>::val_getter_func_ptr_type dummy(CPPMETA_NULL);
        return Property<ClassT, T>(name, dummy, setterPtr);
    }
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#undef CPPMETA_NULL

#define CPPMETA_REFLECT(intype)\
    template<class EntitiesT> \
    struct class_<intype, EntitiesT>{ static EntitiesT members();};\
    template<class EntitiesT> \
    EntitiesT class_<intype, EntitiesT>::members()

#define CPPMETA_REFLECT_ENUM(intype)\
    template<class EntitiesT> \
    struct enum_<intype, EntitiesT>{ static EntitiesT members();};\
    template<class EntitiesT> \
    EntitiesT enum_<intype, EntitiesT>::members()

#define CPPMETA_MEMBERS return cppmeta::members<EntitiesT>(),

#endif
