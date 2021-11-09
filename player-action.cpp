#include "player-action.h"

#include "game-io.h"
#include "consumption-request.h"

#include "player-hand.h"
#include "player-network.h"
#include "player-unbuilt.h"

#include "game-common.h"
#include "util-common.h"

#include <map>
#include <algorithm>

Action::Action(Io& io, ActionRequirementFactory* fty, Map* map)
    : io_(io), fty_(fty), map_(map) {}

Action::~Action() {}

bool Action::ChooseCard(Player* player, int& index) {
  io_.ofstream << "[Action::ChooseCard()] player card list" << NEW_LINE;
  for (const Card* card : player->hand_->cards()) {
    io_.ofstream << "* " << card->name()
                 << " | is wild card = " << card->IsWildCard()
                 << NEW_LINE;
  }
  io_.ofstream.flush();
  if (io_.ChooseCard(index)) {
    io_.ofstream << "[Action::ChooseCard()] chosen card" << NEW_LINE;
    io_.ofstream << ">> card = " << player->hand_->at(index)->name() << " @ index = " << index << std::endl;
    return true;
  }
  return false;
}

bool Action::ChooseCards(Player* player, int num_cards, std::vector<int>& indices) {
  io_.ofstream << "[Action::ChooseCards()] player card list" << NEW_LINE;
  for (const Card* card : player->hand().cards()) {
    io_.ofstream << "* " << card->name()
                 << " | is wild card = " << card->IsWildCard()
                 << NEW_LINE;
  }
  io_.ofstream.flush();
  if (io_.ChooseCards(indices, num_cards)) {
    io_.ofstream << "[Action::ChooseCards()] chosen cards" << NEW_LINE;
    for (int index : indices) {
      io_.ofstream << ">> card = " << player->hand().at(index)->name() << " @ index = " << index << std::endl;
    }
    return true;
  }
  return false;
}

BuildAction::BuildAction(Io& io, ActionRequirementFactory* fty, Map* map,
                         Market* coal_market, Market* iron_market)
    : Action(io, fty, map), coal_market_(coal_market), iron_market_(iron_market) {}

bool BuildAction::CanPerformAction(Player* player) {
  // it may be possible to pre-check things... but we don't do it here
  // there are quite a number of possibilities from the product of
  // card x industries... and computing all combinations of those might freeze
  // our single-threaded app for too long...
  static_cast<void>(player);
  return true;
}

