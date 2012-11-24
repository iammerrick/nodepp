#include <iostream>
#include <vector>
#include <cstdio>
#include <errno.h>
#include <poll.h>
#include <boost/shared_ptr.hpp>
#include "jinx/node.hpp"

using namespace std;
using namespace jinx::node;
using boost::shared_ptr;

namespace jinx {
namespace node {


  struct Buffer {

    Buffer(size_t sz) : m_buf(new vector<char>(sz)) {}
    size_t size() const { return m_buf->size(); }
    char& operator[]( size_t n ) const { return (*m_buf)[n]; }

  private:
    shared_ptr<vector<char>> m_buf;
  };


  namespace fs {

    //! Open a file descriptor.
    void open(string path, string mode, function<void(string,int)> cb) {
      FILE* fh = ::fopen(path.c_str(),mode.c_str());
      if ( !fh ) {
        cb(::strerror(errno),-1);
      } else {
        cb("",::fileno(fh));
      }
    }

    //! Read from the given file descriptor.
    void read( int fd, Buffer buffer, size_t offset, size_t length,
      size_t position, function<void(string,size_t,Buffer)> cb )
    {
      struct pollfd pfd = { fd, POLLIN, 0 };
      const shared_ptr<size_t> cur_offset( new size_t(offset) );
      const shared_ptr<size_t> cur_remaining( new size_t(length) );

      // create the reader lambda
      const function<void()> reader = [=]() {
        auto rdcnt = ::read(fd,&buffer[*cur_offset],*cur_remaining);
        *cur_offset += rdcnt;
        *cur_remaining -= rdcnt;
        if ( *cur_remaining == 0 ) {
          cb("",length,buffer);
        } else {
          attach(pfd,reader);
        }
      };

      // start up the reader
      attach(pfd,reader);
    }
  }


}}



int main() {

  start([]() {
    fs::open("../src/main_node++.cpp","r",[](string err,int fd) {
      Buffer buf(20);
      fs::read(fd,buf,0,20,0,[](string,size_t rdcnt,Buffer buf) {
        cout << "rdcnt: " << rdcnt << endl;
        cout << string(&buf[0],rdcnt) << endl;
      });
    });
    cout << "hello!" << endl;
  });


  return 0;
}
