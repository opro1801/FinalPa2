#include "imtui-io.h"

#if defined __MINGW32__ || defined __MINGW64__
  #include "pdcurses/curses.h"
#endif

ImtuiIo::ImtuiIo(const std::string& log_file_name) : Io(log_file_name) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  screen_ = ImTui_ImplNcurses_Init(true);
  ImTui_ImplText_Init();
}

ImtuiIo::~ImtuiIo() {
  ImTui_ImplText_Shutdown();
  ImTui_ImplNcurses_Shutdown();
}

const ImVec4 ImtuiIo::ToColor(Color color) const {
  switch (color) {
    case Color::kRed:
      return ImVec4(0.8f,0.0f,0.2f,1.0f);
    case Color::kGold:
      return ImVec4(0.8f,0.8f,0.0f,1.0f);
    case Color::kTeal:
      return ImVec4(0.0f,0.5f,0.6f,1.0f);
    case Color::kBlue:
      return ImVec4(0.0f,0.2f,0.8f,1.0f);
    case Color::kViolet:
      return ImVec4(0.4f,0.0f,0.6f,1.0f);
    case Color::kUndefined:
    default:
      return ImVec4(1.0f,1.0f,1.0f,1.0f);
  };
}

void ImtuiIo::DisplayLocation(std::string name, Color color,
                              bool connected_to_merchant,
                              bool in_or_adj_to_network) {
  ImGui::TextColored(ToColor(color), "#"); ImGui::SameLine();
  if (connected_to_merchant) {
    ImGui::Text("&");
  } else {
    ImGui::TextDisabled("&");
  }
  ImGui::SameLine();
  if (in_or_adj_to_network) {
    DisplayColored(kPink, name);
  } else {
    DisplayUnformatted(name);
  }
}

void ImtuiIo::WaitForContinue() {
  action_status_ = ActionStatus::kCompletedAction;
  ok_status_ = OkFlag::kPending;
  while (ok_status_ == OkFlag::kPending) {
    NewFrame();
  }
}

void ImtuiIo::NewFrame() {
  ImTui_ImplNcurses_NewFrame();
  ImTui_ImplText_NewFrame();
  ImGui::NewFrame();
  Display();
  ImGui::Render();
  ImTui_ImplText_RenderDrawData(ImGui::GetDrawData(), screen_);
  ImTui_ImplNcurses_DrawScreen();
}