bool BuildAction::PerformAction(Player* player) {
  bool built = false;
  int index;
  if (CanPerformAction(player)) {
    Card* card;
    if (ChooseCard(player, index, card)) {
      // get the full list of industries, then do a preliminary filtering
      // by considering which card the player plays
      std::vector<Industry*> candidates = map_->industries();
      io_.ofstream << "[BuildAction::PerformAction()] filter overbuilt targets (before considering consumption)" << NEW_LINE;
      std::map<Industry*, bool> will_remove;
      for (Industry* industry : candidates) {
        Industry* to_build = player->unbuilt().GetLowestTechIndustry(industry->industry_type());
        bool card_ok = card->CanIndustryBeOverbuilt(player->network_, industry);
        bool player_industry_ok = to_build->CanOverbuild(industry);
        bool player_affordable = player->CanWithdraw(to_build->properties().basic_price);
        io_.ofstream << "* "
                      << ToString(industry->industry_type()) << "@" << industry->location()->name()
                      << " | card ok = " << card_ok 
                      << " | player industry ok = " << player_industry_ok
                      << " | player affordable = " << player_affordable << NEW_LINE;
        will_remove[industry] = !card_ok || !player_industry_ok || !player_affordable;
      }
      io_.ofstream.flush();

      std::vector<Industry*>::iterator last =
        std::remove_if(candidates.begin(), candidates.end(),
          [&will_remove](Industry* industry){ return will_remove.at(industry); }); 
      candidates.erase(last, candidates.end());

      // now, consider the resource consumption requirements
      std::vector<Industry*> shortlist;
      std::vector<ActionRequirement*> shortlist_reqs;
      io_.ofstream << "[BuildAction::PerformAction()] overbuilt shortlist" << NEW_LINE;
      for (Industry* industry : candidates) {
        Industry* to_build = player->unbuilt().GetLowestTechIndustry(industry->industry_type());
        ActionRequirement* req = CreateActionRequirement(player, industry, to_build->properties());
        if (req->ComputeRequirement(player)) {
          shortlist.push_back(industry);
          shortlist_reqs.push_back(req);
          io_.ofstream << "* "
                       << ToString(industry->industry_type()) << "@" << industry->location()->name()
                       << " | owner = " << industry->player_name()
                       << " | tech level = " << industry->properties().tech_level
                       << " | inventory = " << industry->available_units()
                       << " | eligible for vp = " << industry->IsEligibleForVp()
                       << " | total build $ = " << req->total_price()
                       << NEW_LINE;
        } else {
          delete req;
        }
      }
      io_.ofstream.flush();
      Industry* to_be_overbuilt = nullptr;
      ActionRequirement* req = nullptr;
      if (shortlist.size() == 0) {
        io_ << "Sorry, this card cannot be used for the build action for now." << NEW_LINE;
      } else {
        io_ << player->name() << " performs the Build action." << NEW_LINE;
        while (true) {
          if (ChooseIndustryToBuild(shortlist, shortlist_reqs, to_be_overbuilt, req)) {
            if (req->ChooseIndustriesToConsume(io_)) {
              req->Consume();
              built = true;
              IndustryType industry_type = to_be_overbuilt->industry_type();
              Industry* overbuilding = player->unbuilt().GetLowestTechIndustry(industry_type);
              io_.ofstream << "[BuildAction::PerformAction()] will build " << NEW_LINE
                           << overbuilding->properties() << NEW_LINE
                           << "on " << ToString(to_be_overbuilt->industry_type()) << '@' << to_be_overbuilt->location()->name() << std::endl;
              overbuilding->Build(to_be_overbuilt);
              player->Withdraw(req->total_price());
              io_ << player->name() << " withdraws $" << req->total_price() << '.' << NEW_LINE;
              int immediate_income = 0,
                quantity_sold = 0;
              ResourceType resource_type = overbuilding->resource_type();
              switch (resource_type) {
                case ResourceType::kCoal:
                  if (overbuilding->location()->IsConnectedToMerchantLocation()) {
                    immediate_income = coal_market_->Buy(overbuilding->available_units(), &quantity_sold);
                  }
                  break;
                  case ResourceType::kIron:
                  immediate_income = iron_market_->Buy(overbuilding->available_units(), &quantity_sold);
                  break;
                default:
                  break;
              }
              if (quantity_sold > 0) {
                overbuilding->ProvideResources(quantity_sold);
                player->Deposit(immediate_income);
                io_ << "Selling " << ToString(resource_type) << " x" << quantity_sold
                    << " earns " << player->name()
                    << " $" << immediate_income << '.' << NEW_LINE;
                io_.ofstream << "[BuildAction::PerformAction()] moving " << ToString(resource_type)
                              << " to the market from "
                              << ToString(overbuilding->industry_type()) << '@' << overbuilding->location()->name()
                              << NEW_LINE;
                io_.ofstream << "* " << ToString(resource_type) << " x" << quantity_sold << " => $" << immediate_income << std::endl;
              }
              player->unbuilt().PopLowestTechIndustry(industry_type);
              player->network().AddToNetwork(overbuilding);
              break;
            }
          } else {
            break;
          }
        }
      }
      for (ActionRequirement* req : shortlist_reqs) {
        delete req;
      }
    }
  }
  if (built) {
    player->DiscardCard(index);
  }
  return built;
}

ActionRequirement* BuildAction::CreateActionRequirement(Player* player, Industry* industry, IndustryProperties properties) {
  ActionRequirement* req = fty_->CreateActionRequirement(player, properties.basic_price);
  // A build action may require coal and/or iron.
  req->AddCoalRequirement(industry->location(), properties.coal_cost);
  req->AddIronRequirement(properties.iron_cost);
  return req;
}


bool BuildAction::ChooseCard(Player* player, int& index, Card*& card) {
  bool successful = Action::ChooseCard(player, index);
  if (successful) {
    card = player->hand_->at(index);
  }
  return successful;
}

