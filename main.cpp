#include "include/utils/utils.hpp"
#include "include/threading/thread_utils.hpp"
#include "include/utils/time.hpp"
#include "include/memory/MemoryPool.hpp"
#include "include/utils/LFQueue.hpp"
#include "include/utils/logging/Logger.hpp"


using namespace utils;

  int main(int, char **) {
  char c = 'd';
  int i = 3;
  unsigned long ul = 65;
  float f = 3.4;
  double d = 34.56;
  const char* s = "test C-string";
  std::string ss = "test string";
  logging::Logger logger("logging_example.log");
  logger.log("Logging a char:% an int:% and an unsigned:%\n", c, i, ul);
  logger.log("Logging a float:% and a double:%\n", f, d);
  logger.log("Logging a C-string:'%'\n", s);
  logger.log("Logging a string:'%'\n", ss);
  return 0;
  }