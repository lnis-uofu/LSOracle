#include "kahypar_config.hpp"
#include <fstream>

std::string make_temp_config() {
  std::string config_direct = std::tmpnam(nullptr);
  std::ofstream temp(config_direct);
  temp << KAHYPAR_DEFAULT_CONFIG;
  temp.close();
  return config_direct;
}
