#include <stdio.h>
#include <binder/IInterface.h>
#include <binder/IBinder.h>

#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

#include <time.h>
#include <unistd.h>

namespace android {

char buf[1024 * 1024];

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

// Simple binder interface with one method.
class IIpc : public IInterface {
 public:
  enum {
    PING = IBinder::FIRST_CALL_TRANSACTION,
  };
  virtual void ping(int len) = 0;

  DECLARE_META_INTERFACE(Ipc);
};

// Client interface
class BpIpc : public BpInterface<IIpc> {
 public:
  BpIpc(const sp<IBinder>& impl) : BpInterface<IIpc>(impl) {
    printf("Creating BpIpc\n");
  }

  // Write the payload and transact with binder.
  void ping(int len) {
    Parcel data, reply;
    data.writeInterfaceToken(IIpc::getInterfaceDescriptor());
    data.write(buf, len);
    remote()->transact(PING, data, &reply);
  }
};

IMPLEMENT_META_INTERFACE(Ipc, "Ipc");

class BnIpc : public BnInterface<IIpc> {
  status_t onTransact(uint32_t code,
                      const Parcel& data,
                      Parcel* reply,
                      uint32_t flags = 0);
};

// Server side.
status_t BnIpc::onTransact(uint32_t code,
                           const Parcel& data,
                           Parcel* reply,
                           uint32_t flags) {
  // Don't even dispatch the call to the
  // server side of ping. Just return NO_ERROR.
  return NO_ERROR;
}

class Ipc : public BnIpc {
  void ping(int len) {}
};

sp<IIpc> GetIPCBinder(char* name) {
  sp<IServiceManager> sm = defaultServiceManager();
  sp<IBinder> binder = sm->getService(String16(name));
  sp<IIpc> ipc = interface_cast<IIpc>(binder);
  return ipc;
}

void StartServer(char* name) {
  printf("Starting Server\n");
  defaultServiceManager()->addService(String16(name), new Ipc());
  IPCThreadState::self()->disableBackgroundScheduling(true);
  ProcessState::self()->setThreadPoolMaxThreadCount(0);
  android::ProcessState::self()->startThreadPool();
  IPCThreadState::self()->joinThreadPool();
}

#define NUM 10000

void StartClient(char* name, int p) {
  printf("Starting Client\n");
  sp<IIpc> ipc = GetIPCBinder(name);
  struct timespec start_time;
  struct timespec end_time;
  for (int j = 2; j < (256 * 1024); j *= 2) {
    clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);
    for (int i = 0; i < NUM; i++) {
      ipc->ping(j);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &end_time);
    uint64_t diff = GetDiff(&start_time, &end_time);
    if (!p)
      printf("%d %lld\n", j, (diff / NUM) / 1000);
    fflush(stdout);
  }
}

}  // namespace

// How many extra pairs of client servers to run
// concurrently.
#define CONCURENT 0

int main(int argc, char** argv) {
  printf("Test Binder\n");

  // Only one process will have p==0,
  // the rest p==1. Use this so only one
  // process prints results.
  int p = 0;
  for (int l = 0; l < CONCURENT; l++) {
    if (fork() == 0)
      break;
    p = 1;
  }
  char name[16];
  sprintf(name, "IPC%d", getpid());
  printf("Name: %s\n", name);

  if (fork() == 0) {
    android::StartClient(name, p);
  } else {
    android::StartServer(name);
  }
  return 0;
}
