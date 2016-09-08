#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
#include <inttypes.h>
#include <math.h>

int64_t counter = 0;

void wait_for_a_while(uv_idle_t* handle)
{
  counter++;

  if(fmod(counter, 1e6) == 0) printf("->%" PRId64 "\n", counter);

  if (counter >= 10e6) {
    uv_idle_stop(handle);
  }
}

/* int main(int argc, char *argv[]) */
int main()
{
  uv_idle_t idler;

  printf("Test!\n");

  uv_idle_init(uv_default_loop(), &idler);
  uv_idle_start(&idler, wait_for_a_while);

  printf("Idling...\n");
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  printf("Now quitting.\n");
  uv_loop_close(uv_default_loop());

  return 0;
}
