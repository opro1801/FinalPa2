#ifndef BRASS_LOCATION_H_
#define BRASS_LOCATION_H_

#include "game-common.h"
#include "util-common.h"
#include "game-observer.h"

#include <string>
#include <vector>
#include <set>
#include <map>
#include <stack>

class Link;
class LinkObserver;
class Adjacency;

class Location {
 public:
  using LocationPtrSet = std::set<Location*, ptr_less<Location> >;

  Location(const std::string& name, Color color = Color::kUndefined);
  virtual ~Location();

  virtual bool HasIndustryType(IndustryType industry_type) const = 0;

  void AddLink(Link* link);

  virtual bool IsConnectedTo(const Location* other) const;
  virtual bool IsConnectedToMerchantLocation() const;
  virtual bool IsConnectedToMerchant(IndustryType industry_type) const;

  virtual void Update() const;

  bool operator<(const Location& other) const;

  virtual std::vector<Location*> adjacent_locations() const;
  virtual std::vector<Location*> adjacent_connected_locations() const;
  virtual std::vector<Location*> connected_locations() const;

  virtual int vp_for_adjacent_link() const = 0;

  virtual LocationType location_type() const = 0;
  virtual std::string name() const;
  virtual Color color() const;

  std::vector<Link*> adjacent_links() const;

 protected:
  template <typename F>
  bool IsConnected(F found_func) const;

  std::set<Link*, ptr_less<Link> > adj_links_;

  std::string name_;
  Color color_;

  LinkObserver* link_observer_ = nullptr;
};

#include "link.h"

template <typename F>
bool Location::IsConnected(F found_func) const {
  std::stack<Location*> stack;
  LocationPtrSet connected_locations;

  if (found_func(this)) {  // shortcut
    return true;
  }
  stack.push(const_cast<Location*>(this));

  while (!stack.empty()) {
    Location* location = stack.top();
    stack.pop();
    connected_locations.insert(location);
    for (Location* other : location->adjacent_connected_locations()) {
      if (found_func(other)) {
        return true;
      }
      if (connected_locations.count(other) <= 0) {
        stack.push(other);
      }
    }
  }
  return false;
}

inline std::string Location::name() const {
  return name_;
}

inline Color Location::color() const {
  return color_;
}

inline bool Location::operator<(const Location& other) const {
  if (color_ < other.color_) {
    return true;
  }
  if (color_ > other.color_) {
    return false;
  }
  return name_ < other.name_;
}

#endif  // BRASS_LOCATION_H_
