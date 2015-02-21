#ifndef FLUX_THREADPOOL_HPP
#define FLUX_THREADPOOL_HPP

#include <vector>
#include <tuple>
#include <type_traits>

#include<thread>
#include<deque>
#include<mutex>
#include<functional>
#include<memory>
#include<algorithm>
#include<chrono>
#include<vector>

#include "futures.hpp"
#include "Guard.hpp"
#include<vtl/vtl.hpp>


namespace flux{

struct threadPool{

    threadPool(uint N=std::thread::hardware_concurrency())
        :threads(N)
        ,run(1)
        ,guard(tasks){
        init();
    }

    void init(){
        for(uint i=0;i<threads;++i){
            workers.emplace_back(makeThread());
        }
    }

    template<class F>
    void push(F f){
        guard( [this,f](){
            tasks.push_back(f);
        });
    }

    template<class F>
    void operator()(F f){
        push(f);
    }

    void stop(){ run=0; }
    void join(){
        for(auto& w:workers){
            w.join();
        }
    }

    bool process(){
        std::function<void()>  myTask;
        guard( [this,&myTask](){
            if(!tasks.empty()){
                myTask = tasks.front();
                tasks.pop_front();
            }
        });
        if(myTask){
            myTask();
        }
        return !tasks.empty();
    }

    template<class F>
    bool process(F f){
        push(f);
        return process();
    }


    std::function<void()> makeThread(){
        return [this](){
            while(run){
                if(!this->process()){
                    std::this_thread::sleep_for(1_ms);
                    //this_thread::yield();
                }
            }
        };
    }

    ~threadPool(){
        while(run && this->process() );
        join();
    }


private:
    std::vector<std::thread> workers;
    std::deque< std::function<void(void)> > tasks;
    uint threads;
    bool run;
public:
    Guard<decltype(tasks)&> guard;

};



template<class Async, class B, class E>
auto& parallel(Async& async, B b, E e){

    auto P =new promise<bool>();

    auto results =new std::vector<bool>(e-b);
    auto doWhenReady=[results,P](){
        if(accumulate(results->begin(),results->end(),0u)==results->size()){
            P->pipe(1);
            delete P;
            delete results;
        }
    };

    uint i=0;
    std::for_each(b,e,[&,doWhenReady](auto task){
        async( [i,task,results,doWhenReady](){
            task([&,j=i](){ (*results)[j]=1; doWhenReady(); });
        });
        ++i;
    });

    return P->getFuture();
}


template<class...args>
using cb=std::function<void(args...)>;


template<class Async>
auto& parallel(Async& async, std::vector<std::function<void( cb<> )>> vec ){
    return parallel(async,vec.begin(),vec.end());
}

template<class Async,class...Args>
auto& parallel(Async& async, Args...args ){
    return parallel(async,{args...});
}


}

#endif
