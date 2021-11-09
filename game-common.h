#ifndef BRASS_GAME_GAME_COMMON_H_
#define BRASS_GAME_GAME_COMMON_H_

#include <array>
#include <vector>
#include <map>
#include <set>
#include <string>

enum class ProceedFlag {
  kContinue,
  kRetry,
  kExit
};

enum class OkFlag {
  kPending,
  kCancel,
  kOk,
  kSkip,
  kExit,
};

enum class ActionType {
  kBuild,
  kNetwork,
  kDevelop,
  kSell,
  kLoan,
  kScout,
  kPass,
};

const std::array<ActionType, 7> kActionTypes { 
  ActionType::kBuild, 
  ActionType::kNetwork,
  ActionType::kDevelop,
  ActionType::kSell,
  ActionType::kLoan,
  ActionType::kScout,
  ActionType::kPass
};

enum class ResourceType {
  kCoal,
  kIron,
  kBeer,
  kUndefined,
};

enum class IndustryType {
  kCottonMill,
  kManufacturer,
  kPottery,
  kCoalMine,
  kIronWorks,
  kBrewery,
  kAny,
  kUndefined,
};

const std::array<IndustryType, 6> kIndustryTypes { 
  IndustryType::kCottonMill,
  IndustryType::kManufacturer,
  IndustryType::kPottery,
  IndustryType::kCoalMine,
  IndustryType::kIronWorks,
  IndustryType::kBrewery,
};

enum class MainIndustryType {
  kPrimaryIndustry,
  kSecondaryIndustry,
  kUndefined,
};

enum class LinkType {
  kCanal,
  kRailway,
  kUndefined,
};

enum class LocationType {
  kIndustryLocation,
  kMerchantLocation,
};

enum class Color {
  kRed,
  kGold,
  kTeal,
  kBlue,
  kViolet,
  kUndefined,
};

const char kCoal[] = "Coal";
const char kIron[] = "Iron";
const char kBeer[] = "Beer";

const char kCottonMill[] = "Cotton Mill";
const char kManufacturer[] = "Manufacturer";
const char kPottery[] = "Pottery";
const char kCoalMine[] = "Coal Mine";
const char kIronWorks[] = "Iron Works";
const char kBrewery[] = "Brewery";
const char kAny[] = "Any";

const char kCanal[] = "Canal";
const char kRailway[] = "Railway";

const char kUndefined[] = "Undefined";

const int kNumPricesToShow = 5;

inline int RequiredExpToLevelUp(int income_level) {
  if (income_level <= 0)
    return 1;
  if (income_level <= 10)
    return 2;
  if (income_level <= 20)
    return 3;
  if (income_level < 30)
    return 4;
  return 3;
}

// Data classes used by infrastructures

struct IndustryProperties {
  int tech_level = 0;  // the technology level of the industry
  int basic_price = 0;  // the lowest price a player has to pay for building the industry
  int coal_cost = 0;  // the no. of coal units needed to consume for building the industry
  int iron_cost = 0;  // the no. of iron units needed to consume for building the industry
  int beer_cost = 0;  // the no. of beer units needed to consume for selling the industry
  int output_units = 0; // the no. of resource units produced upon building the industry
  int exp_increase = 0;  // EXP increase for the player when 
  int vp = 0;  // the VP if the industry is eligible for VP calculation
  int vp_for_adj_link = 0;  // the VP for adjacent links
  bool discardable = true;  // if the industry can be discarded in a develop action

  IndustryProperties() = default;
  IndustryProperties(int tech_level, int basic_price, int coal_cost, int iron_cost,
                     int beer_cost, int output_units, int exp_increase,
                     int vp, int vp_for_adj_link, bool discardable = true)
      : tech_level(tech_level), basic_price(basic_price), coal_cost(coal_cost),
        iron_cost(iron_cost), beer_cost(beer_cost),
        output_units(output_units), exp_increase(exp_increase),
        vp(vp), vp_for_adj_link(vp_for_adj_link), discardable(discardable) {}
};

struct LinkProperties {
  int basic_price = 0; // the lowest price a player has to pay for building the link
  int coal_cost = 0; // the no. of coal units needed to consume for building the industry

  LinkProperties() = default;
  LinkProperties(int basic_price, int coal_cost)
      : basic_price(basic_price), coal_cost(coal_cost) {}
};

// For tracking gaming process

struct GameStat {
  int max_num_rounds;
  int max_num_turns;
  int max_num_cards;
  int max_num_actions;
  int curr_round;
  int curr_turn;
  int num_cards_left;
  int curr_action;

  GameStat(int max_num_rounds, int max_num_turns, int max_num_cards)
      : max_num_rounds(max_num_rounds), max_num_turns(max_num_turns), max_num_cards(max_num_cards) {}
};

// The below are used for UI Display