void ImtuiIo::Display() {
  ImVec2 win_size = ImGui::GetIO().DisplaySize;
  win_width_ = win_size[0];
  win_height_ = win_size[1];
  int half_win_width = win_width_ / 2;
  ImGui::SetNextWindowSize(ImVec2(half_win_width, win_height_), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
  DisplayGameBoard();
  ImGui::SetNextWindowSize(ImVec2(half_win_width, win_height_ - message_window_height), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowPos(ImVec2(half_win_width, 0), ImGuiCond_FirstUseEver);
  DisplayPlayerMat();
  ImGui::SetNextWindowSize(ImVec2(half_win_width, message_window_height), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowPos(ImVec2(half_win_width, win_height_ - message_window_height), ImGuiCond_FirstUseEver);
  DisplayMessageWindow();
}

void ImtuiIo::DisplayGameBoard() {
  ImGui::Begin("Brass Game Board", nullptr, ImGuiWindowFlags_NoCollapse);

  // Some game stats
  ImGui::Text("Round:"); ImGui::SameLine(); DisplayRatio(game_stat_->curr_round, game_stat_->max_num_rounds); ImGui::SameLine(0, inline_elem_spacing);
  ImGui::Text("Turn:"); ImGui::SameLine(); DisplayRatio(game_stat_->curr_turn, game_stat_->max_num_turns); ImGui::SameLine(0, inline_elem_spacing);
  ImGui::Text("Cards in Deck:"); ImGui::SameLine(); DisplayRatio(game_stat_->num_cards_left, game_stat_->max_num_cards);
  ImGui::NewLine(); ImGui::Spacing();

  ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  if (resource_info_ != nullptr && ImGui::CollapsingHeader("Resource Information")) {
    if (ImGui::BeginTable("resource_info", 5, kCommonTableFlags)) {
      ImGui::TableSetupColumn("Resource", kCommonColumnFlags);
      ImGui::TableSetupColumn("Industry Supply", kCommonColumnFlags);
      ImGui::TableSetupColumn("Market Supply", kCommonColumnFlags);
      ImGui::TableSetupColumn("Market Selling $", kCommonColumnFlags);
      ImGui::TableSetupColumn("Market Buying $", kCommonColumnFlags);
      //ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
      ImGui::TableHeadersRow();

      for (std::map<ResourceType, ResourceInfo>::const_reference pair : *resource_info_) {
        const ResourceInfo& info = pair.second;
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        DisplayUnformatted(ToString(info.resource_type));
        ImGui::TableNextColumn();
        DisplayUnformatted(info.industry_supply);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.market_supply);
        if (info.market_full) {
          ImGui::SameLine();
          ImGui::TextColored(kRed, "(full)");
        }
        ImGui::TableNextColumn();
        DisplayUnformatted(Join(info.selling_price));
        ImGui::TableNextColumn();
        DisplayUnformatted(Join(info.buying_price));
      }
      ImGui::EndTable();
    }
  }
  ImGui::NewLine(); ImGui::Spacing();

  if (industry_info_ != nullptr && ImGui::CollapsingHeader("Industry Information")) {
    if (ImGui::BeginTabBar("industry_info_tab_bar")) {
      for (std::map<IndustryType, std::vector<IndustryInfo> >::const_reference pair : *industry_info_) {
        bool is_primary_industry = IsPrimaryIndustry(pair.first);
        if (ImGui::BeginTabItem(ToString(pair.first).c_str())) {
          DisplayUnformatted("~ " + ToString(pair.first) + " ~");

          if (ImGui::BeginTable("industry_info", 7, kCommonTableFlags)) {
            ImGui::TableSetupColumn("Location", kCommonColumnFlags);
            ImGui::TableSetupColumn("Owner", kCommonColumnFlags);
            ImGui::TableSetupColumn("Tech", kCommonColumnFlags);
            ImGui::TableSetupColumn("VP", kCommonColumnFlags);
            ImGui::TableSetupColumn("Link VP", kCommonColumnFlags);
            if (is_primary_industry) {
              ImGui::TableSetupColumn("Inventory", kCommonColumnFlags);
            } else {
              ImGui::TableSetupColumn("Beer", kCommonColumnFlags);
            }
            ImGui::TableSetupColumn("VP Eligible?", kCommonColumnFlags);
            ImGui::TableHeadersRow();

            const std::vector<IndustryInfo>& ind_info = pair.second;
            for (const IndustryInfo& info : ind_info) {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              DisplayLocation(info.location, info.location_color,
                              info.connected_to_merchant,
                              IsPlayerInfoReady() && Contains((*player_info_)->network, info.location));
              ImGui::TableNextColumn();
              DisplayUnformatted(info.owner);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.tech_level);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.vp);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.vp_for_link);
              ImGui::TableNextColumn();
              if (is_primary_industry) {
                DisplayUnformatted(info.available_units);
              } else {
                DisplayUnformatted(info.beer_cost);
              }
              ImGui::TableNextColumn();
              ImGui::TextUnformatted((info.eligible_for_vp)? "y" : "n");
            }
            ImGui::EndTable();
          }
          ImGui::EndTabItem();
        }
      }
      ImGui::EndTabBar();
    }
  }
  ImGui::NewLine(); ImGui::Spacing();

  if (link_info_ != nullptr && ImGui::CollapsingHeader("Link Information")) {
    if (ImGui::BeginTable("link_info", 5, kCommonTableFlags)) {
      ImGui::TableSetupColumn("Type", kCommonColumnFlags);
      ImGui::TableSetupColumn("Owner", kCommonColumnFlags);
      ImGui::TableSetupColumn("Location", kCommonColumnFlags);
      ImGui::TableSetupColumn("Location", kCommonColumnFlags);
      ImGui::TableSetupColumn("Prov. VP", kCommonColumnFlags);
      ImGui::TableHeadersRow();

      for (const LinkInfo& info : *link_info_) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        DisplayUnformatted(info.link_type);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.owner);
        ImGui::TableNextColumn();
        bool is_adj = IsPlayerInfoReady() && Contains((*player_info_)->adj_links,
                               std::make_pair(info.location_1, info.location_2));
        DisplayLocation(info.location_1, info.location_1_color,
                        info.location_1_connected_to_merchant, is_adj);
        ImGui::TableNextColumn();
        DisplayLocation(info.location_2, info.location_2_color,
                        info.location_2_connected_to_merchant, is_adj);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.provisional_vp);
      }

      ImGui::EndTable();
    }
  }
  ImGui::End();
}

void ImtuiIo::DisplayActionGroup() {
  ImGui::Text("Action:"); ImGui::SameLine(); DisplayRatio(game_stat_->curr_action, game_stat_->max_num_actions);
  ImGui::NewLine();

  // Display the radio button group for actions
  ImGui::Text("Actions");
  if (action_status_ == ActionStatus::kChooseAction) {
    ImGui::SameLine(); ImGui::TextColored(kYellow, "(pick 1)");
  }

  ImGui::BeginGroup();
  for (std::vector<std::string>::size_type i = 0; i < action_list_.size(); i++) {
    if (action_status_ == ActionStatus::kChooseAction) {
      ImGui::RadioButton(action_list_[i].c_str(), &chosen_action_, i);
    } else {
      int dummy_action = chosen_action_;
      ImGui::RadioButton(action_list_[i].c_str(), &dummy_action, i);
    }
    ImGui::SameLine();
  }
  if (action_status_ == ActionStatus::kChooseAction && chosen_action_ > kUnchosen) {
    ImGui::SameLine(0, 5);
    if (ImGui::Button("OK")) {
      ok_status_ = OkFlag::kOk;
    }
  }
  ImGui::EndGroup();
  ImGui::NewLine();
}

