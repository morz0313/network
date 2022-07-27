/*
 * @Copyright    : Copyright xxx, Inc.
 * @Author       : morz
 * @Date         : 2022-07-14 16:06:03
 * @Description  : xxx
 */

#include "tcp_client.h"

#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>

#include "uv.h"

#define IPNAME_LEN 64

namespace network {
typedef struct {
  connect_cb cb;
  TcpClient *client_ptr;
} ConnectReq;

typedef struct {
  uv_stream_t *handle;
  TcpClient *client_ptr;
} ThreadReq;

void uv_alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = new char[suggested_size];
  buf->len = suggested_size;
}

void uv_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
  TcpClient *ptr = (TcpClient *)stream->data;

  if (ptr->GetRecvmsgCb()) ptr->GetRecvmsgCb()(nread, buf->base);

  delete[] buf->base;
}

void uv_thread_cb(void *arg) {
  ThreadReq *ptr = (ThreadReq *)arg;

  ptr->handle->data = (void *)ptr->client_ptr;
  uv_read_start(ptr->handle, uv_alloc_cb, uv_read_cb);
}

void uv_connect_cb(uv_connect_t *req, int status) {
  ConnectReq *ptr = (ConnectReq *)req->data;

  ptr->cb(status);

  // connect success
  if (0 == status) {
    uv_thread_t tid;
    ThreadReq *t_req = new ThreadReq();
    t_req->handle = req->handle;
    t_req->client_ptr = ptr->client_ptr;
    uv_thread_create(&tid, uv_thread_cb, (void *)t_req);
  }
}

void uv_write_cb(uv_write_t *req, int status) {
  ((sendmsg_cb)req->data)(status);
}

TcpClient::TcpClient() : af_(AF_INET), recv_cb_(NULL) {
  int retval = -1;

  retval = uv_loop_init(&loop);
  if (0 != retval) goto error;

  retval = uv_tcp_init(&loop, &tcp_cli);
  if (0 != retval) goto error;

  return;
error:
  string err_msg(uv_strerror(retval));
  throw std::runtime_error(err_msg);
}

TcpClient::TcpClient(const char *host, int port) : recv_cb_(NULL) {
  int retval = -1;

  retval = uv_loop_init(&loop);
  if (0 != retval) goto error;

  retval = uv_tcp_init(&loop, &tcp_cli);
  if (0 != retval) goto error;

  if (IsIpv4(host)) {
    af_ = AF_INET;
    struct sockaddr_in local_addr;
    retval = uv_ip4_addr(host, port, &local_addr);
    if (0 != retval) goto error;

    retval = uv_tcp_bind(&tcp_cli, (sockaddr *)&local_addr, 0);
  } else {
    af_ = AF_INET6;
    struct sockaddr_in6 local_addr;
    retval = uv_ip6_addr(host, port, (sockaddr_in6 *)&local_addr);
    if (0 != retval) goto error;

    retval = uv_tcp_bind(&tcp_cli, (sockaddr *)&local_addr, 0);
  }
  if (0 != retval) goto error;

  return;
error:
  string err_msg(uv_strerror(retval));
  throw std::runtime_error(err_msg);
}

TcpClient::~TcpClient() {
  uv_close((uv_handle_t *)&tcp_cli, NULL);
  uv_loop_close(&loop);
}

void TcpClient::Connect(const char *peer, int port, connect_cb cb) {
  int retval = -1;
  uv_connect_t req;

  ConnectReq con_req;
  con_req.cb = cb;
  con_req.client_ptr = this;
  uv_req_set_data((uv_req_t *)&req, (void *)&con_req);

  if (IsIpv4(peer)) {
    struct sockaddr_in peer_addr;
    retval = uv_ip4_addr(peer, port, &peer_addr);
    if (0 != retval) goto error;

    retval =
        uv_tcp_connect(&req, &tcp_cli, (sockaddr *)&peer_addr, uv_connect_cb);
    if (0 != retval) goto error;
  } else {
    struct sockaddr_in6 peer_addr;
    retval = uv_ip6_addr(peer, port, &peer_addr);
    if (0 != retval) goto error;

    retval =
        uv_tcp_connect(&req, &tcp_cli, (sockaddr *)&peer_addr, uv_connect_cb);
    if (0 != retval) goto error;
  }

  retval = uv_run(&loop, UV_RUN_DEFAULT);
  if (0 != retval) goto error;

  return;
error:
  string err_msg(uv_strerror(retval));
  throw std::runtime_error(err_msg);
}

