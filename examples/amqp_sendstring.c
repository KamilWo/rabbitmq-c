#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <amqp.h>
#include <amqp_framing.h>

#include <unistd.h>

#include "example_utils.h"

int main(int argc, char const * const *argv) {
  char const *hostname;
  int port;
  char const *exchange;
  char const *routingkey;
  char const *messagebody;

  int sockfd;
  amqp_connection_state_t conn;

  if (argc < 6) {
    fprintf(stderr, "Usage: amqp_sendstring host port exchange routingkey messagebody\n");
    return 1;
  }

  hostname = argv[1];
  port = atoi(argv[2]);
  exchange = argv[3];
  routingkey = argv[4];
  messagebody = argv[5];

  conn = amqp_new_connection();

  die_on_error(sockfd = amqp_open_socket(hostname, port), "Opening socket");
  amqp_set_sockfd(conn, sockfd);
  die_on_amqp_error(amqp_login(conn, "/", 131072, AMQP_SASL_METHOD_PLAIN, "guest", "guest"),
		    "Logging in");

  {
    amqp_basic_properties_t props;
    props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG;
    props.content_type = amqp_cstring_bytes("text/plain");
    die_on_error(amqp_basic_publish(conn,
				    amqp_cstring_bytes(exchange),
				    amqp_cstring_bytes(routingkey),
				    0,
				    0,
				    &props,
				    amqp_cstring_bytes(messagebody)),
		 "Publishing");
  }

  die_on_amqp_error(amqp_channel_close(conn, AMQP_REPLY_SUCCESS), "Closing channel");
  die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
  amqp_destroy_connection(conn);
  die_on_error(close(sockfd), "Closing socket");
  return 0;
}
