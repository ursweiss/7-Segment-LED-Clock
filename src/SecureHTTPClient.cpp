/*
 * This file is part of the 7 Segment LED Clock Project
 *   https://github.com/ursweiss/7-Segment-LED-Clock
 *   https://www.printables.com/model/68013-7-segment-led-clock
 *
 * Copyright (c) 2021-2025 Urs Weiss
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "SecureHTTPClient.h"
#include "Logger.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

SecureHTTPClient::Response SecureHTTPClient::get(const char* url, unsigned long timeout, bool followRedirects) {
  Response response;
  response.success = false;
  response.httpCode = 0;
  response.payload = "";
  response.error = "";

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setTimeout(timeout);

  if (followRedirects) {
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  }

  if (!http.begin(client, url)) {
    response.error = "Failed to initialize HTTPS connection";
    LOG_ERROR(response.error.c_str());
    return response;
  }

  response.httpCode = http.GET();

  if (response.httpCode == HTTP_CODE_OK) {
    response.payload = http.getString();
    response.success = true;
  } else if (response.httpCode > 0) {
    response.error = "HTTP error code: " + String(response.httpCode);
  } else {
    response.error = "Request failed: " + http.errorToString(response.httpCode);
  }

  http.end();
  return response;
}
