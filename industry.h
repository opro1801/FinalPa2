#ifndef BRASS_INDUSTRY_H_
#define BRASS_INDUSTRY_H_

#include "location.h"
#include "infrastructure.h"

#include "game-common.h"
#include "game-observer.h"

#include <set>

class Player;
class Location;
class Infrastructure;
class IndustryObserver;
class ResourceObserver;

class Industry : public Infrastructure {
 public:
  Industry(IndustryType industry_type,
           Location* location,
           ResourceType resource_type);
  virtual ~Industry();

  // Returns true if the industry is built.
  virtual bool IsBuilt() const = 0;

  // Build the industry.
  virtual bool Build(Industry* industry) = 0;

  // Returns true if this industry can overbuild `other`.
  virtual bool CanOverbuild(Industry* other) const = 0;

  // Reacts to being overbuilt.
  virtual void BeOverbuilt(Industry* other) = 0;

  virtual bool IsDemolished() const = 0;

  // Removes the industry from the location.
  virtual void Demolish() = 0;

  // Returns true if the industry is eligible for VP calculation;
  virtual bool IsEligibleForVp() const = 0;

  // Returns true if the industry is sold in the sell action.
  // This does not do any checking against the merchant, and
  // fails only if the industry was sold before.
  virtual bool Sell();

  // Provides a specified number of resource units for free.
  virtual bool ProvideResources(int units);

  bool operator<(const Industry& other) const;

  // This returns the VP worth of the industry if is eligible for VP calculation,
  // otherwise, returns 0.
  virtual int vp() const = 0;

  // This returns the VP awarded for adjacent links if the industry is eligible for VP calculation,
  // otherwise, returns 0.
  virtual int vp_for_adjacent_link() const = 0;

  // Returns the available resource units of the industry.
  virtual int available_units() const = 0;

  virtual Industry* impl() = 0;
  virtual Player* player() const = 0;
  virtual std::string player_name() const = 0;
  Location* location() const;
  virtual IndustryProperties properties() const = 0;
  MainIndustryType main_industry_type() const;
  IndustryType industry_type() const;
  ResourceType resource_type() const;

  virtual void set_player(Player* player) = 0;
  void set_location(Location* location);
  virtual void set_industry_observer(IndustryObserver* observer) = 0;
  virtual void set_resource_observer(ResourceObserver* rsrc_observer) = 0;

 protected:
  // Produces consumable resources.
  virtual void ProduceResources();

  // Removes the consumable resources produced.
  virtual void RemoveResources();

  IndustryType industry_type_;
  ResourceType resource_type_;

  Location* location_;
  IndustryObserver* industry_observer_ = nullptr;
  ResourceObserver* resource_observer_ = nullptr;

 private:
  static int counter;
  const int creation_id_;
};


class IndustrySubscriber {
  friend IndustryObserver;
 
 public:
  virtual ~IndustrySubscriber();

 protected:
  virtual void UpdateIndustryInfo(std::map<IndustryType, std::vector<IndustryInfo> > industry_info) = 0;
};

class IndustryObserver : public Observer {
 public:
  IndustryObserver();
  ~IndustryObserver();

  void UpdateIndustry(Industry* industry);
  void RemoveIndustry(Industry* industry);
  void RegisterSubscriber(IndustrySubscriber* subscriber);
  void Unsubscribe(IndustrySubscriber* subscriber);
  virtual void Notify() override;

 private:
  using IndustryInfoMap = std::map<Industry*, IndustryInfo, ptr_less<Industry> >;

  IndustryInfoMap industry_info_;
  std::set<IndustrySubscriber*> subscribers_;
};

class IndustryImpl : public Industry {
 public:
  IndustryImpl(IndustryType industry_type, IndustryProperties properties,
               ResourceType resource_type = ResourceType::kUndefined);
  virtual ~IndustryImpl();

  virtual bool IsBuilt() const override;
  virtual bool Build(Industry* industry) override;
  virtual bool CanOverbuild(Industry* industry) const = 0;
  virtual void BeOverbuilt(Industry* industry) override;

  virtual bool IsDemolished() const override;
  virtual void Demolish() override;
  virtual bool IsEligibleForVp() const = 0;

  virtual Industry* impl();
  virtual Player* player() const override;
  virtual std::string player_name() const override;
  virtual int vp() const override;
  virtual int vp_for_adjacent_link() const override;
  virtual int available_units() const override;
  virtual IndustryProperties properties() const override;

  virtual void set_player(Player* player) override;

  virtual void set_industry_observer(IndustryObserver* observer);
  virtual void set_resource_observer(ResourceObserver* rsrc_observer) = 0;

 protected:
  const IndustryProperties properties_;
  bool demolished_ = false;
  Player* player_ = nullptr;
};

