#ifndef BRASS_GAME_GAME_IO_H_
#define BRASS_GAME_GAME_IO_H_

#if defined __linux__ || defined __APPLE__
  #define NEW_LINE '\n'
#endif

#if defined __MINGW32__ || defined __MINGW64__
  #define NEW_LINE "\n"
#endif

#include "game-common.h"
#include "util-common.h"

#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

const int kUnchosen = -1;

enum class ActionStatus {
  kChooseAction,
  kChooseCard,
  kPerformAction,
  kConsumeResources,
  kCompletedAction,
  kExitProgram
};

struct OverbuiltIndustryInfo {
  IndustryType industry_type;
  std::string location;
  int tech_level;
  int vp;
  int vp_for_link;
  int available_units;
  int beer_cost;
  bool eligible_for_vp;
  std::string owner;
  int total_price;

  OverbuiltIndustryInfo(IndustryType industry_type, std::string location, int tech_level, int vp, int vp_for_link,
                       int available_units, int beer_cost, bool flipped, std::string owner,
                       int basic_price)
      : industry_type(industry_type), location(location), tech_level(tech_level), vp(vp), vp_for_link(vp_for_link),
        available_units(available_units), beer_cost(beer_cost), eligible_for_vp(flipped), owner(owner),
        total_price(basic_price) {}

  std::ostream& operator<<(std::ostream& os) const {
    os << ToString(industry_type) << "@" << location
       << " | owner = " << owner
       << " | tech level = " << tech_level
       << " | vp = " << vp
       << " | vp for link = " << vp_for_link
       << " | inventory = " << available_units
       << " | beer cost = " << beer_cost
       << " | eligible for vp = " << eligible_for_vp
       << " | total $ = " << total_price;
    return os;
  }
};

inline std::ostream& operator<<(std::ostream& os, const OverbuiltIndustryInfo& info) {
  return info.operator<<(os);
}

struct OverbuiltLinkInfo {
  std::string link_type;
  std::string location_1;
  std::string location_2;
  int total_price;

  OverbuiltLinkInfo(std::string link_type, std::string location_1, std::string location_2,
                    int total_price)
      : link_type(link_type), location_1(location_1), location_2(location_2),
        total_price(total_price) {}

  std::ostream& operator<<(std::ostream& os) const {
    os << link_type << "@" << location_1 << "<->" << location_2
       << " | total $ = " << total_price;
    return os;
  }
};

inline std::ostream& operator<<(std::ostream& os, const OverbuiltLinkInfo& info) {
  return info.operator<<(os);
}

inline std::ostream& operator<<(std::ostream& os, const IndustryProperties& info) {
  os << "tech level = " << info.tech_level
     << " | basic price = " << info.basic_price
     << " | coal cost = " << info.coal_cost
     << " | iron cost = " << info.iron_cost
     << " | beer cost = " << info.beer_cost
     << " | output units = " << info.output_units
     << " | +exp = " << info.exp_increase
     << " | vp = " << info.vp
     << " | vp for link = " << info.vp_for_adj_link
     << " | discardable = " << info.discardable;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const LinkProperties& info) {
  os << " | basic price = " << info.basic_price
     << " | coal cost = " << info.coal_cost;
  return os;
}

class Io {
  template <typename T>
  friend Io& operator<<(Io& io, const T& a);

 public:
  Io(const std::string& log_file_name);

  virtual ~Io();
  
  virtual void WaitForContinue() = 0;
  virtual void PushMessage(const std::string& msg) = 0;

  virtual void Reset() = 0;
  virtual OkFlag ChooseAction(ActionType& action_type) = 0;
  virtual bool ChooseCard(int& index) = 0;
  virtual bool ChooseCards(std::vector<int>& indices,
                           int num_cards = 1) = 0;
  virtual bool ChooseIndustryToBuild(const std::map<IndustryType, std::vector<OverbuiltIndustryInfo> >& industry_info,
                                     IndustryType& industry_type, int& index) = 0;
  virtual OkFlag ChooseLinkToBuild(const std::vector<OverbuiltLinkInfo>& link_info, int& index,
                                     bool allow_cancel, bool allow_skip) = 0;
  virtual OkFlag ChooseIndustryToSell(const std::vector<IndustryInfo>& industry_info, int& index,
                                        bool allow_cancel, bool allow_skip) = 0;
  virtual OkFlag ChooseIndustryToDevelop(int step, int max_num_steps,
                                           const std::vector<UnbuiltIndustryInfo>& industry_info,
                                           int& index, bool allow_cancel, bool allow_skip) = 0;
  virtual bool ChooseIndustriesToConsume(std::map<ResourceType, ConsumptionInfo>& consumption_info) = 0;
  virtual void ChooseIndustriesToLiquidate(const std::vector<IndustryInfo>& industry_info,
                                           int shortfall, const std::vector<int>& liquidation_value,
                                           std::vector<int>& chosen_indices) = 0;

  std::ofstream ofstream;

  void set_game_stat(const GameStat* game_stat);
  void set_resource_info(const std::map<ResourceType, ResourceInfo>* resource_info);
  void set_link_info(const std::vector<LinkInfo>* link_info);
  void set_industry_info(const std::map<IndustryType, std::vector<IndustryInfo> >* industry_info);
  void set_player_info(const PlayerInfo** player_info);

 protected:
  bool IsPlayerInfoReady() const;

  std::vector<std::string> action_list_;

  // game stats
  const GameStat* game_stat_ = nullptr;
  const std::map<ResourceType, ResourceInfo>* resource_info_;
  const std::vector<LinkInfo>* link_info_;
  const std::map<IndustryType, std::vector<IndustryInfo> >* industry_info_;
  const PlayerInfo** player_info_ = nullptr;
  OkFlag ok_status_ = OkFlag::kCancel;
  ActionStatus action_status_ = ActionStatus::kChooseAction;
  ActionType action_type_ = ActionType::kBuild;
  bool allow_skip_ = false;
  bool allow_cancel_ = true;

 private:
  std::stringstream sstream;
  std::stringstream line_sstream;
};

template <typename T>
Io& operator<<(Io& io, const T& a) {
  std::stringstream& ss = io.sstream;
  ss << a;
  ss.seekg(-1, std::ios::end);
  char last = ss.get();
  if (last == '\n') {
    ss.seekg(0, std::ios::beg);
    for (std::string line; std::getline(io.sstream, line, '\n'); ) {
      io.PushMessage(io.sstream.str());
    }
    io.sstream.str("");
    io.sstream.clear();
    io.sstream.seekg(0, std::ios::beg);
  } else {
    ss.unget();
    ss.seekg(0, std::ios::beg);
  }
  return io;
}

inline void Io::set_game_stat(const GameStat* game_stat) {
  game_stat_ = game_stat;
}

inline void Io::set_resource_info(const std::map<ResourceType, ResourceInfo>* resource_info) {
  resource_info_ = resource_info;
}

inline void Io::set_link_info(const std::vector<LinkInfo>* link_info) {
  link_info_ = link_info;
}

inline void Io::set_industry_info(const std::map<IndustryType, std::vector<IndustryInfo> >* industry_info) {
  industry_info_ = industry_info;
}

inline void Io::set_player_info(const PlayerInfo** player_info) {
  player_info_ = player_info;
}

inline bool Io::IsPlayerInfoReady() const {
  return !(player_info_ == nullptr || *player_info_ == nullptr);
}

#endif  // BRASS_GAME_GAME_IO_H_