bool BuildAction::ChooseIndustryToBuild(std::vector<Industry*> shortlist,
                                        std::vector<ActionRequirement*> shortlist_reqs,
                                        Industry*& to_be_overbuilt,
                                        ActionRequirement*& req) {
  std::map<IndustryType, std::vector<OverbuiltIndustryInfo> > info;
  std::map<IndustryType, std::vector<std::pair<Industry*, ActionRequirement*> > > shortlist_map;
  io_.ofstream << "[BuildAction::ChooseIndustryToBuild()] overbuilt shortlist" << NEW_LINE;
  for (std::vector<Industry*>::size_type i = 0; i < shortlist.size(); i++) {
    Industry* industry = shortlist.at(i);
    ActionRequirement* req = shortlist_reqs.at(i);
    const IndustryProperties& properties = industry->properties();
    IndustryType industry_type = industry->industry_type();
    info[industry_type].emplace_back(
      industry_type,
      industry->location()->name(),
      properties.tech_level,
      properties.vp,
      properties.vp_for_adj_link,
      industry->available_units(),
      properties.beer_cost,
      industry->IsEligibleForVp(),
      industry->player_name(),
      req->total_price()
    );
    shortlist_map[industry_type].emplace_back(industry, req);
    io_.ofstream << "* " << info[industry->industry_type()].back() << NEW_LINE;
  }
  io_.ofstream.flush();
  IndustryType industry_type;
  int index;
  if (io_.ChooseIndustryToBuild(info, industry_type, index)) {
    std::pair<Industry*, ActionRequirement*>& pair = shortlist_map.at(industry_type).at(index);
    to_be_overbuilt = pair.first;
    req = pair.second;
    return true;
  }
  to_be_overbuilt = nullptr;
  req = nullptr;
  return false;
}

NetworkAction::NetworkAction(Io& io, ActionRequirementFactory* fty, Map* map)
    : Action(io, fty, map) {}

NetworkAction::~NetworkAction() {
  for (ActionRequirement* req : shortlist_reqs_) {
    delete req;
  }
}

bool NetworkAction::CanPerformAction(Player* player) {
  for (ActionRequirement* req : shortlist_reqs_) {
    delete req;
  }
  shortlist_.clear();
  shortlist_reqs_.clear();
  // get the full list of links on the map, then do a preliminary filtering
  // by considering if the link can overbuilt by the player's links
  std::vector<Link*> candidates = map_->links();
  io_.ofstream << "[NetworkAction::CanPerformAction()] filter overbuilt targets (before considering consumption)" << NEW_LINE;
  bool network_established = player->network().HasEstablished();
  std::map<Link*, bool> will_remove;
  for (Link* link : candidates) {
    Link* to_build = player->unbuilt().GetLink(link->link_type());
    bool player_link_ok = to_build->CanOverbuild(link);
    bool player_affordable = player->CanWithdraw(to_build->properties().basic_price);
    bool adj_to_player_network = player->network().IsAdjacentTo(link);
    const Location* const * ends = link->adjacency()->ends();
    io_.ofstream << "* "
                  << ToString(link->link_type()) << "@" << ends[0]->name() << "<->" << ends[1]->name()
                  << " | player link ok = " << player_link_ok
                  << " | player affordable = " << player_affordable
                  << " | adj to player network = " << adj_to_player_network << NEW_LINE;
    will_remove[link] = !player_link_ok || !player_affordable || (network_established && !adj_to_player_network);
  }
  std::vector<Link*>::iterator last = \
    std::remove_if(candidates.begin(), candidates.end(),
      [&will_remove](Link* link){ return will_remove.at(link); }); 
  candidates.erase(last, candidates.end());
  
  // now, consider the resource consumption requirements
  io_.ofstream << "[NetworkAction::CanPerformAction()] overbuilt shortlist" << NEW_LINE;
  for (Link* link : candidates) {
    Link* to_build = player->unbuilt().GetLink(link->link_type());
    ActionRequirement* req = CreateActionRequirement(
      player, link, to_build->properties(), num_links_built_ > 0);
    if (req->ComputeRequirement(player)) {
      shortlist_.push_back(link);
      shortlist_reqs_.push_back(req);
      const Location* const * ends = link->adjacency()->ends();
      io_.ofstream << "* "
                    << ToString(link->link_type()) << "@" << ends[0]->name() << "<->" << ends[1]->name()
                    << " | owner = " << link->player_name()
                    << " | total build $ = " << req->total_price()
                    << NEW_LINE;
    } else {
      delete req;
    }
  }
  return shortlist_.size() > 0;
}

