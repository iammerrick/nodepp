#ifndef xd02b8719ba36f5119aac9d8a3fdd4c658c591eaf
#define xd02b8719ba36f5119aac9d8a3fdd4c658c591eaf

#include "pimpl.hpp"
#include "core.hpp"

namespace jinx {
namespace node {

  //! This is a boring old data buffer.
  struct Buffer {
    Buffer(size_t sz) : m_buf(new vector<char>(sz)) {}
    size_t size() const { return m_buf->size(); }
    char& operator[]( size_t n ) const { return (*m_buf)[n]; }

  private:
    shared_ptr<vector<char>> m_buf;
  };


  //! This class contains all of the functions that deal with filesystem access,
  //! including file IO.  The interface is analogous to the Node.js File System
  //! interface.
  struct FileSystem : Pimpl<FileSystem> {
    FileSystem( Core );
    void open( string path, string mode, function<void(string,int)> );
    void read( int fd, Buffer buffer, size_t offset, size_t length,
      size_t position, function<void(string,size_t,Buffer)> );
  };

}}

#endif
