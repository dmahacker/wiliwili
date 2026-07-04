#include "fragment/settings_shortcuts_data.hpp"

#include "utils/shortcut_helper.hpp"
#include "utils/base_shortcut_helper.hpp"

namespace {
const std::vector<ShortcutAction> EDITOR_ACTIONS = {
    ShortcutAction::Confirm,
    ShortcutAction::Back,
    ShortcutAction::NavigateUp,
    ShortcutAction::NavigateDown,
    ShortcutAction::NavigateLeft,
    ShortcutAction::NavigateRight,
    ShortcutAction::Refresh,
    ShortcutAction::Search,
    ShortcutAction::Last,
    ShortcutAction::Next,
    ShortcutAction::LastSub,
    ShortcutAction::NextSub,
    ShortcutAction::VolumeUp,
    ShortcutAction::VolumeDown,
    ShortcutAction::Danmaku,
    ShortcutAction::VideoProfile,
    ShortcutAction::VideoQuality,
    ShortcutAction::VideoSpeed,
    ShortcutAction::Playlist,
    ShortcutAction::Setting,
    ShortcutAction::VideoSpeedUp,
    ShortcutAction::Forward,
    ShortcutAction::Rewind,
    ShortcutAction::VideoOsd,
    ShortcutAction::VideoPause,
};
}

const std::vector<ShortcutAction>& shortcutEditorActions() { return EDITOR_ACTIONS; }

brls::BrlsKeyCombination shortcutEditorCurrentKey(ShortcutAction action) {
    switch (action) {
        case ShortcutAction::Confirm:
            return BaseShortcutHelper::getConfirm();
        case ShortcutAction::Back:
            return BaseShortcutHelper::getBack();
        case ShortcutAction::NavigateUp:
            return BaseShortcutHelper::getNavigateUp();
        case ShortcutAction::NavigateDown:
            return BaseShortcutHelper::getNavigateDown();
        case ShortcutAction::NavigateLeft:
            return BaseShortcutHelper::getNavigateLeft();
        case ShortcutAction::NavigateRight:
            return BaseShortcutHelper::getNavigateRight();
        case ShortcutAction::Refresh:
            return ShortcutHelper::getRefresh();
        case ShortcutAction::Search:
            return ShortcutHelper::getSearch();
        case ShortcutAction::Last:
            return ShortcutHelper::getLast();
        case ShortcutAction::Next:
            return ShortcutHelper::getNext();
        case ShortcutAction::LastSub:
            return ShortcutHelper::getLastSub();
        case ShortcutAction::NextSub:
            return ShortcutHelper::getNextSub();
        case ShortcutAction::VolumeUp:
            return ShortcutHelper::getVolumeUp();
        case ShortcutAction::VolumeDown:
            return ShortcutHelper::getVolumeDown();
        case ShortcutAction::VideoProfile:
            return ShortcutHelper::getVideoProfile();
        case ShortcutAction::Danmaku:
            return ShortcutHelper::getDanmaku();
        case ShortcutAction::Playlist:
            return ShortcutHelper::getPlaylist();
        case ShortcutAction::Forward:
            return ShortcutHelper::getForward();
        case ShortcutAction::Rewind:
            return ShortcutHelper::getRewind();
        case ShortcutAction::Setting:
            return ShortcutHelper::getSetting();
        case ShortcutAction::VideoQuality:
            return ShortcutHelper::getVideoQuality();
        case ShortcutAction::VideoSpeed:
            return ShortcutHelper::getVideoSpeed();
        case ShortcutAction::VideoSpeedUp:
            return ShortcutHelper::getVideoSpeedUp();
        case ShortcutAction::VideoOsd:
            return ShortcutHelper::getVideoOsd();
        case ShortcutAction::VideoPause:
            return ShortcutHelper::getVideoPause();
    }
    return {brls::BRLS_KBD_KEY_UNKNOWN};
}

