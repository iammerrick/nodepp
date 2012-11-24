#include "node.hpp"
#include <vector>
#include <boost/noncopyable.hpp>
using namespace std;


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
  void attach( const pollfd& pfd, const function<void()>& cb ) {
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
      ::poll(pfds,cnt,0);

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


void start( function<void()> main ) {
  Session sess;
  main();
  sess.start();
}

void attach( const pollfd& pfd, function<void()> cb ) {
  Session::active().attach(pfd,cb);
}


}}
