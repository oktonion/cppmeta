

#include "tools/tools.h"

#include "./../cppmeta.hpp"


TEST_SUITE("runtime reflection")
{

TEST_CASE("reflect some types"){
    using namespace cppmeta;

    int some_data_rt = tools::rand();
    const int some_data_rt_const = tools::rand();

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
}

TEST_CASE("reflect some class"){
    using namespace cppmeta;

    typedef tools::type_for_reflection type_for_rt_reflection;

    reflect<type_for_rt_reflection>::
        name = "type_for_rt_reflection",
        members = 
             member("type_for_reflection()", &class_<type_for_rt_reflection>::default_constructor)
            ,member("data1", &type_for_rt_reflection::data1)
        ;
    
    SUBCASE("resolving class data")
    {
        int type_for_rt_reflection::*  data1_tmp =
            resolve<type_for_rt_reflection>::member<int>("data1").value;

        type_for_rt_reflection obj;
        obj.data1 = true;
        CHECK(obj.data1 == obj.*data1_tmp); // true
        obj.data1 = false;
        CHECK(obj.data1 == obj.*data1_tmp); // true
    }
}



}
