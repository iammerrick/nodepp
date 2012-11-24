#include "filesystem.hpp"
#include <errno.h>


namespace nodepp {

  template <>
  struct Pimpl<Fd>::Inner {
    int fd;

    Inner( int fd ) : fd(fd) {}
    ~Inner() {
      if ( fd != -1 )
        ::close(fd);
    }
  };

  Fd::Fd( int fd )
  : Pimpl<Fd>(fd)
  {}

  bool Fd::isOpen() const { return self.fd != -1; }

  int Fd::fd() const { return self.fd; }

  void Fd::close() {
    if ( self.fd != -1 ) {
      ::close(self.fd);
      self.fd = -1;
    }
  }


  template <>
  struct Pimpl<FileSystem>::Inner {
    Inner( Core c ) : core(c) {}
    Core core;
  };

  FileSystem::FileSystem( Core c )
  : Pimpl<FileSystem>(c) {}

  void FileSystem::open( string path, string mode,
    function<void(string,Fd)> cb )
  {
    self.core.defer([=]() {
      FILE* fh = ::fopen(path.c_str(),mode.c_str());
      if ( !fh ) {
        cb(::strerror(errno),Fd(-1));
      } else {
        Fd fd(::dup(::fileno(fh)));
        ::fclose(fh);
        cb("",fd);
      }
    });
  }

  namespace {
    struct Reader : Poller {

      Reader( Fd fd, Buffer buffer, size_t offset, size_t size,
        function<void(string,size_t,Buffer)> callback
      )
      : m_callback(callback)
      , m_fd(fd)
      , m_size(size)
      , m_buffer(buffer)
      , m_offset(offset)
      {}

      bool collect() {

        // read from the file descriptor
        auto outptr = &m_buffer[m_offset];
        auto rdcnt = ::read(m_fd.self.fd,outptr,m_size-m_offset);

        // handle errors
        if ( rdcnt == -1 ) {
          m_callback(strerror(errno),m_offset,m_buffer);
          return true;
        }

        // check for completion
        m_offset += rdcnt;
        if ( m_offset >= m_size ) {
          m_callback("",m_size,m_buffer);
          return true;
        }

        return false;
      }
    
    private:
      const function<void(string,size_t,Buffer)> m_callback;
      const Fd m_fd;
      const size_t m_size;
      Buffer m_buffer;
      size_t m_offset;

    };
  }

  void FileSystem::read( Fd fd, Buffer buffer, size_t offset, size_t length,
    size_t position, function<void(string,size_t,Buffer)> cb )
  {
    const pollfd pfd = { fd.self.fd, POLLIN, 0 };
    shared_ptr<Reader> reader( new Reader(fd,buffer,offset,length,cb) );
    self.core.poll(pfd,reader);
  }

}

