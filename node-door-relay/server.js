"use strict";

const fs = require("node:fs");
const http = require("node:http");
const https = require("node:https");
const path = require("node:path");
const { URL } = require("node:url");

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
  const config = {
    port: parseInteger(env.PORT, 8080),
    deviceUrl: env.DEVICE_URL || "",
    deviceApiKey: env.DEVICE_API_KEY || "",
    relayToken: env.RELAY_TOKEN || "",
    allowOrigin: env.ALLOW_ORIGIN || "*",
    requestTimeoutMs: parseInteger(env.REQUEST_TIMEOUT_MS, 5000)
  };

  if (!config.deviceUrl) {
    throw new Error("DEVICE_URL nao definido.");
  }

  if (!config.deviceApiKey) {
    throw new Error("DEVICE_API_KEY nao definido.");
  }

  if (!config.relayToken) {
    throw new Error("RELAY_TOKEN nao definido.");
  }

  return config;
}

function parseInteger(value, fallbackValue) {
  const parsed = Number.parseInt(value, 10);
  if (!Number.isFinite(parsed) || parsed <= 0) {
    return fallbackValue;
  }

  return parsed;
}

function setCorsHeaders(response, allowOrigin) {
  response.setHeader("Access-Control-Allow-Origin", allowOrigin);
  response.setHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
  response.setHeader("Access-Control-Allow-Headers", "Content-Type, X-Relay-Token, Authorization");
}

function writeJson(response, statusCode, payload, allowOrigin) {
  const body = JSON.stringify(payload);

  response.statusCode = statusCode;
  response.setHeader("Content-Type", "application/json; charset=utf-8");
  response.setHeader("Content-Length", Buffer.byteLength(body));
  setCorsHeaders(response, allowOrigin);
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

function drainRequest(request) {
  return new Promise((resolve, reject) => {
    request.on("error", reject);
    request.on("end", resolve);
    request.resume();
  });
}

function forwardDoorOpen(config) {
  return new Promise((resolve, reject) => {
    const target = new URL(config.deviceUrl);
    const transport = target.protocol === "https:" ? https : http;
    const requestOptions = {
      protocol: target.protocol,
      hostname: target.hostname,
      port: target.port || (target.protocol === "https:" ? 443 : 80),
      path: `${target.pathname}${target.search}`,
      method: "POST",
      headers: {
        "X-API-KEY": config.deviceApiKey,
        "Accept": "application/json",
        "Content-Length": "0"
      }
    };

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
    upstreamRequest.end();
  });
}

function createServer(config) {
  return http.createServer(async (request, response) => {
    const requestUrl = new URL(request.url || "/", `http://${request.headers.host || "localhost"}`);

    if (request.method === "OPTIONS") {
      response.statusCode = 204;
      setCorsHeaders(response, config.allowOrigin);
      response.end();
      return;
    }

    if ((request.method === "GET") && (requestUrl.pathname === "/health")) {
      writeJson(response, 200, { ok: true, service: "node-door-relay" }, config.allowOrigin);
      return;
    }

    if ((request.method === "POST") && (requestUrl.pathname === "/door/open")) {
      try {
        await drainRequest(request);
      } catch (error) {
        writeJson(response, 400, { ok: false, error: "invalid_request", detail: error.message }, config.allowOrigin);
        return;
      }

      if (!requestIsAuthorized(request, config.relayToken)) {
        writeJson(response, 401, { ok: false, error: "unauthorized" }, config.allowOrigin);
        return;
      }

      try {
        const upstream = await forwardDoorOpen(config);
        const success = upstream.statusCode >= 200 && upstream.statusCode < 300;
        const payload = {
          ok: success,
          upstreamStatus: upstream.statusCode,
          upstreamBody: upstream.body
        };

        writeJson(response, success ? 200 : 502, payload, config.allowOrigin);
      } catch (error) {
        writeJson(response, 502, { ok: false, error: "relay_failed", detail: error.message }, config.allowOrigin);
      }

      return;
    }

    writeJson(response, 404, { ok: false, error: "not_found" }, config.allowOrigin);
  });
}

function startServer(config) {
  const server = createServer(config);

  server.listen(config.port, () => {
    console.log(`[node-door-relay] ouvindo em http://0.0.0.0:${config.port}`);
    console.log(`[node-door-relay] encaminhando para ${config.deviceUrl}`);
  });

  return server;
}

if (require.main === module) {
  try {
    startServer(loadConfig());
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
