#ifndef BRASS_INFRASTRUCTURE_H_
#define BRASS_INFRASTRUCTURE_H_

#include <string>

const char kEmptyPlayerName[] = "None";

class Player;

class Infrastructure {
 public:
  virtual ~Infrastructure();

  virtual Player* player() const = 0;
  virtual void set_player(Player* player) = 0;
  virtual std::string player_name() const = 0;

 protected:
  // Returns true if the infrastructure has been built.
  virtual bool IsBuilt() const = 0;

  // Returns the VP this infrastructure is worth for scoring purposes.
  virtual int vp() const = 0;
};

#endif  // BRASS_INFRASTRUCTURE_H_
