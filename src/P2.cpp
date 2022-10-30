#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <data.h>  //structure for shared memory
#include <unistd.h>
#include <signal.h>
using namespace std::chrono_literals;

volatile sig_atomic_t stop;
bool thread_dead = true;

void inthand(int signum)
{
  stop = 1;
}

void increment_thread(struct data_in_shm* shared_struct)
{
  printf("Incrementing Thread Started");
    thread_dead = false;

  while (!stop)
  {
    std::unique_lock<std::mutex> lk(shared_struct->mutex);
    shared_struct->cv.wait(lk);
    shared_struct->stuff++;
    lk.unlock();
    printf("Incremented Variable, sleeping now.\n");
  }
    thread_dead = true;

}

int main(void)
{
  signal(SIGINT, inthand);
  printf("Starting P2 Incremented thread and attaching to shared memory\n");
  int key = 65;
  int shmid = shmget(key, sizeof(struct data_in_shm), 0666 | IPC_CREAT);
  if (shmid == -1)
  {
    printf("SHMID generation failed, exiting with error code: %i\n", errno);
    return errno;
  }
  struct data_in_shm* shared_struct = (struct data_in_shm*)shmat(shmid, (void*)0, 0);
  if (shared_struct == (void*)-1)
  {
    printf("SHMAT generation failed, exiting with error code: %i\n", errno);
    return errno;
  }
  printf("Shared memory attached\n");
  std::thread{ &increment_thread, shared_struct }.detach(); 
  while (!stop || !thread_dead)
  {
  };

  if (shmctl(shmid, IPC_RMID, NULL) == -1)
  {
    printf("Marking shared memory for destruction failed: error %i\n", errno);
  }
  if (shmdt(shared_struct) == -1)
  {
    printf("Detaching from shared memory failed: error%i\n", errno);
  }
}
