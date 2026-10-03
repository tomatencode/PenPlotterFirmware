#include "../WebInterface.hpp"
#include "config/DirectoriesConfig.hpp"
#include <esp_log.h>

static const char* TAG = "HandleDownload";
static constexpr size_t DOWNLOAD_BUFFER_SIZE = 4096;

// WebServer handlers run synchronously on SystemTask, so this is not shared
// between concurrent requests. Keeping it static avoids exhausting that task's
// 6 KiB stack while the filesystem and TCP call stacks are active.
static uint8_t downloadBuffer[DOWNLOAD_BUFFER_SIZE];


void WebInterface::handleDownloadJob() {
    if (!_httpServer.hasArg("file"))
    {
        _httpServer.send(400, "text/plain", "Missing 'file' parameter");
        return;
    }

    std::string filename = _httpServer.arg("file").c_str();

    if (!validateFileName(filename) || !filename.ends_with(".gcode"))
    {
        _httpServer.send(400, "text/plain", "Invalid filename");
        return;
    }

    if (!_fileManager.fileExists(PLOTTING_DIRECTORY + filename))
    {
        _httpServer.send(404, "text/plain", "File not found");
        return;
    }

    File file = _fileManager.openFileRead(PLOTTING_DIRECTORY + filename);
    if (!file)
    {
        _httpServer.send(500, "text/plain", "Failed to open file");
        return;
    }

    const size_t expected = file.size();
    _httpServer.setContentLength(expected);
    _httpServer.sendHeader("Content-Disposition", ("attachment; filename=\"" + filename + "\"").c_str());
    _httpServer.send(200, "application/octet-stream", "");

    // handleClient() sets this to HTTP_MAX_SEND_WAIT (5 s) before dispatching.
    // A multi-MB body legitimately takes longer, so widen it for this request.
    WiFiClient client = _httpServer.client();
    client.setTimeout(0); // non-blocking: we do our own waiting below

    size_t totalSent = 0;
    bool aborted = false;
    unsigned long lastYieldMs = millis();

    while (totalSent < expected)
    {
        const size_t want = (expected - totalSent) < DOWNLOAD_BUFFER_SIZE
                          ? (expected - totalSent)
                          : DOWNLOAD_BUFFER_SIZE;

        const size_t bytesRead = file.read(downloadBuffer, want);
        if (bytesRead == 0)
            break; // EOF (file shorter than advertised)

        size_t written = 0;
        while (written < bytesRead)
        {
            const size_t n = client.write(downloadBuffer + written, bytesRead - written);
            if (n > 0)
            {
                written += n;
                continue;
            }

            if (!client.connected())
            {
                // Genuinely gone: the app cancelled this preview, or the host
                // dropped us. Stop reading the file.
                aborted = true;
                break;
            }

            vTaskDelay(1); // socket full — back off, stay connected
        }

        if (aborted)
            break;

        totalSent += written;

        // Yield in ~10 ms batches. Frequent enough to stay far inside the 5 s
        // watchdog, rare enough that throughput no longer scales with delay.
        const unsigned long now = millis();
        if (now - lastYieldMs >= 10)
        {
            vTaskDelay(1);
            lastYieldMs = millis();
        }
    }

    file.close();

    if (totalSent < expected && !client.connected())
    {
        ESP_LOGW(TAG, "Download of \"%s\" aborted by peer at %u/%u bytes",
                 filename.c_str(), (unsigned)totalSent, (unsigned)expected);
        return;
    }

    if (totalSent < expected)
    {
        ESP_LOGW(TAG, "Download of \"%s\" truncated at %u/%u bytes",
                 filename.c_str(), (unsigned)totalSent, (unsigned)expected);
        client.stop();
    }
}