/*
 * @Copyright    : Copyright Datang Mobile, Inc.
 * @Author       : morz
 * @Date         : 2022-07-15 14:34:24
 * @Description  : xxx
 */

#include <cstring>
#include <iostream>
#include <string>

#include "tcp_client.h"

void ConnectCb(int status) {
  std::cout << "connect status:" << status << std::endl;
}

void SendMsgCb(int status) {
  std::cout << "send msg status:" << status << std::endl;
}

void RecvMsgCb(ssize_t nread, char *buf) {
  std::cout << "recv msg size:" << nread << ",msg:" << buf << std::endl;
}

int main() {
  network::TcpClient tcp_client("0.0.0.0", 30002);

  tcp_client.SetRecvmsgCb(RecvMsgCb);
  tcp_client.Connect("172.21.33.48", 33306, ConnectCb);
  uv_sleep(1000 * 3);

  char buf[10] = {0};
  tcp_client.SendMsg(buf, 10, SendMsgCb);

  while (true) {
    /* code */
    uv_sleep(1000 * 3);
  }

  return 0;
}
