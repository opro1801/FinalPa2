#include "game-io.h"

#include "util-common.h"

Io::Io(const std::string& log_file_name) {
  ofstream.open(log_file_name);
  ofstream << std::boolalpha;
  for (ActionType action_type : kActionTypes) {
    action_list_.push_back(ToString(action_type));
  }
}

Io::~Io() {
  ofstream.close();
}
