//Taken from https://stackoverflow.com/questions/15215865/netlink-sockets-in-c-using-the-3-x-linux-kernel?lq=1

#include <linux/kernel.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/version.h>
#include <net/netlink.h>
#include <net/sock.h>

#define NETLINK_USER 31
#define NETLINK_GROUP_ID 1

#define USING_BUFFER

typedef struct {
    int pid;
    unsigned int vl;
    char v[0];
}  ProtoBuffer;

struct sock *nl_sk = NULL;
static size_t message_count = 0;

static void hello(struct sk_buff *__skb);

static void nl_recv_msg(struct sock *sk, int len);

static void nl_send_msg(int pid, char *msg, int len);

static void xx_netlink_kernel_release(struct sock *sk);

static void xx_netlink_kernel_release(struct sock *sk) {
  if (sk != NULL && sk->sk_socket != NULL) {
    sock_release(sk->sk_socket);
  }
}

static void hello(struct sk_buff *skb) {
    // printk("bgn %s check skb is shared: %d\n", __func__, skb_shared(skb));
    struct nlmsghdr* nhl;
    if (skb->len >= NLMSG_SPACE(0)) {
        nhl = (struct nlmsghdr*)skb->data;
        ProtoBuffer *pb = (ProtoBuffer*) nhl;
        // printk("client applcation connected, pid: %d, content length: %u, package length: %u\n", 
        //      pb->pid, pb->vl, skb->len
        // );
        // printk("netlink received msg payload: %s\n", pb->v);
        nl_send_msg(pb->pid, "testing", 8);
    }
    // printk("end %s check skb is shared: %d\n", __func__, skb_shared(skb));
    if (++message_count % 5000 == 0) {
        printk("recv message count: %lu\n", message_count);
    }
}

static void nl_recv_msg(struct sock *sk, int len) {
    struct sk_buff *skb;
    unsigned int qlen;
    for (qlen = skb_queue_len(&sk->sk_receive_queue); qlen; qlen--) {
		skb = skb_dequeue(&sk->sk_receive_queue);
        // printk("bgn %s check skb is shared: %d\n", __func__, skb_shared(skb));
        hello(skb);
        kfree_skb(skb);
        // printk("end %s check skb is shared: %d\n", __func__, skb_shared(skb));
    }
}

static void nl_recv_msg5x(struct sk_buff *skb) {
  struct nlmsghdr *nlh = (struct nlmsghdr *)skb->data;
  int pid = nlh->nlmsg_pid;
  char *msg = (char *)nlmsg_data(nlh);

  printk("nerlink recvived from pid: %d, message: %s\n", pid, msg);
}

static void nl_send_msg(int pid, char *msg, int len) {
    struct sk_buff *sk_msg_buff = NULL;
    struct nlmsghdr *nl_msg_hd = NULL;
    unsigned char *old_tail;
    int nl_total_len = NLMSG_SPACE(len);
    // 为新的 sk_buffer申请空间
    sk_msg_buff = alloc_skb(nl_total_len, GFP_KERNEL);
    if(NULL == sk_msg_buff)
    {
        printk(KERN_ERR "netlink alloc_skb error\n");
        return;
    }
    old_tail = (unsigned char *)(sk_msg_buff->tail);

    // 设置netlink消息头部
    nl_msg_hd = nlmsg_put(sk_msg_buff, 0, 0, NLMSG_NOOP, nl_total_len - sizeof(struct nlmsghdr), 0);
    if(NULL == nl_msg_hd)
    {
        kfree_skb(sk_msg_buff);
        printk(KERN_ERR "netlink nlmsg_put error\n");
        return;
    }
    memcpy(NLMSG_DATA(nl_msg_hd), msg, len);
    nl_msg_hd->nlmsg_len = (unsigned char *)(sk_msg_buff->tail) - old_tail;
    // 将消息发送用户空间, 由pid指定进程
    if (netlink_unicast(nl_sk, sk_msg_buff, pid, 0) < 0)
    {
        printk(KERN_ERR "netlink send data error\n");
        return;
    }
}

int kmodule_init(void) {

  printk("Entering: %s\n", __FUNCTION__);
#if 1
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 0, 0)
  nl_sk = netlink_kernel_create(31, 1, &nl_recv_msg, THIS_MODULE);
#else
  struct netlink_kernel_cfg cfg = {
      .input = nl_recv_msg5x,
  };
  nl_sk = netlink_kernel_create(&init_net, 31, &cfg);
#endif
  // nl_sk = netlink_kernel_create(31, 1, NULL, THIS_MODULE);
  if (!nl_sk) {
    printk(KERN_ALERT "Error creating socket.\n");
    return -1;
  }
#endif
  return 0;
}

void kmodule_exit(void) {
  printk(KERN_INFO "exiting hello module\n");
  xx_netlink_kernel_release(nl_sk);
}
