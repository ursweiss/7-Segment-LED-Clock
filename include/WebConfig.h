#ifndef WEBCONFIG_H
#define WEBCONFIG_H

#include <ESPAsyncWebServer.h>

// Initialize web configuration server
bool initWebConfig(AsyncWebServer* server);

// Start mDNS service
bool startMDNS(const char* hostname);

// Check if restart was requested
bool isRestartRequested();

#endif // WEBCONFIG_H
