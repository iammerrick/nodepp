#include "core.hpp"
using namespace std;


namespace jinx {
namespace node {

  template <>
  struct Pimpl<Core>::Inner {
    vector<pair<pollfd,shared_ptr<Poller>>> polls;
    list<function<void()>> defers;

    bool hasMoreEvents() const {
      return !polls.empty()
          || !defers.empty();
    }

    //! Call the next deferred function.
    void callNextDeferred() {
      if ( !defers.empty() ) {
        auto func = defers.front();
        defers.pop_front();
        func();
      }
    }

    //! Poll for new events.    
    void pollForEvents() {
      if ( !polls.empty() ) {

        // create the poll structure
        const auto cnt = polls.size();
        pollfd pfds[cnt];
        for ( size_t i=0; i<cnt; i++ )
          pfds[i] = polls[i].first;

        // enter the poll request
        ::poll(pfds,cnt,0);

        // handle all of the fds that had an event
        // we do this in reverse order so that we can safely delete the entries
        // from the polls object.
        list<size_t> erase;
        for ( ssize_t i=cnt-1; i>=0; i-- ) {
          if ( pfds[i].revents ) {
            auto poller = polls[i].second;
            if ( poller->collect() )
              erase.push_back(i);
          }
        }

        // erase the deleted polls
        for ( auto iter = erase.rbegin(); iter != erase.rend(); ++iter )
          polls.erase(polls.begin()+*iter);
      }
    }
  };

  Core::Core() {}

  void Core::start( function<void()> entry ) {
    entry();
    next();
  }

  void Core::next() {
    auto& polls = self.polls;
    auto& defers = self.defers;

    while ( self.hasMoreEvents() ) {

      // check for any deferred functions
      // TODO this could cause starvation if each time a defer function is
      //   called, it defers again.  We should add something that limits the
      //   number of times we defer before dropping into other events.
      while ( !defers.empty() ) {
        self.callNextDeferred();
      }

      // perform a poll
      if ( !polls.empty() )
        self.pollForEvents();
    }
  }

  void Core::defer( function<void()> f ) {
    self.defers.push_back(f);
  }

  void Core::poll( const pollfd& pfd, const shared_ptr<Poller>& cb ) {
    self.polls.push_back( make_pair(pfd,cb) );
  }

}}
