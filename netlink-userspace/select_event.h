#ifndef SELECT_EVENT_H
#define SELECT_EVENT_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <functional>
#include <iostream>

using namespace std;

namespace netlink {
namespace userspace {

/**
 * @brief The SelectEvent class 基于select模型实现的事件处理机制
 */
class SelectEvent {
public:
    enum {
        DO_READ,    // 处理读事件
        DO_WRITE,   // 处理写事件
        DO_ERROR,   // 处理错误
        DO_TIMEOUT, // 处理超时
        DO_EVENT_MAX
    };

public:
    SelectEvent();
    /**
     * @brief SelectEvent
     * @param fd 文件描述符
     * @param tv 超时时间
     */
    SelectEvent(int fd, struct timeval& tv);

    ~SelectEvent();

    /**
     * @brief init 初始化内部信息
     * @param fd 文件描述符
     * @param tv 超时时间
     */
    void init(int fd, struct timeval& tv);

    /**
     * @brief async_write 设置句柄有写事件
     * @param fd
     */
    void async_write(int fd);

    /**
     * @brief run_one 执行事件环
     * @attention run方法需要放到单独的线程执行
     */
    void run_one();

    /**
     * @brief set_event
     * @param event_id
     * @param event
     */
    void set_do_event(int event_id, std::function<void()> event);

  private:
    /**
     * @brief on_read 处理读事件
     * @param fds 可以读的文件描述符集合
     */
    void on_read(fd_set& fds);

    /**
     * @brief on_write 处理写事件
     * @param fds 可以写的文件描述符集合
     */
    void on_write(fd_set& fds);

    /**
     * @brief on_error 处理select错误
     */
    void on_error();

    /**
     * @brief on_timeout 处理select超时事件
     */
    void on_timeout();

private:
    // 禁止拷贝构造函数和赋值方法
    SelectEvent(const SelectEvent&) = delete;
    SelectEvent& operator=(const SelectEvent&) = delete;

private:
    int                    m_fd;
    struct timeval         m_tv;
    std::atomic_uint64_t m_want_to_write;

    std::function<void()> m_do_events[DO_EVENT_MAX];
};

} // namespace userspace
} // namespace netlink
#endif // SELECT_EVENT_H
