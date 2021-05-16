#include <assert.h>
#include <endian.h>
#include <libdiscord/wsc.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Start the websocket handshake.
void handshake(ws_conn_t *conn, const char *query_params) {

  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();

  SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
  if (!ctx) {
    fprintf(stderr, "SSL_CTX_new() failed.\n");
    exit(-1);
  }

  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;

  int ec = 0;
  if ((ec = getaddrinfo(conn->uri, conn->port, &hints, &res) != 0)) {
    gai_strerror(ec);
    exit(-1);
  }

  int sock_fd = 0;
  for (struct addrinfo *i = res; i != NULL; i = i->ai_next) {

    sock_fd = socket(i->ai_family, i->ai_socktype, i->ai_protocol);
    if (sock_fd == 0) {
      continue;
    }

    if (connect(sock_fd, i->ai_addr, i->ai_addrlen) != 0) {
      continue;
    }

    break;
  }

  SSL *ssl = SSL_new(ctx);
  if (!ssl) {
    fprintf(stderr, "SSL_new() failed.\n");
    exit(-1);
  }

  if (!SSL_set_tlsext_host_name(ssl, conn->uri)) {
    fprintf(stderr, "SSL_set_tlsext_host_name() failed.\n");
    exit(-1);
  }
  SSL_set_fd(ssl, sock_fd);
  if (SSL_connect(ssl) == -1) {
    fprintf(stderr, "SSL_connect() failed.\n");
    exit(-1);
  }

  char handshake_req[1024];

  int handshake_req_len =
      sprintf(handshake_req,
              "GET %s HTTP/1.1\r\n"
              "Host: %s\r\n"
              "Upgrade: websocket\r\n"
              "Connection: Upgrade\r\n"
              "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
              "Sec-WebSocket-Version: 13\r\n"
              "\r\n",
              query_params, conn->uri);

  int r = SSL_write(ssl, handshake_req, handshake_req_len);
  printf("Written %d bytes\n", r);

  char read_buf[1024];
  int n_bytes_read = SSL_read(ssl, read_buf, 1024);

  if (n_bytes_read < 2 || read_buf[n_bytes_read - 2] != '\r' ||
      read_buf[n_bytes_read - 1] != '\n') {
    fprintf(stderr, "[ERR]: websocket handshake failed\n");
    exit(-1);
  }

  char *resp_line = strtok(read_buf, "\r\n");
  if (strcmp(resp_line, "HTTP/1.1 101") == 12) {
    fprintf(stderr,
            "[ERR]: websocket handshake failed (invalid request line)\n");
    exit(-1);
  }

  conn->ssl = ssl;
  conn->sock_fd = sock_fd;

  freeaddrinfo(res);
  SSL_CTX_free(ctx);
}

// Send a frame to the `ws` endpoint.
void send_frame(ws_conn_t *conn, ws_opcode op, char *payload,
                size_t payload_len) {

  char op_and_len[2];
  op_and_len[0] = (1 << 7) | op;

  int len = 0;
  if (payload_len < 126) {
    op_and_len[1] = (1 << 7) | payload_len;
    SSL_write(conn->ssl, op_and_len, 2);
  } else if (payload_len <= UINT16_MAX) {
    op_and_len[1] = (1 << 7) | 126u;
    SSL_write(conn->ssl, op_and_len, 2);

    uint16_t len = htons(payload_len);
    uint8_t len_d[2];
    memcpy(len_d, &len, 2);
    SSL_write(conn->ssl, len_d, 2);
  } else {
    op_and_len[1] = (1 << 7) | 127u;
    SSL_write(conn->ssl, op_and_len, 2);

    uint64_t len = htobe64(payload_len);
    uint8_t len_d[8];
    memcpy(len_d, &len, 8);

    SSL_write(conn->ssl, len_d, 8);
  }

  if (conn->debug)
    printf("[WRITE]: Opcode: %d, Payload_len: %ld, Payload: %s\n", op,
           payload_len, payload);

  if (conn->debug)
    printf("================================================\n");

  uint8_t mask_key[4] = {1, 2, 3, 4};
  SSL_write(conn->ssl, mask_key, 4);

  mask_payload((uint8_t *)payload, mask_key, payload_len);
  SSL_write(conn->ssl, payload, payload_len);
}

