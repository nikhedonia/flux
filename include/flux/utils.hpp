#ifndef FLUX_UTILS_HPP
#define FLUX_UTILS_HPP

#include<chrono>

namespace flux{
    template<class F,class...D>
    auto callIF(F f,D&&...d) ->
    typename std::enable_if<!!sizeof...(D),decltype( f(std::forward<D>(d)...) )>::type {
        return f(std::forward<D>(d)...);
    }

    template<class F,class...args>
    auto callIF(F f,args...) -> decltype( f() ) {
        return f();
    }

    template<class T>
    T Obj(...);

    auto  operator""_s(unsigned long long const t){
        return std::chrono::seconds(t);
    }

    auto operator ""_m(unsigned long long const t){
        return std::chrono::minutes(t);
    }

    auto operator ""_h(unsigned long long const t){
        return std::chrono::hours(t);
    }

    auto operator ""_ms(unsigned long long const t){
        return std::chrono::milliseconds(t);
    }

}


#endif // UTILS_HPP