ShortcutBinding shortcutEditorCurrentBinding(ShortcutAction action) {
    SettingItem item{};
    if (shortcutEditorSettingItem(action, item)) {
        const std::string config = ProgramConfig::instance().getSettingItem(item, shortcutEditorDefaultConfigKey(action));
        ShortcutBinding binding = ShortcutBindingHelper::parseBinding(config);
        if (binding.device != ShortcutDevice::Unsupported) return binding;
    }

    ShortcutBinding binding;
    binding.key = shortcutEditorCurrentKey(action);
    if (binding.key.code != brls::BRLS_KBD_KEY_UNKNOWN) {
        binding.device = ShortcutDevice::Keyboard;
    }
    return binding;
}

bool shortcutEditorSettingItem(ShortcutAction action, SettingItem& item) {
    switch (action) {
        case ShortcutAction::Confirm:
            item = SettingItem::SHORTCUT_CONFIRM;
            return true;
        case ShortcutAction::Back:
            item = SettingItem::SHORTCUT_BACK;
            return true;
        case ShortcutAction::NavigateUp:
            item = SettingItem::SHORTCUT_NAVIGATE_UP;
            return true;
        case ShortcutAction::NavigateDown:
            item = SettingItem::SHORTCUT_NAVIGATE_DOWN;
            return true;
        case ShortcutAction::NavigateLeft:
            item = SettingItem::SHORTCUT_NAVIGATE_LEFT;
            return true;
        case ShortcutAction::NavigateRight:
            item = SettingItem::SHORTCUT_NAVIGATE_RIGHT;
            return true;
        case ShortcutAction::Refresh:
            item = SettingItem::SHORTCUT_REFRESH;
            return true;
        case ShortcutAction::Search:
            item = SettingItem::SHORTCUT_SEARCH;
            return true;
        case ShortcutAction::Last:
            item = SettingItem::SHORTCUT_LAST;
            return true;
        case ShortcutAction::Next:
            item = SettingItem::SHORTCUT_NEXT;
            return true;
        case ShortcutAction::LastSub:
            item = SettingItem::SHORTCUT_LAST_SUB;
            return true;
        case ShortcutAction::NextSub:
            item = SettingItem::SHORTCUT_NEXT_SUB;
            return true;
        case ShortcutAction::VolumeUp:
            item = SettingItem::SHORTCUT_VOLUME_UP;
            return true;
        case ShortcutAction::VolumeDown:
            item = SettingItem::SHORTCUT_VOLUME_DOWN;
            return true;
        case ShortcutAction::VideoProfile:
            item = SettingItem::SHORTCUT_VIDEO_PROFILE;
            return true;
        case ShortcutAction::Danmaku:
            item = SettingItem::SHORTCUT_DANMAKU;
            return true;
        case ShortcutAction::Playlist:
            item = SettingItem::SHORTCUT_PLAYLIST;
            return true;
        case ShortcutAction::Forward:
            item = SettingItem::SHORTCUT_FORWARD;
            return true;
        case ShortcutAction::Rewind:
            item = SettingItem::SHORTCUT_REWIND;
            return true;
        case ShortcutAction::Setting:
            item = SettingItem::SHORTCUT_SETTING;
            return true;
        case ShortcutAction::VideoQuality:
            item = SettingItem::SHORTCUT_VIDEO_QUALITY;
            return true;
        case ShortcutAction::VideoSpeed:
            item = SettingItem::SHORTCUT_VIDEO_SPEED;
            return true;
        case ShortcutAction::VideoSpeedUp:
            item = SettingItem::SHORTCUT_VIDEO_SPEEDUP;
            return true;
        case ShortcutAction::VideoOsd:
            item = SettingItem::SHORTCUT_VIDEO_OSD;
            return true;
        case ShortcutAction::VideoPause:
            item = SettingItem::SHORTCUT_VIDEO_PAUSE;
            return true;
    }
    return false;
}

