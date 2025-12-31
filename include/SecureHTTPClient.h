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

#ifndef SECURE_HTTP_CLIENT_H
#define SECURE_HTTP_CLIENT_H

#include <Arduino.h>

/**
 * Utility class for making secure HTTPS requests
 * Centralizes TLS configuration and error handling
 */
class SecureHTTPClient {
public:
  struct Response {
    bool success;
    int httpCode;
    String payload;
    String error;
  };

  /**
   * Perform a GET request to the specified URL
   * @param url The HTTPS URL to request
   * @param timeout Request timeout in milliseconds (default: 5000ms)
   * @param followRedirects Whether to follow HTTP redirects (default: false)
   * @return Response structure with success status, HTTP code, payload, and error message
   */
  static Response get(const char* url, unsigned long timeout = 5000, bool followRedirects = false);
};

#endif // SECURE_HTTP_CLIENT_H