bool NetworkAction::PerformAction(Player* player) {
  int index;
  num_links_built_ = 0;
  if (CanPerformAction(player)) {
    if (ChooseCard(player, index)) {
      io_ << player->name() << " performs the Network action." << NEW_LINE;
      do {
        Link* to_be_overbuilt = nullptr;
        ActionRequirement* req = nullptr;
        OkFlag ok = OkFlag::kPending;
        while (true) {
          ok = ChooseLinkToBuild(shortlist_, shortlist_reqs_, to_be_overbuilt, req);
          if (ok == OkFlag::kOk) {
            if (req->ChooseIndustriesToConsume(io_)) {
              req->Consume();
              player->Withdraw(req->total_price());
              LinkType link_type = to_be_overbuilt->link_type();
              Link* overbuilding = player->unbuilt().PopLink(link_type);
              io_.ofstream << "[Network::PerformAction()] will build " << NEW_LINE
                           << overbuilding->properties() << NEW_LINE
                           << "on " << ToString(to_be_overbuilt->link_type())
                           << '@' << to_be_overbuilt->adjacency()->name() << std::endl;
              overbuilding->Build(to_be_overbuilt);
              player->network().AddToNetwork(overbuilding);
              ++num_links_built_;
              break;
            }
          } else {
            break;
          }
        }
        if (ok != OkFlag::kOk) {
          break;
        }
      } while ((num_links_built_ < 2) && CanPerformAction(player));
    }
    if (num_links_built_ > 0) {
      player->DiscardCard(index);
      return true;
    }
  } else {
    io_ << "You cannot perform the Network action." << NEW_LINE;
  }
  return false;
}

ActionRequirement* NetworkAction::CreateActionRequirement(Player* player, Link* link, LinkProperties properties, bool second_link) {
  ActionRequirement* req = fty_->CreateActionRequirement(player, properties.basic_price + ((second_link)? 5 : 0));
  // A network action which builds rail may require coal.
  // Building two rail links additionally requires non-merchant beer.
  // - Consume one coal after each rail link is placed.
  // - Consume one beer after two rail links are placed.
  if (second_link) {
    req->AddCoalRequirement(link->adjacency(), properties.coal_cost);
    req->AddBeerRequirement(link->adjacency(), 1);
  } else {
    req->AddCoalRequirement(link->adjacency(), properties.coal_cost);
  }
  return req;
}

OkFlag NetworkAction::ChooseLinkToBuild(std::vector<Link*> shortlist,
                                          std::vector<ActionRequirement*> shortlist_reqs,
                                          Link*& to_be_overbuilt,
                                          ActionRequirement*& req) {
  std::vector<OverbuiltLinkInfo> info;
  io_.ofstream << "[NetworkAction::ChooseLinkToBuild()] overbuilt shortlist" << NEW_LINE;
  for (std::vector<Link*>::size_type i = 0; i < shortlist.size(); i++) {
    const Link* link = shortlist.at(i);
    const ActionRequirement* req = shortlist_reqs.at(i);
    info.emplace_back(
      ToString(link->link_type()),
      link->adjacency()->ends()[0]->name(),
      link->adjacency()->ends()[1]->name(),
      req->total_price()
    );
    io_.ofstream << "* " << info.back() << NEW_LINE;
  }
  io_.ofstream.flush();
  int index;
  OkFlag ok = io_.ChooseLinkToBuild(info, index, num_links_built_ == 0, num_links_built_ > 0);
  if (ok == OkFlag::kOk) {
    to_be_overbuilt = shortlist[index];
    req = shortlist_reqs[index];
  } else {
    to_be_overbuilt = nullptr;
    req = nullptr;
  }
  return ok;
}

