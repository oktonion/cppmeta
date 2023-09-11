
#include "tools/tools.h"

#include <string>
#include <tuple>

namespace cppmeta
{
    static int members;
    template<class A, class B>
    static int member(A, B) { return -1; }

    static void default_constructor() {}
    template<class Arg0T>
    static void constructor(Arg0T arg0) {  }
    template<class Arg0T, class Arg1T>
    static void constructor(Arg0T arg0, Arg1T arg1) {  }
    template<class Arg0T, class Arg1T, class Arg2T>
    static void constructor(Arg0T arg0, Arg1T arg1, Arg2T arg2) {  }
    static void destructor() {}

    static std::string names;

    namespace declare
    {
        template<class MemberT, MemberT, const char*>
        struct declare;

        template<class PrevMemberT, PrevMemberT PrevMember, const char* PrevName>
        struct declare_impl
        {
            template<class MemberT, MemberT Member, const char* Name>
            struct declare_next
                : cppmeta::declare::declare<MemberT, Member, Name>
            { 
                typedef declare<MemberT, Member, Name> type;
                typedef declare_next<PrevMemberT, PrevMember, PrevName> prev_type;
            };
        };

        template<class MemberT, MemberT Member, const char* Name>
        struct declare
            : cppmeta::declare::declare_impl<MemberT, Member, Name>
        { 
            typedef declare<MemberT, Member, Name> type;
        };
    }

    template<class T>
    struct reflexpr
    {
    private:
        class meta_type {
        public:
            meta_type(int) {}
            int operator[](int) { return -1; }
        };
    public:
        struct meta_info;
        
        static meta_type meta;
    private:
        static std::string names;
        static int members;
    };

    namespace reflect
    {
        template<class T>
        struct get_data_members
        {
            typedef typename reflexpr<T>::meta_info::type type;
        };

        template<class>
        struct get_type;
        template<class>
        struct get_name;

        template<class MemberT, MemberT Member, const char* Name>
        struct get_type<
            declare::declare<MemberT, Member, Name>
        >
        {
            typedef MemberT type;
        };
    }

    template<class T> std::string reflexpr<T>::names;
    template<class T> int reflexpr<T>::members;
    template<class T> typename reflexpr<T>::meta_type reflexpr<T>::meta;

    int olololo;
}

namespace cppmeta
{
    namespace type_for_reflection_names
    {
        extern const char default_constructor[] = "type_for_reflection()";
        extern const char constructor_1[]       = "type_for_reflection(int, float)";
        extern const char constructor_2[]       = "type_for_reflection(int, int)";
        extern const char destructor[]          = "~type_for_reflection()";
        extern const char data1[]               = "int type_for_reflection::data1";
        extern const char data2[]               = "float type_for_reflection::data2";
        extern const char func1[]               = "void type_for_reflection::func1(float)";
        extern const char templ_func1[]         = "template <class T> void type_for_reflection::templ_func1(T)";
    }
    template<>
    struct reflexpr<tools::type_for_reflection>::meta_info
    {
        typedef 
        declare::declare<void(*)(), &default_constructor, type_for_reflection_names::default_constructor>
               ::declare_next<void(*)(int, float), &constructor<int, float>, type_for_reflection_names::constructor_1>
               ::declare<void(*)(int, int), &constructor<int, int>, type_for_reflection_names::constructor_2>
               ::declare_next<void(*)(), &destructor, type_for_reflection_names::destructor>
               ::declare<int tools::type_for_reflection::*, &tools::type_for_reflection::data1, type_for_reflection_names::data1>
               ::declare_next<float tools::type_for_reflection::*, &tools::type_for_reflection::data2, type_for_reflection_names::data2>
               ::declare<void (tools::type_for_reflection::*)(float), &tools::type_for_reflection::func1, type_for_reflection_names::func1>
               ::declare_next<void (tools::type_for_reflection::*)(int), &tools::type_for_reflection::templ_func1<int>, type_for_reflection_names::templ_func1>
        ::type type;
    };
}

template<> 
cppmeta::reflexpr<tools::type_for_reflection>::meta_type
cppmeta::reflexpr<tools::type_for_reflection>::meta =
(
    names = "type_for_ct_reflection",
    members =
        member("type_for_ct_reflection()", &default_constructor),
        member("type_for_ct_reflection(int, float)", &constructor<int, float>),
        member("constructor2", &constructor<int, int>),
        member("~type_for_ct_reflection", &destructor),
        member("data1", &tools::type_for_reflection::data1),
        member("data2", &tools::type_for_reflection::data2),
        member("func1", &tools::type_for_reflection::func1),
        member("templ_func1(int)", &tools::type_for_reflection::templ_func1<int>)
);

TEST_SUITE("syntax")
{
    TEST_CASE("reflect some types") 
    {
        typedef
        cppmeta::reflexpr<tools::type_for_reflection> reflection;

        reflection::meta;

        cppmeta::reflect::get_type< reflection::meta_info::type>::type a; (void)(a);
    }

    TEST_CASE("reflect some types")
    { 
        {
            using namespace cppmeta;
            reflexpr<tools::type_for_reflection>::meta =
            (
                names = "type_for_ct_reflection",
                members =
                    member("type_for_ct_reflection()", &default_constructor),
                    member("type_for_ct_reflection(int, float)", &constructor<int, float>),
                    member("constructor2", &constructor<int, int>),
                    member("~type_for_ct_reflection", &destructor),
                    member("data1", &tools::type_for_reflection::data1),
                    member("data2", &tools::type_for_reflection::data2),
                    member("func1", &tools::type_for_reflection::func1),
                    member("templ_func1(int)", &tools::type_for_reflection::templ_func1<int>)
            );
        }
    }
}