void ImtuiIo::DisplayCardGroup() {
  // Display the checkbox group for cards
  ImGui::Text("Cards");
  if (action_status_ == ActionStatus::kChooseCard) {
    ImGui::SameLine(); ImGui::TextColored(kYellow, ("(pick " + std::to_string(num_cards_to_choose_) + ")").c_str());
  }
  
  ImGui::BeginGroup();
  const std::vector<std::string>& cards = (*player_info_)->cards;
  for (std::vector<std::string>::size_type i = 0; i < cards.size(); i++) {
    if (i % 4 != 0) {
      ImGui::SameLine(0, 5);
    }
    static bool card_selection_changed = false;
    if (action_status_ == ActionStatus::kChooseCard) {
      if (num_cards_to_choose_ == 1) {
        card_selection_changed = ImGui::RadioButton(cards[i].c_str(), &chosen_card_index_, i);
      } else {
        card_selection_changed = ImGui::Checkbox(cards[i].c_str(), &cards_selected_[i]);
      }
      if (card_selection_changed && num_cards_to_choose_ > 1) {
        if (cards_selected_[i]) {
          if (num_cards_chosen_ == num_cards_to_choose_) {
            cards_selected_[i] = false;
          } else {
            ++num_cards_chosen_;
          }
        } else {
          --num_cards_chosen_;
        }
      }
    } else {
      static int chosen_card_index = chosen_card_index_;
      ImGui::RadioButton(cards[i].c_str(), &chosen_card_index, i);
      chosen_card_index = chosen_card_index_;
    }
  }

  if (action_status_ == ActionStatus::kChooseCard) {
    ImGui::SameLine(0, 5);
    if (((num_cards_to_choose_ == 1 && chosen_card_index_ > kUnchosen)
            || (num_cards_to_choose_ > 1 && num_cards_chosen_ == num_cards_to_choose_))) {
      if (ImGui::Button("OK")) {
        ok_status_ = OkFlag::kOk;
      }
      ImGui::SameLine();
    }
    if (ImGui::Button("Cancel")) {
      ok_status_ = OkFlag::kCancel;
      chosen_card_index_ = kUnchosen;
    }
  } else if (action_status_ == ActionStatus::kCompletedAction) {
    ImGui::SameLine(0, 5);
    if (ImGui::Button("Done")) {
      ok_status_ = OkFlag::kOk;
    }
    ImGui::NewLine();
  }

  ImGui::EndGroup();
  ImGui::NewLine();
}

void ImtuiIo::DisplayPlayerUnbuilt() {
  if (!IsPlayerInfoReady()) {
    return;
  }
  // display a table of the player's unbuilt tiles
  if (ImGui::CollapsingHeader("Unbuilt Industries")) {
    if (ImGui::BeginTabBar("industry_info_tab_bar")) {
      for (std::map<IndustryType, std::vector<UnbuiltIndustryInfo> >::const_reference pair : (*player_info_)->unbuilt_industry_info) {
        IndustryType industry_type = pair.first;
        bool is_primary_industry = IsPrimaryIndustry(industry_type);
        if (ImGui::BeginTabItem(ToString(industry_type).c_str())) {
          ImGui::Text(("~ " + ToString(industry_type) + " ~").c_str());
          if (ImGui::BeginTable("unbuilt_industry_info", 9, kCommonTableFlags)) {
            ImGui::TableSetupColumn("Tech", kCommonColumnFlags);
            ImGui::TableSetupColumn("VP", kCommonColumnFlags);
            ImGui::TableSetupColumn("Link VP", kCommonColumnFlags);
            ImGui::TableSetupColumn("EXP+", kCommonColumnFlags);
            ImGui::TableSetupColumn("Basic $", kCommonColumnFlags);
            ImGui::TableSetupColumn("Coal\n(Build)", kCommonColumnFlags);
            ImGui::TableSetupColumn("Iron\n(Build)", kCommonColumnFlags);
            if (is_primary_industry) {
              ImGui::TableSetupColumn("Output", kCommonColumnFlags);
            } else {
              ImGui::TableSetupColumn("Beer\n(Sell)", kCommonColumnFlags);
            }
            ImGui::TableSetupColumn("Count", kCommonColumnFlags);
            // ImGui::TableSetupColumn("Count", kCommonColumnFlags);
            ImGui::TableHeadersRow();

            const std::vector<UnbuiltIndustryInfo>& ind_info = pair.second;
            for (const UnbuiltIndustryInfo& info : ind_info) {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              DisplayUnformatted(info.tech_level);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.vp);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.vp_for_link);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.exp_increase);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.basic_price);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.coal_cost);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.iron_cost);
              if (is_primary_industry) {
                ImGui::TableNextColumn();
                DisplayUnformatted(info.output_units);
              } else {
                ImGui::TableNextColumn();
                DisplayUnformatted(info.beer_cost);
              }
              ImGui::TableNextColumn();
              DisplayUnformatted(info.count);
            }
            ImGui::EndTable();
          }
          ImGui::EndTabItem();
        }
      }
      ImGui::EndTabBar();
    }
  }

  ImGui::NewLine();

  if (ImGui::CollapsingHeader("Unbuilt Links")) {
    if (ImGui::BeginTable("unbuilt_link_info", 4, kCommonTableFlags)) {
      ImGui::TableSetupColumn("Type", kCommonColumnFlags);
      ImGui::TableSetupColumn("Busic $", kCommonColumnFlags);
      ImGui::TableSetupColumn("Coal", kCommonColumnFlags);
      ImGui::TableSetupColumn("Count", kCommonColumnFlags);
      ImGui::TableHeadersRow();

      for (const UnbuiltLinkInfo& info : (*player_info_)->unbuilt_link_info) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        DisplayUnformatted(ToString(info.link_type));
        ImGui::TableNextColumn();
        DisplayUnformatted(info.basic_price);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.coal_cost);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.count);
      }
      ImGui::EndTable();
    }
  }
  ImGui::NewLine();
}

