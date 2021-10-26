#ifndef NETLINKTRANSPORT_H
#define NETLINKTRANSPORT_H

#include <string>

class NetlinkTransportPrivate;

class NetlinkTransport {
public:
  NetlinkTransport(int proto_id, int group_id);
  virtual ~NetlinkTransport();
  /**
   * @brief connect 发起与内核netlink的连接
   * @return
   */
  bool connect();

  /**
   * @brief close 关闭与内核netlink的连接
   */
  void close();

  /**
   * @brief ok 检查netlink状态是否ok
   * @return true/false
   */
  bool ok();

  /**
   * @brief send 发送数据接口
   * @param msg 消息体
   * @param msg_len 消息长度
   * @return
   */
  bool send(void *msg, size_t msg_len);

  /**
   * @brief recv 接收数据
   * @param recv_data 收到的数据内容
   * @return 当接收失败，或数据为空时返回false; 收到数据时返回true
   */
  int recv(std::string &recv_data);

  /**
   * @brief error_str
   * @brief error_code
   * @return 错误码或者错误信息
   */
  int error_code();
  std::string error_str();

  /**
   * @brief fd 获取文件描述符
   * @return
   */
  int fd();

private:
  NetlinkTransportPrivate *m_handle;
};

#endif // NETLINKTRANSPORT_H
