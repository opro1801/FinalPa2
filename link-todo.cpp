
#include "link.h"

//// TODO: complete the member function defintions of LinkImpl and LinkProxy

LinkImpl::LinkImpl(LinkType link_type, LinkProperties properties) : Link(link_type), properties_(properties) {}

bool LinkImpl::IsBuilt() const {
  return adj_ != nullptr;
}

bool LinkImpl::Build(Link* link) {
  if(CanOverbuild(link)) {
    adj_ = link->adjacency();
    link->BeOverbuilt(this);
    return true;
  }
  return false;
}

bool LinkImpl::CanOverbuild(Link* link) const {
  return !IsBuilt() && !link->IsBuilt() && link_type() == link->link_type();
}

int LinkImpl::vp() const {
  return adjacency()->vp_for_link();
}

LinkProperties LinkImpl::properties() const {
  return properties_;
}

Player* LinkImpl::player() const {
  return player_;
}

std::string LinkImpl::player_name() const {
  return player_->name();
}

void LinkImpl::set_player(Player* player) {
  player_ = player;
}

void LinkImpl::set_observer(LinkObserver* observer) {
  static_cast<void>(observer);
}


LinkProxy::LinkProxy(LinkType link_type, Adjacency* adj) : Link(link_type, adj) {}

bool LinkProxy::IsBuilt() const {
  return (impl_ != nullptr && (impl_->IsBuilt()));
}

bool LinkProxy::Build(Link* link) {
    static_cast<void>(link);
  return false;
}

bool LinkProxy::CanOverbuild(Link* link) const {
  static_cast<void>(link);
  return false;
}

void LinkProxy::BeOverbuilt(Link* link) {
    impl_ = static_cast<LinkImpl*>(link);
    impl_->set_adjacency(adj_);
}

int LinkProxy::vp() const {
  return (impl_ == nullptr) ? 0 : impl_->vp();
}

LinkProperties LinkProxy::properties() const {
  if(impl_ != nullptr) {
    return impl_->properties();
  }
  return {};
}

Player* LinkProxy::player() const {
  return (impl_ != nullptr) ? nullptr : impl_->player();
}

std::string LinkProxy::player_name() const {
  if (impl_ != nullptr) {
    return impl_->player_name();
  }
  return kEmptyPlayerName;
}

void LinkProxy::set_player(Player* player) {
  static_cast<void>(player);
}

void LinkProxy::set_observer(LinkObserver* observer) {
  observer_ = observer;
}

//// TODO ends
