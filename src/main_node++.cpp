#include <iostream>
#include <vector>
#include <cstdio>
#include <errno.h>
#include <poll.h>
#include <boost/shared_ptr.hpp>
#include "jinx/node/filesystem.hpp"

using namespace jinx::node;


int main() {
  Core core;
  FileSystem fs(core);

  core.start([=]() mutable {
    fs.open("../src/main_node++.cpp","r",[=](string err,Fd fd) mutable {
      cout << "Hi!" << endl;
      Buffer buf(20);
      fs.read(fd,buf,0,20,0,[=](string,size_t rdcnt,Buffer buf) {
        cout << "rdcnt: " << rdcnt << endl;
        cout << string(&buf[0],rdcnt) << endl;
      });
      cout << "R" << endl;
    });
    cout << "X" << endl;
  });
  cout << "U" << endl;

  return 0;
}