DevelopAction::DevelopAction(Io& io, ActionRequirementFactory* fty, Map* map)
    : Action(io, fty, map) {}

DevelopAction::~DevelopAction() {
  for (ActionRequirement* req : reqs_) {
    delete req;
  }
}

bool DevelopAction::CanPerformAction(Player* player) {
  for (ActionRequirement* req : reqs_) {
    delete req;
  }
  reqs_.clear();
  bool possible = false;
  const PlayerUnbuilt& unbuilt = player->unbuilt();
  for (IndustryType industry_type : kIndustryTypes) {
    if (unbuilt.GetLowestTechIndustry(industry_type) != nullptr) {
      possible = true;
      break;
    }
  }
  if (possible) {
    for (int i = 1; i <= 2; i++) {
      ActionRequirement* req = CreateActionRequirement(player, i);
      if (req->ComputeRequirement(player)) {
        reqs_.push_back(req);
      } else {
        delete req;
        break;
      }
    }
    if (reqs_.size() > 0) {
      return true;
    }
  }
  return false;
}

bool DevelopAction::PerformAction(Player* player) {
  num_dev_ = 0;
  if (CanPerformAction(player)) {
    int index;
    if (ChooseCard(player, index)) {
      io_ << player->name() << " performs the Develop action." << NEW_LINE;
      std::stack<Industry*> stack;
      while (true) {
        OkFlag ok = OkFlag::kPending;
        for (std::vector<ActionRequirement*>::size_type i = 1; i <= reqs_.size(); i++) {
          PlayerUnbuilt& unbuilt = player->unbuilt();
          std::vector<Industry*> industries;
          for (IndustryType industry_type : kIndustryTypes) {
            Industry* industry = unbuilt.GetLowestTechIndustry(industry_type);
            if (industry != nullptr && industry->properties().discardable) {
              industries.push_back(industry);
            }
          }
          IndustryType industry_type;
          ok = ChooseIndustryToDevelop(industries, industry_type, i, reqs_.size());
          if (ok == OkFlag::kOk) {
            ++num_dev_;
            stack.push(player->unbuilt().PopLowestTechIndustry(industry_type));
          } else {
            if (ok == OkFlag::kCancel) {
              num_dev_ = 0;
            }
            break;  // also for ok == OkFlag::kSkip
          }
        }
        if (ok == OkFlag::kCancel) {  // cancel the action
          while (!stack.empty()) {
            player->unbuilt().PutIndustry(stack.top());
            stack.pop();
          }
          break;
        } else {
          ActionRequirement* req = reqs_.at(stack.size() - 1);
          if (req->ChooseIndustriesToConsume(io_)) {
            req->Consume();
            player->Withdraw(req->total_price());
            break;
          } else {  // cancel consumption
            num_dev_ = 0;
            while (!stack.empty()) {
              player->unbuilt().PutIndustry(stack.top());
              stack.pop();
            }
          }
        }
      }
      if (num_dev_ > 0) {
        io_.ofstream << "[DevelopAction::PerformAction()] developing industies" << NEW_LINE;
        while (!stack.empty()) {
          Industry* dev = stack.top();
          io_.ofstream << "* " << ToString(dev->industry_type()) << " | " << dev->properties() << NEW_LINE;
          delete dev;
          stack.pop();
        }
        player->DiscardCard(index);
        return true;
      }
    }
  } else {
    io_ << "You cannot perform the Develop action." << NEW_LINE;
  }
  return false;
}

ActionRequirement* DevelopAction::CreateActionRequirement(Player* player, int num_discards) {
  ActionRequirement* req = fty_->CreateActionRequirement(player);
  // Removing one unbuilt industry requires one iron.
  req->AddIronRequirement(num_discards);
  return req;
}

