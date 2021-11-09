#include "game.h"
#include "game-common.h"
#include "player-action.h"
#include "consumption-request.h"

#include "player-hand.h"
#include "player-unbuilt.h"
#include "player-network.h"

#include "link.h"
#include "industry.h"
#include "game-observer.h"

#include "config.h"

#include <algorithm>
#include <iostream>

Game::Game(Map* map, DrawDeck* card_deck, 
           Market* coal_market, Market* iron_market, Io& io)
    : map_(map), card_deck_(card_deck), 
      coal_market_(coal_market), iron_market_(iron_market), io_(io) {
  action_req_fty_ = new ActionRequirementFactory(map, coal_market, iron_market);
  resource_osvr_ = new ResourceObserver();
  coal_market_->RegisterObserver(resource_osvr_);
  iron_market_->RegisterObserver(resource_osvr_);
  link_osvr_ = new LinkObserver();
  industry_osvr_ = new IndustryObserver();
  map_->RegisterObserverForLinks(link_osvr_);
  map_->RegisterObserverForIndustries(industry_osvr_, resource_osvr_);
  player_osvr_ = new PlayerObserver();
  resource_osvr_->RegisterSubscriber(this);
  link_osvr_->RegisterSubscriber(this);
  industry_osvr_->RegisterSubscriber(this);
  player_osvr_->RegisterSubscriber(this);
}

Game::~Game() {
  delete map_;
  delete coal_market_;
  delete iron_market_;
  delete action_req_fty_;
  for (Player* player: players_) {
    delete player;
  }
  delete card_deck_;
  delete resource_osvr_;
  delete link_osvr_;
  delete industry_osvr_;
  delete player_osvr_;
}

void Game::UpdateResourceInfo(std::map<ResourceType, ResourceInfo> resource_info) {
  resource_info_ = resource_info;
}

void Game::UpdateLinkInfo(std::vector<LinkInfo> link_info) {
  link_info_ = link_info;
}

void Game::UpdateIndustryInfo(std::map<IndustryType, std::vector<IndustryInfo> > industry_info) {
  industry_info_ = industry_info;
}

void Game::UpdatePlayerInfo(std::map<Player*, PlayerInfo> player_info) {
  player_info_ = player_info;
  if (curr_player_ != nullptr) {
    curr_player_info_ = &player_info_.at(curr_player_);
  }
}

void Game::Run() {
  int num_rounds = 12 - players_.size();
  bool warn_exhausted = true;

  if (card_deck_->Shuffle()) {
    io_ << "The card deck is shuffled." << NEW_LINE;
  } else {
    io_ << "The card deck is not shuffled." << NEW_LINE;
  }

  for (Player* player : players_) {
    RefillHand(player);
  }

  GameStat game_stat(num_rounds, players_.size(), card_deck_->draw_deck_size());
  io_.set_game_stat(&game_stat);
  io_.set_resource_info(&resource_info_);
  io_.set_link_info(&link_info_);
  io_.set_industry_info(&industry_info_);

  for (game_stat.curr_round = 1; game_stat.curr_round <= game_stat.max_num_rounds; game_stat.curr_round++) {
    io_.ofstream << "[Game::Run()] round " << game_stat.curr_round << " of " << game_stat.max_num_rounds << std::endl;
    io_ << "Round " << game_stat.curr_round << NEW_LINE;
    int max_num_actions = (game_stat.curr_round > 1)? 2 : 1;
    game_stat.max_num_actions = max_num_actions;
    if (PlayRound(max_num_actions, game_stat, warn_exhausted) == ProceedFlag::kExit) {
      return;
    }
    if (AreCardsAllPlayed()) {
      io_ << "Playing cards are all played." << NEW_LINE;
      break;
    } else {
      ShufflePlayersForNextRound();
      for (const Player* player: players_) {
        io_ << player->name() << " spent $" << player->spending() << "." << NEW_LINE;
      }
      UpdateBalance();
    }
  }
  io_ << "End of the industry era." << NEW_LINE;
  WriteResult();
}

