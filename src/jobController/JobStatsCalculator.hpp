#pragma once
#include <Arduino.h>
#include <FS.h>
#include <string>
#include <vector>

#include "storage/FileManager.hpp"
#include "settings/RuntimeSettings.hpp"

struct PlotJobStats {
    uint32_t totalLines;
    uint32_t totalTimeSeconds;
};

inline PlotJobStats calculateStats(FileManager& fileManager, RuntimeSettings& runtimesettings, const std::string& filename) {
    PlotJobStats stats{0, 0};
    File file = fileManager.openFileRead(filename);
    if (!file) {
        return stats;
    }

    // Read through the file once to count total lines and extract stats for time estimation
    if  (file.available()) {
        String stats_line = file.readStringUntil('\n');

        unsigned int travel_mm, draw_mm, pen_lifts;
        if (sscanf(stats_line.c_str(), "; STATS: travel_mm=%u draw_mm=%u pen_lifts=%u",
                &travel_mm, &draw_mm, &pen_lifts) == 3) {
            
            if (runtimesettings.travelFeedRate_mm_per_s() == 0 || runtimesettings.drawFeedRate_mm_per_s() == 0) {
                ESP_LOGW("JobStatsCalculator", "Feed rates cannot be zero, defaulting to 0 total time");
                stats.totalTimeSeconds = 0;
            } else {

                stats.totalTimeSeconds = static_cast<uint32_t>(
                    (travel_mm / runtimesettings.travelFeedRate_mm_per_s()) +
                    (draw_mm / runtimesettings.drawFeedRate_mm_per_s()) +
                    (pen_lifts * runtimesettings.penLiftTime_s() * 2) +
                    10 // homing time
                );
            }
        } else {
            ESP_LOGW("JobStatsCalculator", "Failed to parse stats line, defaulting to 0 total time");
            stats.totalTimeSeconds = 0;
        }
        stats.totalLines++;
    }

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        if (line.length() > 0 && line[0] != ';') { // Skip empty lines and comment lines
            stats.totalLines++;
        }
    }

    file.close();
    return stats;
}