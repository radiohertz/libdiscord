#ifndef _WSC_H
#define _WSC_H

#include <openssl/ssl.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {

  CONT = 0x0,

  TEXT,

  BIN,

  CLOSE = 0x8,

  PING,

  PONG

} ws_opcode;

typedef struct {
  // the websocket endpoint.
  // Starts with either `ws://` or `wss://`.
  // Both should be omitted.
  char *uri;
  // the port of the websocket endpoint.
  // `80` for `ws://`.
  // `443` for `wss://`.
  char *port;
  int sock_fd;
  SSL *ssl;
  bool debug;
} ws_conn_t;

typedef struct {
  bool fin;
  uint8_t opcode;
  size_t payload_len;
  char *payload;
} ws_frame;

void mask_payload(uint8_t *payload, uint8_t key[4], size_t payload_len);

void handshake(ws_conn_t *conn, const char *query_params);

void send_frame(ws_conn_t *conn, ws_opcode op, char *payload,
                size_t payload_len);

ws_frame *read_frame(ws_conn_t *conn);

void free_frame(ws_frame *frame);

bool ping(ws_conn_t *conn);

void ws_close(ws_conn_t *conn);

void mask_payload(uint8_t *payload, uint8_t key[4], size_t payload_len);

#endif