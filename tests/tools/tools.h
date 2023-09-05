#ifndef CPPMETA_TEST_TOOLS_H
#define CPPMETA_TEST_TOOLS_H


#include "./doctest/doctest.h"

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

    inline unsigned int rand()
    {
        static unsigned int seed = 4541;
        seed = (8253729 * seed + 2396403);
        return seed % 32768;
    }
}

#endif // CPPMETA_TEST_TOOLS_H
