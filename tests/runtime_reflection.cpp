

#include "tools/tools.h"

#include "./../cppmeta.hpp"

template<>
cppmeta::reflexpr<tools::type_for_reflection>::meta_type
cppmeta::reflexpr<tools::type_for_reflection>::meta =
(
    name = "type_for_ct_reflection1",
    members =
        cppmeta::member("type_for_ct_reflection()", &default_constructor),
        cppmeta::member("type_for_ct_reflection(int, float)", &constructor<int, float>),
        cppmeta::member("constructor2", &constructor<int, int>),
        cppmeta::member("~type_for_ct_reflection", &destructor),
        cppmeta::member("data2", &tools::type_for_reflection::data2),
        cppmeta::member("func1", &tools::type_for_reflection::func1),
        cppmeta::member("templ_func1(int)", &tools::type_for_reflection::templ_func1<int>)
);

TEST_SUITE("runtime reflection")
{

TEST_CASE("reflect some types"){
    using namespace cppmeta;

    int some_data_rt = tools::rand();
    const int some_data_rt_const = tools::rand();

    reflexpr<int>::meta = 
    (
        objects += 
              object("some_data_rt", &some_data_rt)
            , object("some_data_rt_ref", some_data_rt)
            , object("some_data_rt_const", some_data_rt_const)
        ,values +=
            constant("max_int", 2048)
        ;
     );
    
    SUBCASE("resolving object by value")
    {
        int some_data_rt_resolved =
            reflexpr<int>::object("some_data_rt").value;
        const int& some_data_rt_const_resolved =
            reflexpr<int>::object("some_data_rt_const").value;

        CHECK(some_data_rt_resolved == some_data_rt);
        CHECK(some_data_rt_const_resolved == some_data_rt_const);
    }

    SUBCASE("resolving object by const value")
    {
        const int& some_data_rt_resolved =
            reflexpr<const int>::object("some_data_rt").value;
        const int& some_data_rt_const_resolved =
            reflexpr<const int>::object("some_data_rt_const").value;

        CHECK(some_data_rt_resolved == some_data_rt);
        CHECK(some_data_rt_const_resolved == some_data_rt_const);
    }

    SUBCASE("resolving object by reference")
    {
        int& some_data_rt_resolved =
            reflexpr<int&>::object("some_data_rt").value;
        CHECK(some_data_rt_resolved == some_data_rt);

        REQUIRE(some_data_rt_resolved != 42);
        some_data_rt_resolved = 42;
        CHECK(some_data_rt_resolved == 42);
        CHECK(some_data_rt == 42);

        const int& some_data_rt_const_resolved =
            reflexpr<const int&>::object("some_data_rt_const").value;

        CHECK(some_data_rt_const_resolved == some_data_rt_const);
    }
}

TEST_CASE("reflect some class"){
    using namespace cppmeta;

    typedef tools::type_for_reflection type_for_rt_reflection;

    reflexpr<type_for_rt_reflection>::meta =
    (
        name = "type_for_rt_reflection",
        members = 
             member("type_for_reflection()", &class_<type_for_rt_reflection>::default_constructor)
            ,member("data1", &type_for_rt_reflection::data1)
    );
    
    SUBCASE("resolving class data")
    {
        int type_for_rt_reflection::*  data1_tmp =
            resolve<type_for_rt_reflection>::member<int>("data1").value;
    );
    
    SUBCASE("resolving class data1")
    {
        int type_for_rt_reflection::*  data1_tmp =
            reflexpr<type_for_rt_reflection>::member<int>("data1").value;

        type_for_rt_reflection obj;
        obj.data1 = true;
        CHECK(obj.data1 == obj.*data1_tmp); // true
        obj.data1 = false;
        CHECK(obj.data1 == obj.*data1_tmp); // true
    }

    SUBCASE("resolving class data2")
    {
        float type_for_rt_reflection::* data2_tmp =
            reflexpr<type_for_rt_reflection>::member<float>("data2").value;

        type_for_rt_reflection obj;
        obj.data2 = true;
        CHECK(obj.data2 == obj.*data2_tmp); // true
        obj.data2 = false;
        CHECK(obj.data2 == obj.*data2_tmp); // true
    }
}



}
