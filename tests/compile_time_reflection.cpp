

#include "tools/tools.h"

#include "./../cppmeta.hpp"

namespace resources
{
    typedef tools::type_for_reflection type_for_ct_reflection;


    int some_data_rt = tools::rand();
    const int some_data_rt_const = tools::rand();
}



template<>
struct cppmeta::reflect<resources::type_for_ct_reflection, cppmeta::reflection::compile_time>
{   
    typedef resources::type_for_ct_reflection type_for_ct_reflection;

    template<class meta>
    class info
    {
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

TEST_SUITE("compile-time reflection")
{
    using namespace resources;

TEST_CASE("reflect some types"){
    using namespace cppmeta;


    reflect<int>::
        objects += 
              object("some_data_rt", &some_data_rt)
            , object("some_data_rt_ref", some_data_rt)
            , object("some_data_rt_const", some_data_rt_const)
        ,values +=
            constant("max_int", 2048)
        ;
    
    SUBCASE("resolving object by value")
    {
        int some_data_rt_resolved =
            resolve<int>::object("some_data_rt").value;
        const int& some_data_rt_const_resolved =
            resolve<int>::object("some_data_rt_const").value;

        CHECK(some_data_rt_resolved == some_data_rt);
        CHECK(some_data_rt_const_resolved == some_data_rt_const);
    }

    SUBCASE("resolving object by const value")
    {
        const int& some_data_rt_resolved =
            resolve<const int>::object("some_data_rt").value;
        const int& some_data_rt_const_resolved =
            resolve<const int>::object("some_data_rt_const").value;

        CHECK(some_data_rt_resolved == some_data_rt);
        CHECK(some_data_rt_const_resolved == some_data_rt_const);
    }

    SUBCASE("resolving object by reference")
    {
        int& some_data_rt_resolved =
            resolve<int&>::object("some_data_rt").value;
        CHECK(some_data_rt_resolved == some_data_rt);

        REQUIRE(some_data_rt_resolved != 42);
        some_data_rt_resolved = 42;
        CHECK(some_data_rt_resolved == 42);
        CHECK(some_data_rt == 42);

        const int& some_data_rt_const_resolved =
            resolve<const int&>::object("some_data_rt_const").value;

        CHECK(some_data_rt_const_resolved == some_data_rt_const);
    }
}

TEST_CASE("reflect some class"){
    using namespace cppmeta;

    
    SUBCASE("resolving class data")
    {
        int type_for_ct_reflection::*  data1_tmp =
            resolve<type_for_ct_reflection>::member<int>("data1").value;

        type_for_ct_reflection obj;
        obj.data1 = true;
        CHECK(obj.data1 == obj.*data1_tmp); // true
        obj.data1 = false;
        CHECK(obj.data1 == obj.*data1_tmp); // true
    }
}



}
