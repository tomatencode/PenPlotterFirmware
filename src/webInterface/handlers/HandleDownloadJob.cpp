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

    // Stream the file content
    uint8_t buffer[512];
    size_t bytesRead;
    while ((bytesRead = file.read(buffer, sizeof(buffer))) > 0)
    {
        _httpServer.client().write(buffer, bytesRead);
    }
    file.close();
}