ProceedFlag Game::PlayRound(int max_num_actions, GameStat& game_stat, bool& warn_exhausted) {
  game_stat.curr_turn = 1;
  for (Player* player : players_) {
    curr_player_ = player;
    curr_player_info_ = &(player_info_.at(player));
    io_.set_player_info(&curr_player_info_);
    io_.ofstream << "[Game::PlayRound()]"
                 << " turn " << game_stat.curr_turn << " of " << game_stat.max_num_turns
                 << " | player = " << player->name()
                 << std::endl;
    if (player->num_cards_in_hand() > 0) {
      game_stat.curr_action = 1;
      while ((player->num_cards_in_hand() > 0) && (game_stat.curr_action <= max_num_actions)) {
        io_.ofstream << "[Game::PlayRound()] resource info" << NEW_LINE;
        LogResourceInfo();
        LogPlayerInfo();
        game_stat.num_cards_left = card_deck_->draw_deck_size();
        io_ << player->name() << ", perform an action." << NEW_LINE;
        io_.ofstream << "[Game::PlayRound()]"
                     << " action " << game_stat.curr_action << " of " << game_stat.max_num_actions << std::endl;
        io_.Reset();
        ProceedFlag flag = ProceedFlag::kContinue;
        bool successful = PerformAction(player, flag);
        if (successful){
          io_.ofstream << "[Game::PlayRound()] action completed" << std::endl;
          io_ << "Action completed." << NEW_LINE;
          io_.WaitForContinue();
          ++game_stat.curr_action;
        } else if (flag == ProceedFlag::kContinue) {
          io_ << "Try again." << NEW_LINE;
        } else {
          return flag;
        }
      }
      io_.ofstream << "[Game::PlayRound()] all actions completed" << std::endl;
      LogResourceInfo();
      LogPlayerInfo();
      CollectDiscardedWildCards(player);
      int num_cards_refilled = RefillHand(player);
      if (num_cards_refilled > 0) {
        io_ << player->name() << " refilled " << num_cards_refilled << " cards." << NEW_LINE;
      }
      if (warn_exhausted && card_deck_->IsExhausted()) {
        io_ << "The card deck is exhausted. No more refills from now on." << NEW_LINE;
        warn_exhausted = false;
      }
    } else {
      io_ << "Skipping " << player->name() << " because you have no more cards left." << NEW_LINE;
      io_.WaitForContinue();
    }
    ++game_stat.curr_turn;
  }
  return ProceedFlag::kContinue;
}

bool Game::AreCardsAllPlayed() const {
  int num_cards = 0;
  for (Player* player: players_) {
    num_cards += player->num_cards_in_hand();
  }
  return num_cards == 0;
}

int Game::RefillHand(Player* player) {
  int num_cards_refilled = 0;
  while (!card_deck_->IsExhausted() && player->DrawCard(card_deck_->Back())) {
    card_deck_->Pop();
    ++num_cards_refilled;
  }
  return num_cards_refilled;
}

void Game::CollectDiscardedWildCards(Player* player) {
  PlayerHand& hand = player->hand();
  while (hand.HasDiscardedWildCard()) {
    card_deck_->Put(hand.PopDiscardedWildCard());
  }
}

void Game::AddPlayer(const std::string& name, PlayerUnbuilt* unbuilt) {
  Player* player = new Player(name, new PlayerHand, unbuilt, new PlayerNetwork);
  player->set_observer(player_osvr_);
  players_.push_back(player);
}

bool Game::PerformAction(Player* player, ProceedFlag& flag) {
  Action* action;
  ActionType action_type;
  OkFlag status = io_.ChooseAction(action_type);
  if (status == OkFlag::kExit) {
    flag = ProceedFlag::kExit;
    return false;
  }
  switch (action_type) {
    case ActionType::kBuild:
      action = new BuildAction(io_, action_req_fty_, map_, coal_market_, iron_market_);
      break;
    case ActionType::kNetwork:
      action = new NetworkAction(io_, action_req_fty_, map_);
      break;
    case ActionType::kDevelop:
      action = new DevelopAction(io_, action_req_fty_, map_);
      break;
    case ActionType::kSell:
      action = new SellAction(io_, action_req_fty_, map_);
      break;
    case ActionType::kLoan:
      action = new LoanAction(io_);
      break;
    case ActionType::kScout:
      action = new ScoutAction(io_, card_deck_);
      break;
    case ActionType::kPass:
      action = new PassAction(io_);
      break;
    default:
      return false;
  }
  io_.ofstream << "[Game::PerformAction()] "
               << "action = " << ToString(action_type)
               << std::endl;
  bool successful = action->PerformAction(player);
  delete action;
  flag = ProceedFlag::kContinue;
  return successful;
}