OkFlag DevelopAction::ChooseIndustryToDevelop(std::vector<Industry*> shortlist,
                                            IndustryType& industry_type,
                                            int step, int max_num_steps) {
  std::vector<UnbuiltIndustryInfo> info;
  for (std::vector<Industry*>::size_type i = 0; i < shortlist.size(); i++) {
    const Industry* industry = shortlist.at(i);
    const IndustryProperties& properties = industry->properties();
    info.emplace_back(
      industry->industry_type(),
      properties,
      1
    );
  }
  int index;
  OkFlag ok = io_.ChooseIndustryToDevelop(step, max_num_steps, info, index,
                                            true, num_dev_ > 0);
  if (ok == OkFlag::kOk) {
    industry_type = shortlist.at(index)->industry_type();
  } else {
    industry_type = IndustryType::kUndefined;
  }
  return ok;
}

SellAction::SellAction(Io& io, ActionRequirementFactory* fty, Map* map)
    : Action(io, fty, map) {}

SellAction::~SellAction() {
  for (ActionRequirement* req : shortlist_reqs_) {
    delete req;
  }
}

bool SellAction::CanPerformAction(Player* player) {
  for (ActionRequirement* req : shortlist_reqs_) {
    delete req;
  }
  shortlist_.clear();
  shortlist_reqs_.clear();
  std::vector<Industry*> candidates = player->network().industries_of_type(MainIndustryType::kSecondaryIndustry);
  io_.ofstream << "[SellAction::CanPerformAction()] filter targets for sale (before considering consumption)" << NEW_LINE;
  std::map<Industry*, bool> will_remove;
  for (Industry* industry : candidates) {
    bool sold = static_cast<SecondaryIndustry*>(industry)->sold();
    bool connected_to_merchant = industry->location()->IsConnectedToMerchant(industry->industry_type());
    io_.ofstream << "* "
                 << ToString(industry->industry_type()) << "@" << industry->location()->name()
                 << " | sold = " << sold 
                 << " | connected to merchant = " << connected_to_merchant << NEW_LINE;
    will_remove[industry] = sold || !connected_to_merchant;
  }
  io_.ofstream.flush();

  std::vector<Industry*>::iterator last = \
    std::remove_if(candidates.begin(), candidates.end(),
      [&will_remove](Industry* industry){ return will_remove.at(industry); });
  candidates.erase(last, candidates.end());

  io_.ofstream << "[SellAction::CanPerformAction()] shortlist for sale" << NEW_LINE;
  for (Industry* industry : candidates) {
    ActionRequirement* req = CreateActionRequirement(player, industry);
    if (req->ComputeRequirement(player)) {
      shortlist_.push_back(industry);
      shortlist_reqs_.push_back(req);
      io_.ofstream << "* "
                   << ToString(industry->industry_type()) << "@" << industry->location()->name()
                   << " | tech level = " << industry->properties().tech_level
                   << NEW_LINE;
    } else {
      delete req;
    }
  }
  return shortlist_.size() > 0;
}

bool SellAction::PerformAction(Player* player) {
  num_sales_ = 0;
  int index;
  if (CanPerformAction(player)) {
    if (ChooseCard(player, index)) {
      io_ << player->name() << " performs the Sell action." << NEW_LINE;
      do {
        Industry* for_sale;
        ActionRequirement* req;
        OkFlag ok = OkFlag::kPending;
        while (true) {
          ok = ChooseIndustryToSell(shortlist_, shortlist_reqs_, for_sale, req);
          if (ok == OkFlag::kOk) {
            if (req->ChooseIndustriesToConsume(io_)) {
              req->Consume();
              player->Withdraw(req->total_price());
              io_.ofstream << "[SellAction::PerformAction()] will sell " << NEW_LINE
                           << "on " << ToString(for_sale->industry_type()) << '@' << for_sale->location()->name()
                           << for_sale->properties() << std::endl;
              for_sale->Sell();
              ++num_sales_;
            }
          } else {
            break;
          }
        }
        if (ok != OkFlag::kOk) {
          break;
        }
     } while (CanPerformAction(player));
      if (num_sales_ > 0) {
        player->DiscardCard(index);
        return true;
      }
    }
  } else {
    io_ << "You cannot perform the Sell action." << NEW_LINE;
  }
  return false;
}

ActionRequirement* SellAction::CreateActionRequirement(Player* player, Industry* industry) {
  ActionRequirement* req = fty_->CreateActionRequirement(player);
  // A sell action requires beer, which could be merchant beer.
  req->AddBeerRequirement(industry->location(), industry->properties().beer_cost);
  return req;
}