void ImtuiIo::DisplayPlayerMat() {
  if (!IsPlayerInfoReady()) {
    return;
  }
  ImGui::Begin(((*player_info_)->name + "'s Console").c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
  ImGui::Text("Income Level:"); ImGui::SameLine(); DisplayUnformatted((*player_info_)->income_level); ImGui::SameLine(0, inline_elem_spacing);
  ImGui::Text("EXP:"); ImGui::SameLine(); DisplayRatio((*player_info_)->exp, RequiredExpToLevelUp((*player_info_)->income_level)); ImGui::SameLine(0, inline_elem_spacing);
  ImGui::Text("VP:"); ImGui::SameLine(); DisplayUnformatted((*player_info_)->vp); ImGui::SameLine(); DisplayUnformatted("(" + std::to_string((*player_info_)->provisional_vp) + ")"); ImGui::SameLine(0, inline_elem_spacing);
  ImGui::Text("Balance:"); ImGui::SameLine(); DisplayUnformatted((*player_info_)->balance); ImGui::SameLine(0, inline_elem_spacing);
  DisplayActionGroup();
  DisplayCardGroup();
  DisplayPlayerUnbuilt();
  if (action_status_ == ActionStatus::kChooseAction && ImGui::Button("Exit")) {
    ok_status_ = OkFlag::kExit;
  }
  
  if (action_status_ == ActionStatus::kPerformAction) {
    ImGui::SetNextWindowSize(ImVec2(win_width_ / 2, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(win_width_ / 4, 0), ImGuiCond_FirstUseEver);
    switch (action_type_) {
      case ActionType::kBuild:
        DisplayBuildWindow();
        break;
      case ActionType::kNetwork:
        DisplayNetworkWindow();
        break;
      case ActionType::kDevelop:
        DisplayDevelopWindow();
        break;
      case ActionType::kSell:
        DisplaySellWindow();
        break;
      default:
        // No pop-up for other actions
        break;
    }
  }

  if (action_status_ == ActionStatus::kConsumeResources) {
    ImGui::SetNextWindowSize(ImVec2(win_width_ / 2, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(win_width_ / 4, 0), ImGuiCond_FirstUseEver);
    DisplayConsumptionWindow();
  }
  ImGui::End();
}

void ImtuiIo::DisplayMessageWindow() {
  ImGui::Begin("Output", nullptr, ImGuiWindowFlags_NoCollapse);
  for (const std::string& msg : messages_) {
    ImGui::BulletText(msg.c_str());
  }
  ImGui::SetScrollHere(1.0f);
  ImGui::End();
}

void ImtuiIo::DisplayBuildWindow() {
  if (ImGui::Begin("Build Action", nullptr, ImGuiWindowFlags_NoCollapse)) {
    ImGui::Text("Choose an industry to overbuild:"); ImGui::SameLine();
    ImGui::Text("Step"); ImGui::SameLine(); DisplayRatio(1, 1);
    ImGui::NewLine();

    int i = 0;
    if (ImGui::BeginTabBar("industry_info_tab_bar")) {
      for (std::map<IndustryType, std::vector<OverbuiltIndustryInfo> >::const_reference pair : overbuilt_ind_info_) {
        if (ImGui::BeginTabItem(ToString(pair.first).c_str())) {
          build_industry_type_ = pair.first;
          ImGui::Text(("~ " + ToString(pair.first) + " ~").c_str());
          if (ImGui::BeginTable("industry_info", 9, kCommonTableFlags)) {
            ImGui::TableSetupColumn("Location", kCommonColumnFlags);
            ImGui::TableSetupColumn("Owner", kCommonColumnFlags);
            ImGui::TableSetupColumn("Tech", kCommonColumnFlags);
            ImGui::TableSetupColumn("VP", kCommonColumnFlags);
            ImGui::TableSetupColumn("Link VP", kCommonColumnFlags);
            ImGui::TableSetupColumn("Inventory", kCommonColumnFlags);
            ImGui::TableSetupColumn("VP Eligible?", kCommonColumnFlags);
            ImGui::TableSetupColumn("Total $", kCommonColumnFlags);
            ImGui::TableSetupColumn("Overbuild?", kCommonColumnFlags);
            ImGui::TableHeadersRow();

            const std::vector<OverbuiltIndustryInfo>& ind_info = pair.second;
            for (const OverbuiltIndustryInfo& info : ind_info) {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              DisplayUnformatted(info.location);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.owner);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.tech_level);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.vp);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.vp_for_link);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.available_units);
              ImGui::TableNextColumn();
              ImGui::TextUnformatted((info.eligible_for_vp)? "y" : "n");
              ImGui::TableNextColumn();
              DisplayUnformatted(info.total_price);
              ImGui::TableNextColumn();
              ImGui::RadioButton(("(" + std::to_string(i+1) + ")").c_str(), &chosen_industry_index_, i);
              ++i;
            }
            ImGui::EndTable();
          }
          ImGui::EndTabItem();
        }
      }
      ImGui::EndTabBar();
    }

    ImGui::NewLine();
    if (chosen_industry_index_ > kUnchosen
          && chosen_industry_index_ < (int)overbuilt_ind_info_.at(build_industry_type_).size()) {
      if (ImGui::Button("OK")) {
        ok_status_ = OkFlag::kOk;
      }
      ImGui::SameLine();
    }
    if (ImGui::Button("Cancel")) { 
      ok_status_ = OkFlag::kCancel;
      chosen_industry_index_ = kUnchosen;
    }
    ImGui::End();
  }
}

