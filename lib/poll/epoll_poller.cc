#include "../../include/poll/epoll_poller.h"
#include "../../include/exception.h"
#include "../../include/logging.h"

namespace collie {
namespace poll {

EPollPoller::EPollPoller(const unsigned max_event)
    : Poller(max_event), revents_(new Event[kMaxEvent]) {
  Log(TRACE) << "EPoller constructing";
}

EPollPoller::~EPollPoller() { Log(TRACE) << "EPoller destructing"; }

void EPollPoller::Open() {
  fd_ = epoll_create1(0);
  Log(TRACE) << "EPoller create new epoll " << fd_;
  REQUIRE_SYS(fd_ != -1);

  is_init_ = true;
}

void EPollPoller::Insert(const int fd, const unsigned events) {
  REQUIRE_(is_init_, "Epoller is not inited");
  Log(TRACE) << "EPoller " << fd_ << " insert " << fd << " with events "
             << events;
  Event event;
  event.data.fd = fd;
  event.events = events;
  const int ret = epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &event);
  REQUIRE_SYS(ret != -1);
}

void EPollPoller::Modify(const int fd, const unsigned events) {
  REQUIRE_(is_init_, "Epoller is not inited");
  Log(TRACE) << "EPoller " << fd_ << " modify " << fd << " with events "
             << events;
  Event event;
  event.data.fd = fd;
  event.events = events;
  const int ret = epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &event);
  REQUIRE_SYS(ret != -1);
}

void EPollPoller::Remove(const int fd) {
  REQUIRE_(is_init_, "Epoller is not inited");
  Log(TRACE) << "EPoller " << fd_ << " remove " << fd;
  // Since Linux 2.6.9, event can be specified as NULL when using
  // EPOLL_CTL_DEL.
  const int ret = epoll_ctl(fd_, EPOLL_CTL_DEL, fd, NULL);
  REQUIRE_SYS(ret != -1);
}

void EPollPoller::Poll(PollCallback cb, const int timeout) {
  REQUIRE_(is_init_, "Epoller is not inited");
  Log(TRACE) << "EPoller " << fd_ << " is polling ";
  int event_num = epoll_wait(fd_, revents_.get(), kMaxEvent, timeout);
  REQUIRE_SYS(event_num != -1);

  Log(TRACE) << "Epoller " << fd_ << " get " << event_num << " events";

  for (int idx = 0; idx < event_num; ++idx) {
    const Event& curEvent = revents_.get()[idx];
    REQUIRE_(cb, "PollCallback is not callable");
    cb(curEvent.data.fd, curEvent.events);  // XXX
  }
}
}
}
