struct data_in_shm{
  int stuff = 0;
  std::mutex mutex;
  std::condition_variable cv;
};
