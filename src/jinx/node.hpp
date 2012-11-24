#include <boost/function.hpp>
#include <sys/poll.h>


namespace jinx {
namespace node {

// #### TYPEDEFS ####
using boost::function;


//! Start the node session by calling the given callback.  This function will
//! return once all of the execution threads ahve completed.
void start( function<void()> );

//! Attach a callback to the given poll query.
void attach( const pollfd&, function<void()> );

}}
