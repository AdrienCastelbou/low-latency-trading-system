#include "include/common/common.hpp"
#include "include/common/time/time.hpp"
#include "include/common/data_structures/LFQueue.hpp"
#include "include/common/logging/Logger.hpp"


using namespace common;

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