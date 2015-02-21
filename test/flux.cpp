#include<igloo/igloo_alt.h>
#include<vtl/vtl.hpp>
#include<flux/futures.hpp>
#include<flux/threadpool.hpp>
#include<type_traits>
#include<tuple>
#include<iostream>

using namespace igloo;
using namespace vtl;
using namespace flux;




Describe(flux_tests){
  It(should_fullfill_promises){
    promise<int> p;
    auto& f = p.getFuture();

    f.then([](auto x){
      Assert::That(x,Equals(1));
      return x+1;
    }).then([](auto x){
      Assert::That(x,Equals(2));
      return x+1;      
    });
    
   // p.pipe(1);

  };
};

int main(int argc, char const* argv[])
{
  TestRunner::RunAllTests(argc, argv);
}
