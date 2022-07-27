# cppmeta
one header tiny C++ type meta-information library for reflection and serialization for classes, functions, types

- single header
- no external dependencies except for small set of C++98 std library headers
- terms used for registering and retrieving meta-information are standard complient

----------

Usage examples
----------

## Compile-time reflection

### Manual registration
```cpp
#include <cppmeta.hpp>

struct type_for_ct_reflection
{
    type_for_ct_reflection() {}
    type_for_ct_reflection(int, float) {}
    type_for_ct_reflection(int, int) {}

    int data1;

    void func1(float) {}

    template<class T>
    void templ_func1(T) {}
};

template<>
struct cppmeta::reflect<type_for_ct_reflection, cppmeta::reflection::compile_time>
{
    template<class meta>
    static void info()
    {
        meta::
            name = "type_for_ct_reflection",
            members = 
                member("type_for_ct_reflection()", &class_<type_for_ct_reflection>::default_constructor),
                member("type_for_ct_reflection(int, float)", &class_<type_for_ct_reflection>::constructor<int, float>),
                member("constructor2", &class_<type_for_ct_reflection>::constructor<int, int>),
                member("~type_for_ct_reflection", &class_<type_for_ct_reflection>::destructor),
                member("data1", &type_for_ct_reflection::data1),
                member("func1", &type_for_ct_reflection::func1),
                member("templ_func1(int)", &type_for_ct_reflection::templ_func1<int>)
            ;
    }
};

int some_data_ct;

template<>
struct cppmeta::reflect_ct<int>
{
    template<class meta>
    static void info()
    {
        meta::
            objects += 
                object("some_data_ct", &some_data_ct);
            ;
    }
};
```

### Class member pointer resolve
```cpp
using namespace cppmeta;
int type_for_ct_reflection::* data1_tmp =
    resolve<type_for_ct_reflection>::member<int>("data1").value;
type_for_ct_reflection obj;
obj.data1 == obj.*data1_tmp; // true
```

## Runtime reflection

### Manual registration
```cpp
#include <cppmeta.hpp>

struct type_for_rt_reflection
{
    type_for_rt_reflection() {}
    type_for_rt_reflection(int, float) {}
    type_for_rt_reflection(int, int) {}

    int data1;

    void func1(float) {}

    template<class T>
    void templ_func1(T) {}
};

struct type_for_rt_reflection_child
    : type_for_rt_reflection 
{
    int data2;
};

int some_data_rt = 0;
const int some_data_rt_const = 0;

enum enum_for_rt_reflection
{
    rt_1,
    rt_2
};

void MyCode()
{
    using namespace cppmeta;

    reflect<type_for_rt_reflection>::
        name = "type_for_rt_reflection",
        members = 
             member("type_for_rt_reflection()", &class_<type_for_rt_reflection>::default_constructor)
            ,member("data1", &type_for_rt_reflection::data1)
        ;

    reflect<type_for_rt_reflection_child>::
        name = "type_for_rt_reflection_child",
        members =
              member("type_for_rt_reflection_child()", &class_<type_for_rt_reflection_child>::default_constructor)
            , member("data2", &type_for_rt_reflection_child::data2)
            , reflect<type_for_rt_reflection>::members // add parent members to child (optional)
        ;

    reflect<int>::
        objects += 
              object("some_data_rt", &some_data_rt)
            , object("some_data_rt_ref", some_data_rt)
            , object("some_data_rt_const", some_data_rt_const)
        ,values +=
            constant("max_int", 2048)
        ;

    reflect<enum_for_rt_reflection>::
            name = "enum_for_rt_reflection",
            values = 
                constant("rt_1", rt_1),
                constant("rt_2", rt_2)
            ;
}

```
### Class member pointer resolve
```cpp
int type_for_rt_reflection::*  data1_tmp =
    resolve<type_for_rt_reflection>::member<int>("data1").value;

type_for_rt_reflection obj;
obj.data1 == obj.*data1_tmp; // true

int type_for_rt_reflection_child::* child_data1_tmp = 
    resolve<type_for_rt_reflection_child>::member<int>("data1").value;

type_for_rt_reflection_child obj_child;
obj_child.data1 == obj_child.*child_data1_tmp; // true
```

### Object by value resolve
```cpp
int some_data_rt_resolved =
    resolve<int>::object("some_data_rt").value;
const int& some_data_rt_resolved =
    resolve<int>::object("some_data_rt_const").value;
```

### Object by const value resolve
```cpp
const int& some_data_rt_const_resolved =
    resolve<const int>::object("some_data_rt").value;
const int& some_data_rt_const_resolved =
    resolve<const int>::object("some_data_rt_const").value;
```
### Object by reference resolve
```cpp
int& some_data_rt_ref_resolved =
    resolve<int&>::object("some_data_rt").value;
some_data_rt_ref_resolved = 42; // changes `some_data_rt`
const int& some_data_rt_ref_const_resolved =
    resolve<const int&>::object("some_data_rt_const").value;
```

### Object by pointer resolve
```cpp
int* some_data_rt_ptr_resolved =
    resolve<int*>::object("some_data_rt").value;
(*some_data_rt_ref_resolved) = 42; // changes `some_data_rt`
const int* some_data_rt_ptr_const_resolved =
    resolve<const int*>::object("some_data_rt_const").value;
```

### Const value resolve
```cpp
const int& max_int_const =
    resolve<const int>::constant("max_int").value;
```

### Enum values resolve
```cpp
const enum_for_rt_reflection& rt_1_const =
    resolve<enum_for_rt_reflection>::value("rt_1").value;
```

### Invoke functions
```cpp
// in progress
```