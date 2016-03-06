#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#include <binder/Parcel.h>

// Return the number of nanoseconds between two timespecs
uint64_t GetDiff(struct timespec* start_time, struct timespec* end_time) {
  time_t secs = end_time->tv_sec - start_time->tv_sec;
  uint64_t diff = (uint64_t)secs * 1000000000;

  if (end_time->tv_nsec >= start_time->tv_nsec) {
    diff += end_time->tv_nsec - start_time->tv_nsec;
  } else {
    diff -= start_time->tv_nsec - end_time->tv_nsec;
  }
  return diff;
}

#define NUM 10000
char buf[1024 * 1024];

// How many extra pairs of client servers to run
// concurrently.
#define CONCURENT 0

int main() {
  int p = 0;
  for (int l = 0; l < CONCURENT; l++) {
    if (fork() == 0)
      break;
    p = 1;
  }

  // Make sure there is some data, so the kernel
  // can't optimise for zero pages.
  memset(buf, 1, sizeof(buf));

  printf("Socket Ping Tests %d %d\n", getpid(), p);
  
  int fds[2];

  char* aligned_buf = (char*)((int)(buf + 0xFFF) & (int)~(0xFFF));
  // Uncomment this to enforce unaligned buffers. Makes big difference.
  // aligned_buf +=4;
  printf("%p %p\n", buf, aligned_buf);
  if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0, fds) != 0) {
    printf("Couldn't make sockets\n");
    return 1;
  }

  int server = fds[0];
  int client = fds[1];

  if (fork() == 0) {
    // Child - client
    struct timespec start_time;
    struct timespec end_time;
    int j;
    for (j = 2; j < (256 * 1024); j *= 2) {
      clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);
      int i;
      for (i = 0; i < NUM; i++) {
        android::Parcel data;
        data.write(aligned_buf, j);
        if (write(client, aligned_buf, j) != j) {
          printf("BAD WRITE\n");
          fflush(stdout);
        }
        read(client, aligned_buf, 1);
      }
      clock_gettime(CLOCK_MONOTONIC_RAW, &end_time);
      uint64_t diff = GetDiff(&start_time, &end_time);
      if (!p)
        printf("%d %lld\n", j, (diff / NUM) / 1000);
      fflush(stdout);
    }
  } else {
    // Parent - Server
    // Parent just replies to ping.
    int j;
    for (j = 2; j < (256 * 1024); j *= 2) {
      int rx = 0;
      int i;
      for (i = 0; i < NUM; i++) {
        rx += read(server, aligned_buf, 1024 * 1024);
        write(server, aligned_buf, 1);
      }
    }
  }

  return 0;
}