struct ResourceInfo {
  ResourceType resource_type;
  int industry_supply = 0;
  int market_supply = 0;
  bool market_full = false;
  std::array<int, kNumPricesToShow> selling_price {};
  std::array<int, kNumPricesToShow> buying_price {};

  ResourceInfo(ResourceType resource_type) : resource_type(resource_type) {}
};

struct LinkInfo {
  std::string link_type;
  std::string location_1;
  std::string location_2;
  LocationType location_1_type;
  LocationType location_2_type;
  Color location_1_color;
  Color location_2_color;
  std::string owner;
  bool location_1_connected_to_merchant = false;
  bool location_2_connected_to_merchant = false;
  int provisional_vp = 0;

  LinkInfo(std::string link_type, std::string location_1, std::string location_2,
           LocationType loc_1_type, LocationType loc_2_type,
           Color loc_1_color, Color loc_2_color)
      : link_type(link_type), location_1(location_1), location_2(location_2),
        location_1_type(loc_1_type), location_2_type(loc_2_type),
        location_1_color(loc_1_color), location_2_color(loc_2_color) {}

  void set_owner(std::string owner) {
    this->owner = owner;
  }
};

struct IndustryInfo {
  std::string industry_type;
  std::string location;
  Color location_color;
  int tech_level;
  int vp;
  int vp_for_link;
  int beer_cost;
  int available_units = 0;
  bool eligible_for_vp = false;
  std::string owner = "(unbuilt)";
  bool connected_to_merchant = false;
  bool demolished = false;

  IndustryInfo(std::string industry_type, std::string location, Color location_color,
               int tech_level, int vp, int vp_for_link, int beer_cost)
      : industry_type(industry_type), location(location), location_color(location_color),
        tech_level(tech_level),
        vp(vp), vp_for_link(vp_for_link), beer_cost(beer_cost) {}

  IndustryInfo(std::string industry_type, std::string location, Color location_color,
               int tech_level, int vp, int vp_for_link, int beer_cost,
               int available_units, bool eligible_for_vp, std::string owner)
      : industry_type(industry_type), location(location), location_color(location_color),
        tech_level(tech_level),
        vp(vp), vp_for_link(vp_for_link), beer_cost(beer_cost),
        available_units(available_units), eligible_for_vp(eligible_for_vp), owner(owner) {}
};

struct UnbuiltIndustryInfo {
  IndustryType industry_type;
  int tech_level;
  int vp;
  int vp_for_link;
  int exp_increase;
  int basic_price;
  int coal_cost;
  int iron_cost;
  int output_units;
  int beer_cost;
  int count;

  UnbuiltIndustryInfo(IndustryType industry_type, int tech_level, int vp, int vp_for_link, int exp_increase, int basic_price,
                      int coal_cost, int iron_cost, int output_units,
                      int beer_cost, int count = 1)
      : industry_type(industry_type), tech_level(tech_level), vp(vp), vp_for_link(vp_for_link), exp_increase(exp_increase), basic_price(basic_price),
        coal_cost(coal_cost), iron_cost(iron_cost), output_units(output_units),
        beer_cost(beer_cost), count(count) {}

  UnbuiltIndustryInfo(IndustryType industry_type, const IndustryProperties& properties, int count)
      : UnbuiltIndustryInfo(industry_type, properties.tech_level, properties.vp, properties.vp_for_adj_link, properties.exp_increase, properties.basic_price,
                            properties.coal_cost, properties.iron_cost, properties.output_units, properties.beer_cost, count) {}
};

struct UnbuiltLinkInfo {
  LinkType link_type;
  int basic_price;
  int coal_cost;
  int count;

  UnbuiltLinkInfo(LinkType link_type, int basic_price, int coal_cost, int count)
      : link_type(link_type), basic_price(basic_price), coal_cost(coal_cost),
        count(count) {}
  
  UnbuiltLinkInfo(LinkType link_type, const LinkProperties& properties, int count)
      : UnbuiltLinkInfo(link_type, properties.basic_price, properties.coal_cost, count) {}
};

struct ConsumptionInfo {
  int industry_units;
  int market_units;
  int market_price;
  std::vector<std::pair<IndustryInfo, int> > chosen_units;

  ConsumptionInfo(int industry_units, int market_units = 0, int market_price = 0)
      : industry_units(industry_units), market_units(market_units),
        market_price(market_price) {}
};

struct PlayerInfo {
  std::string name;
  int vp = 0;
  int income_level = 0;
  int exp = 0;
  int balance = 0;
  int provisional_vp = 0;
  std::vector<std::string> cards;
  std::set<std::string> network;
  std::set<std::pair<std::string, std::string> > adj_links;
  std::map<IndustryType, std::vector<UnbuiltIndustryInfo> > unbuilt_industry_info;
  std::vector<UnbuiltLinkInfo> unbuilt_link_info;
  
  PlayerInfo(std::string name) : name(name) {}
};

#endif  // BRASS_GAME_GAME_COMMON_H_
