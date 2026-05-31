#pragma once

#include "ui/framework/screen/Screen.hpp"

#include "jobController/JobController.hpp"
#include "jobController/JobObserver.hpp"

// Include related screens to enable navigation
#include "ui/framework/router/Router.hpp"

// Include components and widgets used in this screen
#include "ui/components/HeaderLine.hpp"
#include "ui/framework/widgets/leaves/Button.hpp"
#include "ui/framework/widgets/leaves/Label.hpp"
#include "ui/framework/widgets/leaves/ProgressBar.hpp"
#include "ui/framework/widgets/leaves/Conditional.hpp"
#include "ui/screens/ScreensContext.hpp"

namespace ui {
namespace screens {

class PlottingScreen : public Screen, public JobObserver
{
public:
    PlottingScreen(const ScreensContext& ctx)
    : Screen(
        std::make_unique<widgets::LinearLayout>(
            widgets::LinearLayoutStyle{.axis = widgets::Axis::Vertical, .horizontalAlign = widgets::HorizontalAlignment::Center},
            std::make_unique<components::HeaderLine>(components::HeaderLineProps{
                .textProvider = [&jc = ctx.jobController]() {
                    std::string filename = jc.getCurrentFile();
                    if (filename.ends_with(".gcode")) {
                        filename = filename.substr(0, filename.length() - 6);
                    }
                    if (filename == "") {
                        filename = "No Job";
                    }
                    return filename;
                },
                .wifiStatusProvider = ctx.wifiStatusProvider,
                .onBackPress = [this]() {
                    if (router()) router()->popScreen();
                }
            }),

            std::make_unique<widgets::ProgressBar>(widgets::ProgressBarProps{
                .getProgress = [&jc = ctx.jobController]() {
                    return jc.getProgress();
                }
            }),


            std::make_unique<widgets::Label>([&jc = ctx.jobController]() {
                return "-" + std::to_string(jc.getTimeRemainingSeconds()) + "s";
            }),

            std::make_unique<widgets::Conditional>(
                widgets::ConditionalProps{
                    .condition = [&jc = ctx.jobController]() {
                        return jc.isActive();
                    }
                },
                std::make_unique<widgets::LinearLayout>(
                    widgets::LinearLayoutStyle{.axis = widgets::Axis::Horizontal, .spacingMode = widgets::SpacingMode::SpaceAround},

                    std::make_unique<widgets::Button>(
                        widgets::ButtonProps{
                            .onPress = [&jc = ctx.jobController, &ms = ctx.motionState]() {
                                if (ms.getState() == MotionStateType::PAUSED) {
                                    jc.resume();
                                } else {
                                    jc.pause();
                                }
                            }
                        },
                        std::make_unique<widgets::Label>([&jc = ctx.jobController, &ms = ctx.motionState]() {
                            return ms.getState() == MotionStateType::PAUSED ? "Resume" : "Pause";
                        })
                    ),

                    std::make_unique<widgets::Button>(
                        widgets::ButtonProps{
                            .onPress = [&jc = ctx.jobController]() { jc.abort(); }
                        },
                        std::make_unique<widgets::Label>("Abort")
                    )
                )
            )
        )
    , 1)/*start with pause button focused*/, _jobController(ctx.jobController)
    {
        ctx.jobController.registerObserver(this);
    }

    ~PlottingScreen() {
        _jobController.unregisterObserver(this);
    }

    void onJobEvent(const JobEventType& event) override {
        if (event.type == JobEvent::ABORTED) {
            if (router()) {  
                router()->popScreen();
            }
        }
    }

private:
    JobController& _jobController;
};

} // namespace screens
} // namespace ui