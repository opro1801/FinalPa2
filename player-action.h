#ifndef BRASS_PLAYER_ACTION_H_
#define BRASS_PLAYER_ACTION_H_

#include "game-io.h"
#include "player.h"
#include "consumption-request.h"
#include "draw-deck.h"

#include <vector>

const int kLoanAmount = 30;
const int kIncomeLevelDecreaseForLoan = 3;

class Action {
 public:
  Action(Io& io, ActionRequirementFactory* fty, Map* map);
  virtual ~Action();

  virtual bool PerformAction(Player* player) = 0;

 protected:
  virtual bool CanPerformAction(Player* player) = 0;

  bool ChooseCard(Player* player, int& index);
  bool ChooseCards(Player* player, int num_cards, std::vector<int>& indices);

  Io& io_;
  ActionRequirementFactory* fty_;
  Map* map_;
};

// Overbuilds an industry. The required cost and resources must be settleable.
class BuildAction : public Action {
 public:
  BuildAction(Io& io, ActionRequirementFactory* fty, Map* map, Market* coal_market, Market* iron_market);
 
 private:
  bool CanPerformAction(Player* player);
  bool PerformAction(Player* player);
  ActionRequirement* CreateActionRequirement(Player* player, Industry* industry, IndustryProperties properties);
  
  bool ChooseCard(Player* player, int& index, Card*& card);
  bool ChooseIndustryToBuild(std::vector<Industry*> shortlist,
                             std::vector<ActionRequirement*> shortlist_reqs,
                             Industry*& to_be_overbuilt,
                             ActionRequirement*& req);

  Market* coal_market_;
  Market* iron_market_;
};

// Places an `LinkTile` on the game board to bulid a link, returns true if
// the requested link is built. This is similar to the Build action.
class NetworkAction : public Action {
 public:
  NetworkAction(Io& io, ActionRequirementFactory* fty, Map* map);
  ~NetworkAction();
 
 private:
  bool CanPerformAction(Player* player);
  bool PerformAction(Player* player);
  ActionRequirement* CreateActionRequirement(Player* player, Link* link, LinkProperties properties, bool second_link=false);

  OkFlag ChooseLinkToBuild(std::vector<Link*> shortlist,
                             std::vector<ActionRequirement*> shortlist_reqs,
                             Link*& to_be_overbuilt,
                             ActionRequirement*& req);

  int num_links_built_ = 0;
  std::vector<Link*> shortlist_;
  std::vector<ActionRequirement*> shortlist_reqs_;
};

// Skips the build process of lower-level indusrty tiles by discarding them.
class DevelopAction : public Action {
 public:
  DevelopAction(Io& io, ActionRequirementFactory* fty, Map* map);
  ~DevelopAction();
 
 private:
  bool CanPerformAction(Player* player);
  bool PerformAction(Player* player);
  ActionRequirement* CreateActionRequirement(Player* player, int num_discards = 1);

  OkFlag ChooseIndustryToDevelop(std::vector<Industry*> shortlist,
                                   IndustryType& industry_type,
                                   int step, int max_num_steps);

  int num_dev_ = 0;
  std::vector<ActionRequirement*> reqs_;
};

// Sell goods from the player's cotton mills, manufacturers, or pottery tiles
// and consume beer if needed. No limit is placed on the number of industries
// to sell goods from, as long as the player can consume the required number
// of beers.
class SellAction : public Action {
 public:
  SellAction(Io& io, ActionRequirementFactory* fty, Map* map);
  ~SellAction();
 
 private:
  bool CanPerformAction(Player* player);
  bool PerformAction(Player* player);
  ActionRequirement* CreateActionRequirement(Player* player, Industry* industry);

  OkFlag ChooseIndustryToSell(std::vector<Industry*> shortlist,
                                std::vector<ActionRequirement*> shortlist_reqs,
                                Industry*& to_be_sold,
                                ActionRequirement*& req);

  int num_sales_ = 0;
  std::vector<Industry*> shortlist_;
  std::vector<ActionRequirement*> shortlist_reqs_;
};

// Take 30 dollars from the bank, and lower the income by 3 major levels.
class LoanAction : public Action {
 public:
  LoanAction(Io& io);
 
 private:
  bool CanPerformAction(Player* player);
  bool PerformAction(Player* player);
};

// Discard 3 cards, and draw 1 wild industry card and 1 wild location card.
class ScoutAction : public Action {
 public:
  ScoutAction(Io& io, DrawDeck* draw_deck_);
 
 private:
  bool CanPerformAction(Player* player);
  bool PerformAction(Player* player);

  DrawDeck* draw_deck_;
};

// Just discard a card and pass.
class PassAction : public Action {
 public:
  PassAction(Io& io);
 
 private:
  bool CanPerformAction(Player* player);
  bool PerformAction(Player* player);
};

#endif  // BRASS_PLAYER_ACTION_H_
