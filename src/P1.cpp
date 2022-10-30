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

void wake_thread(struct data_in_shm* shared_struct)
{
  printf("Waker thread launched \n");
  thread_dead = false;
  while (!stop)
  {
    std::this_thread::sleep_for(1000ms);
    {
      std::scoped_lock<std::mutex> lk(shared_struct->mutex);
      printf("Shared Memory Variable is: %i\n", shared_struct->stuff);
      shared_struct->cv.notify_one();
    }
  }
  thread_dead = true;
  printf("Waker thread destroyed \n");
}

int main(void)
{
  printf("Starting P1, attaching to shm and starting wake thread\n");
  signal(SIGINT, inthand);
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
  printf("Shared memory attached, launching waker thread\n");
  std::thread{ &wake_thread, shared_struct }.detach();  
  while (!stop || !thread_dead)
  {
  };
  printf("Exiting, and destroying shared memory\n");
  if (shmctl(shmid, IPC_RMID, NULL) == -1)
  {
    printf("Marking shared memory for destruction failed: error %i\n", errno);
  }
  if (shmdt(shared_struct) == -1)
  {
    printf("Detaching from shared memory failed: error%i\n", errno);
  }
}
