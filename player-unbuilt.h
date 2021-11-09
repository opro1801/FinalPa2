#ifndef BRASS_PLAYER_UNBUILT_H_
#define BRASS_PLAYER_UNBUILT_H_

#include "link.h"
#include "industry.h"

#include <map>
#include <stack>

class Player;
class PlayerObserver;

// The PlayerUnbuilt class. It stores industries and links that are
// not yet built by the player who is associated to it.
class PlayerUnbuilt {
  friend PlayerObserver;

 public:
  using LinkStack = std::stack<Link*>;
  using IndustryStack = std::map<int, std::stack<Industry*> >;
  using LinkStackMap = std::map<LinkType, LinkStack>;
  using IndustryStackMap = std::map<IndustryType, IndustryStack>;

  PlayerUnbuilt();
  ~PlayerUnbuilt();

  // Returns a pointer to a link of `link_type`.
  Link* GetLink(LinkType link_type) const;

  // Adds an unbuilt link (tile).
  void PutLink(Link* link);

  // Removes and returns the pointer to a link of `link_type`.
  Link* PopLink(LinkType link_type);

  // Returns a pointer to the unused lowest-level industry of `industry_type`.
  Industry* GetLowestTechIndustry(IndustryType industry_type) const;

  // Adds an unbuilt industry (tile).
  void PutIndustry(Industry* industry);

  // Removes and returns the pointer to the industry of `industry_type`
  // and is of the lowest technology level.
  Industry* PopLowestTechIndustry(IndustryType industry_type);

  // Sets the player who is associated to this object.
  void set_player(Player* player);

 private:
  Player* player_ = nullptr;
  LinkStackMap links_;
  IndustryStackMap industries_;
};

#endif  // BRASS_PLAYER_UNBUILT_H_
