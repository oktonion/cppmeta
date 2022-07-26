

#include "tools/tools.h"

#include "./../cppmeta.hpp"


TEST_SUITE("runtime reflection")
{


TEST_CASE("reflect some types"){
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