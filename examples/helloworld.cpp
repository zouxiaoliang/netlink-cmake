#include <NetlinkTransport.h>
#include <iostream>
#include <memory>
#include <string.h>
#include <thread>
#include <unistd.h>

int main(int argc, char *argv[]) {
  std::cout << "Hello world" << std::endl;
  auto netlinke_session = std::make_shared<NetlinkTransport>(31, 1);
  if (nullptr == netlinke_session) {
    return 1;
  }

  const char *msg = "hello world, i'm userspace.";
  std::thread t([&] {
    while (true) {
      if (!netlinke_session->connect()) {
        std::cout << "connecto to kernel failed." << std::endl;
        sleep(1);
      }

      std::cout << "send message to kernel ..." << std::endl;
      if (!netlinke_session->send((void *)msg, strlen(msg))) {
        std::cout << "sent the message to kernel failed." << std::endl;
        netlinke_session->close();
      }
      sleep(1);
    }
  });

  t.join();
  return 0;
}
