#include "NetlinkTransport.h"
#include "NetlinkTransportPrivate.h"

#include <string.h>

#define DEF_ASSERT_R(condition, result)                                        \
  if (!(condition)) {                                                          \
    return result;                                                             \
  }
#define DEF_ASSERT(condition)                                                  \
  if (!(condition)) {                                                          \
    return;                                                                    \
  }

NetlinkTransport::NetlinkTransport(int proto_id, int group_id) {
  m_handle = new (std::nothrow) NetlinkTransportPrivate(proto_id, group_id);
}

NetlinkTransport::~NetlinkTransport() {
  if (NULL != m_handle) {
    delete m_handle;
  }
  m_handle = NULL;
}

bool NetlinkTransport::connect() {
  DEF_ASSERT_R(NULL != m_handle, false);
  return m_handle->connect();
}

void NetlinkTransport::close() {
  DEF_ASSERT(NULL != m_handle);
  m_handle->close();
}

bool NetlinkTransport::ok() {
  DEF_ASSERT_R(NULL != m_handle, false);
  return m_handle->ok();
}

bool NetlinkTransport::send(void *msg, size_t msg_len) {
  DEF_ASSERT_R(NULL != m_handle, -1);
  return m_handle->send(msg, msg_len);
}

int NetlinkTransport::recv(std::string &recv_data) {
  DEF_ASSERT_R(NULL != m_handle, -1);
  return m_handle->recv(recv_data);
}

int NetlinkTransport::error_code() { return m_handle->error_code(); }

std::string NetlinkTransport::error_str() {
  char error_string_temp[256];
  memset(error_string_temp, 0x0, sizeof(error_string_temp));

  strerror_r(error_code(), error_string_temp, sizeof(error_string_temp));

  return error_string_temp;
}

int NetlinkTransport::fd() { return m_handle->fd(); }