void ImtuiIo::DisplayNetworkWindow() {
  if (ImGui::Begin("Network Action", nullptr, ImGuiWindowFlags_NoCollapse)) {
    ImGui::Text("Choose a Link to overbuild:");
    ImGui::TextColored(kOrange, "Additional cost for 2nd Link: 1 Beer + $5");
    ImGui::NewLine();

    if (ImGui::BeginTable("link_info", 5, kCommonTableFlags)) {
      ImGui::TableSetupColumn("Type", kCommonColumnFlags);
      ImGui::TableSetupColumn("Location", kCommonColumnFlags);
      ImGui::TableSetupColumn("Location", kCommonColumnFlags);
      ImGui::TableSetupColumn("Total $", kCommonColumnFlags);
      ImGui::TableSetupColumn("Overbuild?", kCommonColumnFlags);
      ImGui::TableHeadersRow();

      int i = 0;
      for (const OverbuiltLinkInfo& info : overbuilt_link_info_) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        DisplayUnformatted(info.link_type);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.location_1);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.location_2);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.total_price);
        ImGui::TableNextColumn();
        ImGui::RadioButton(("(" + std::to_string(i+1) + ")").c_str(), &chosen_link_index_, i);
        ++i;
      }
      ImGui::EndTable();
    }

    ImGui::NewLine();
    ImGui::Spacing();
    ImGui::NewLine();
    if (chosen_link_index_ > kUnchosen) {
      ImGui::SameLine();
      if (ImGui::Button("OK")) {
        ok_status_ = OkFlag::kOk;
      }
    }
    if (allow_skip_) {
      ImGui::SameLine();
      if (ImGui::Button("Skip")) { 
        ok_status_ = OkFlag::kSkip;
      }
    }
    if (allow_cancel_) {
      ImGui::SameLine();
      if (ImGui::Button("Cancel")) { 
        ok_status_ = OkFlag::kCancel;
        chosen_link_index_ = kUnchosen;
      }
    }
    ImGui::End();
  }
}

void ImtuiIo::DisplayDevelopWindow() {
  if (ImGui::Begin("Develop Action", nullptr, ImGuiWindowFlags_NoCollapse)) {
    ImGui::TextColored(kYellow, "Choose an industry to develop:"); ImGui::SameLine();
    ImGui::Text("Step"); ImGui::SameLine(); DisplayRatio(discard_step_, max_num_discard_steps_);
    ImGui::Text("This allows you to skip building the least technologically advanced industries.");
    ImGui::TextColored(kOrange, "Cost: 1 iron for each developing industry.");
    ImGui::NewLine();

    if (ImGui::BeginTable("unbuilt_industry_info", 10, kCommonTableFlags)) {
      ImGui::TableSetupColumn("Type", kCommonColumnFlags);
      ImGui::TableSetupColumn("Tech", kCommonColumnFlags);
      ImGui::TableSetupColumn("VP", kCommonColumnFlags);
      ImGui::TableSetupColumn("Link VP", kCommonColumnFlags);
      ImGui::TableSetupColumn("Basic $\n(Build)", kCommonColumnFlags);
      ImGui::TableSetupColumn("Coal\n(Build)", kCommonColumnFlags);
      ImGui::TableSetupColumn("Iron\n(Build)", kCommonColumnFlags);
      ImGui::TableSetupColumn("Output", kCommonColumnFlags);
      ImGui::TableSetupColumn("Beer\n(Sell)", kCommonColumnFlags);
      ImGui::TableSetupColumn("Develop?", kCommonColumnFlags);
      ImGui::TableHeadersRow();

      int i = 0;
      for (const UnbuiltIndustryInfo& info : discarding_ind_info_) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        DisplayUnformatted(ToString(info.industry_type));
        ImGui::TableNextColumn();
        DisplayUnformatted(info.tech_level);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.vp);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.vp_for_link);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.basic_price);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.coal_cost);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.iron_cost);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.output_units);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.beer_cost);
        ImGui::TableNextColumn();
        ImGui::RadioButton(("(" + std::to_string(i+1) + ")").c_str(), &chosen_discarding_industry_index_, i);
        ++i;
      }
      ImGui::EndTable();
    }

    ImGui::NewLine();
    ImGui::Spacing();
    ImGui::NewLine();
    if ((chosen_discarding_industry_index_ > kUnchosen)) {
      ImGui::SameLine();
      if (ImGui::Button("OK")) {
        ok_status_ = OkFlag::kOk;
      }
    }
    if (allow_skip_) {
      ImGui::SameLine();
      if (ImGui::Button("Skip")) { 
        ok_status_ = OkFlag::kSkip;
      }
    }
    if (allow_cancel_) {
      ImGui::SameLine();
      if (ImGui::Button("Cancel")) { 
        ok_status_ = OkFlag::kCancel;
        chosen_discarding_industry_index_ = kUnchosen;
      }
    }
    ImGui::End();
  }
}