void TcpClient::SetNodelay(bool on) { uv_tcp_nodelay(&tcp_cli, on); }

void TcpClient::SetKeepAlive(bool on, unsigned int delay) {
  uv_tcp_keepalive(&tcp_cli, (int)on, delay);
}

string TcpClient::GetLocalIp() {
  char ipname[IPNAME_LEN] = {0};

  if (AF_INET == af_) {
    struct sockaddr_in sockname;
    int namelen = sizeof(sockaddr);
    uv_tcp_getsockname(&tcp_cli, (struct sockaddr *)&sockname, &namelen);
    uv_ip_name((struct sockaddr *)&sockname, ipname, IPNAME_LEN);
  } else if (AF_INET6 == af_) {
    struct sockaddr_in6 sockname;
    int namelen = sizeof(sockaddr);
    uv_tcp_getsockname(&tcp_cli, (struct sockaddr *)&sockname, &namelen);
    uv_ip_name((struct sockaddr *)&sockname, ipname, IPNAME_LEN);
  }
  return string(ipname);
}

string TcpClient::GetPeerIp() {
  char ipname[IPNAME_LEN] = {0};

  if (AF_INET == af_) {
    struct sockaddr_in sockname;
    int namelen = sizeof(sockaddr);
    uv_tcp_getpeername(&tcp_cli, (struct sockaddr *)&sockname, &namelen);
    uv_ip_name((struct sockaddr *)&sockname, ipname, IPNAME_LEN);
  } else if (AF_INET6 == af_) {
    struct sockaddr_in6 sockname;
    int namelen = sizeof(sockaddr);
    uv_tcp_getpeername(&tcp_cli, (struct sockaddr *)&sockname, &namelen);
    uv_ip_name((struct sockaddr *)&sockname, ipname, IPNAME_LEN);
  }
  return string(ipname);
}

int TcpClient::GetSendBufSize() {
  int val;
  uv_send_buffer_size((uv_handle_t *)&tcp_cli, &val);
  return val;
}

int TcpClient::GetRecvBufSize() {
  int val;
  uv_recv_buffer_size((uv_handle_t *)&tcp_cli, &val);
  return val;
}

void TcpClient::SendMsg(char *buf, int len, sendmsg_cb cb) {
  int retval = -1;
  uv_write_t req;
  uv_req_set_data((uv_req_t *)&req, (void *)cb);

  uv_buf_t buf_temp = uv_buf_init(buf, len);

  retval = uv_write(&req, (uv_stream_t *)&tcp_cli, &buf_temp, 1, uv_write_cb);
  if (0 != retval) goto error;

  retval = uv_run(&loop, UV_RUN_DEFAULT);
  if (0 != retval) goto error;

  return;
error:
  string err_msg(uv_strerror(retval));
  throw std::runtime_error(err_msg);
}

void TcpClient::SetRecvmsgCb(recvmsg_cb cb) { recv_cb_ = cb; }

recvmsg_cb TcpClient::GetRecvmsgCb() { return recv_cb_; }

void TcpClient::Close() { uv_close((uv_handle_t *)&tcp_cli, NULL); }

bool TcpClient::IsIpv4(const char *ip) {
  struct sockaddr_in addr;
  return 0 == uv_inet_pton(AF_INET, ip, &(addr.sin_addr.s_addr));
}
}  // namespace network
