#include "../include/socks5_copy.h"




////////////////////////////////////////////////////////////////////////
// COPY
////////////////////////////////////////////////////////////////////////

void copy_init(const unsigned state, struct selector_key *key) {

  struct socks5 *sockState = ATTACHMENT(key);

  // Init of the copy for the client
  struct copy_st *client_copy = &sockState->client_handler.copy;
  client_copy->fd = sockState->client_fd;
  client_copy->rb = &sockState->read_buffer;
  client_copy->wb = &sockState->write_buffer;
  client_copy->interest = OP_READ | OP_WRITE;
  client_copy->other_copy = &sockState->orig_handler.copy;

  // Init of the copy for the origin
  struct copy_st *orig_copy = &sockState->orig_handler.copy;
  orig_copy->fd = sockState->sel_origin_fd;
  orig_copy->rb = &sockState->write_buffer;
  orig_copy->wb = &sockState->read_buffer;
  orig_copy->interest = OP_READ | OP_WRITE;
  orig_copy->other_copy = &sockState->client_handler.copy;
}

void copy_close(const unsigned state, struct selector_key *key) {
  /*TODO. */
}

/**
 *  Gets the pointer to the copy_st depending on the selector fired
 * */
struct copy_st *get_copy_ptr(struct selector_key *key) {
  // Getting the copy struct for the client
  struct copy_st *client_copy = &ATTACHMENT(key)->client_handler.copy;

  // Checking if the selector fired is the client by comparing the fd
  if (client_copy->fd != key->fd) {
    client_copy = client_copy->other_copy;
  }

  return client_copy;
}

/**
 * Determines the new interest of the given copy_st and sets it in the selector
 */
fd_interest copy_determine_interests(fd_selector s,
                                            struct copy_st *data) {
  // Basic interest of no operation
  fd_interest interest = OP_NOOP;

  // If the copy_st is interested in reading and we can write in its buffer
  if ((data->interest & OP_READ) && buffer_can_write(data->rb)) {
    // Add the interest to read
    interest |= OP_READ;
  }

  // If the copy_st is interested in writing and we can read from its buffer
  if ((data->interest & OP_WRITE) && buffer_can_read(data->wb)) {
    // Add the interest to write
    interest |= OP_WRITE;
  }

  // Set the interests for the selector
  if (SELECTOR_SUCCESS != selector_set_interest(s, data->fd, interest)) {
    printf("Could not set interest of %d for %d\n", interest, data->fd);
    abort();
  }
  return interest;
}

unsigned copy_read(struct selector_key *key) {

  struct socks5 *s = ATTACHMENT(key);
  // Getting the state struct
  struct copy_st *data = get_copy_ptr(key);

  log(INFO, "esoty en copy_read");

  // Getting the read buffer
  buffer *b = data->rb;
  unsigned ret = COPY;
  uint8_t *ptr;
  size_t count; // Maximum data that can set in the buffer
  ssize_t n;
  // int errored = 0;
  // buffer *aux_b;

  // Setting the buffer to read
  ptr = buffer_write_ptr(b, &count);
  // Receiving data
  n = recv(key->fd, ptr, count, 0);
  if (n > 0) {

    // Notifying the data to the buffer
    buffer_write_adv(b, n);

  } else {
    // Closing the socket for reading
    shutdown(data->fd, SHUT_RD);
    // Removing the interest to read from this copy
    data->interest &= ~OP_READ;
    // If the other fd is still open
    if (data->other_copy->fd != -1) {
      // Closing the socket for writing
      shutdown(data->other_copy->fd, SHUT_WR);
      // Remove the interest to write
      data->other_copy->interest &= ~OP_WRITE;
    }
  }

  // Determining the new interests for the selectors
  copy_determine_interests(key->s, data);
  copy_determine_interests(key->s, data->other_copy);

  // Checking if the copy_st is not interested anymore in interacting -> Close
  // it
  if (data->interest == OP_NOOP) {
    ret = DONE;
  }

  return ret;
}

unsigned copy_write(struct selector_key *key) {
  // Getting the state struct
  struct copy_st *data = get_copy_ptr(key);

  // Getting the read buffer
  buffer *b = data->wb;
  unsigned ret = COPY;
  uint8_t *ptr;
  size_t count; // Maximum data that can set in the buffer
  ssize_t n;

  // Setting the buffer to read
  ptr = buffer_read_ptr(b, &count);

  // Receiving data
  n = send(key->fd, ptr, count, MSG_NOSIGNAL);

  if (n > 0) {
    // TODO: Metrics
    // Notifying the data to the buffer
    buffer_read_adv(b, n);
  } else {
    // Closing the socket for writing
    shutdown(data->fd, SHUT_WR);
    // Removing the interest to write from this copy
    data->interest &= ~OP_WRITE;
    // If the other fd is still open
    if (data->other_copy->fd != -1) {
      // Closing the socket for reading
      shutdown(data->other_copy->fd, SHUT_RD);
      // Remove the interest for reading
      data->other_copy->interest &= ~OP_READ;
    }
  }

  // Determining the new interests for the selectors
  copy_determine_interests(key->s, data);
  copy_determine_interests(key->s, data->other_copy);

  // Checking if the copy_st is not interested anymore in interacting -> Close
  // it
  if (data->interest == OP_NOOP) {
    ret = DONE;
  }

  return ret;
}


