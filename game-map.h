#ifndef BRASS_GAME_MAP_
#define BRASS_GAME_MAP_

#include "game-common.h"

#include "link.h"
#include "industry.h"
#include "merchant.h"

#include "location.h"
#include "merchant-location.h"
#include "industry-location.h"

#include <set>
#include <vector>
#include <map>

class Game;

// Holds the components which form a map:
// Locations and Adjacencies, IndustryProxies and LinkProxies, as well as Merchants.
class Map {
  friend Game;
  using IndustryLocationIndustryPair = std::pair<IndustryLocation*, Industry*>;
  using MerchantLocationMerchantPair = std::pair<MerchantLocation*, Merchant*>;
  using IndustryIndustryLocationPair = std::pair<Industry*, IndustryLocation*>;
  using MerchantMerchantLocationPair = std::pair<Merchant*, MerchantLocation*>;
  using AdjacencyLinkPair = std::pair<Adjacency*, Link*>;
  using LinkAdjacencyPair = std::pair<Link*, Adjacency*>;

 public:
  Map() = default;
  ~Map();

  void Add(IndustryLocation* location);
  void Add(MerchantLocation* location);
  void Add(IndustryLocation* location, Industry* industry);
  void Add(MerchantLocation* location, Merchant* merchant);
  void Add(Adjacency* location);
  void Add(Adjacency* adjacency, Link* location);

  // Returns all industries.
  std::vector<Industry*> industries() const;

  // Returns all links.
  std::vector<Link*> links() const;

  // Returns all industries of a specific industry type.
  std::vector<Industry*> industries_of_type(IndustryType industry_type) const;

  // Lets all links be observed by `observer`.
  void RegisterObserverForLinks(LinkObserver* observer);

  // Lets all industries be observed by `observer`.
  void RegisterObserverForIndustries(IndustryObserver* observer, ResourceObserver* robserver);

 private:
  std::set<IndustryLocation*, ptr_less<Location> > industry_locations_;
  std::set<MerchantLocation*, ptr_less<Location> > merchant_locations_;
  std::set<Adjacency*, ptr_less<Adjacency> > adjacencies_;
  std::set<Industry*, ptr_less<Industry> > industries_;
  std::map<IndustryType, std::set<Industry*, ptr_less<Industry> > > industries_dict_;
  std::set<Merchant*, ptr_less<Merchant> > merchants_;
  std::set<Link*, ptr_less<Link> > links_;
};

#endif  // BRASS_GAME_MAP_
