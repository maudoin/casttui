


#pragma once

#include <type_traits>

template<typename T, std::enable_if_t<std::is_enum_v<T>, bool> = true>
struct Flags
{
    constexpr Flags():bits{0}{}
    template <typename ...Args, std::enable_if_t<(std::is_enum_v<Args>&&...), bool> = true>
    constexpr Flags(Args&&...args):bits{( mask(args) | ... )}{}
    constexpr bool operator &&(T const e)const{return  mask(e) == (bits & mask(e));}
    const unsigned int bits;
private:
    static constexpr unsigned int mask(T const i){return 1<<static_cast<int>(i);}
};

enum class Test : int{ A=0, B, C};
static_assert(!(Flags<Test>(Test::A, Test::C)&&Test::B), "?");
static_assert(!(Flags<Test>(Test::A, Test::B)&&Test::C), "?");
static_assert(!(Flags<Test>(Test::B, Test::C)&&Test::A), "?");
static_assert(Flags<Test>().bits==0, "?");
static_assert(Flags<Test>(Test::A).bits==1, "?");
static_assert(Flags<Test>(Test::B).bits==2, "?");
static_assert(Flags<Test>(Test::C).bits==4, "?");
static_assert(Flags<Test>(Test::A, Test::B).bits==3, "?");
static_assert(Flags<Test>(Test::A, Test::C).bits==5, "?");
static_assert(Flags<Test>(Test::B, Test::C).bits==6, "?");
static_assert(Flags<Test>(Test::A, Test::B, Test::C).bits==7, "?");
static_assert(Flags<Test>(Test::A, Test::B, Test::C)&&Test::A, "?");
static_assert(Flags<Test>(Test::A, Test::B, Test::C)&&Test::B, "?");
static_assert(Flags<Test>(Test::A, Test::B, Test::C)&&Test::C, "?");