class PrimaryIndustry : public IndustryImpl {
 public:
  PrimaryIndustry(IndustryType industry_type, IndustryProperties properties,
                  ResourceType resource_type);
  ~PrimaryIndustry() = default;

  // This can overbuild the `other` industry if this is more technologically advanced,
  // and they are of the same industry type. Usually the owner of this industry must
  // also own the industry that he overbuilds on. However, if the game board has
  // exhausted all the resources, or the `other` industry is not yet built
  // (i.e. representing an empty slot), any player can overbuild the given industry
  // and is still subject to the tech level requirement.
  // Hint: As we alway overbuild through an industry proxy, an an industry proxy always
  //       has a resource observer attached, we can assume `other` is assoicated
  //       to a valid resource observer.
  virtual bool CanOverbuild(Industry* other) const override;

  // Returns true if all of its resource units are consumed.
  virtual bool IsEligibleForVp() const override;

  // Provides the given nubmer of resource units if possible.
  // Notifies the resource and industry observes of the change.
  virtual bool ProvideResources(int units) override;

  // Returns the inventory of the industry.
  virtual int available_units() const override;

  // Sets the resource observer of the industry.
  virtual void set_resource_observer(ResourceObserver* rsrc_observer) override;

 private:
  // Produces the number of output units specified by its properties,
  // and notifies the resource and industry observers of the change.
  virtual void ProduceResources() override;

  // Removes all the resource units available and notifies the
  // resource and industry observers of the change.
  virtual void RemoveResources() override;

  int available_units_ = 0;
};

class SecondaryIndustry : public IndustryImpl {
 public:
  SecondaryIndustry(IndustryType industry_type, IndustryProperties properties);
  ~SecondaryIndustry() = default;

  // This is similar to the function you solved for PrimaryIndustry.
  // Just that we do not allow other players who do not own the `other`
  // industry to build on it, if it is already built.
  virtual bool CanOverbuild(Industry* other) const override;

  // Returns true if the industry is sold.
  virtual bool IsEligibleForVp() const;

  // Sells the industry, and increase the player's EXP by a number
  // specified in the industry's properties.
  virtual bool Sell() override;

  // Returns true if the industry is already sold.
  bool sold() const;

  // Ignores the resource observer as a secondary industry does not produce
  // consumerable resources for players.
  virtual void set_resource_observer(ResourceObserver* rsrc_observer) override;

 private:
  bool sold_ = false;
};

class IndustryProxy : public Industry, IndustrySubscriber {
 public:
  IndustryProxy(IndustryType industry_type, Location* location,
                ResourceType resource_type = ResourceType::kUndefined);
  ~IndustryProxy();

  virtual bool IsBuilt() const override final;
  virtual bool Build(Industry* industry) override;
  virtual bool CanOverbuild(Industry* industry) const override final;

  // Let the underlying industry be overbuilt by the `other` industry.
  virtual void BeOverbuilt(Industry* other) override final;

  virtual bool IsDemolished() const override;
  virtual void Demolish() override;

  virtual bool IsEligibleForVp() const override;
  virtual bool ProvideResources(int units) override;

  void UpdateIndustryInfo(std::map<IndustryType, std::vector<IndustryInfo> > industry_info);

  virtual Industry* impl();
  virtual int vp() const override final;
  virtual int vp_for_adjacent_link() const override final;

  virtual int available_units() const;

  virtual Player* player() const;
  virtual std::string player_name() const override;
  virtual IndustryProperties properties() const;

  virtual void set_player(Player* player);

  virtual void set_industry_observer(IndustryObserver* industry_observer) override;
  virtual void set_resource_observer(ResourceObserver* resource_observer) override;
  
 private:
  IndustryImpl* impl_ = nullptr;
  IndustryObserver* impl_observer_;  // the observer that this proxy owns to observe the underlying industry
};

inline IndustryType Industry::industry_type() const {
  return industry_type_;
}

inline ResourceType Industry::resource_type() const {
  return resource_type_;
}

inline bool IndustryImpl::IsDemolished() const {
  return demolished_;
}

inline bool IndustryProxy::IsDemolished() const {
  return false;
}

inline 
bool Industry::operator<(const Industry& other) const {
  if (main_industry_type() < other.main_industry_type()) {
    return true;
  }
  if (main_industry_type() > other.main_industry_type()) {
    return false;
  }
  if (industry_type_ < other.industry_type_) {
    return true;
  }
  if (industry_type_ > other.industry_type_) {
    return false;
  }
  if (*location_ < *other.location_) {
    return true;
  }
  if (*other.location_ < *location_) {
    return false;
  }
  if (creation_id_ < other.creation_id_) {
    return true;
  }
  if (creation_id_ > other.creation_id_) {
    return false;
  }
  return false;
}

#endif  // BRASS_INDUSTRY_H_
