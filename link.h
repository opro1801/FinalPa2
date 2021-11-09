#ifndef BRASS_LINK_H_
#define BRASS_LINK_H_

#include "game-common.h"
#include "location.h"
#include "infrastructure.h"

#include "player.h"
#include "util-common.h"

#include <set>
#include <map>
#include <string>

const int kNumEnds = 2;

class Location;
class Infrastructure;

class Adjacency {
 public:
  Adjacency(Location* location_1, Location* location_2);
  ~Adjacency() = default;

  bool operator<(const Adjacency& other); 
  bool operator==(const Adjacency& other);

  // Returns true if the location is one of the ends of this adjacency.
  bool IsAdjacentTo(const Location* location) const;

  std::string name() const;

  // Returns the other end of the adjacency, if the given `location` is one legit end.
  Location* another_end(const Location* location) const;

  // Returns the two ends of the adjacency.
  Location* const * ends() const;

  // Returns the VP for the link which connects the locations defined by this adjacency.
  int vp_for_link() const;

 private:
  Location* locations_[kNumEnds];
};

class Location;
class LinkObserver;

class Link : public Infrastructure {
 public:
  Link(LinkType link_type, Adjacency* adj = nullptr);
  virtual ~Link() = default;

  // Returns true if the location belongs to one of the ends of the adjacency.
  // If two locations are adjacent to each other, it just means the map defines that
  // it is possible to build a link between them. It is not necessary that the two 
  // locations are already connected.
  bool IsAdjacentTo(const Location* location) const;

  // Returns true if the link is built, and `location` is one of the ends of the adjacency.
  bool IsConnectedTo(const Location* location) const;

  // Returns true if the link has been built.
  virtual bool IsBuilt() const = 0;

  // Returns true if over-building the provided `link` is successful.
  virtual bool Build(Link* link) = 0;

  // Returns true if it is possible to overbuild `link`.
  virtual bool CanOverbuild(Link* link) const = 0;

  // Reacts to being over-built by another `link`. Should be used in `Build()` only.
  // Note: this can be made protected if we set LinkProxy as a friend of the Link class.
  // But doing this might confuse you and we decide not to do so.
  virtual void BeOverbuilt(Link* link);

  bool operator<(const Link& other) const;

  // Returns another end of the adjacency, if the given location is an end of the adjacency
  // that this link is associated to.
  // Otherwise, returns a nullptr.
  Location* another_end(const Location* location) const;

  // Returns the VP, which is the sum of "VP for adjacent links" from adjacent industries.
  virtual int vp() const = 0;

  // Sets the adjacency of this link.
  void set_adjacency(Adjacency* adj);

  // Returns the link type.
  LinkType link_type() const;

  // Returns the adjacency.
  Adjacency* adjacency() const;

  // Returns the properties (a data class) of the link.
  virtual LinkProperties properties() const = 0;

  // Returns the player associated to this link.
  virtual Player* player() const = 0;
  virtual std::string player_name() const = 0;

  // Associates a player to this link.
  virtual void set_player(Player* player) = 0;

  // Sets the observer for this link.
  virtual void set_observer(LinkObserver* observer) = 0;

 protected:
  LinkType link_type_;
  Adjacency* adj_;
};

class LinkImpl : public Link {
 public:
  LinkImpl(LinkType link_type, LinkProperties properties);
  ~LinkImpl() = default;

  virtual bool IsBuilt() const override;
  virtual bool Build(Link* link) override;
  virtual bool CanOverbuild(Link* link) const override;

  virtual int vp() const override;

  virtual LinkProperties properties() const override;

  virtual Player* player() const override;
  virtual std::string player_name() const override;

  virtual void set_player(Player* player) override;
  virtual void set_observer(LinkObserver* observer) override;

 protected:
  Player* player_ = nullptr;
  const LinkProperties properties_;
};

class LinkProxy : public Link {
 public:
  LinkProxy(LinkType link_type, Adjacency* adj);
  ~LinkProxy() = default;

  virtual bool IsBuilt() const override final;
  virtual bool Build(Link* link) override;
  virtual bool CanOverbuild(Link* link) const override final;
  virtual void BeOverbuilt(Link* link) override final;

  virtual int vp() const override final;

  virtual LinkProperties properties() const override;

  virtual Player* player() const override;
  virtual std::string player_name() const override;  // returns kEmptyPlayerName if not available

  virtual void set_player(Player* player) override;
  virtual void set_observer(LinkObserver* observer);

 private:
  LinkObserver* observer_ = nullptr;
  LinkImpl* impl_ = nullptr;
};


class LinkSubscriber {
  friend LinkObserver;
 
 public:
  virtual ~LinkSubscriber();

 protected:
  virtual void UpdateLinkInfo(std::vector<LinkInfo> link_info) = 0;
};

class LinkObserver : public Observer {
 public:
  LinkObserver();
  ~LinkObserver();

  void UpdateLink(Link* link);
  void RegisterSubscriber(LinkSubscriber* subscriber);
  void Unsubscribe(LinkSubscriber* subscriber);
  virtual void Notify() override;

 private:
  using LinkInfoMap = std::map<Link*, LinkInfo, ptr_less<Link> >;
  LinkInfoMap link_info_;
  std::set<LinkSubscriber*> subscribers_;
  std::map<const Location*, bool> connected_to_merchant_;
};

inline Adjacency* Link::adjacency() const {
  return adj_;
}


inline bool Adjacency::operator<(const Adjacency& other) {
  if (*locations_[0] < *other.locations_[0]) {
    return true;
  }
  if (*other.locations_[0] < *locations_[0]) {
    return false;
  }
  return *locations_[1] < *other.locations_[1];
}

inline bool Adjacency::IsAdjacentTo(const Location* location) const {
  return (locations_[0] == location) || (locations_[1] == location);
}

inline bool Link::operator<(const Link& other) const {
  if (link_type_ < other.link_type_) {
    return true;
  }
  if (link_type_ > other.link_type_) {
    return false;
  }
  return *adj_ < *other.adj_;
}

#endif  // BRASS_LINK_H_