void ImtuiIo::DisplaySellWindow() {
  if (ImGui::Begin("Sell Action", nullptr, ImGuiWindowFlags_NoCollapse)) {
    ImGui::Text("Choose an industry to sell:");
    ImGui::Separator();

    if (ImGui::BeginTable("industry_info", 7, kCommonTableFlags)) {
      ImGui::TableSetupColumn("Type", kCommonColumnFlags);
      ImGui::TableSetupColumn("Location", kCommonColumnFlags);
      ImGui::TableSetupColumn("Tech", kCommonColumnFlags);
      ImGui::TableSetupColumn("VP", kCommonColumnFlags);
      ImGui::TableSetupColumn("LVP", kCommonColumnFlags);
      ImGui::TableSetupColumn("Beer", kCommonColumnFlags);
      ImGui::TableSetupColumn("Sell?", kCommonColumnFlags);
      ImGui::TableHeadersRow();

      int i = 0;
      for (const IndustryInfo& info : selling_ind_info_) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        DisplayUnformatted(info.industry_type);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.location);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.tech_level);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.vp);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.vp_for_link);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.beer_cost);
        ImGui::TableNextColumn();
        ImGui::RadioButton("", &chosen_selling_industry_index_, i);
      }
      ImGui::EndTable();
    }

    ImGui::NewLine();
    ImGui::Spacing();
    ImGui::NewLine();
    if (chosen_selling_industry_index_ > kUnchosen) {
      ImGui::SameLine();
      if (ImGui::Button("OK")) {
        ok_status_ = OkFlag::kOk;
      }
    }
    if (allow_skip_) {
      ImGui::SameLine();
      if (ImGui::Button("Skip")) { 
        ok_status_ = OkFlag::kSkip;
      }
    }
    if (allow_cancel_) {
      ImGui::SameLine();
      if (ImGui::Button("Cancel")) { 
        ok_status_ = OkFlag::kCancel;
        chosen_selling_industry_index_ = kUnchosen;
      }
    }
    ImGui::End();
  }
}

void ImtuiIo::DisplayConsumptionWindow() {
  if (ImGui::Begin("Consumption", nullptr, ImGuiWindowFlags_NoCollapse)) {
    ImGui::Text("Choose industries to consume from:");
    ImGui::NewLine();

    bool satisfied = true;
    if (ImGui::BeginTabBar("consumption_info_tab_bar")) {
      for (std::map<ResourceType, ConsumptionInfo>::reference pair : consumption_info_) {
        if (ImGui::BeginTabItem(ToString(pair.first).c_str())) {
          DisplayUnformatted("~ " + ToString(pair.first) + " ~");
          if (ImGui::BeginTable("consume_industry_info", 6, kCommonTableFlags)) {
            ImGui::TableSetupColumn("Location", kCommonColumnFlags);
            ImGui::TableSetupColumn("Owner", kCommonColumnFlags);
            ImGui::TableSetupColumn("VP", kCommonColumnFlags);
            ImGui::TableSetupColumn("Link VP", kCommonColumnFlags);
            ImGui::TableSetupColumn("Inventory", kCommonColumnFlags);
            ImGui::TableSetupColumn("Consume?", kCommonColumnFlags);
            ImGui::TableHeadersRow();

            int i = 0;
            for (std::vector<std::pair<IndustryInfo, int> >::reference info_units : pair.second.chosen_units) {
              const IndustryInfo& info = info_units.first;
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              DisplayUnformatted(info.location);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.owner);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.vp);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.vp_for_link);
              ImGui::TableNextColumn();
              DisplayUnformatted(info.available_units);
              ImGui::TableNextColumn();
              int max_units = std::min(info.available_units, pair.second.industry_units);
              ImGui::DragInt("units", &info_units.second, 1, 0, max_units);
              info_units.second = std::min(info_units.second, max_units);
              info_units.second = std::max(info_units.second, 0);
            }
            ++i;
            ImGui::EndTable();
          }
          int num_units_chosen = 0;
          for (std::vector<std::pair<IndustryInfo, int> >::reference info_units : pair.second.chosen_units) {
            num_units_chosen += info_units.second;
          }
          ImGui::NewLine();
          ImGui::NewLine();
          ImGui::TextUnformatted("Industry units required:"); ImGui::SameLine();
          DisplayRatio(num_units_chosen, pair.second.industry_units);
          DisplayUnformatted(pair.second.market_units); ImGui::SameLine();
          ImGui::TextUnformatted("units will be bought from the market for a total of $"); ImGui::SameLine();
          DisplayUnformatted(pair.second.market_price);
          ImGui::EndTabItem();
        }
      }
      ImGui::EndTabBar();
    }
    ImGui::NewLine();
    ImGui::NewLine();

    std::vector<ResourceType> unsatisfied_resources;
    for (std::map<ResourceType, ConsumptionInfo>::reference pair : consumption_info_) {
      int num_units_chosen = 0;  // we can cache these calculations, but let's forget about that
      for (std::vector<std::pair<IndustryInfo, int> >::reference info_units : pair.second.chosen_units) {
        num_units_chosen += info_units.second;
      }
      if (num_units_chosen != pair.second.industry_units) {
        unsatisfied_resources.push_back(pair.first);
        satisfied = false;
      }
    }
    if (satisfied) {
      if (ImGui::Button("OK")) {
        ok_status_ = OkFlag::kOk;
      }
      ImGui::SameLine();
    } else {
      std::string msg = "The requirements of ";
      for (ResourceType res : unsatisfied_resources) {
        msg += ToString(res) + ", ";
      }
      msg.pop_back();
      msg.pop_back();
      msg += " are not satisifed.";
      ImGui::TextColored(kRed, msg.c_str());
    }

    if (ImGui::Button("Cancel")) {
      ok_status_ = OkFlag::kCancel;
    }
    ImGui::End();
  }
}

