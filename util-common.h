#ifndef BRASS_UTIL_COMMON_H_
#define BRASS_UTIL_COMMON_H_

#include "game-common.h"

#include <string>
#include <set>

// Returns the string representation of the enum classes.
std::string ToString(ActionType action_type);
std::string ToString(ResourceType resource_type);
std::string ToString(IndustryType industry_type);
std::string ToString(LinkType link_type);

// Returns the enumeration corresponding to the provided string representation.
ResourceType ToResourceType(const std::string& resource_type);
IndustryType ToIndustryType(const std::string& industry_type);
LinkType ToLinkType(const std::string& link_type);
MainIndustryType ToMainIndustryType(const std::string& industry_type);
Color ToColorType(const std::string& color);

bool IsPrimaryIndustry(IndustryType industry_type);
MainIndustryType ToMainIndustryType(IndustryType industry_type);

inline int max(int a, int b) {
  return (a > b)? a : b;
}

inline int min(int a, int b) {
  return (a < b)? a : b;
}

// Checks set membership.
template<typename T, template<class,class...> class C, class... Args>
bool Contains(const C<T,Args...>& set_like, const T& a) {
  return set_like.find(a) != set_like.end();
}

// Joins a list of strings.
template <typename T>
std::string Join(const T& a) {
  std::string result = "";
  for (typename T::const_reference c : a) {
    result += std::to_string(c);
    result += ",";
  }
  if (result.length() > 0) {
    result.pop_back();
  }
  return result;
}

template <>
std::string Join(const std::set<std::string>& a);

template <>
std::string Join(const std::set<std::pair<std::string, std::string> >& a);

// A functor for comparing objects through pointers.
template<class T>
struct ptr_less {
  bool operator()(T*& a, T* const& b) const {
    if (a == nullptr) {
      return true;
    }
    if (b == nullptr) {
      return false;
    }
    return *a < *b;
  }

  bool operator()(T* const& a, T* const& b) const {
    if (a == nullptr) {
      return true;
    }
    if (b == nullptr) {
      return false;
    }
    return *a < *b;
  }
};

// A functor for comparing two pairs of objects through pointers.
template<class T>
struct pair_ptr_less {
  bool operator()(std::pair<T*, T*>& a, std::pair<T*, T*> const& b) const {
    if (a.first == nullptr) {
      return true;
    }
    if (b.first == nullptr) {
      return false;
    }
    if (*a.first < *b.first) {
      return true;
    }
    if (*b.first < *a.first) {
      return false;
    }
    if (a.second == nullptr) {
      return true;
    }
    if (b.second == nullptr) {
      return false;
    }
    if (*a.second < *b.second) {
      return true;
    }
    // *b.second <= *a.second
    return false;
  }

  bool operator()(std::pair<T*, T*> const& a, std::pair<T*, T*> const& b) const {
    if (a.first == nullptr) {
      return true;
    }
    if (b.first == nullptr) {
      return false;
    }
    if (*a.first < *b.first) {
      return true;
    }
    if (*b.first < *a.first) {
      return false;
    }
    if (a.second == nullptr) {
      return true;
    }
    if (b.second == nullptr) {
      return false;
    }
    if (*a.second < *b.second) {
      return true;
    }
    // *b.second < *a.second
    return false;
  }
};

#endif  // BRASS_UTIL_COMMON_H_
