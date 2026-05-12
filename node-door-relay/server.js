"use strict";

const fs = require("node:fs");
const http = require("node:http");
const https = require("node:https");
const path = require("node:path");
const { URL } = require("node:url");

const MAX_DOOR_OPEN_BODY_BYTES = 512;

loadDotEnv();

function loadDotEnv(filePath = path.join(__dirname, ".env")) {
  if (!fs.existsSync(filePath)) {
    return;
  }

  const content = fs.readFileSync(filePath, "utf8");
  const lines = content.split(/\r?\n/);

  for (const rawLine of lines) {
    const line = rawLine.trim();
    const separatorIndex = line.indexOf("=");

    if (!line || line.startsWith("#") || separatorIndex <= 0) {
      continue;
    }

    const key = line.slice(0, separatorIndex).trim();
    const value = line.slice(separatorIndex + 1).trim();

    if (!(key in process.env)) {
      process.env[key] = value;
    }
  }
}

function loadConfig(env = process.env) {
  const deviceUrl = parseDeviceUrl(env.DEVICE_URL || "");
  const config = {
    port: parseIntegerInRange(env.PORT, 8080, 1, 65535, "PORT"),
    host: env.HOST || "127.0.0.1",
    deviceUrl: deviceUrl.toString(),
    deviceApiKey: env.DEVICE_API_KEY || "",
    relayToken: env.RELAY_TOKEN || "",
    allowOrigins: parseAllowOrigins(env.ALLOW_ORIGIN || ""),
    requestTimeoutMs: parseIntegerInRange(env.REQUEST_TIMEOUT_MS, 5000, 500, 30000, "REQUEST_TIMEOUT_MS"),
    openCooldownMs: parseIntegerInRange(env.OPEN_COOLDOWN_MS, 2000, 0, 60000, "OPEN_COOLDOWN_MS"),
    exposeUpstreamBody: env.EXPOSE_UPSTREAM_BODY === "1"
  };

  if (!config.deviceApiKey) {
    throw new Error("DEVICE_API_KEY nao definido.");
  }
  if (isPlaceholderSecret(config.deviceApiKey)) {
    throw new Error("DEVICE_API_KEY ainda esta com valor de exemplo.");
  }

  if (!config.relayToken) {
    throw new Error("RELAY_TOKEN nao definido.");
  }
  if (isPlaceholderSecret(config.relayToken)) {
    throw new Error("RELAY_TOKEN ainda esta com valor de exemplo.");
  }

  return config;
}

function isPlaceholderSecret(value) {
  const placeholders = new Set([
    "troque-pela-api-key-da-placa",
    "gere-um-token-longo-e-aleatorio",
    "troque-esta-chave"
  ]);

  return placeholders.has(value);
}

function normalizeRuntimeConfig(config) {
  return {
    ...config,
    host: config.host || "127.0.0.1",
    allowOrigins: Array.isArray(config.allowOrigins) ? config.allowOrigins : [],
    requestTimeoutMs: Number.isFinite(config.requestTimeoutMs) ? config.requestTimeoutMs : 5000,
    openCooldownMs: Number.isFinite(config.openCooldownMs) ? config.openCooldownMs : 2000,
    exposeUpstreamBody: config.exposeUpstreamBody === true
  };
}

function parseDeviceUrl(value) {
  if (!value) {
    throw new Error("DEVICE_URL nao definido.");
  }

  const parsed = new URL(value);
  if ((parsed.protocol !== "http:") && (parsed.protocol !== "https:")) {
    throw new Error("DEVICE_URL deve usar http ou https.");
  }

  return parsed;
}

function parseAllowOrigins(value) {
  return value
    .split(",")
    .map((origin) => origin.trim())
    .filter(Boolean);
}

function parseIntegerInRange(value, fallbackValue, minValue, maxValue, name) {
  const parsed = Number.parseInt(value, 10);
  if (!Number.isFinite(parsed)) {
    return fallbackValue;
  }

  if ((parsed < minValue) || (parsed > maxValue)) {
    throw new Error(`${name} deve ficar entre ${minValue} e ${maxValue}.`);
  }

  return parsed;
}

function originIsAllowed(origin, allowOrigins) {
  if (!origin || allowOrigins.length === 0) {
    return false;
  }

  return allowOrigins.includes("*") || allowOrigins.includes(origin);
}

function setCorsHeaders(request, response, config) {
  const origin = request.headers.origin;

  if (!originIsAllowed(origin, config.allowOrigins)) {
    return false;
  }

  response.setHeader("Access-Control-Allow-Origin", config.allowOrigins.includes("*") ? "*" : origin);
  response.setHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
  response.setHeader("Access-Control-Allow-Headers", "Content-Type, X-Relay-Token, Authorization");
  response.setHeader("Vary", "Origin");
  return true;
}

