# Node Door Relay

Relay HTTP em Node.js para abrir a porta da placa sem expor diretamente a `API_KEY` do firmware no navegador.

## O que ele faz

- expõe `POST /door/open` no PC/servidor onde o Node estiver rodando
- valida um token próprio do relay
- encaminha a chamada para `POST /api/door/open` da placa
- devolve JSON com o status do upstream
- responde `OPTIONS` apenas para origens CORS explicitamente liberadas

## Estrutura

- `server.js`: servidor HTTP do relay
- `.env.example`: variáveis de ambiente necessárias
- `package.json`: scripts básicos

## Configuração

Copie `.env.example` para `.env` e ajuste os valores:

- `PORT`: porta do relay
- `HOST`: interface de rede onde o relay escuta; por padrão, `127.0.0.1`
- `DEVICE_URL`: URL da placa, por exemplo `http://192.168.11.2/api/door/open`
- `DEVICE_API_KEY`: a `API_KEY` configurada no firmware
- `RELAY_TOKEN`: token que o cliente vai usar para falar com o relay
- `ALLOW_ORIGIN`: origens liberadas para CORS, separadas por virgula; vazio desativa CORS
- `REQUEST_TIMEOUT_MS`: timeout da chamada até a placa
- `OPEN_COOLDOWN_MS`: intervalo mínimo entre comandos de abertura
- `EXPOSE_UPSTREAM_BODY`: use `1` apenas para diagnostico local, pois expoe a resposta da placa

Os valores de exemplo de `DEVICE_API_KEY` e `RELAY_TOKEN` sao recusados no startup. Use valores reais antes de iniciar o relay.

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

Autenticacao aceita:

- `X-Relay-Token: <token>`
- `Authorization: Bearer <token>`

Exemplo recomendado a partir de outro backend ou script Node.js:

```js
const response = await fetch("http://127.0.0.1:8080/door/open", {
  method: "POST",
  headers: {
    "Authorization": "Bearer gere-um-token-longo-e-aleatorio"
  }
});

console.log(await response.json());
```

Exemplo no navegador, somente se esse navegador fizer parte de uma aplicacao confiavel e `ALLOW_ORIGIN` estiver restrito ao dominio dela:

```env
HOST=0.0.0.0
ALLOW_ORIGIN=http://origem-exata-do-frontend
```

```js
await fetch("http://SEU-PC:8080/door/open", {
  method: "POST",
  headers: {
    "X-Relay-Token": "gere-um-token-longo-e-aleatorio"
  }
});
```

Resposta típica:

```json
{
  "ok": true,
  "upstreamStatus": 200
}
```

## Observações de segurança

- troque a `DEVICE_API_KEY` padrão do firmware antes de usar fora de testes
- troque também o `RELAY_TOKEN`
- mantenha `HOST=127.0.0.1` quando o relay rodar atras de outro backend ou proxy local
- se precisar expor na rede, use firewall, HTTPS/reverse proxy e token forte
- nao coloque o `RELAY_TOKEN` em paginas publicas ou codigo distribuido ao navegador
- se precisar usar CORS, defina `ALLOW_ORIGIN` para o dominio exato do frontend
