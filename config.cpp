#include "config.h"

#include "link.h"
#include "industry.h"

#include "fast-cpp-csv-parser/csv.h"

namespace config {

void CreateMapAndDrawDeck(const std::string& config_dir,
                          const std::string& default_config_dir, int num_players,
                          Map*& map, DrawDeck*& deck) {
  std::map<std::string, IndustryLocation*> industry_locations;
  std::map<std::string, MerchantLocation*> merchant_locations;
  std::set<std::set<std::string> > adjacency_str_set;
  map = new Map();
  {
    io::CSVReader<2> in(DetermineConfigFilePath("industry-locations.csv", config_dir, default_config_dir));
    in.read_header(io::ignore_extra_column, "location", "color");
    std::string location_str, color;
    while (in.read_row(location_str, color)) {
      if (!Contains(industry_locations, location_str)) {
        IndustryLocation* industry_location = new IndustryLocation(location_str, ToColorType(color));
        industry_locations.emplace(location_str, industry_location);
        map->Add(industry_location);
      }
    }
  }
  {
    io::CSVReader<2> in(DetermineConfigFilePath("merchant-locations.csv", config_dir, default_config_dir));
    in.read_header(io::ignore_extra_column, "location", "vp_for_adj_link");
    std::string location_str;
    int vp_for_adj_link;
    while (in.read_row(location_str, vp_for_adj_link)) {
      if (!Contains(merchant_locations, location_str)) {
        MerchantLocation* merchant_location = new MerchantLocation(location_str, vp_for_adj_link);
        merchant_locations.emplace(location_str, merchant_location);
      }
    }
  }
  {
    io::CSVReader<3> in(DetermineConfigFilePath("adjacencies.csv", config_dir, default_config_dir));
    in.read_header(io::ignore_extra_column, "location_a", "location_b", "link_type");
    std::string location_a, location_b, link_type_str;
    while (in.read_row(location_a, location_b, link_type_str)) {
      std::set<std::string> pair { location_a, location_b };
      if (!Contains(adjacency_str_set, pair)) {
        adjacency_str_set.insert(pair);
        Location *loc_a = nullptr, *loc_b = nullptr;
        if (Contains(industry_locations, location_a)) {
          loc_a = industry_locations.at(location_a);
        } else if (Contains(merchant_locations, location_a)) {
          loc_a = merchant_locations.at(location_a);
        }
        if (Contains(industry_locations, location_b)) {
          loc_b = industry_locations.at(location_b);
        } else if (Contains(merchant_locations, location_b)) {
          loc_b = merchant_locations.at(location_b);
        }
        if (loc_a != nullptr && loc_b != nullptr) {
          Adjacency* adj = new Adjacency(loc_a, loc_b);
          map->Add(adj, new LinkProxy(ToLinkType(link_type_str), adj));
        }
      }
    }
  }
  {
    io::CSVReader<3> in(DetermineConfigFilePath("industry-dist.csv", config_dir, default_config_dir));
    in.read_header(io::ignore_extra_column, "location", "industry_type", "count");
    std::string location_name, industry_type_str;
    int count;
    while (in.read_row(location_name, industry_type_str, count)) {
      for (int i = 0; i < count; i++) {
        IndustryType industry_type = ToIndustryType(industry_type_str);
        if (industry_type != IndustryType::kUndefined) {
          IndustryLocation* location = industry_locations.at(location_name);
          map->Add(location, new IndustryProxy(industry_type, location));
        }
      }
    }
  }
  {
    io::CSVReader<3> in(DetermineConfigFilePath("merchant-dist.csv", config_dir, default_config_dir));
    in.read_header(io::ignore_extra_column, "location", "industry_type", "count");
    std::string location_name, industry_type_str;
    int count;
    while (in.read_row(location_name, industry_type_str, count)) {
      for (int i = 0; i < count; i++) {
        IndustryType industry_type = ToIndustryType(industry_type_str);
        if (industry_type != IndustryType::kUndefined) {
          MerchantLocation* location = merchant_locations.at(location_name);
          map->Add(location, new Merchant(industry_type, 0));
        }
      }
    }
  }
  deck = new DrawDeck();
  {
    io::CSVReader<1> in(DetermineConfigFilePath("cards.csv", config_dir, default_config_dir));
    in.read_header(io::ignore_extra_column, "card_name");
    std::string card_name;
    while (in.read_row(card_name)) {
      if (Contains(industry_locations, card_name)) {
        deck->Put(new LocationCard(industry_locations.at(card_name)));
      } else {
        IndustryType industry_type = ToIndustryType(card_name);
        if (industry_type != IndustryType::kUndefined) {
          deck->Put(new IndustryCard(industry_type));
        }
      }
    }
  }
  for (int i = 0; i < num_players; i++) {
    deck->Put(new WildIndustryCard);
    deck->Put(new WildLocationCard);
  }
  deck->Reverse();
}

PlayerUnbuilt* CreatePlayerUnbuilt(const std::string& config_dir,
                                   const std::string& default_config_dir) {
  PlayerUnbuilt* unbuilt = new PlayerUnbuilt;
  {
    io::CSVReader<4> in(DetermineConfigFilePath("link-tile-dist.csv", config_dir, default_config_dir));
    in.read_header(io::ignore_extra_column, "link_type", "basic_price", "coal_cost", "count");
    std::string link_type_str;
    int basic_price, coal_cost, count;
    while (in.read_row(link_type_str, basic_price, coal_cost, count)) {
      LinkProperties properties(basic_price, coal_cost);
      for (int i = 0; i < count; i++) {
        unbuilt->PutLink(new LinkImpl(ToLinkType(link_type_str), properties));
      }
    }
  }
  {
    io::CSVReader<12> in(DetermineConfigFilePath("industry-tile-dist.csv", config_dir, default_config_dir));
    in.read_header(io::ignore_extra_column, "industry_type", "tech_level", "basic_price", "coal_cost", "iron_cost",
      "beer_cost", "output_units", "exp_increase", "vp", "vp_for_adj_link", "discardable", "count");
    std::string industry_type_str;
    int tech_level, basic_price, coal_cost, iron_cost, beer_cost, output_units,
      exp_increase, vp, vp_for_adj_link, count;
    int discardable;
    while (in.read_row(industry_type_str, tech_level, basic_price,
                       coal_cost, iron_cost, beer_cost, output_units,
                       exp_increase, vp, vp_for_adj_link, discardable, count)) {
      IndustryProperties properties(tech_level, basic_price, coal_cost, iron_cost,
                                    beer_cost, output_units, exp_increase,
                                    vp, vp_for_adj_link, discardable);
      IndustryType industry_type = ToIndustryType(industry_type_str);
      for (int i = 0; i < count; i++) {
        Industry* industry;
        if (IsPrimaryIndustry(industry_type)) {
          ResourceType resource_type;
          switch (industry_type) {
            case IndustryType::kCoalMine:
              resource_type = ResourceType::kCoal;
              break;
            case IndustryType::kIronWorks:
              resource_type = ResourceType::kIron;
              break;
            case IndustryType::kBrewery:
              resource_type = ResourceType::kBeer;
              break;
            default:
              resource_type = ResourceType::kUndefined;
          }
          if (resource_type != ResourceType::kUndefined) {
            industry = new PrimaryIndustry(industry_type, properties, resource_type);
            unbuilt->PutIndustry(industry);
          }
        } else {
          industry = new SecondaryIndustry(industry_type, properties);
          unbuilt->PutIndustry(industry);
        }
      }
    }
  }
  return unbuilt;
}

void CreateMarkets(const std::string& config_dir, 
                   const std::string& default_config_dir,
                   Market*& iron_market, Market*& coal_market) {
  io::CSVReader<4> in(DetermineConfigFilePath("markets.csv", config_dir, default_config_dir));
  in.read_header(io::ignore_extra_column, "resouce_type", "init_quantity", "max_quantity", "max_marginal_price");
  std::string resource_type_str;
  int init_quantity, max_quantity, max_marginal_price;
  iron_market = nullptr;
  coal_market = nullptr;
  while (in.read_row(resource_type_str, init_quantity, max_quantity, max_marginal_price)) {
    ResourceType resource_type = ToResourceType(resource_type_str);
    Market** target_market;
    switch (resource_type) {
      case ResourceType::kCoal:
        target_market = &coal_market;
        break;
      case ResourceType::kIron:
        target_market = &iron_market;
        break;
      default:
        target_market = nullptr;
    }
    if (target_market != nullptr)
      *target_market = new Market(resource_type, init_quantity, max_quantity, max_marginal_price);
  }
}

}
