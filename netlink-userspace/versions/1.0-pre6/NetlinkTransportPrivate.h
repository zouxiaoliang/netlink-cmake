#ifndef NETLINKTRANSPORTPRIVATE_H
#define NETLINKTRANSPORTPRIVATE_H

#include "nldef.h"
#include <string>

struct nl_handle;
struct nl_cb;

/**
 * @brief The NLClient class
 */
class NetlinkTransportPrivate {
public:
    /**
     * @brief NLClient 构造方法
     * @param proto_id 协议id
     * @param group_id 消息组id
     * @param debug 是否开启libnl库的调试状态
     */
  NetlinkTransportPrivate(int proto_id, int group_id, int debug = 0);
  ~NetlinkTransportPrivate();

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
   * @return nl::RECV_CODE
   */
  int recv(std::string &recv_data);

  /**
   * @brief run
   */
  void run();

  /**
   * @brief fd 获取socket文件描述符
   * @return 文件描述符
   */
  int fd();

  /**
   * @brief do_recv_def 执行默认的数据接收操作
   * @details 当有消息处理时，会调用on_recv接口进行数据接收
   */
  void do_recv_def();

  /**
   * @brief error_code
   * @return 错误码或者错误信息
   */
  int error_code();

public:
    /**
     * @brief on_recv 当有消息从内核到用户时，调用do_recv_def接口，会内部自动调用on_recv接口
     * @param msg 消息体
     * @param msg_len 消息长度
     */
    void on_recv(const void* msg, size_t msg_len);

private:
    int  m_proto_id;
    int  m_group_id;
    bool m_connected;

    nl_handle* m_nl_handle;
    nl_cb*     m_nl_cb;
    int        m_nl_fd;
    int        m_error_code;
};

#endif // NETLINKTRANSPORTPRIVATE_H