void ImtuiIo::DisplayLiquidationWindow() {
  if (ImGui::Begin("Liquidation", nullptr, ImGuiWindowFlags_NoCollapse)) {
    ImGui::Text("Choose industries to liquidate:");

    if (ImGui::BeginTable("industry_info", 9, kCommonTableFlags)) {
      ImGui::TableSetupColumn("Industry Type", kCommonColumnFlags);
      ImGui::TableSetupColumn("Location", kCommonColumnFlags);
      ImGui::TableSetupColumn("Tech", kCommonColumnFlags);
      ImGui::TableSetupColumn("VP", kCommonColumnFlags);
      ImGui::TableSetupColumn("Link VP", kCommonColumnFlags);
      ImGui::TableSetupColumn("Inventory", kCommonColumnFlags);
      ImGui::TableSetupColumn("VP Eligible?", kCommonColumnFlags);
      ImGui::TableSetupColumn("Liquidation $", kCommonColumnFlags);
      ImGui::TableSetupColumn("Liquidate?", kCommonColumnFlags);
      ImGui::TableHeadersRow();

      for (std::vector<IndustryInfo>::size_type i = 0; liquidate_industry_info_.size(); i++) {
        const IndustryInfo& info = liquidate_industry_info_.at(i);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        DisplayUnformatted(info.industry_type);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.location);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.tech_level);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.vp);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.vp_for_link);
        ImGui::TableNextColumn();
        DisplayUnformatted(info.available_units);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted((info.eligible_for_vp)? "y" : "n");
        ImGui::TableNextColumn();
        DisplayUnformatted(liquidation_value_.at(i));
        ImGui::TableNextColumn();
        ImGui::Checkbox(("(" + std::to_string(i+1) + ")").c_str(), &industries_selected_[i]);
      }
      ImGui::EndTable();
    }
    ImGui::NewLine();
    int total_value = 0;
    int new_shortfall = shortfall_;
    bool enough = true;
    bool excessive = false;
    for (std::vector<IndustryInfo>::size_type i = 0; i < liquidation_value_.size(); i++) {
      int value = liquidation_value_.at(i);
      total_value += value;
      new_shortfall -= value;
      if (new_shortfall <= 0) {
        if (!enough) {
          enough = true;
        } else {
          excessive = true;
        }
      }
    }
    ImGui::Text("Amount raised:"); ImGui::SameLine();
    DisplayRatio(total_value, shortfall_);
    if (enough && !excessive) {
      if (ImGui::Button("OK")) {
        ok_status_ = OkFlag::kOk;
      }
    }
    ImGui::End();
  }
}

OkFlag ImtuiIo::ChooseAction(ActionType& action_type) {
  // reset choice
  action_status_ = ActionStatus::kChooseAction;
  ok_status_ = OkFlag::kPending;
  chosen_action_ = kUnchosen;
  // display
  while (ok_status_ == OkFlag::kPending) {
    NewFrame();
  }
  if (ok_status_ == OkFlag::kOk) {
    action_type = kActionTypes[chosen_action_];
  }
  return ok_status_;
}

bool ImtuiIo::ChooseCard(int& index) {
  // reset choice
  action_status_ = ActionStatus::kChooseCard;
  ok_status_ = OkFlag::kPending;
  num_cards_to_choose_ = 1;
  chosen_card_index_ = kUnchosen;
  // display
  while (ok_status_ == OkFlag::kPending) {
    NewFrame();
  }
  if (ok_status_ == OkFlag::kCancel) {
    return false;
  }
  index = chosen_card_index_;
  chosen_card_index_ = kUnchosen;
  return true;
}

bool ImtuiIo::ChooseCards(std::vector<int>& indices,
                          int num_cards) {
  // reset choice
  action_status_ = ActionStatus::kChooseCard;
  ok_status_ = OkFlag::kPending;
  num_cards_to_choose_ = num_cards;
  num_cards_to_choose_ = num_cards;
  num_cards_chosen_ = 0;
  for (int i = 0; i < kMaxCardsInHand; i++) {
    cards_selected_[i] = false;
  }
  // display
  while (ok_status_ == OkFlag::kPending) {
    NewFrame();
  }
  if (ok_status_ == OkFlag::kCancel) {
    return false;
  }
  indices.clear();
  for (int i = 0; i < kMaxCardsInHand; i++) {
    if (cards_selected_[i]) {
      indices.push_back(i);
    }
  }
  return true;
}

