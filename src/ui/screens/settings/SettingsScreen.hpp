#pragma once

#include <functional>

#include "ui/framework/screen/Screen.hpp"

#include "settings/SettingPersistence.hpp"
#include "settings/RuntimeSettings.hpp"

// Include related screens to enable navigation
#include "WifiSettingsScreen.hpp"
#include "NameSettingScreen.hpp"
#include "PlottingSettingsScreen.hpp"
#include "HomingSettingsScreen.hpp"
#include "SteppersSettingsScreen.hpp"
#include "PenServoSettingsScreen.hpp"
#include "PenSlotsScreen.hpp"
#include "ui/framework/router/Router.hpp"

// Include components and widgets used in this screen
#include "ui/components/HeaderLine.hpp"
#include "ui/components/PressHoldButton.hpp"
#include "ui/framework/widgets/leaves/Button.hpp"
#include "ui/framework/widgets/leaves/Label.hpp"
#include "ui/framework/widgets/layouts/LinearLayout.hpp"

#include "ui/styles/ButtonStyles.hpp"
#include "ui/screens/ScreensContext.hpp"

namespace ui {
namespace screens {

class SettingsScreen : public Screen
{
public:
    SettingsScreen(const ScreensContext& ctx)
    : Screen(
        std::make_unique<widgets::LinearLayout>(
            widgets::LinearLayoutStyle{.axis = widgets::Axis::Vertical},

            std::make_unique<components::HeaderLine>(components::HeaderLineProps{
                .textProvider = "Settings",
                .wifiStatusProvider = ctx.wifiStatusProvider,
                .onBackPress = [this]() {
                    if (router()) router()->popScreen();
                }
            }),

            std::make_unique<widgets::ScrollableVerticalLayout>(
                widgets::ScrollableVerticalLayoutStyle{},

                std::make_unique<widgets::Button>(
                    widgets::ButtonProps{
                        .style = styles::listButtonStyle,
                        .onPress = [this, ctx]() {
                            if (router()) {
                                auto nameSettingScreen = std::make_unique<NameSettingScreen>(ctx);
                                router()->pushScreen(std::move(nameSettingScreen));
                            }
                        }
                    },
                    std::make_unique<widgets::Label>("Name & DNS")
                ),

                /*

                currently hidden as the hardware for pen slots is not yet implemented
                
                std::make_unique<widgets::Button>(
                    widgets::ButtonProps{
                        .style = styles::listButtonStyle,
                        .onPress = [this, ctx]() {
                            if (router()) {
                                auto penSlotsScreen = std::make_unique<PenSlotsScreen>(ctx);
                                router()->pushScreen(std::move(penSlotsScreen));
                            }
                        }
                    },
                    std::make_unique<widgets::Label>("Pen Slots")
                ),

                */
                
                std::make_unique<widgets::Button>(
                    widgets::ButtonProps{
                        .style = styles::listButtonStyle,
                        .onPress = [this, ctx]() {
                            if (router()) {
                                auto wifiSettingsScreen = std::make_unique<WifiSettingsScreen>(ctx);
                                router()->pushScreen(std::move(wifiSettingsScreen));
                            }
                        }
                    },
                    std::make_unique<widgets::Label>("Wifi")
                ),

                std::make_unique<widgets::Button>(
                    widgets::ButtonProps{
                        .style = styles::listButtonStyle,
                        .onPress = [this, ctx]() {
                            if (router()) {
                                auto plottingSettingsScreen = std::make_unique<PlottingSettingsScreen>(ctx);
                                router()->pushScreen(std::move(plottingSettingsScreen));
                            }
                        }
                    },
                    std::make_unique<widgets::Label>("Plotting Speeds")
                ),

                std::make_unique<widgets::Button>(
                    widgets::ButtonProps{
                        .style = styles::listButtonStyle,
                        .onPress = [this, ctx]() {
                            if (router()) {
                                auto homingSettingsScreen = std::make_unique<HomingSettingsScreen>(ctx);
                                router()->pushScreen(std::move(homingSettingsScreen));
                            }
                        }
                    },
                    std::make_unique<widgets::Label>("Homing")
                ),

                std::make_unique<widgets::Button>(
                    widgets::ButtonProps{
                        .style = styles::listButtonStyle,
                        .onPress = [this, ctx]() {
                            if (router()) {
                                auto steppersSettingsScreen = std::make_unique<SteppersSettingsScreen>(ctx);
                                router()->pushScreen(std::move(steppersSettingsScreen));
                            }
                        }
                    },
                    std::make_unique<widgets::Label>("Steppers")
                ),

                std::make_unique<widgets::Button>(
                    widgets::ButtonProps{
                        .style = styles::listButtonStyle,
                        .onPress = [this, ctx]() {
                            if (router()) {
                                auto penSettingsScreen = std::make_unique<PenServoScreen>(ctx);
                                router()->pushScreen(std::move(penSettingsScreen));
                            }
                        }
                    },
                    std::make_unique<widgets::Label>("Pen Servo")
                ),

                std::make_unique<components::PressHoldButton>(
                    components::PressHoldButtonProps{
                        .onHoldComplete = [&sp = ctx.settingsPersistence]() {
                            sp.clearAllSettings();
                        },
                        .holdDurationMs = 3000,
                    },
                    std::make_unique<widgets::Label>("Reset All")
                )
            )
        )
    )
    {}
};

} // namespace screens
} // namespace ui