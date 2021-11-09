#include "util-common.h"

std::string ToString(ActionType action_type) {
  switch (action_type) {
    case ActionType::kBuild:
      return "Build";
    case ActionType::kDevelop:
      return "Develop";
    case ActionType::kLoan:
      return "Loan";
    case ActionType::kNetwork:
      return "Network";
    case ActionType::kPass:
      return "Pass";
    case ActionType::kScout:
      return "Scout";
    case ActionType::kSell:
      return "Sell";
  }
  return kUndefined;
}

std::string ToString(ResourceType resource_type) {
  switch (resource_type) {
    case ResourceType::kCoal:
      return kCoal;
    case ResourceType::kIron:
      return kIron;
    case ResourceType::kBeer:
      return kBeer;
  }
  return kUndefined;
}

std::string ToString(IndustryType industry_type) {
  switch (industry_type) {
    case IndustryType::kCottonMill:
      return kCottonMill;
    case IndustryType::kManufacturer:
      return kManufacturer;
    case IndustryType::kPottery:
      return kPottery;
    case IndustryType::kCoalMine:
      return kCoalMine;
    case IndustryType::kIronWorks:
      return kIronWorks;
    case IndustryType::kBrewery:
      return kBrewery;
    case IndustryType::kAny:
      return kAny;
  }
  return kUndefined;
}

std::string ToString(LinkType link_type) {
  switch (link_type) {
    case LinkType::kCanal:
      return kCanal;
    case LinkType::kRailway:
      return kRailway;
  }
  return kUndefined;
}

ResourceType ToResourceType(const std::string& resource_type) {
  if (resource_type == kCoal)
    return ResourceType::kCoal;
  if (resource_type == kIron)
    return ResourceType::kIron;
  if (resource_type == kBeer)
    return ResourceType::kBeer;
  return ResourceType::kUndefined;
}

IndustryType ToIndustryType(const std::string& industry_type) {
  if (industry_type == kCottonMill)
    return IndustryType::kCottonMill;
  if (industry_type == kManufacturer)
    return IndustryType::kManufacturer;
  if (industry_type == kPottery)
    return IndustryType::kPottery;
  if (industry_type == kCoalMine)
    return IndustryType::kCoalMine;
  if (industry_type == kIronWorks)
    return IndustryType::kIronWorks;
  if (industry_type == kBrewery)
    return IndustryType::kBrewery;
  if (industry_type == kAny)
    return IndustryType::kAny;
  return IndustryType::kUndefined;
}

LinkType ToLinkType(const std::string& link_type) {
  if (link_type == kCanal)
    return LinkType::kCanal;
  if (link_type == kRailway)
    return LinkType::kRailway;
  return LinkType::kUndefined;
}

MainIndustryType ToMainIndustryType(const std::string& industry_type) {
  IndustryType converted_type = ToIndustryType(industry_type);
  return ToMainIndustryType(converted_type);
}

MainIndustryType ToMainIndustryType(IndustryType industry_type) {
  switch (industry_type) {
    case IndustryType::kCottonMill:
    case IndustryType::kManufacturer:
    case IndustryType::kPottery:
      return MainIndustryType::kSecondaryIndustry;
    case IndustryType::kCoalMine:
    case IndustryType::kIronWorks:
    case IndustryType::kBrewery:
      return MainIndustryType::kPrimaryIndustry;
  }
  return MainIndustryType::kUndefined;
}

bool IsPrimaryIndustry(IndustryType industry_type) {
  switch (industry_type) {
    case IndustryType::kCoalMine:
    case IndustryType::kIronWorks:
    case IndustryType::kBrewery:
      return true;
  }
  return false;
}

Color ToColorType(const std::string& color) {
  if (color == "Teal") {
    return Color::kTeal;
  }
  if (color == "Blue") {
    return Color::kBlue;
  }
  if (color == "Red") {
    return Color::kRed;
  }
  if (color == "Gold") {
    return Color::kGold;
  }
  if (color == "Violet") {
    return Color::kViolet;
  }
  return Color::kUndefined;
}

template <>
std::string Join(const std::set<std::string>& a) {
  std::string result = "";
  for (std::set<std::string>::const_reference c : a) {
    result += c;
    result += ",";
  }
  if (result.length() > 0) {
    result.pop_back();
  }
  return result;
}

template <>
std::string Join(const std::set<std::pair<std::string, std::string> >& a) {
  std::string result = "";
  for (std::set<std::pair<std::string, std::string> >::const_reference c : a) {
    result += "(" + c.first + "," + c.second + ")";
    result += ",";
  }
  if (result.length() > 0) {
    result.pop_back();
  }
  return result;
}
