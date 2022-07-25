/*
 * @Copyright    : Copyright xxx, Inc.
 * @Author       : morz
 * @Date         : 2022-07-14 15:13:13
 * @Description  : xxx
 */

#ifndef NETWORK_TCP_CLIENT_H_
#define NETWORK_TCP_CLIENT_H_

#include <string>

#include "uv.h"

using std::string;

namespace network {

typedef void (*connect_cb)(int status);
typedef void (*sendmsg_cb)(int status);
typedef void (*recvmsg_cb)(ssize_t nread, char *buf);

class TcpClient {
 public:
  /**
   * @description: 初始化 TcpClient 类的新实例
   */
  TcpClient();

  /**
   * @description: 初始化 TcpClient 类的新实例，并将其绑定到指定的地址和端口
   * @param {char} *ip 绑定的地址
   * @param {int} port 绑定的端口
   * @return {*}
   */
  TcpClient(const char *host, int port);

  ~TcpClient();

  void Connect(const char *peer, int port, connect_cb cb);
  
  /**
   * @description: 设置禁用延迟，该值在发送或接收缓冲区未满时禁用延迟，默认值为false
   * @param {bool} on
   * @return {*}
   */  
  void SetNodelay(bool on);

  /**
   * @description: 设置保活
   * @param {bool} on 
   * @param {unsigned int} delay 单位秒
   * @return {*}
   */  
  void SetKeepAlive(bool on, unsigned int delay);

  int GetSendBufSize();

  int GetRecvBufSize();

  string GetLocalIp();

  string GetPeerIp();

  void SendMsg(char *buf, int len, sendmsg_cb cb);

  void SetRecvmsgCb(recvmsg_cb cb);

  recvmsg_cb GetRecvmsgCb();

  void Close();

 private:
  int af_;
  uv_loop_t loop;
  uv_tcp_t tcp_cli;
  recvmsg_cb recv_cb_;

  bool IsIpv4(const char *ip);
};

}  // namespace network

#endif  // NETWORK_TCP_CLIENT_H_