bool ImtuiIo::ChooseIndustryToBuild(const std::map<IndustryType, std::vector<OverbuiltIndustryInfo> >& industry_info,
                                    IndustryType& industry_type, int& index) {
  action_status_ = ActionStatus::kPerformAction;
  action_type_ = ActionType::kBuild;
  ok_status_ = OkFlag::kPending;
  build_industry_type_ = IndustryType::kUndefined;
  chosen_industry_index_ = kUnchosen;
  overbuilt_ind_info_ = industry_info;
  while (ok_status_ == OkFlag::kPending) {
    NewFrame();
  }
  if (ok_status_ == OkFlag::kCancel) {
    return false;
  }
  index = chosen_industry_index_;
  industry_type = build_industry_type_;
  return true;
}

OkFlag ImtuiIo::ChooseLinkToBuild(const std::vector<OverbuiltLinkInfo>& link_info,
                                int& index, bool allow_cancel, bool allow_skip) {
  action_status_ = ActionStatus::kPerformAction;
  action_type_ = ActionType::kNetwork;
  ok_status_ = OkFlag::kPending;
  chosen_link_index_ = kUnchosen;
  overbuilt_link_info_ = link_info;
  allow_cancel_ = allow_cancel;
  allow_skip_ = allow_skip;
  while (ok_status_ == OkFlag::kPending) {
    NewFrame();
  }
  if (ok_status_ == OkFlag::kOk) {
    index = chosen_link_index_;
  }
  return ok_status_;
}

void ImtuiIo::Reset() {
  action_status_ = ActionStatus::kChooseAction;
  ok_status_ = OkFlag::kPending;
  chosen_card_index_ = kUnchosen;
  num_cards_chosen_ = 0;
  for (int i = 0; i < kMaxCardsInHand; i++) {
    cards_selected_[i] = false;
  }
}

OkFlag ImtuiIo::ChooseIndustryToDevelop(int step, int max_num_steps,
                                      const std::vector<UnbuiltIndustryInfo>& industry_info,
                                      int& index, bool allow_cancel, bool allow_skip) {
  action_status_ = ActionStatus::kPerformAction;
  action_type_ = ActionType::kDevelop;
  ok_status_ = OkFlag::kPending;
  discard_step_ = step;
  max_num_discard_steps_ = max_num_steps;
  chosen_discarding_industry_index_ = kUnchosen;
  discarding_ind_info_ = industry_info;
  allow_cancel_ = allow_cancel;
  allow_skip_ = allow_skip;
  while (ok_status_ == OkFlag::kPending) {
    NewFrame();
  }
  if (ok_status_ == OkFlag::kOk) {
    index = chosen_discarding_industry_index_;
  }
  return ok_status_;
}

OkFlag ImtuiIo::ChooseIndustryToSell(const std::vector<IndustryInfo>& industry_info,
                                       int& index, bool allow_cancel, bool allow_skip) {
  action_status_ = ActionStatus::kPerformAction;
  action_type_ = ActionType::kSell;
  ok_status_ = OkFlag::kPending;
  chosen_selling_industry_index_ = kUnchosen;
  selling_ind_info_ = industry_info;
  allow_cancel_ = allow_cancel;
  allow_skip_ = allow_skip;
  while (ok_status_ == OkFlag::kPending) {
    NewFrame();
  }
  if (ok_status_ == OkFlag::kOk) {
    index = chosen_selling_industry_index_;
  }
  return ok_status_;
}

bool ImtuiIo::ChooseIndustriesToConsume(std::map<ResourceType, ConsumptionInfo>& consumption_info) {
  action_status_ = ActionStatus::kConsumeResources;
  ok_status_ = OkFlag::kPending;
  consumption_info_ = consumption_info;
  while (ok_status_ == OkFlag::kPending) {
    NewFrame();
  }
  if (ok_status_ == OkFlag::kCancel) {
    return false;
  }
  consumption_info = consumption_info_;
  return true;
}

void ImtuiIo::ChooseIndustriesToLiquidate(const std::vector<IndustryInfo>& industry_info,
                                          int shortfall, const std::vector<int>& liquidation_value,
                                          std::vector<int>& chosen_indices) {
  action_status_ = ActionStatus::kCompletedAction;
  ok_status_ = OkFlag::kPending;
  shortfall_ = shortfall;
  liquidate_industry_info_ = industry_info;
  liquidation_value_ = liquidation_value;
  industries_selected_ = new bool[liquidation_value.size()];
  int size = static_cast<int>(liquidation_value.size());
  while (ok_status_ == OkFlag::kPending) {
    NewFrame();
  }
  for (int i = 0; i < size; i++) {
    if (industries_selected_[i])
      chosen_indices.push_back(i);
  }
  delete industries_selected_;
}

void ImtuiIo::PushMessage(const std::string& msg) {
  messages_.push_back(msg);
}
