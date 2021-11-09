#include "player-unbuilt.h"

PlayerUnbuilt::PlayerUnbuilt() {}

PlayerUnbuilt::~PlayerUnbuilt() {
  for (LinkStackMap::value_type& pair : links_) {
    LinkStack& stack = pair.second;
    while (!stack.empty()) {
      delete stack.top();
      stack.pop();
    }
  }
  for (IndustryStackMap::value_type& pair : industries_) {
    IndustryStack& industries = pair.second;
    for (IndustryStack::reference p : industries) {
      std::stack<Industry*> stack = p.second;
      while (!stack.empty()) {
        delete stack.top();
        stack.pop();
      }
    }
  }
}

Link* PlayerUnbuilt::GetLink(LinkType link_type) const {
  const LinkStack& stack = links_.at(link_type);
  if (stack.empty()) {
    return nullptr;
  }
  Link* link = stack.top();
  link->set_player(player_);
  return link;
}

void PlayerUnbuilt::PutLink(Link* link) {
  links_[link->link_type()].push(link);
  if (player_ != nullptr)
    player_->Notify();
}

Link* PlayerUnbuilt::PopLink(LinkType link_type) {
  Link* link = GetLink(link_type);
  if (link != nullptr) {
    links_.at(link_type).pop();
  }
  if (player_ != nullptr)
    player_->Notify();
  return link;
}

Industry* PlayerUnbuilt::GetLowestTechIndustry(IndustryType industry_type) const {
  const IndustryStack& industries = industries_.at(industry_type);
  for (IndustryStack::const_reference pair : industries) {
    if (!pair.second.empty()) {
      Industry* industry = pair.second.top();
      industry->set_player(player_);
      return industry;
    }
  }
  return nullptr;
}

void PlayerUnbuilt::PutIndustry(Industry* industry) {
  industries_[industry->industry_type()][industry->properties().tech_level].push(industry);
  if (player_ != nullptr)
    player_->Notify();
}

Industry* PlayerUnbuilt::PopLowestTechIndustry(IndustryType industry_type) {
  Industry* industry = GetLowestTechIndustry(industry_type);
  if (industry != nullptr) {
    int level = industry->properties().tech_level;
    std::stack<Industry*>& stack = industries_.at(industry_type).at(level);
    stack.pop();
    if (player_ != nullptr)
      player_->Notify();
  }
  return industry;
}

void PlayerUnbuilt::set_player(Player* player) { player_ = player; }
