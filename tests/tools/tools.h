#ifndef CPPMETA_TEST_TOOLS_H
#define CPPMETA_TEST_TOOLS_H

#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
    #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
    #include "./doctest/doctest.h"
#endif // DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

namespace tools
{
    struct type_for_reflection
    {
        type_for_reflection() {}
        type_for_reflection(int, float) {}
        type_for_reflection(int, int) {}

        int data1;

        void func1(float) {}

        template<class T>
        void templ_func1(T) {}
    };

    struct type_for_reflection_child
        : type_for_reflection 
    {
        int data2;
    };

    int some_data_rt = 0;
    const int some_data_rt_const = 0;

    enum enum_for_reflection
    {
        en_1,
        en_2
    };
}

#endif // CPPMETA_TEST_TOOLS_H