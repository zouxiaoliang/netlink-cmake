#ifndef NETLINKTRANSPORTPRIVATE_H
#define NETLINKTRANSPORTPRIVATE_H

#include <string>

#include <linux/netlink.h>
#include <linux/socket.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nldef.h"

template<int BUFFER_LENGTH = 1024>
class Buffer
{
public:
    Buffer():
        m_msg_header(NULL){
        memset(m_message_buffer, 0, LENGTH);
        memset(&m_sock_addr, 0, sizeof(sockaddr_nl));
        memset(&m_msg_entity, 0, sizeof(m_msg_entity));
    }

    /**
     * @brief init
     * @param pid
     * @param group_id
     */
    void init(int pid, int group_id) {

        m_sock_addr.nl_family = AF_NETLINK;         /* */
        m_sock_addr.nl_pid = 0;                     /* For Linux Kernel */
        m_sock_addr.nl_groups = group_id;           /* unicast */

        /* Fill the netlink message header */
        m_msg_header=(struct nlmsghdr *)m_message_buffer;
        m_msg_header->nlmsg_len = NLMSG_SPACE(BUFFER_LENGTH); /* 设置消息总长度*/
        m_msg_header->nlmsg_pid = pid;              /* self pid */
        m_msg_header->nlmsg_seq = 0;
        m_msg_header->nlmsg_flags = 0;

        /* Fill in the netlink message payload */
        m_msg.iov_base = (void *)m_msg_header;
        m_msg.iov_len = m_msg_header->nlmsg_len;

        m_msg_entity.msg_name = (void *)&m_sock_addr;
        m_msg_entity.msg_namelen = sizeof(m_sock_addr);
        m_msg_entity.msg_iov = msg();
        m_msg_entity.msg_iovlen = 1;

        m_msg_body = NLMSG_DATA(m_msg_header);
    }

    /**
     * @brief msg
     * @return
     */
    struct iovec *msg() {
        return &m_msg;
    }

    /**
     * @brief pack_msg 将数据装在到缓冲区中
     * @param msg 消息
     * @param msg_len 消息长度
     * @return
     */
    bool pack_msg(const void* msg, size_t msg_len)
    {
        ::memset(msg_body(), 0x0, BUFFER_LENGTH);
        if (NULL == msg || msg_len > BUFFER_LENGTH)
        {
            return false;
        }

        set_msg_len(msg_len);
        ::memcpy(msg_body(), msg, msg_len);

        return true;
    }

    void set_msg_len(size_t message_length)
    {
        m_msg_header->nlmsg_len = NLMSG_SPACE(message_length);
        m_msg.iov_len = m_msg_header->nlmsg_len;
    }

    /**
     * @brief entity
     * @return
     */
    struct msghdr *entity() {
        return &m_msg_entity;
    }

private:
    void *msg_body() {
        return m_msg_body;
    }

private:
    // 地址信息
    struct sockaddr_nl m_sock_addr;

    // send to kernel message header
    struct nlmsghdr *m_msg_header;
    void * m_msg_body;

    // content (message base, msg len)
    struct iovec m_msg;

    // message entity
    struct msghdr m_msg_entity;

    // 缓冲期总大小
    static const size_t LENGTH = BUFFER_LENGTH + sizeof(struct nlmsghdr);

    // 消息缓冲区大小
    char m_message_buffer[LENGTH];
};

/**
 * @brief The CNetLink class
 */
class NetlinkTransportPrivate {
public:
    /**
     * @brief CNetLink
     * @param proto_id 协议id
     * @param group_id 组id
     */
  NetlinkTransportPrivate(int proto_id, int group_id);

  /**
   * @brief ~CNetLink
   */
  ~NetlinkTransportPrivate();

  /**
   * @brief fd 获取socket文件描述符
   * @return 文件描述符
   */
  int fd();

  /**
   * @brief send 向内核模块发送消息
   * @param msg_content 带发送的应用层消息
   * @param msg_content_length 带发送的应用层消息长度
   * @return 发送结果
   */
  bool send(void *msg_content, size_t msg_content_length);

  /**
   * @brief recv 从内核模块接受消息
   * @brief recv_data 接收到的应答数据
   * @return nl:RECV_CODE
   */
  int recv(std::string &recv_data);

  /**
   * @brief connect 内核通讯组件初始化
   * @return
   */
  bool connect();

  /**
   * @brief netlink_fini 数据销毁
   */
  void close();

  /**
   * @brief valid 判断netlink是否有效
   * @return true
   */
  bool ok();

  /**
   * @brief error_code
   * @return 错误码或者错误信息
   */
  int error_code();
private:
    /**
     * @brief m_status socket初始化状态
     */
    bool m_status;

    /**
     * @brief m_socket_fd sock句柄ID
     */
    int m_socket_fd;

    /**
     * @brief m_kernel_msg 内核消息发送与接受缓冲区
     */
    Buffer<1024> m_recv_buffer;
    Buffer<1024> m_send_buffer;

    /**
     * @brief m_proto_id 与驱动层协商的协议id
     */
    int m_proto_id;

    /**
     * @brief m_group_id 与驱动层协商的组id
     */
    int m_group_id;

    /**
     * @brief m_error_code 错误码
     */
    int m_error_code;
};

#endif // NETLINKTRANSPORTPRIVATE_H
