#ifndef FLUX_FUTURES_HPP
#define FLUX_FUTURES_HPP

#include "utils.hpp"
#include<vtl/vtl.hpp>


namespace flux{

template<class...T>
struct concept{
    virtual void feed(concept*)=0;
    virtual void pipe(T&&...)=0;
    virtual bool ready()=0;
    virtual void reset()=0;
    virtual ~concept(){}
};

template<>
struct concept<void> : concept<>
{};






template<class...T>
struct store : concept<T...>{
    bool isReady;
    std::tuple<T...> data;

    template<class...U>
    store(U&&...X):isReady(sizeof...(U)),data(std::move(X)...){}


    virtual void feed(concept<T...> *c){
        vtl::tupleCall( std::move(data) , [c](auto&&...X){ return c->pipe(std::move(X)...);} );
    }

    virtual void pipe(T&&...X){
        isReady=1;
        data = std::make_tuple(std::move(X)...);
    }

    virtual void reset(){ isReady=0; }
    virtual bool ready(){
        return isReady;
    }
    virtual ~store(){}
};

template<>
struct store<void> : store<>{
    using store<>::store;
};




template<class F,class P, class...T>
struct model : concept<T...>{
    bool isReady;
    F next;
    P fut;
    model(F f)
        :isReady(0)
        ,next(f)
    {}

    virtual void feed(concept<T...>*){}
    P& getFuture(){ return fut; }

    template<class...U>
    auto _pipe(U&&...X) ->
    typename std::enable_if<
        std::is_same<void,decltype(next(std::move(X)...))>::value,
        void
    >::type{
        next(std::move(X)...);
        fut.pipe();
    }

    template<class...U>
    auto _pipe(U&&...X) ->
    typename std::enable_if<!std::is_same<void,decltype(next(std::move(X)...))>::value,void>::type{
        fut.pipe( next(std::move(X)...) );
    }

    virtual void pipe(T&&...X){
        isReady=1;
        _pipe(std::move(X)...);
    }

    virtual void reset(){ isReady=0; }
    virtual bool ready(){
        return isReady;
    }
    virtual ~model(){}

};

template<class F,class P>
struct model<F,P,void> : model<F,P>
{
    using model<F,P>::model;
};





template<class...T>
struct future{


private:

    std::shared_ptr< concept<T...> > pimpl;
    template<class F,class...U>
    struct nextFuture{
      using type = future<decltype( callIF( Obj<F>(), Obj<U>()... ) )>;
    };


public:


    template<class...U>
    future(U&&...X)
        :pimpl(new store<T...>(std::move(X)...))
    {}


    future(future<T...> const& fut) : pimpl(fut.pimpl)
    {}

    template<class...U>
    auto pipe(U&&...X) -> decltype( this->pimpl->pipe(std::move(X)...) ){
        pimpl->pipe(std::move(X)...);
    }


    virtual void reset(){ pimpl->reset(); }
    bool ready(){ return pimpl->ready(); }

    template<class F>
    auto then(F f)
    -> typename nextFuture<F,T...>::type&
    {
        using nt=typename nextFuture<F,T...>::type;
        auto nextPimpl=new model<F,nt,T...>(f);
        auto tmp = pimpl;
        this->pimpl=std::shared_ptr<concept<T...>>((concept<T...>*)nextPimpl);
        if(tmp->ready()){
            tmp->feed( pimpl.get() );
        }
        return nextPimpl->getFuture();
    }



};


template<>
struct future<void> : future<>{
    using future<>::future;
};



template<class...T>
struct promise{
    future<T...> f;
    future<T...>& getFuture(){ return f; }
    void pipe(T&&...X){ f.pipe(std::move(X)...); }
    void reset(){ f.reset(); }

};





}



#endif // FUTURES_HPP