OkFlag SellAction::ChooseIndustryToSell(std::vector<Industry*> shortlist,
                                          std::vector<ActionRequirement*> shortlist_reqs,
                                          Industry*& to_be_sold,
                                          ActionRequirement*& req) {
  std::vector<IndustryInfo> info;
  for (std::vector<Industry*>::size_type i = 0; i < shortlist.size(); i++) {
    const Industry* industry = shortlist.at(i);
    const IndustryProperties& properties = industry->properties();
    info.emplace_back(
      ToString(industry->industry_type()),
      industry->location()->name(),
      industry->location()->color(),
      properties.tech_level,
      properties.vp,
      properties.vp_for_adj_link,
      0,
      properties.beer_cost,
      false,
      ""
    );
  }
  int index;
  OkFlag ok = io_.ChooseIndustryToSell(info, index, num_sales_ == 0, num_sales_ > 0);
  if (ok == OkFlag::kOk) {
    to_be_sold = shortlist[index];
    req = shortlist_reqs[index];
  } else {
    to_be_sold = nullptr;
    req = nullptr;
  }
  return ok;
}

LoanAction::LoanAction(Io& io) : Action(io, nullptr, nullptr) {}

bool LoanAction::CanPerformAction(Player* player) {
  return (player->income_level() - kIncomeLevelDecreaseForLoan >= kMinIncomeLevel);
}

bool LoanAction::PerformAction(Player* player) {
  if (CanPerformAction(player)) {
    io_ << player->name() << " performs the Loan action." << NEW_LINE;
    int index;
    if (ChooseCard(player, index)) {
      player->DiscardCard(index);
      player->Deposit(kLoanAmount);
      player->DecreaseIncomeLevels(kIncomeLevelDecreaseForLoan);
      io_.ofstream << "[LoanAction::PerformAction()] "
                   << "new balance = " << player->balance()
                   << " | new income level = " << player->income_level() << '-' << player->exp()
                   << std::endl;
      return true;
    } else {
      io_ << "You cannot perform the Loan action." << NEW_LINE;
    }
  }
  return false;
}

ScoutAction::ScoutAction(Io& io, DrawDeck* draw_deck)
    : Action(io, nullptr, nullptr), draw_deck_(draw_deck) {}

bool ScoutAction::CanPerformAction(Player* player) {
  return (player->num_cards_in_hand() >= 3) && !(player->HasWildCardInHand());
}

bool ScoutAction::PerformAction(Player* player) {
  if (CanPerformAction(player)) {
    std::vector<int> indices;
    if (ChooseCards(player, 3, indices)) {
      io_ << player->name() << " performs the Scout action." << NEW_LINE;
      io_.ofstream << "[ScoutAction::PerformAction()] cards drawn" << NEW_LINE;
      player->DiscardCard(indices);
      io_ << player->name() << " discards three cards." << NEW_LINE;
      Card* card = draw_deck_->PopWildIndustryCard();
      player->DrawCard(card);
      io_ << player->name() << " draws a " << card->name() << " card." << NEW_LINE;
      io_.ofstream << "* " << card->name() << NEW_LINE;
      card = draw_deck_->PopWildLocationCard();
      player->DrawCard(card);
      io_ << player->name() << " draws a " << card->name() << " card." << NEW_LINE;
      io_.ofstream << "* " << card->name() << NEW_LINE;
      io_.ofstream.flush();
      return true;
    }
  } else {
    io_ << "You cannot perform the Scout action." << NEW_LINE;
  }
  return false;
}

PassAction::PassAction(Io& io) : Action(io, nullptr, nullptr) {}

bool PassAction::CanPerformAction(Player* player) {
  static_cast<void>(player);
  return true;
}

bool PassAction::PerformAction(Player* player) {
  int index;
  if (ChooseCard(player, index)) {
    io_ << player->name() << " passes the action." << NEW_LINE;
    player->DiscardCard(index);
    io_ << player->name() << " discards a card." << NEW_LINE;
    return true;
  }
  return false;
}