function writeJson(request, response, statusCode, payload, config) {
  const body = JSON.stringify(payload);

  response.statusCode = statusCode;
  response.setHeader("Content-Type", "application/json; charset=utf-8");
  response.setHeader("Content-Length", Buffer.byteLength(body));
  setCorsHeaders(request, response, config);
  response.end(body);
}

function requestIsAuthorized(request, relayToken) {
  const relayHeader = request.headers["x-relay-token"];
  const authHeader = request.headers.authorization;

  if ((typeof relayHeader === "string") && (relayHeader === relayToken)) {
    return true;
  }

  if (typeof authHeader === "string") {
    const prefix = "Bearer ";
    if (authHeader.startsWith(prefix)) {
      return authHeader.slice(prefix.length).trim() === relayToken;
    }
  }

  return false;
}

function readRequestBody(request, maxBytes) {
  return new Promise((resolve, reject) => {
    let body = "";
    let receivedBytes = 0;

    request.setEncoding("utf8");
    request.on("data", (chunk) => {
      receivedBytes += Buffer.byteLength(chunk);
      if (receivedBytes > maxBytes) {
        reject(new Error("body_too_large"));
        request.destroy();
        return;
      }
      body += chunk;
    });
    request.on("end", () => resolve(body));
    request.on("error", reject);
  });
}

function parseDoorOpenPayload(body, requestUrl) {
  const payload = {};
  const userFromQuery = requestUrl.searchParams.get("user_name") ||
    requestUrl.searchParams.get("name") ||
    requestUrl.searchParams.get("user");

  if (userFromQuery) {
    payload.user_name = userFromQuery.trim();
  }

  if (!body) {
    return payload;
  }

  const trimmedBody = body.trim();
  if (!trimmedBody) {
    return payload;
  }

  if (!trimmedBody.startsWith("{")) {
    const formBody = new URLSearchParams(trimmedBody);
    const userFromForm = formBody.get("user_name") ||
      formBody.get("name") ||
      formBody.get("user");

    payload.user_name = (userFromForm || trimmedBody).trim();
    return payload;
  }

  let parsedBody;
  try {
    parsedBody = JSON.parse(trimmedBody);
  } catch (error) {
    throw new Error("invalid_json");
  }

  if ((typeof parsedBody !== "object") || (parsedBody === null) || Array.isArray(parsedBody)) {
    throw new Error("invalid_json");
  }

  const userFromBody = parsedBody.user_name || parsedBody.name || parsedBody.user;
  if (typeof userFromBody === "string") {
    payload.user_name = userFromBody.trim();
  }

  return payload;
}

function payloadHasBody(payload) {
  return !!(payload && typeof payload.user_name === "string" && payload.user_name.length > 0);
}

function requestHasNoBody(request) {
  const contentLength = request.headers["content-length"];
  const transferEncoding = request.headers["transfer-encoding"];

  if (typeof transferEncoding === "string" && transferEncoding.length > 0) {
    return false;
  }

  if (typeof contentLength === "undefined") {
    return true;
  }

  return Number.parseInt(contentLength, 10) === 0;
}

function forwardDoorOpen(config, payload) {
  return new Promise((resolve, reject) => {
    const target = new URL(config.deviceUrl);
    const transport = target.protocol === "https:" ? https : http;
    const upstreamBody = payloadHasBody(payload) ? JSON.stringify({ user_name: payload.user_name }) : "";
    const requestOptions = {
      protocol: target.protocol,
      hostname: target.hostname,
      port: target.port || (target.protocol === "https:" ? 443 : 80),
      path: `${target.pathname}${target.search}`,
      method: "POST",
      headers: {
        "X-API-KEY": config.deviceApiKey,
        "Accept": "application/json",
        "Content-Length": Buffer.byteLength(upstreamBody)
      }
    };

    if (upstreamBody.length > 0) {
      requestOptions.headers["Content-Type"] = "application/json";
    }

    const upstreamRequest = transport.request(requestOptions, (upstreamResponse) => {
      let responseBody = "";

      upstreamResponse.setEncoding("utf8");
      upstreamResponse.on("data", (chunk) => {
        if (responseBody.length < 8192) {
          responseBody += chunk;
        }
      });
      upstreamResponse.on("end", () => {
        resolve({
          statusCode: upstreamResponse.statusCode || 502,
          body: responseBody
        });
      });
    });

    upstreamRequest.setTimeout(config.requestTimeoutMs, () => {
      upstreamRequest.destroy(new Error("Tempo limite ao chamar a placa."));
    });

    upstreamRequest.on("error", reject);
    upstreamRequest.end(upstreamBody);
  });
}