std::string shortcutEditorActionLabelKey(ShortcutAction action) {
    switch (action) {
        case ShortcutAction::Confirm:
            return "shortcuts/actions/confirm";
        case ShortcutAction::Back:
            return "shortcuts/actions/back";
        case ShortcutAction::NavigateUp:
            return "shortcuts/actions/navigate_up";
        case ShortcutAction::NavigateDown:
            return "shortcuts/actions/navigate_down";
        case ShortcutAction::NavigateLeft:
            return "shortcuts/actions/navigate_left";
        case ShortcutAction::NavigateRight:
            return "shortcuts/actions/navigate_right";
        case ShortcutAction::Refresh:
            return "shortcuts/actions/refresh";
        case ShortcutAction::Search:
            return "shortcuts/actions/search";
        case ShortcutAction::Last:
            return "shortcuts/actions/last";
        case ShortcutAction::Next:
            return "shortcuts/actions/next";
        case ShortcutAction::LastSub:
            return "shortcuts/actions/last_sub";
        case ShortcutAction::NextSub:
            return "shortcuts/actions/next_sub";
        case ShortcutAction::VolumeUp:
            return "shortcuts/actions/volume_up";
        case ShortcutAction::VolumeDown:
            return "shortcuts/actions/volume_down";
        case ShortcutAction::Danmaku:
            return "shortcuts/actions/danmaku";
        case ShortcutAction::VideoProfile:
            return "shortcuts/actions/video_profile";
        case ShortcutAction::VideoQuality:
            return "shortcuts/actions/video_quality";
        case ShortcutAction::VideoSpeed:
            return "shortcuts/actions/video_speed";
        case ShortcutAction::Playlist:
            return "shortcuts/actions/playlist";
        case ShortcutAction::Setting:
            return "shortcuts/actions/setting";
        case ShortcutAction::VideoSpeedUp:
            return "shortcuts/actions/video_speed_up";
        case ShortcutAction::Forward:
            return "shortcuts/actions/forward";
        case ShortcutAction::Rewind:
            return "shortcuts/actions/rewind";
        case ShortcutAction::VideoOsd:
            return "shortcuts/actions/video_osd";
        case ShortcutAction::VideoPause:
            return "shortcuts/actions/video_pause";
    }
    return "shortcuts/actions/unknown";
}

std::string shortcutEditorDefaultConfigKey(ShortcutAction action) {
    switch (action) {
        case ShortcutAction::Confirm:
            return "enter";
        case ShortcutAction::Back:
            return "escape";
        case ShortcutAction::NavigateUp:
            return "up";
        case ShortcutAction::NavigateDown:
            return "down";
        case ShortcutAction::NavigateLeft:
            return "left";
        case ShortcutAction::NavigateRight:
            return "right";
        case ShortcutAction::Refresh:
#ifdef __APPLE__
            return "meta-r";
#else
            return "ctrl-r";
#endif
        case ShortcutAction::Search:
#ifdef __APPLE__
            return "meta-f";
#else
            return "ctrl-f";
#endif
        case ShortcutAction::Last:
            return "pgup";
        case ShortcutAction::Next:
            return "pgdn";
        case ShortcutAction::LastSub:
            return "shift-pgup";
        case ShortcutAction::NextSub:
            return "shift-pgdn";
        case ShortcutAction::VolumeUp:
            return "0";
        case ShortcutAction::VolumeDown:
            return "9";
        case ShortcutAction::VideoProfile:
            return "f1";
        case ShortcutAction::Danmaku:
            return "d";
        case ShortcutAction::Playlist:
            return "f4";
        case ShortcutAction::Forward:
            return "]";
        case ShortcutAction::Rewind:
            return "[";
        case ShortcutAction::Setting:
            return "f5";
        case ShortcutAction::VideoQuality:
            return "f2";
        case ShortcutAction::VideoSpeed:
            return "f3";
        case ShortcutAction::VideoSpeedUp:
            return "p";
        case ShortcutAction::VideoOsd:
            return "o";
        case ShortcutAction::VideoPause:
            return "space";
    }
    return {};
}

ShortcutBinding shortcutEditorDefaultBinding(ShortcutAction action) {
    return ShortcutBindingHelper::parseBinding(shortcutEditorDefaultConfigKey(action));
}
