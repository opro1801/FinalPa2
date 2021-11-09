#include "player.h"

#include "game-common.h"
#include "util-common.h"

#include "player-hand.h"
#include "player-unbuilt.h"
#include "player-network.h"

#include <vector>
#include <map>
#include <set>
#include <algorithm>

Player::Player(const std::string& name,
               PlayerHand* hand,
               PlayerUnbuilt* unbuilt, PlayerNetwork* network)
    : name_(name), hand_(hand), unbuilt_(unbuilt), network_(network) {
  unbuilt_->set_player(this);
}

Player::~Player() {
  delete hand_;
  delete network_;
  delete unbuilt_;
}

void Player::DecreaseIncomeLevels(int num_levels) {
  income_level_ = max(kMinIncomeLevel, income_level_ - num_levels);
  exp_ = RequiredExpToLevelUp(income_level_) - 1;
  Notify();
}

void Player::IncreaseExp(int points) {
  int req_exp = RequiredExpToLevelUp(income_level_);
  while ((points > 0) && (income_level_ < kMaxIncomeLevel)) {
    --points;
    if (req_exp == exp_ + 1) {
      ++income_level_;
      exp_ = 0;
      req_exp = RequiredExpToLevelUp(income_level_);
    } else {
      ++exp_;
    }
  }
  Notify();
}

bool Player::HasWildCardInHand() const {
  return hand_->HasWildCard();
}

void Player::DiscardCard(int index) {
  hand_->Discard(index);
  Notify();
}

void Player::DiscardCard(std::vector<int>& indices) {
  hand_->Discard(indices);
  Notify();
}

bool Player::DrawCard(Card* card) {
  bool successful = hand_->DrawCard(card);
  Notify();
  return successful;
}

void Player::ScoreVp() {
  vp_ += network_->ScoreVp();
  Notify();
}

int Player::num_cards_in_hand() const {
  return hand_->num_cards();
}

int Player::num_built_industries() const {
  return network_->num_built_industries();
}

PlayerHand& Player::hand() const {
  return *hand_;
}

PlayerNetwork& Player::network() const {
  return *network_;
}

PlayerUnbuilt& Player::unbuilt() const {
  return *unbuilt_;
}

int Player::provisional_vp() const {
  return vp_ + network_->ScoreVp();
}

void Player::Notify() {
  observer_->UpdatePlayer(this);
}

PlayerObserver::PlayerObserver() {}
PlayerObserver::~PlayerObserver() {}

void PlayerObserver::UpdatePlayer(Player* player) {
  using LocationPtrSet = std::set<Location*, ptr_less<Location> >;
  using LinkPtrSet = std::set<Link*, ptr_less<Link> >;

  if (!Contains(player_info_, player)) {
    player_info_.emplace(
      player, 
      PlayerInfo {
        player->name()
      }
    );
  }
  PlayerInfo& info = player_info_.at(player);
  info.vp = player->vp();
  info.income_level = player->income_level();
  info.exp = player->exp();
  info.balance = player->balance();
  info.cards.clear();
  std::map<std::string, int> count;
  for (Card* card : player->hand().cards()) {
    std::string name = card->name();
    if (!Contains(count, name)) {
      count[name] = 0;
    } else {
      ++count.at(name);
      name += (" (" + std::to_string(count.at(name) + 1) + ")");
    }
    info.cards.push_back(name);
  }
  info.unbuilt_link_info.clear();
  const PlayerUnbuilt::LinkStackMap& links = player->unbuilt().links_;
  for (PlayerUnbuilt::LinkStackMap::const_reference pair : links) {
    if (!pair.second.empty()) {
      info.unbuilt_link_info.emplace_back(pair.first, pair.second.top()->properties(), pair.second.size());
    }
  }
  info.unbuilt_industry_info.clear();
  const PlayerUnbuilt::IndustryStackMap& industries = player->unbuilt().industries_;
  for (PlayerUnbuilt::IndustryStackMap::const_reference pair : industries) {
    std::vector<UnbuiltIndustryInfo>& vec = info.unbuilt_industry_info[pair.first];
    for (PlayerUnbuilt::IndustryStack::const_reference stack : pair.second) {
      if (!stack.second.empty()) {
        Industry* industry = stack.second.top();
        vec.emplace_back(pair.first, industry->properties(), stack.second.size());
      }
    }
  }
  LinkPtrSet adj_links;
  LocationPtrSet network = player->network().network(&adj_links);
  std::set<std::string>& network_str = info.network;
  network_str.clear();
  std::set<std::pair<std::string, std::string> >& adj_links_str = info.adj_links;
  adj_links_str.clear();
  for (Location* loc : network) {
    network_str.insert(loc->name());
    for (Link* link : adj_links) {
      Location * const * ends = link->adjacency()->ends();
      adj_links_str.emplace(ends[0]->name(), ends[1]->name());
    }
  }
  info.provisional_vp = player->provisional_vp();
  Notify();
}

void PlayerObserver::Notify() {
  for (PlayerSubscriber* subscriber : subscribers_) {
    subscriber->UpdatePlayerInfo(player_info_);
  }
}

PlayerSubscriber::~PlayerSubscriber() {}

void PlayerObserver::RegisterSubscriber(PlayerSubscriber* subscriber) {
  subscribers_.push_back(subscriber);
  Notify();
}

void Player::RemoveFromNetwork(Industry* industry) {
  network_->Remove(industry);
  Notify();
}
