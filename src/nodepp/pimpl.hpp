#ifndef x2dc915b8959a865be19766fa618b7578a6d899d9
#define x2dc915b8959a865be19766fa618b7578a6d899d9

#include "stdinc.hpp"
#include <utility>


namespace nodepp {

  //! This class sets a class up as a pimpl. In order to define the internal
  //! contents, you must define Pimpl<T>::Inner with template specialization.
  template <typename Self>
  struct Pimpl {
  protected:

    struct Inner;

    Pimpl()
    : m_inner( new Inner )
    , self(*m_inner)
    {}

    template <typename... Args>
    Pimpl( Args&&... args )
    : m_inner( new Inner(args...) )
    , self(*m_inner)
    {}

  private:
    const shared_ptr<Inner> m_inner;

  public:
    Inner& self;
  };

}

#endif
