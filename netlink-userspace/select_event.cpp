#include "select_event.h"

namespace netlink {
namespace userspace {

void _event(int nfds, fd_set &rfds, fd_set &wfds, timeval tv,
            std::function<void(fd_set &)> _on_read,
            std::function<void(fd_set &)> _on_write,
            std::function<void()> _on_error,
            std::function<void()> _on_timeout) {
  int retval = select(nfds, &rfds, &wfds, NULL, &tv);
  /* Don't rely on the value of tv now! */
  if (retval == -1) {
    // cout << "socket select() error" << endl;
    if (_on_error)
      _on_error();
  } else if (retval) {
    // cout << "Data is available now (retval = "<< retval << ")" << endl;
    if (_on_write)
      _on_write(wfds);
    if (_on_read)
      _on_read(rfds);
  } else {
    // cout << "No data within "<< tv.tv_sec << " seconds" << endl;
    if (_on_timeout)
      _on_timeout();
  }
}

SelectEvent::SelectEvent() : m_fd(0), m_tv({5, 0}), m_want_to_write(0) {}

SelectEvent::SelectEvent(int fd, struct timeval& tv) : m_fd(fd), m_tv(tv), m_want_to_write(0) {}

SelectEvent::~SelectEvent() {}

void SelectEvent::init(int fd, struct timeval& tv) {
    m_fd            = fd;
    m_tv.tv_sec     = tv.tv_sec;
    m_tv.tv_usec    = tv.tv_usec;
    m_want_to_write = 0;
}

void SelectEvent::async_write(int fd) {
    ++m_want_to_write;
}

void SelectEvent::run_one() {
    namespace argv = std::placeholders;

    // cout << "set " << m_fd << " to rfds" << endl;
    // cout << "tv: (" << m_tv.tv_sec << ", " << m_tv.tv_usec << ")" << endl;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(m_fd, &rfds);

    fd_set wfds;
    FD_ZERO(&wfds);
    if (m_want_to_write > 0) {
        FD_SET(m_fd, &wfds);
    }

    _event(
        m_fd + 1, rfds, wfds, m_tv, std::bind(&SelectEvent::on_read, this, argv::_1),
        std::bind(&SelectEvent::on_write, this, argv::_1), std::bind(&SelectEvent::on_error, this),
        std::bind(&SelectEvent::on_timeout, this));
}

void SelectEvent::set_do_event(int event_id, std::function<void()> event) {
  m_do_events[event_id] = event;
}

void SelectEvent::on_read(fd_set& fds) {
    if (FD_ISSET(m_fd, &fds)) {
        if (m_do_events[DO_READ])
            m_do_events[DO_READ]();
    }
}

void SelectEvent::on_write(fd_set& fds) {
    if (FD_ISSET(m_fd, &fds)) {
        // 处理发送时间
        if (m_do_events[DO_WRITE])
            m_do_events[DO_WRITE]();
        --m_want_to_write;
    }
}

void SelectEvent::on_error() {
    if (m_do_events[DO_ERROR])
        m_do_events[DO_ERROR]();
}

void SelectEvent::on_timeout() {
    if (m_do_events[DO_TIMEOUT])
        m_do_events[DO_TIMEOUT]();
}

} // namespace userspace
} // namespace netlink