void Game::ShufflePlayersForNextRound() {
  // resolve ties by stable sort
  std::stable_sort(
    players_.begin(), players_.end(), [](Player* a, Player* b) {
      return a->spending() < b->spending();
    });
}

void Game::UpdateBalance() {
  for (Player* player : players_) {
    curr_player_ = player;
    curr_player_info_ = &(player_info_.at(player));
    io_.set_player_info(&curr_player_info_);
    player->ResetSpending();
    int income_level = player->income_level();
    bool successful = (income_level >= 0)? player->Deposit(income_level) 
                                  : player->Withdraw(-income_level);
    if (!successful) {
      int surplus = Liquidate(player->network(), -income_level - player->balance());
      if (surplus > 0) {
        player->Deposit(surplus);
      } else {
        player->DecreaseVp(-surplus);
      }
    }
  }
}

int Game::Liquidate(PlayerNetwork& network, int shortfall) {
  PlayerNetwork::IndustryVector vec;
  std::vector<IndustryInfo> info;
  std::vector<int> liquidation_value, chosen_indices;
  PlayerNetwork::IndustryVectorMap ivm = network.industries_;
  for (PlayerNetwork::IndustryVectorMap::reference pair : ivm) {
    vec.insert(vec.end(), pair.second.begin(), pair.second.end());
  }
  for (Industry* industry : vec) {
    IndustryProperties props = industry->properties();
    info.emplace_back(
      ToString(industry->industry_type()),
      industry->location()->name(),
      industry->location()->color(),
      props.tech_level,
      props.vp,
      props.vp_for_adj_link,
      props.beer_cost,
      industry->available_units(),
      industry->IsEligibleForVp(),
      ""
    );
  }
  io_.ChooseIndustriesToLiquidate(info, shortfall, liquidation_value, chosen_indices);
  int total_value = 0;
  for (int index : chosen_indices) {
    vec.at(index)->Demolish();
    total_value += liquidation_value.at(index);
  }
  return total_value - shortfall;
}

void Game::WriteResult() {
  for (Player* player : players_) {
    player->ScoreVp();
  }
  std::stable_sort(players_.begin(), players_.end(),
                   [](Player* a, Player* b) { return a->vp() > b->vp(); });

  io_ << "Result:" << NEW_LINE;

  int i = 0;
  for (Player* player : players_) {
    io_ << ++i << ": " << player->name() << ", "
                << "VP: " << player->vp() << NEW_LINE;
  }
}

void Game::LogResourceInfo() const {
  for (std::map<ResourceType, ResourceInfo>::const_reference p : resource_info_) {
    io_.ofstream << "[Game::LogResourceInfo()] resource info" << NEW_LINE
                 << "* resource type = " << ToString(p.first)
                 << " | industry supply = " << p.second.industry_supply
                 << " | market supply = " << p.second.market_supply
                 << " | market selling prices = " << Join(p.second.selling_price)
                 << " | market buying prices = " << Join(p.second.buying_price) << NEW_LINE;
  }
  io_.ofstream.flush();
}

void Game::LogPlayerInfo() const {
  io_.ofstream << "[Game::LogPlayerInfo()] player info" << NEW_LINE
               << "income level = " << curr_player_info_->income_level
               << " | exp = " << curr_player_info_->exp
               << " | perm. vp = " << curr_player_info_->vp
               << " | prov. vp = " << curr_player_info_->provisional_vp
               << " | balance = " << curr_player_info_->balance
               << " | network = " << Join(curr_player_info_->network)
               << " | adj links = " << Join(curr_player_info_->adj_links)
               << std::endl;
}
