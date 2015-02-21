#ifndef FLUX_GUARD_HPP
#define FLUX_GUARD_HPP

#include<type_traits>
#include<mutex>
#include "utils.hpp"

namespace flux{

template<class mutex=std::mutex>
struct CodeGuard{

    mutex m;

    template<class F>
    bool tryEnter(F f){
        if(m.try_lock()){
            f();
            m.unlock();
            return true;
        }
        return false;
    }

    template<class F>
    void enter(F f){
        while(!tryEnter(f)){
         std::this_thread::yield();
        }
    }

    template<class F>
    void operator()(F f){
        this->enter(f);
    }
};


template<class T, class mutex=std::mutex>
struct DataGuard{
    mutex m;
    T Data;

    template<class...Args>
    DataGuard(Args&&...args): Data(std::forward<Args>(args)...)
    {}

    template<class F>
    bool tryEnter(F f) { //try to aquire lock and enter the critical section
        if(m.try_lock()){
            callIF(f,Data);
            m.unlock();
            return true;
        }
        return false;
    }

    template<class F>
    void enter(F f){ // try to enter till success
        while(!tryEnter(f)){
          std::this_thread::yield();
        }
    }

    template<class F>
    void operator()(F f){
        this->enter(f);
    }
};

template<class T=void,class m=std::mutex>
using Guard = typename std::conditional<std::is_same<T,void>::value,CodeGuard<m>,DataGuard<T,m>>::type;



}


#endif // GUARD_HPP
