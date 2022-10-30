struct data_in_shm{
  int stuff = 0;
  std::mutex mutex;
  std::condition_variable cv;
  pthread_mutex_t pthread_mutex;
  pthread_cond_t pthread_cv;
};
