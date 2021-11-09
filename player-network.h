#ifndef BRASS_PLAYER_NETWORK_H_
#define BRASS_PLAYER_NETWORK_H_

#include "location.h"
#include "industry.h"
#include "link.h"

#include <vector>
#include <stack>

class Game;

// The PlayerNetwork class. It contains the industries and links that the player has built.
// Moreover, it defines a method which specifies the network of the player.
class PlayerNetwork {
  friend Game;

  using LocationPtrSet = std::set<Location*, ptr_less<Location> >;
  using LinkPtrSet = std::set<Link*, ptr_less<Link> >;

  using LinkVector = std::vector<Link*>;
  using LinkSet = std::vector<Link*>;
  using IndustryVector = std::vector<Industry*>;
  using LinkVectorMap = std::map<LinkType, std::vector<Link*> >;
  using IndustryVectorMap = std::map<IndustryType, std::set<Industry*, ptr_less<Industry> > >;
  using IndustrySetMap = std::map<IndustryType, std::set<Industry*, ptr_less<Industry> > >;

 public:
  PlayerNetwork() = default;
  ~PlayerNetwork();

  // Returns true if the player has built either a Link or an Industry.
  bool HasEstablished() const;

  // Returns true if the player has built on the given `industry_location`.
  bool HasBuiltIndustryOn(const Location* industry_location) const;

  // Returns true if the player's network covers the given `industry_location`.
  // A player's network consists of all industry locations that his built industries reside on,
  // and all industry locations accessible through his built links.
  bool IsCovering(const Location* industry_location) const;

  // Returns true if the player's network is adjacent to the given `link`.
  // This means the given link is next to any industry location that the player has
  // an industry built on, and the link is not already a part of the player's network.
  bool IsAdjacentTo(const Link* link) const;

  // Adds an infrastructure to the network.
  void AddToNetwork(Infrastructure* infrastructure);

  // Discards the given list of industries.
  void Discard(const std::set<Industry*>& industries);

  // Returns the total colleteral value, which is half of the costs of the industries built.
  int TotalColleteralValue() const;

  // Computes and returns the final VP that the network is worth.
  // Usually used at the end of the game.
  int ScoreVp();
  
  // Removes an industry from the network.
  void Remove(Industry* industry);

  // Returns the network of the player. A player's network consists of the locations that
  // are reachable through the player's built industries and links.
  // If `adj_links` is not a nullptr, adds all links that are adjacent to the player's network
  // to `adj_links`.
  LocationPtrSet network(LinkPtrSet* adj_links = nullptr) const;

  // Returns the industries of `industry_type`.
  IndustryVector industries_of_type(IndustryType industry_type) const;
  
  // Returns the industries of `main_industry_type`.
  IndustryVector industries_of_type(MainIndustryType main_industry_type) const;

  // Returns the number of industries built by the player that are also not being overbuilt.
  int num_built_industries() const;

  // Returns the number of links built by the player that are also not being overbuilt.
  int num_built_links() const;

 private:
  LinkVectorMap links_;
  IndustrySetMap industries_;
  IndustryVector removed_industries_;
};

#endif  // BRASS_PLAYER_NETWORK_H_