// Read a frame from the `ws` endpoint.
ws_frame *read_frame(ws_conn_t *conn) {

  char op_and_len[2];
  SSL_read(conn->ssl, op_and_len, 2);

  bool is_fin = op_and_len[0] >> 7 & 1;
  uint8_t op = ~(1 << 7) & op_and_len[0];

  uint8_t maybe_payload_len = ~(1 << 7) & op_and_len[1];

  size_t payload_len = 0;

  if (maybe_payload_len < 125) {
    payload_len = maybe_payload_len;
  } else if (maybe_payload_len == 126 || maybe_payload_len == 127) {
    int payload_len_bytes = maybe_payload_len == 126 ? 2 : 8;
    uint8_t len[payload_len_bytes];
    SSL_read(conn->ssl, len, payload_len_bytes);

    if (payload_len_bytes == 2) {
      uint16_t pay_len;
      memcpy(&pay_len, len, 2);
      payload_len = ntohs(pay_len);
    } else {
      uint64_t pay_len;
      memcpy(&pay_len, len, 8);
      payload_len = be64toh(pay_len);
    }
  }

  if (conn->debug)
    printf("[READ]: Fin: %hhu,Opcode: %hhu, Payload_Length: %ld, ", is_fin, op,
           payload_len);

  char *payload = (char *)calloc(payload_len, sizeof(char));
  int read_bytes = SSL_read(conn->ssl, payload, payload_len);

  if (conn->debug)
    printf("Read from socket: %d\n", read_bytes);

  while (read_bytes != payload_len) {
    int bytes_left_to_read = payload_len - read_bytes;
    int more = SSL_read(conn->ssl, &payload[read_bytes], bytes_left_to_read);
    read_bytes += more;
  }

  if (conn->debug)
    printf("Payload: %s\n", payload);

  if (conn->debug)
    printf("================================================\n");
  while (!is_fin) {
    read_frame(conn);
  }

  ws_frame *frame = (ws_frame *)malloc(sizeof(ws_frame));
  frame->fin = is_fin;
  frame->opcode = op;
  frame->payload = payload;
  frame->payload_len = payload_len;
  return frame;
}
void free_frame(ws_frame *frame) {
  if (!frame)
    return;

  if (frame->payload)
    free(frame->payload);

  free(frame);
}

// Send a ping frame.
bool ping(ws_conn_t *conn) {
  char ping_msg[] = "ping";

  send_frame(conn, PING, ping_msg, strlen(ping_msg));
  ws_frame *frame = read_frame(conn);
  assert(frame->opcode == PONG);
  free_frame(frame);
  return true;
}

// Close the websocket connection.
void ws_close(ws_conn_t *conn) {
  // send close frame.

  uint16_t code = htons(1000);
  uint8_t payload[2];

  memcpy(payload, &code, 2);

  send_frame(conn, CLOSE, (char *)payload, 2);

  ws_frame *close_ack = read_frame(conn);

  uint16_t ack_code;
  memcpy(&ack_code, close_ack->payload, 2);
  ack_code = ntohs(ack_code);
  free_frame(close_ack);

  assert(ack_code == 1000);

  SSL_shutdown(conn->ssl);
  close(conn->sock_fd);
  SSL_free(conn->ssl);
}

// Mask the payload in place.
void mask_payload(uint8_t *payload, uint8_t key[4], size_t payload_len) {
  for (int i = 0; i < payload_len; i++) {
    payload[i] ^= key[i % 4];
  }
}
