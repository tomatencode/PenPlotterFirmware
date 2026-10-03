#include "../WebInterface.hpp"
#include "config/DirectoriesConfig.hpp"

static const char* TAG = "WebInterface";

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

    _httpServer.setContentLength(file.size());
    _httpServer.sendHeader("Content-Disposition", ("attachment; filename=\"" + filename + "\"").c_str());
    _httpServer.send(200, "application/octet-stream", "");

    const size_t expected = file.size();
    size_t totalSent = 0;
    uint8_t buffer[1460];                 // 1× TCP MSS; still safe on the task stack
    WiFiClient client = _httpServer.client();

    while (true)
    {
        if (!client.connected())
            break;                        // peer aborted (app cancels superseded downloads)

        size_t bytesRead = file.read(buffer, sizeof(buffer));
        if (bytesRead == 0)
            break;                        // EOF

        // write() may return a short count — never drop bytes silently.
        size_t written = 0;
        while (written < bytesRead)
        {
            size_t n = client.write(buffer + written, bytesRead - written);
            if (n == 0)
            {
                // Peer gone or socket wedged: kill the connection so the client
                // sees a clean truncation error (and retries) instead of waiting
                // forever for the promised Content-Length bytes.
                client.stop();
                file.close();
                return;
            }
            written += n;
        }
        totalSent += written;

        vTaskDelay(1);                    // keep: feeds the idle-task watchdog
    }
    file.close();

    if (totalSent != expected)
        client.stop();
}