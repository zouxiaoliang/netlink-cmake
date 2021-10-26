#include "NetlinkTransportPrivate.h"

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <iostream>
#include <errno.h>
#include <fcntl.h>

using namespace std;

NetlinkTransportPrivate::NetlinkTransportPrivate(int proto_id, int group_id)
    : m_status(false), m_socket_fd(0), m_proto_id(proto_id),
      m_group_id(group_id) {
  m_send_buffer.init(::getpid(), group_id);
  m_recv_buffer.init(::getpid(), group_id);
}

NetlinkTransportPrivate::~NetlinkTransportPrivate() { this->close(); }

int NetlinkTransportPrivate::fd() { return this->m_socket_fd; }

bool NetlinkTransportPrivate::connect() {
  if (m_status) {
    // cout<< "netlink init success, fd: " << m_socket_fd;
    return m_status;
  }

  m_socket_fd = ::socket(AF_NETLINK, SOCK_RAW, m_proto_id);
  if (m_socket_fd < 0) {
    return m_status;
  }

  struct timeval timeout = {3, 0};
  //设置发送超时
  setsockopt(m_socket_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
             sizeof(struct timeval));
  //设置接收超时
  setsockopt(m_socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
             sizeof(struct timeval));

  // 设置文件描述符不可copy-inheriting
  fcntl(m_socket_fd, F_SETFD, FD_CLOEXEC);

  // 链接的netlink的地址信息
  struct sockaddr_nl src_addr;
  memset(&src_addr, 0, sizeof(src_addr));
  src_addr.nl_family = AF_NETLINK;
  src_addr.nl_pid = getpid();
  src_addr.nl_groups = m_group_id;

  // 绑定sock到netlink
  int bind_rt =
      ::bind(m_socket_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
  if (bind_rt < 0) {
    char err_str[256];
    memset(err_str, 0x0, 256);
    strerror_r(errno, err_str, 256);
    this->close();
    return false;
  }
  m_status = true;
  return m_status;
}

void NetlinkTransportPrivate::close() {
  if (0 == m_socket_fd) {
    return;
  }
  ::shutdown(m_socket_fd, SHUT_RDWR);
  ::close(m_socket_fd);
  m_socket_fd = 0;
  m_status = false;
}

bool NetlinkTransportPrivate::ok() { return m_status; }

bool NetlinkTransportPrivate::send(void *msg_content,
                                   size_t msg_content_length) {
  if (NULL == msg_content || msg_content_length > 1024) {
    return false;
  }

  if (!m_send_buffer.pack_msg(msg_content, msg_content_length)) {
    return false;
  }

  int state = sendmsg(m_socket_fd, m_send_buffer.entity(), 0);
  if (-1 == state) {
    m_error_code = errno;
    if (ECONNREFUSED == m_error_code) {
      close();
      return false;
    }
  }

  return (state >= 0);
}

int NetlinkTransportPrivate::recv(std::string &recv_data) {
  if (!m_status) {
    return nl::ERROR;
  }

  // std::cout << "recv message from netlink ..." << std::endl;
  int state = recvmsg(m_socket_fd, m_recv_buffer.entity(), 0);
  if (-1 == state) {
    m_error_code = errno;
    // std::cout << "recv: " << strerror(m_error_code) << std::endl;
    if (EDEADLK == m_error_code) {
      return nl::ERROR;
    }
    // 如果驱动无数据
    return nl::EMPTY;
  }

  // std::cout << "recv conecnt..." << std::endl;
  char *iov_base = (char *)NLMSG_DATA(m_recv_buffer.msg()->iov_base);
  size_t iov_len = (size_t)NLMSG_DATA(m_recv_buffer.msg()->iov_len);

  recv_data.assign(iov_base, iov_len);

  return nl::OK;
}

int NetlinkTransportPrivate::error_code() { return m_error_code; }
