#include <iostream>
#include <functional>
#include <vector>
#include <cstdio>
#include <errno.h>
#include <poll.h>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

using namespace std;
using boost::shared_ptr;
using boost::function;


namespace jinx {
namespace node {

  //! This is the main class which manages the control flow of a node-style
  //! execution environment.  There can only be one of these classes active
  //! at a time.  If another is created, it will result in an exception being
  //! thrown from the constructor.
  struct Session : boost::noncopyable {

    //! Constructor: verify that only one exists.
    Session() {
      if ( s_active )
        throw runtime_error("node session already active");
      s_active = this;
    }

    //! Destructor
    ~Session() {
      s_active = NULL;
    }

    //! Add a callback to the list of polls.
    void poll( const pollfd& pfd, const function<void()>& cb ) {
      m_polls.push_back( make_pair(pfd,cb) );
    }

    //! Start listening for poll events.
    void start() {
      while ( m_polls.size() ) {

        // create the poll structure
        const auto cnt = m_polls.size();
        pollfd pfds[cnt];
        for ( size_t i=0; i<cnt; i++ )
          pfds[i] = m_polls[i].first;

        // enter the poll request
        int nfds = ::poll(pfds,cnt,0);

        // handle all of the fds that had an event
        // we do this in reverse order so that we can safely delete the entries
        // from the m_polls object.
        for ( ssize_t i=cnt-1; i>=0; i-- ) {
          if ( pfds[i].revents ) {
            const auto cb = m_polls[i].second;
            m_polls.erase(m_polls.begin()+i);
            cb();
          }
        }
      }
    }


    //! Get the active node session.
    static Session& active() {
      if ( !s_active )
        throw runtime_error("no active node session");
      return *s_active;
    }

  private:

    static Session* s_active;
    vector<pair<pollfd,function<void()>>> m_polls;

  };

  Session* Session::s_active = NULL;


  //! Start the node session.
  void node_start( const function<void()>& main ) {
    Session sess;
    main();
    sess.start();
  }

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
          Session::active().poll( pfd, reader );
        }
      };

      // start up the reader
      Session::active().poll( pfd, reader );
    }
  }

}}

using namespace jinx::node;


int main() {

  node_start([]() {
    fs::open("../src/main_node++.cpp","r",[](string err,int fd) {
      Buffer buf(20);
      fs::read(fd,buf,0,20,0,[](string,size_t rdcnt,Buffer buf) {
        cout << "rdcnt: " << rdcnt << endl;
        cout << string(&buf[0],rdcnt) << endl;
      });
    });
  });


  return 0;
}
