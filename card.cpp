#include "card.h"

#include "util-common.h"

//// TODO: Complete the member function defintion of the cardd classes

Card::Card(const std::string& name) : name_(name) {}

Card::~Card() {}

std::string Card::name() const {
    return name_;
}

LocationCard::LocationCard(Location* location) : 
Card(location->name()), location_(location) {}

bool LocationCard::IsWildCard() const {
    return false;
}

bool LocationCard::CanIndustryBeOverbuilt(const PlayerNetwork* network, const Industry* industry) const {
    static_cast<void>(network);
    return industry->location() == location_;
}


CardType LocationCard::card_type() const {
    return static_cast<CardType>(0);
}


WildLocationCard::WildLocationCard() : Card("Wild Location") {}

bool WildLocationCard::IsWildCard() const {
    return true;
}

CardType WildLocationCard::card_type() const {
    return static_cast<CardType>(2);
}

bool WildLocationCard::CanIndustryBeOverbuilt(const PlayerNetwork* network, const Industry* industry) const {
    static_cast<void>(network);
    static_cast<void>(industry);
    return true;
}


IndustryCard::IndustryCard(IndustryType industry_type) : Card(ToString(industry_type)), industry_type_(industry_type) {}

bool IndustryCard::IsWildCard() const {
    return false;
}

bool IndustryCard::CanIndustryBeOverbuilt(const PlayerNetwork* network, const Industry* industry) const {
    return (industry_type_ == industry->industry_type()) && network->IsCovering(industry->location());
}

CardType IndustryCard::card_type() const {
    return static_cast<CardType>(1);
}

WildIndustryCard::WildIndustryCard() : Card("Wild Industry") {}

bool WildIndustryCard::IsWildCard() const {
    return true;
}

bool WildIndustryCard::CanIndustryBeOverbuilt(const PlayerNetwork* network, const Industry* industry) const {
    static_cast<void>(industry);
    return network->IsCovering(industry->location());
}

CardType WildIndustryCard::card_type() const {
    return static_cast<CardType>(3);
}

//// TODO ends
