#ifndef x2dc915b8959a865be19766fa618b7578a6d899d9
#define x2dc915b8959a865be19766fa618b7578a6d899d9

#include "stdinc.hpp"
#include <utility>


namespace jinx {
namespace node {

  //! This class sets a class up as a pimpl. In order to define the internal
  //! contents, you must define Pimpl<T>::Inner with template specialization.
  template <typename Self>
  struct Pimpl {
  protected:
    Pimpl() { m_inner.reset(new Inner); }

    template <typename... Args>
    Pimpl( Args&&... args ) {
      m_inner.reset( new Inner( args... ) );
    }

    struct Inner;
    Inner& self() { return *m_inner; }
    const Inner& self() const { return *m_inner; }

  private:
    shared_ptr<Inner> m_inner;
  };

}}

#endif
