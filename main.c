#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

int main(int argc, char *argv[])
{
  printf("Test!\n");

  uv_loop_t *loop = malloc(sizeof(uv_loop_t));
  uv_loop_init(loop);

  printf("Now quitting.\n");
  uv_run(loop, UV_RUN_DEFAULT);


  free(loop);
}
