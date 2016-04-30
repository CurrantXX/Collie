#include <sys/socket.h>
#include "../inc/async_tcp_stream.h"
#include "../inc/tcp_socket.h"
#include "../inc/logger.h"
#include "../inc/poll/event_pool.h"

namespace collie {

AsyncTcpStream::AsyncTcpStream(std::shared_ptr<TcpSocket> peer_fd) noexcept
    : peer_fd_(peer_fd),
      read_size_(3000) {
  write_handler_ = [](auto) { LOG(WARN) << "No write handler"; };
  read_handler_ = [](auto) { LOG(WARN) << "No write handler"; };
  error_handler_ = [](auto) { LOG(WARN) << "No error handler"; };
  close_handler_ = [](auto) { LOG(WARN) << "No close handler"; };
}

AsyncTcpStream::~AsyncTcpStream() noexcept {}

int AsyncTcpStream::GetDescriptor() const noexcept {
  return peer_fd_->GetDescriptor();
}

void AsyncTcpStream::Write(const std::string& buf,
                           const AsyncCallback& callback) throw(TcpException) {
  if (status_ == ABORT) {
    LOG(WARN) << "TCP stream is closed";
    return;
  }
  write_handler_ = [buf, callback](std::shared_ptr<AsyncTcpStream> stream) {
    if (stream->status() == ABORT) {
      // FIXME
      LOG(WARN) << "TCP stream is closed";
      return;
    }
    // TODO sendfile
    char buffer[buf.length() + 1];
    ::strcpy(buffer, buf.c_str());
    const int len = strlen(buffer);
    // Non blocking I/O
    int sent_size = ::send(stream->GetDescriptor(), buffer, len, MSG_DONTWAIT);
    if (sent_size == -1) {
      throw TcpException("TCP send");
    } else if (sent_size < len) {
      // Some bytes are not sent, resend the left
      stream->Write(buf.substr(sent_size), callback);
      return;
    }
    stream->event_.SetWrite(false);
    stream->event_pool_->Update(stream);
    if (!callback) {
      stream->Abort();
    } else {
      callback(stream);
    }
  };

  event_.SetWrite(true);
  if (!event_pool_) {
    LOG(ERROR) << "Event pool is not available";
    return;
  }

  event_pool_->Update(shared_from_this());
}

void AsyncTcpStream::Read(const AsyncCallback& callback) throw(TcpException) {
  if (status_ == ABORT) {
    LOG(WARN) << "TCP stream is closed";
    return;
  }

  read_handler_ = [callback](std::shared_ptr<AsyncTcpStream> stream) {
    char buffer[stream->read_size_];
    const int recv_size = ::recv(stream->GetDescriptor(), buffer,
                                 stream->read_size_, MSG_DONTWAIT);
    // int recv_size = ::read(stream->GetDescriptor(), buffer,
    // stream->read_size_);
    LOG(DEBUG) << buffer << stream->read_size_;

    if (recv_size == -1) {
      throw TcpException("TCP recv");
    } else if (recv_size == 0) {
      // close
      stream->Abort();
      return;
    }
    LOG(DEBUG) << "recv: " << buffer;
    stream->event_.SetRead(false);
    stream->event_pool_->Update(stream);
    stream->read_buffer_ = buffer;

    if (!callback) {
      stream->Abort();
    } else {
      callback(stream);
    }
  };
  event_.SetRead(true);
  if (!event_pool_) {
    LOG(ERROR) << "Event pool is not available";
    return;
  }
  event_pool_->Update(shared_from_this());
}

void AsyncTcpStream::ReadUntil(const char,
                               const AsyncCallback&) throw(TcpException) {}

void AsyncTcpStream::ReadLine(const AsyncCallback&) throw(TcpException) {}

void AsyncTcpStream::Abort() noexcept {
  if (!event_pool_) {
    LOG(ERROR) << "Event pool is not available";
    return;
  }
  event_pool_->Delete(shared_from_this());
  event_pool_ = nullptr;
}
}