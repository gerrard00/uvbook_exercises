#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
#include <assert.h>
/* just for STDOUT_FILENO, not needed by example */
#include <unistd.h>

void on_open(uv_fs_t *req);
void on_read(uv_fs_t *req);
void on_write(uv_fs_t *req);

uv_fs_t open_req;
uv_fs_t read_req;
uv_fs_t write_req;
static char buffer[1024];

static uv_buf_t iov;

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Enter a target file name.\n");
  }

  uv_fs_open(uv_default_loop(), &open_req, argv[1], O_RDONLY, 0, on_open);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  uv_fs_req_cleanup(&open_req);
  uv_fs_req_cleanup(&read_req);
  uv_fs_req_cleanup(&write_req);

  return 0;
}

void on_open(uv_fs_t *req)
{
  // The request passed to the callback is the same as the one the call setup
  // function was passed.
  assert(req == &open_req);

  if(req->result >= 0) {
    iov = uv_buf_init(buffer, sizeof(buffer));
    uv_fs_read(uv_default_loop(), &read_req, req->result,
        &iov, 1, -1, on_read);
  } else {
    fprintf(stderr, "error opening file: %s\n", uv_strerror((int)req->result));
  }
}

void on_read(uv_fs_t *req)
{
  //NOTE: req->result is length of buffer, or negative for error
  if(req->result < 0) {
    //TODO: why don't we have to case req-result to int here?
    fprintf(stderr, "Read error: %s\n", uv_strerror(req->result));
  } else if (req->result == 0) {
    // empty file, just close
    uv_fs_t close_req;
    // synchronous
    uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
  } else {
    iov.len = req->result;
    /* uv_fs_write(uv_default_loop(), &write_req, 1, &iov, 1, -1, on_write); */
    uv_fs_write(uv_default_loop(), &write_req, STDOUT_FILENO, &iov, 1, -1, on_write);
  }
}

void on_write(uv_fs_t *req)
{
  printf("\n**********on_write called**********\n");
  if (req->result < 0) {
    fprintf(stderr, "Write error: %s\n", uv_strerror((int)req->result));
  } else {
    uv_fs_read(uv_default_loop(), &read_req, open_req.result, &iov, 1, -1, on_read);
  }
}
