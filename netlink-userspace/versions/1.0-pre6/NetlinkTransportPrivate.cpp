#include "NetlinkTransportPrivate.h"
#include "proto_buffer.h"

#include <time.h>
/* According to POSIX.1-2001 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

#include <linux/types.h>

extern "C" {
#include <netlink/netlink.h>
#include <netlink/types.h>
#include <netlink/msg.h>
#include <netlink/object.h>
#include <netlink/netlink-kernel.h>
}

#include <iostream>

using namespace std;

int recv_message_cb(nl_msg *msg, void *arg)
{
    struct nlmsghdr *hdr = nlmsg_hdr(msg);
    NetlinkTransportPrivate *self = (NetlinkTransportPrivate *)arg;
    self->on_recv(nlmsg_data(hdr), nlmsg_len(hdr));
    return NL_PROCEED;
}

NetlinkTransportPrivate::NetlinkTransportPrivate(int proto_id, int group_id,
                                                 int debug)
    : m_proto_id(proto_id), m_group_id(group_id), m_connected(false),
      m_nl_handle(NULL), m_nl_cb(NULL) {
  nl_debug = debug;
  cout << "this >> " << (int64_t)this << endl;
  m_nl_handle = nl_handle_alloc();
  if (NULL != m_nl_handle) {
    nl_disable_sequence_check(m_nl_handle);
    nl_handle_set_pid(m_nl_handle, ::getpid());
    nl_join_group(m_nl_handle, group_id);
    m_nl_cb = nl_handle_get_cb(m_nl_handle);
    if (NULL == m_nl_cb) {
      cout << "nl_handle_get_cb(m_nl_handle) -> null" << std::endl;
      return;
    }

    nl_cb_set(m_nl_cb, NL_CB_MSG_IN, NL_CB_CUSTOM, recv_message_cb, this);
  }
}

NetlinkTransportPrivate::~NetlinkTransportPrivate() {
  if (NULL != m_nl_handle) {
    nl_handle_destroy(m_nl_handle);
    m_nl_handle = NULL;
  }
}

bool NetlinkTransportPrivate::connect() {
  m_connected = 0 == nl_connect(m_nl_handle, m_proto_id);
  m_nl_fd = nl_handle_get_fd(m_nl_handle);
  struct timeval timeout = {1, 0};
  //设置发送超时
  setsockopt(m_nl_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
             sizeof(struct timeval));
  //设置接收超时
  setsockopt(m_nl_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
             sizeof(struct timeval));

  // 设置文件描述符不可copy-inheriting
  fcntl(m_socket_fd, F_SETFD, FD_CLOEXEC);

  return m_connected;
}

void NetlinkTransportPrivate::close() {
  nl_close(m_nl_handle);
  m_nl_handle = 0;
}

bool NetlinkTransportPrivate::ok() {
  if (NULL == m_nl_handle) {
    return false;
  }

  return m_connected;
}

bool NetlinkTransportPrivate::send(void *msg, size_t msg_len) {
  return nl_sendto(m_nl_handle, msg, msg_len) == msg_len;
}

int NetlinkTransportPrivate::recv(std::string &recv_data) {
  unsigned char *buf = NULL;
  struct nlmsghdr *hdr = NULL;
  struct sockaddr_nl nla;
  nla.nl_family = AF_NETLINK;
  nla.nl_groups = m_group_id;
  nla.nl_pid = ::getpid();

  struct nl_msg *msg = NULL;
  struct ucred *creds = NULL;

  bool recv_result = true;

  int n = nl_recv(m_nl_handle, &nla, &buf, &creds);
  // cout << "recv package length >> " << n << endl;
  if (n <= 0) {
    return nl::ERROR;
  }

  hdr = (nlmsghdr *)buf;
  if (nlmsg_ok(hdr, n)) {
    nlmsg_free(msg);
    msg = nlmsg_convert(hdr);
    if (!msg) {
      recv_result = nl::EMPTY;
      goto out;
    }

    recv_data.assign((char *)nlmsg_data(hdr), nlmsg_len(hdr));
  }

out:
    nlmsg_free(msg);
    free(buf);
    free(creds);
    return recv_result;
}

void NetlinkTransportPrivate::run() {
  if (NULL == m_nl_handle) {
    return;
  }

#if 1
    int loop_count = 0;

#ifdef USING_BUFFER
    string helloworld = "hello world hello world hello world hello world";
    char msg[4096];
    bzero(msg, 4096);
    sprintf(msg, "%s_%d", helloworld.c_str(), loop_count);
    ProtoBuffer<4096> buf(msg, strlen(msg) + 1);
#endif

    while (1)
    {
    #ifdef USING_BUFFER
        cout << "content length: " << strlen(msg) << ", buffer size: " << msg << endl;
        cout << "send length: " << this->send(buf.c_data(), buf.c_length()) << endl;
    #else
        this->send((void *)helloworld.c_str(), helloworld.length() + 1);
    #endif
        std::string recv_buffer;
        this->recv(recv_buffer);
        cout << "recv content: " << recv_buffer << endl;
        usleep(10);
        cout << "=>" << ++loop_count << "<=" << endl;
    }
#else
    while (1)
    {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(m_nl_fd, &rfds);
        /* wait for an incoming message on the netlink socket */
        int retval = select(m_nl_fd+1, &rfds, NULL, NULL, NULL);
        if (retval)
        {
            /* FD_ISSET(fd, &rfds) will be true */
            nl_recvmsgs_def(m_nl_handle);
        }
    }
#endif
}

int NetlinkTransportPrivate::fd() { return m_nl_fd; }

void NetlinkTransportPrivate::do_recv_def() { nl_recvmsgs_def(m_nl_handle); }

void NetlinkTransportPrivate::on_recv(const void *msg, size_t msg_len) {
  cout << "on_recv msg: " << (char *)msg << endl;
  cout << "on_recv msg: " << msg_len << endl;
}

int NetlinkTransportPrivate::error_code() { return m_error_code; }
