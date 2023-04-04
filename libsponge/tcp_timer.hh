#ifndef SPONGE_LIBSPONGE_TCP_TIMER_HH
#define SPONGE_LIBSPONGE_TCP_TIMER_HH

#include <cstdint>

class TCPTimer{
private:
  unsigned int rto; //  retransmission timeout 
public:
  TCPTimer(){

  }

  void restart(){}

};

#endif