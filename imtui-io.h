#include "game-io.h"

#include "util-common.h"

#include "imtui/imtui.h"
#include "imtui/imtui-impl-ncurses.h"

#include <map>

static const int inline_elem_spacing = 5;
static const ImGuiTableFlags kCommonTableFlags = ImGuiTableFlags_BordersV;
static const ImGuiTableColumnFlags kCommonColumnFlags = ImGuiTableColumnFlags_WidthFixed;
static const int message_window_height = 5;

// The I/O class that uses imtui.
class ImtuiIo : public Io {
 public:
  ImtuiIo(const std::string& log_file_name);
  ~ImtuiIo();

  virtual void WaitForContinue() override;

  virtual OkFlag ChooseAction(ActionType& action_type) override;
  virtual bool ChooseCard(int& index) override;
  virtual bool ChooseCards(std::vector<int>& indices,
                           int num_cards = 1) override;
  virtual bool ChooseIndustryToBuild(const std::map<IndustryType, std::vector<OverbuiltIndustryInfo> >& industry_info,
                                     IndustryType& industry_type, int& index) override;
  virtual OkFlag ChooseLinkToBuild(const std::vector<OverbuiltLinkInfo>& link_info, int& index,
                                     bool allow_cancel, bool allow_skip) override;
  virtual OkFlag ChooseIndustryToSell(const std::vector<IndustryInfo>& industry_info, int& index,
                                       bool allow_cancel, bool allow_skip) override;
  virtual OkFlag ChooseIndustryToDevelop(int step, int max_num_steps,
                                           const std::vector<UnbuiltIndustryInfo>& industry_info,
                                           int& index, bool allow_cancel, bool allow_skip) override;
  virtual bool ChooseIndustriesToConsume(std::map<ResourceType, ConsumptionInfo>& consumption_info) override;
  virtual void ChooseIndustriesToLiquidate(const std::vector<IndustryInfo>& industry_info,
                                           int shortfall, const std::vector<int>& liquidation_value,
                                           std::vector<int>& chosen_indices) override;

  virtual void PushMessage(const std::string& msg) override;
  
 private:
  void NewFrame();
  void Reset();
  void Display();

  void DisplayActionGroup();
  void DisplayCardGroup();
  void DisplayPlayerUnbuilt();

  template <typename T>
  void DisplayUnformatted(const T& a);
  template <typename T>
  void DisplayColored(const ImVec4& col, const T& a);
  void DisplayRatio(int numerator, int denominator);
  void DisplayGameBoard();
  void DisplayPlayerMat();
  void DisplayMessageWindow();
  void DisplayBuildWindow();
  void DisplayNetworkWindow();
  void DisplayDevelopWindow();
  void DisplaySellWindow();
  void DisplayConsumptionWindow();
  void DisplayLiquidationWindow();

  const ImVec4 ToColor(Color color) const;
  void DisplayLocation(std::string name, Color color,
                       bool connected_to_merchant, bool in_or_adj_to_network);

  ImTui::TScreen* screen_;

  int win_width_ = 0;
  int win_height_ = 0;

  // output
  std::vector<std::string> output_lines_;

  const ImVec4 kRed = ImVec4(1.0f,0.0f,0.0f,1.0f);
  const ImVec4 kGreen = ImVec4(0.0f,1.0f,0.0f,1.0f);
  const ImVec4 kYellow = ImVec4(1.0f,1.0f,0.0f,1.0f);
  const ImVec4 kOrange = ImVec4(1.0f,0.6f,0.0f,1.0f);
  const ImVec4 kPink = ImVec4(0.95f,0.6f,0.7f,1.0f);

  int chosen_action_ = kUnchosen;

  int chosen_card_index_ = kUnchosen;
  int num_cards_to_choose_ = 0;
  int num_cards_chosen_ = 0;

  static const int kMaxCardsInHand = 8;
  bool cards_selected_[kMaxCardsInHand] {};

  IndustryType build_industry_type_ = IndustryType::kUndefined;
  int chosen_industry_index_ = kUnchosen;
  std::map<IndustryType, std::vector<OverbuiltIndustryInfo> > overbuilt_ind_info_;

  int chosen_link_index_ = kUnchosen;
  std::vector<OverbuiltLinkInfo> overbuilt_link_info_;

  int discard_step_ = 1;
  int max_num_discard_steps_ = 1;
  int chosen_discarding_industry_index_ = kUnchosen;
  std::vector<UnbuiltIndustryInfo> discarding_ind_info_;

  int chosen_selling_industry_index_ = kUnchosen;
  std::vector<IndustryInfo> selling_ind_info_;

  int chosen_consuming_industry_index_ = kUnchosen;
  std::map<ResourceType, ConsumptionInfo> consumption_info_;

  int shortfall_;
  std::vector<IndustryInfo> liquidate_industry_info_;
  std::vector<int> liquidation_value_;
  bool* industries_selected_ = nullptr;

  std::vector<std::string> messages_;
};

inline void ImtuiIo::DisplayRatio(int numerator, int denominator) {
  std::string str = (std::to_string(numerator) + " / " + std::to_string(denominator));
  if (numerator == denominator) {
    ImGui::TextColored(kGreen, str.c_str());
  } else if (numerator > denominator) {
    ImGui::TextColored(kRed, str.c_str());
  } else {
    ImGui::Text(str.c_str());
  }
}

template <typename T>
inline void ImtuiIo::DisplayUnformatted(const T& a) {
  ImGui::TextUnformatted(std::to_string(a).c_str());
}

template <>
inline void ImtuiIo::DisplayUnformatted(const std::string& a) {
  ImGui::TextUnformatted(a.c_str());
}

template <typename T>
inline void ImtuiIo::DisplayColored(const ImVec4& col, const T& a) {
  ImGui::TextColored(col, std::to_string(a).c_str());
}

template <>
inline void ImtuiIo::DisplayColored(const ImVec4& col, const std::string& a) {
  ImGui::TextColored(col, a.c_str());
}
