#pragma once

#include <functional>
#include <map>
#include <string>

#include "ui/framework/screen/Screen.hpp"

#include "jobController/JobController.hpp"

// Include related screens to enable navigation
#include "ui/framework/router/Router.hpp"
#include "PlottingScreen.hpp"

// Include components and widgets used in this screen
#include "ui/components/HeaderLine.hpp"
#include "ui/framework/widgets/leaves/Button.hpp"
#include "ui/framework/widgets/leaves/Label.hpp"
#include "ui/framework/widgets/leaves/Switch.hpp"
#include "ui/components/PressHoldButton.hpp"
#include "ui/screens/ScreensContext.hpp"

#include "config/DirectoriesConfig.hpp"

#include "jobController/JobStatsCalculator.hpp"

namespace ui {
namespace screens {

namespace {
    std::string formatFileSize(size_t bytes) {
        char buf[32];
        if (bytes < 1024)                      snprintf(buf, sizeof(buf), "%u B",    (unsigned)bytes);
        else if (bytes < 1024 * 1024)          snprintf(buf, sizeof(buf), "%.2f KB", bytes / 1024.0);
        else if (bytes < 1024 * 1024 * 1024)   snprintf(buf, sizeof(buf), "%.2f MB", bytes / (1024.0 * 1024));
        else                                   snprintf(buf, sizeof(buf), "%.2f GB", bytes / (1024.0 * 1024 * 1024));
        return std::string(buf);
    }

    std::string formatPlotTime(size_t seconds) {
        size_t mins = seconds / 60;
        size_t secs = seconds % 60;
        return std::to_string(mins) + "m " + std::to_string(secs) + "s";
    }
}

class FileDetailsScreen : public Screen
{
public:
    FileDetailsScreen(const std::string& filename, const ScreensContext& ctx)
    : Screen(
        std::make_unique<widgets::Switch<bool>>(
            widgets::SwitchProps{
                .selector = [filename, &fm = ctx.fileManager]() { return fm.fileExists(PLOTTING_DIRECTORY + filename); },
                .evaluationMode = widgets::SwitchEvaluationMode::Lazy
            },
            std::make_unique<widgets::Switch<bool>::Branch>(
                true, 
                std::make_unique<widgets::LinearLayout>(
                    widgets::LinearLayoutStyle{.axis = widgets::Axis::Vertical},

                    std::make_unique<components::HeaderLine>(components::HeaderLineProps{
                        .textProvider = filename.substr(0, filename.length() - 6),
                        .wifiStatusProvider = ctx.wifiStatusProvider,
                        .onBackPress = [this]() {
                            if (router()) router()->popScreen();
                        }
                    }),

                    std::make_unique<widgets::Spacer>(1, 1),

                    std::make_unique<widgets::LinearLayout>(
                        widgets::LinearLayoutStyle{
                            .axis = widgets::Axis::Horizontal,
                            .spacingMode = widgets::SpacingMode::SpaceBetween,
                            .marginLeft = 1,
                            .marginRight = 1
                        },

                        std::make_unique<widgets::LinearLayout>(
                            widgets::LinearLayoutStyle{
                                .axis = widgets::Axis::Vertical,
                                .horizontalAlign = widgets::HorizontalAlignment::Center
                            },
                            
                            std::make_unique<widgets::Button>(
                                widgets::ButtonProps{
                                    .onPress = [filename, &jc = ctx.jobController]() {
                                        jc.start(filename);
                                    }
                                },
                                std::make_unique<widgets::Label>("Plot")
                            ),
                            std::make_unique<components::PressHoldButton>(
                                components::PressHoldButtonProps{
                                    .onHoldComplete = [this, filename, &fm = ctx.fileManager]() {
                                        fm.deleteFile(PLOTTING_DIRECTORY + filename);
                                        if (router()) {
                                            router()->popScreen();
                                        }
                                    },
                                    .holdDurationMs = 2000,
                                },
                                std::make_unique<widgets::Label>("Delete")
                            )
                        ),
                        
                        std::make_unique<widgets::LinearLayout>(
                            widgets::LinearLayoutStyle{
                                .axis = widgets::Axis::Vertical,
                                .horizontalAlign = widgets::HorizontalAlignment::Center,
                            },

                            std::make_unique<widgets::Label>([filename, &fm = ctx.fileManager]() {
                                size_t size = fm.getFileSize(PLOTTING_DIRECTORY + filename);
                                return formatFileSize(size);
                            }),

                            std::make_unique<widgets::Label>([filename, ctx]() {
                                size_t plotTimeSeconds = calculateStats(ctx.fileManager, ctx.runtimeSettings, PLOTTING_DIRECTORY + filename).totalTimeSeconds;
                                return formatPlotTime(plotTimeSeconds);
                            })
                        )
                    )
                )
            ),

            std::make_unique<widgets::Switch<bool>::Branch>(
                false,
                std::make_unique<widgets::LinearLayout>(
                    widgets::LinearLayoutStyle{.axis = widgets::Axis::Vertical},

                    std::make_unique<components::HeaderLine>(components::HeaderLineProps{
                        .textProvider = filename.substr(0, filename.length() - 6),
                        .wifiStatusProvider = ctx.wifiStatusProvider,
                        .onBackPress = [this]() {
                            if (router()) router()->popScreen();
                        }
                    }),

                    std::make_unique<widgets::Label>("File does not exist!")
                )
            )
        )
    , 1)
    {}

    void onUnPause() override {
        if (router()) {
            // this screen is not interesting when returning to it,
            // so pop it to return to the file list
            router()->popScreen();
        }
    }
};

} // namespace screens
} // namespace ui