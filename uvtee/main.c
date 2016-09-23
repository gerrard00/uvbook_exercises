#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <uv.h>

typedef struct {
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

uv_pipe_t stdin_pipe, stdout_pipe;
/* TODO: test for this */
#define MAX_NUMBER_FILES 10
uv_pipe_t file_pipes[MAX_NUMBER_FILES];
void alloc_buffer(uv_handle_t*, size_t, uv_buf_t*);
void read_stdin(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
void free_write_req(uv_write_t *req);
void on_stdout_write(uv_write_t *req, int status);
void on_file_write(uv_write_t *req, int status);
void write_data(uv_stream_t *dest, size_t size, uv_buf_t buf, uv_write_cb cb, int file_index);

bool read_complete;
/* the number of writes before we can clear the buffer */
int outstanding_writes;
/* number of output files */
int number_of_files;

int
main(int argc, char **argv)
{
  read_complete = false;
  uv_fs_t file_req;

  uv_loop_t *loop = uv_default_loop();

  uv_pipe_init(loop, &stdin_pipe, 0);
  uv_pipe_open(&stdin_pipe, 0);

  uv_pipe_init(loop, &stdout_pipe, 0);
  uv_pipe_open(&stdout_pipe, 1);

  number_of_files = argc - 1;

  if (number_of_files > MAX_NUMBER_FILES) {
    fprintf(stderr, "Maximum number of files is %d\n", MAX_NUMBER_FILES);
    return -1;
  }

  if (number_of_files) {
    for(int file_index = 0; file_index < number_of_files; file_index++) {
      /* the argv index is the file index + 1 for the process name */
      int fd = uv_fs_open(loop, &file_req,
          argv[file_index + 1], O_CREAT | O_RDWR, 0644, NULL);
      uv_pipe_init(loop, &file_pipes[file_index], 0);
      uv_pipe_open(&file_pipes[file_index], fd);
    }
  }

  uv_read_start((uv_stream_t*)&stdin_pipe, alloc_buffer, read_stdin);

  uv_run(loop, UV_RUN_DEFAULT);
  uv_loop_close(loop);
  /* we are using the default loop, don't have to close it */
  /* free(loop); */
  return 0;
}

void
alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
  *buf = uv_buf_init((char*) malloc(suggested_size), suggested_size);
}

void
read_stdin(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
  if (nread < 0) {
    if (nread == UV_EOF) {
      read_complete = true;
      // end of file
      uv_close((uv_handle_t*)&stdin_pipe, NULL);
    }
  } else if (nread > 0) {
    /* +1 for stdout */
    outstanding_writes = number_of_files + 1;
    /* TODO: make that -1 a macro */
    write_data((uv_stream_t*)&stdout_pipe, nread, *buf, on_stdout_write, -1);

    for(int file_index = 0; file_index < number_of_files; file_index++) {
      write_data((uv_stream_t*)&file_pipes[file_index],
          nread, *buf, on_file_write, file_index);
    }
  }
}

void
free_write_req(uv_write_t *req)
{
  write_req_t *wr = (write_req_t*) req;

  if (--outstanding_writes <= 0) {
    free(wr->buf.base);
  }

  free(wr);
}

void
on_stdout_write(uv_write_t *req, int status)
{
  free_write_req(req);

  if (read_complete) {
    uv_close((uv_handle_t*)&stdout_pipe, NULL);
  }
}

void
on_file_write(uv_write_t *req, int status)
{
  /* need to get int from void pointer */
  int file_index = *((int*)req->data);
  free_write_req(req);

  if (read_complete && file_index > -1) {
    uv_close((uv_handle_t*)&file_pipes[file_index], NULL);
  }
}

void
write_data(uv_stream_t *dest,
    size_t size, uv_buf_t buf, uv_write_cb cb, int file_index)
{

  write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
  req->buf = buf;

  /* set the data on the uv_write_req to the file_index */
  req->req.data = malloc(sizeof(int));
  *((int*)req->req.data) = file_index;

  uv_write((uv_write_t*) req, (uv_stream_t*) dest, &req->buf, 1, cb);
}
