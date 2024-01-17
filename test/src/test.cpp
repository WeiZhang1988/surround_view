#include <iostream>
#include "base_thread.hpp"

class MyThread : public SVS::BaseThread {
  protected:
  void run() {
    std::cout<<"running thread"<<std::endl;
  }
};

int main(){
  std::cout<<"running main"<<std::endl;
  MyThread t;
  t.start();
  std::cout<<"t is started"<<std::endl;
  t.stop();
  std::cout<<"t is stopped"<<std::endl;
  t.start();
  std::cout<<"t is started again"<<std::endl;
  while(true){}
}