function createServer(config) {
  const runtimeConfig = normalizeRuntimeConfig(config);
  let lastDoorOpenAt = 0;
  let doorOpenInFlight = false;

  return http.createServer(async (request, response) => {
    let requestUrl;

    try {
      requestUrl = new URL(request.url || "/", "http://localhost");
    } catch (error) {
      writeJson(request, response, 400, { ok: false, error: "invalid_request" }, runtimeConfig);
      return;
    }

    if (request.method === "OPTIONS") {
      if ((requestUrl.pathname !== "/door/open") || !setCorsHeaders(request, response, runtimeConfig)) {
        writeJson(request, response, 403, { ok: false, error: "origin_not_allowed" }, runtimeConfig);
        return;
      }

      response.statusCode = 204;
      response.end();
      return;
    }

    if ((request.method === "GET") && (requestUrl.pathname === "/health")) {
      writeJson(request, response, 200, { ok: true, service: "node-door-relay" }, runtimeConfig);
      return;
    }

    if ((request.method === "POST") && (requestUrl.pathname === "/door/open")) {
      let payload;

      if (!requestIsAuthorized(request, runtimeConfig.relayToken)) {
        writeJson(request, response, 401, { ok: false, error: "unauthorized" }, runtimeConfig);
        return;
      }

      try {
        const body = requestHasNoBody(request)
          ? ""
          : await readRequestBody(request, MAX_DOOR_OPEN_BODY_BYTES);
        payload = parseDoorOpenPayload(body, requestUrl);
      } catch (error) {
        const statusCode = error.message === "body_too_large" ? 413 : 400;
        writeJson(request, response, statusCode, { ok: false, error: error.message || "invalid_body" }, runtimeConfig);
        return;
      }

      if (doorOpenInFlight) {
        writeJson(request, response, 429, { ok: false, error: "open_in_progress" }, runtimeConfig);
        return;
      }

      if ((runtimeConfig.openCooldownMs > 0) && ((Date.now() - lastDoorOpenAt) < runtimeConfig.openCooldownMs)) {
        writeJson(request, response, 429, { ok: false, error: "cooldown_active" }, runtimeConfig);
        return;
      }

      doorOpenInFlight = true;
      try {
        const upstream = await forwardDoorOpen(runtimeConfig, payload);
        const success = upstream.statusCode >= 200 && upstream.statusCode < 300;
        const responsePayload = {
          ok: success,
          upstreamStatus: upstream.statusCode
        };

        if (success) {
          lastDoorOpenAt = Date.now();
        }

        if (runtimeConfig.exposeUpstreamBody) {
          responsePayload.upstreamBody = upstream.body;
        }

        writeJson(request, response, success ? 200 : 502, responsePayload, runtimeConfig);
      } catch (error) {
        console.warn(`[node-door-relay] falha ao chamar a placa: ${error.message}`);
        writeJson(request, response, 502, { ok: false, error: "relay_failed" }, runtimeConfig);
      } finally {
        doorOpenInFlight = false;
      }

      return;
    }

    writeJson(request, response, 404, { ok: false, error: "not_found" }, runtimeConfig);
  });
}

function startServer(config) {
  const runtimeConfig = normalizeRuntimeConfig(config);
  const server = createServer(runtimeConfig);

  server.requestTimeout = runtimeConfig.requestTimeoutMs + 1000;
  server.headersTimeout = 5000;
  server.keepAliveTimeout = 5000;

  server.listen(runtimeConfig.port, runtimeConfig.host, () => {
    const address = server.address();
    const actualPort = (address && typeof address === "object") ? address.port : runtimeConfig.port;

    console.log(`[node-door-relay] ouvindo em http://${runtimeConfig.host}:${actualPort}`);
    console.log(`[node-door-relay] encaminhando para ${runtimeConfig.deviceUrl}`);
  });

  return server;
}

if (require.main === module) {
  try {
    const server = startServer(loadConfig());
    server.on("error", (error) => {
      console.error(`[node-door-relay] erro do servidor: ${error.message}`);
      process.exitCode = 1;
    });
  } catch (error) {
    console.error(`[node-door-relay] falha ao iniciar: ${error.message}`);
    process.exitCode = 1;
  }
}

module.exports = {
  createServer,
  forwardDoorOpen,
  loadConfig,
  requestIsAuthorized,
  startServer
};
