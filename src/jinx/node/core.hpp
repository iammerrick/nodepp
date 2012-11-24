#ifndef x22e26dfb5da7e7fc109b7bbc6bffcdafac6e07c9
#define x22e26dfb5da7e7fc109b7bbc6bffcdafac6e07c9

#include "pimpl.hpp"
#include <sys/poll.h>


namespace jinx {
namespace node {

  //! This class is used to poll a file descriptor, and collect the results.
  //! Each time the polling returns, the collect method is called.  Polling
  //! continues until the collect method retuns true.
  struct Poller {
    virtual ~Poller() {}
    virtual bool collect() = 0;
  };


  //! This is the class which contains the core event-loop.
  struct Core : Pimpl<Core> {

    //! Create a brand new event-loop
    Core();

    //! Start a session by calling the given function. This method does not
    //! return until all of the event handlers have completed.  This is the
    //! usual entry point.
    void start( function<void()> );

    //! This is the method that runs the main event loop.  It will block until
    //! no more events are available for execution.
    void next();

    //! This will cause the given function to be executed on the next loop
    //! iteration.
    void defer( function<void()> );

    //! Add a polling event.
    void poll( const pollfd&, const shared_ptr<Poller>& );
  };

}}

#endif
