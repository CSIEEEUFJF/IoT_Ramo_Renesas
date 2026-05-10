# Node Door Relay

Relay HTTP em Node.js para abrir a porta da placa sem expor diretamente a `API_KEY` do firmware no navegador.

## O que ele faz

- expõe `POST /door/open` no PC/servidor onde o Node estiver rodando
- valida um token próprio do relay
- encaminha a chamada para `POST /api/door/open` da placa
- devolve JSON com o status do upstream
- responde `OPTIONS` para facilitar uso a partir de JavaScript no navegador

## Estrutura

- `server.js`: servidor HTTP do relay
- `.env.example`: variáveis de ambiente necessárias
- `package.json`: scripts básicos

## Configuração

Copie `.env.example` para `.env` e ajuste os valores:

- `PORT`: porta do relay
- `DEVICE_URL`: URL da placa, por exemplo `http://192.168.11.2/api/door/open`
- `DEVICE_API_KEY`: a `API_KEY` configurada no firmware
- `RELAY_TOKEN`: token que o cliente vai usar para falar com o relay
- `ALLOW_ORIGIN`: origem liberada para CORS, ou `*`
- `REQUEST_TIMEOUT_MS`: timeout da chamada até a placa

## Execução

```powershell
cd C:\Users\CS\Documents\IoT_Ramo_Renesas\node-door-relay
copy .env.example .env
node server.js
```

Ou com `npm`:

```powershell
cd C:\Users\CS\Documents\IoT_Ramo_Renesas\node-door-relay
npm start
```

## Rotas

### `GET /health`

Resposta esperada:

```json
{"ok":true,"service":"node-door-relay"}
```

### `POST /door/open`

Autenticação aceita:

- `X-Relay-Token: <token>`
- `Authorization: Bearer <token>`

Exemplo com `fetch` no navegador:

```js
await fetch("http://SEU-PC:8080/door/open", {
  method: "POST",
  headers: {
    "X-Relay-Token": "troque-esta-chave"
  }
});
```

Exemplo com `fetch` em Node.js:

```js
const response = await fetch("http://127.0.0.1:8080/door/open", {
  method: "POST",
  headers: {
    "Authorization": "Bearer troque-esta-chave"
  }
});

console.log(await response.json());
```

Resposta típica:

```json
{
  "ok": true,
  "upstreamStatus": 200,
  "upstreamBody": "{\"ok\":true,\"message\":\"Door open command sent.\"}"
}
```

## Observações de segurança

- troque a `DEVICE_API_KEY` padrão do firmware antes de usar fora de testes
- troque também o `RELAY_TOKEN`
- se o uso for apenas local, prefira definir `ALLOW_ORIGIN` para o domínio exato do seu frontend
