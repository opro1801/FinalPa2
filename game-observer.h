#ifndef GAME_OBSERVER_H_
#define GAME_OBSERVER_H_

#include "game-common.h"
#include "util-common.h"

#include <array>
#include <vector>
#include <map>

class Observer {
 public:
  Observer() = default;
  virtual ~Observer();

  // Notifies its subscriber of the change.
  virtual void Notify() = 0;

 protected:
  bool updating_ = false;
  bool notifying_ = false;
};

class ResourceObserver;

class ResourceSubscriber {
  friend ResourceObserver;
 
 public:
  virtual ~ResourceSubscriber();

 protected:
  virtual void UpdateResourceInfo(std::map<ResourceType, ResourceInfo> resource_info) = 0;
};

// Oversees the number of resource present on the game board.
class ResourceObserver : public Observer {
 public:
  ResourceObserver();
  ~ResourceObserver();

  // Returns true if there are some resources of the the `resource_type` left on 
  // the game board. This includes the production units of the industries
  // and market units that occupy a space in the market.
  bool HasResource(ResourceType resource_type) const;

  // Update the industry resource count.
  void UpdateResourceCount(ResourceType resource_type, int units = 1);

  // Update the market resource count.
  void UpdateMarketResourceCount(ResourceType resource_type, int units,
                                 std::array<int, kNumPricesToShow> selling_price,
                                 std::array<int, kNumPricesToShow> buying_price, bool full);

  // Register a subscriber who will be informed of any changes of the state
  // of the observer.
  void RegisterSubscriber(ResourceSubscriber* subscriber);

  // Notify the subscribers. Most likely used by this observer itself only.
  virtual void Notify() override;

 private:
  std::map<ResourceType, ResourceInfo> resource_info_;
  std::vector<ResourceSubscriber*> subscribers_;
};

#endif  // GAME_OBSERVER_H_
