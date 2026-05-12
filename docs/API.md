# API HTTP do Controle de Acesso

Este documento descreve as rotas HTTP pensadas para integracao externa com o firmware do controle de acesso do Ramo Estudantil IEEE UFJF.

As paginas web administrativas continuam existindo separadamente. Aqui ficam apenas os endpoints de API que um aplicativo, servidor intermediario ou automacao pode chamar diretamente.

## Base da API

Use o IP atual exibido na tela LCD ou na interface web.

Exemplo:

```text
http://192.168.15.125
```

O servidor HTTP embarcado usa respostas pequenas de proposito, para evitar estouro de pacote no NetX Duo.

## Autenticacao

Todas as rotas `/api/...` exigem a chave configurada na constante `API_KEY`, definida em `src/main.c`.

Envie a chave em um destes formatos:

```http
X-API-KEY: <API_KEY>
```

ou:

```http
Authorization: Bearer <API_KEY>
```

Recomendacao: troque a chave padrao antes de usar o sistema em producao.

Resposta comum quando a chave esta ausente ou incorreta:

```json
{"ok":false,"error":"unauthorized"}
```

## Formatos aceitos

As rotas `POST` aceitam JSON ou formulario `application/x-www-form-urlencoded`, dependendo do endpoint.

Para `POST` sem corpo, envie `Content-Length: 0`. Alguns clientes HTTP omitem esse cabecalho e o NetX Duo pode responder:

```text
NetX HTTP Invalid Content Length
```

Exemplo seguro para POST sem corpo:

```powershell
curl.exe -i -X POST "http://192.168.15.125/api/door/open" `
  -H "X-API-KEY: <API_KEY>" `
  -H "Content-Length: 0"
```

## POST /api/door/open

Solicita a abertura da porta.

O endpoint apenas dispara o pulso de abertura. O fechamento do rele e feito diretamente pela logica de GPIO depois do tempo configurado, sem depender de evento de fechamento na fila.

### Cabecalhos

```http
X-API-KEY: <API_KEY>
Content-Type: application/json
```

### Corpo

O corpo e opcional. Quando enviado, o nome e gravado no log de acesso como responsavel pela abertura via aplicativo.

Campos aceitos:

- `user_name`
- `name`
- `user`

Exemplo JSON:

```json
{
  "user_name": "Aplicativo IEEE"
}
```

Tambem e aceito por query string:

```text
/api/door/open?user_name=Aplicativo%20IEEE
```

### Exemplo curl

```powershell
$body = '{"user_name":"Aplicativo IEEE"}'

curl.exe -i -X POST "http://192.168.15.125/api/door/open" `
  -H "X-API-KEY: <API_KEY>" `
  -H "Content-Type: application/json" `
  --data-binary $body
```

### Resposta de sucesso

```json
{
  "ok": true,
  "message": "Door open command sent.",
  "user_name": "Aplicativo IEEE"
}
```

### Erros

| HTTP | `error` | Causa provavel |
| --- | --- | --- |
| 401 | `unauthorized` | Chave ausente ou incorreta. |
| 409 | `cooldown` | A API recebeu outro comando de abertura muito recentemente. |

## POST /api/meeting/schedule

Agenda uma reuniao futura para ativar o modo reuniao automaticamente.

Durante o modo reuniao, apenas os perfis selecionados podem abrir a porta. Perfis administradores continuam podendo abrir a porta em qualquer instante.

O agendamento e salvo em QSPI no arquivo `meeting.json`. Quando a reuniao inicia, o estado ativo e salvo em `meeting_active.json`, permitindo restauracao depois de reboot se a reuniao ainda nao tiver terminado.

### Cabecalhos

```http
X-API-KEY: <API_KEY>
Content-Type: application/json
```

### Campos obrigatorios

| Campo | Tipo | Descricao |
| --- | --- | --- |
| `meeting_chapter` | string | Texto exibido no LCD abaixo de `MODO REUNIAO`. |
| `start_utc` | string | Inicio em UTC, formato `YYYY-MM-DDTHH:MM:SSZ`. |
| `end_utc` | string | Fim em UTC, formato `YYYY-MM-DDTHH:MM:SSZ`. |
| `profile_indices` ou `profile_names` | array | Participantes autorizados. |

`end_utc` precisa ser maior que `start_utc`. A duracao aceita vai de 1 minuto a 24 horas.

`delay_seconds` nao e aceito. Use sempre data e horario absolutos em UTC.

Em payload de formulario, `chapter` tambem e aceito como alias de `meeting_chapter`.

### Participantes por indice

Use `profile_indices` quando o integrador ja conhece os indices atuais dos perfis.

```json
{
  "meeting_chapter": "RAS",
  "start_utc": "2030-01-01T12:37:00Z",
  "end_utc": "2030-01-01T13:37:00Z",
  "profile_indices": [0, 4, 12]
}
```

Compatibilidade: o campo `profiles` tambem e aceito para lista de indices.

Mesmo quando a entrada usa indices, o firmware salva internamente `profile_key_hashes`, derivadas dos cartoes dos usuarios. Isso evita liberar a pessoa errada caso a ordem da lista de perfis mude depois do agendamento, sem manter listas grandes de UIDs em RAM.

### Participantes por nome e capitulo

Use `profile_names` quando a filtragem vier do sistema externo.

```json
{
  "meeting_chapter": "RAS",
  "start_utc": "2030-01-01T12:37:00Z",
  "end_utc": "2030-01-01T13:37:00Z",
  "profile_names": [
    {
      "name": "Rafael Lago",
      "chapter": "CS"
    },
    {
      "name": "Maria Eduarda de Sa",
      "chapter": "RAS"
    }
  ]
}
```

Aliases aceitos para a mesma lista:

- `profile_names`
- `participants`
- `profiles`

Regras da comparacao:

- `name` e `chapter` precisam existir no cadastro local.
- Espacos no inicio e no fim sao ignorados.
- Acentos, letras, pontuacao e espacos internos precisam bater com o cadastro.
- Se o par `name` + `chapter` nao existir, for ambiguo ou apontar para perfil sem cartao, o agendamento e rejeitado.

### Recorrencia

Sem recorrencia:

```json
{
  "meeting_chapter": "RAS",
  "start_utc": "2030-01-01T12:37:00Z",
  "end_utc": "2030-01-01T13:37:00Z",
  "profile_indices": [0, 4, 12],
  "recurrence": "none"
}
```

Recorrencia diaria:

```json
{
  "meeting_chapter": "RAS",
  "start_utc": "2030-01-01T12:37:00Z",
  "end_utc": "2030-01-01T13:37:00Z",
  "profile_indices": [0, 4, 12],
  "recurrence": "daily"
}
```

Recorrencia semanal:

```json
{
  "meeting_chapter": "RAS",
  "start_utc": "2030-01-01T12:37:00Z",
  "end_utc": "2030-01-01T13:37:00Z",
  "profile_indices": [0, 4, 12],
  "recurrence": "weekly",
  "weekdays": [1, 3, 5]
}
```

Valores aceitos para `recurrence`:

- `none`, `once` ou `0`
- `daily`, `diario`, `diaria` ou `1`
- `weekly`, `semanal` ou `2`

Dias da semana:

- `0`: domingo
- `1`: segunda-feira
- `2`: terca-feira
- `3`: quarta-feira
- `4`: quinta-feira
- `5`: sexta-feira
- `6`: sabado

Tambem e aceito `weekdays_mask`, com os bits `0..6` representando domingo a sabado.

Se `recurrence` for `weekly` e `weekdays` nao for enviado, o firmware usa automaticamente o dia da semana de `start_utc`.

Em recorrencias, a duracao e preservada. Depois de cada execucao, o firmware avanca o proximo `start_utc` e recalcula `end_utc` mantendo o mesmo intervalo.

### Exemplo curl

```powershell
$body = @'
{
  "meeting_chapter": "RAS",
  "start_utc": "2030-01-01T12:37:00Z",
  "end_utc": "2030-01-01T13:37:00Z",
  "profile_indices": [0, 4, 12],
  "recurrence": "weekly",
  "weekdays": [1, 3, 5]
}
'@

curl.exe -i -X POST "http://192.168.15.125/api/meeting/schedule" `
  -H "X-API-KEY: <API_KEY>" `
  -H "Content-Type: application/json" `
  --data-binary $body
```

### Resposta de sucesso

```json
{
  "ok": true,
  "id": 3,
  "pending_count": 1,
  "active": false,
  "time_synced": true,
  "now_utc": "2030-01-01T12:30:00Z",
  "now_unix": 1893501000,
  "start_utc": "2030-01-01T12:37:00Z",
  "start_unix": 1893501420,
  "end_utc": "2030-01-01T13:37:00Z",
  "end_unix": 1893505020,
  "meeting_chapter": "RAS",
  "profile_count": 3,
  "recurrence": "weekly",
  "weekdays_mask": 42
}
```

### Erros

| HTTP | `error` | Causa provavel |
| --- | --- | --- |
| 400 | `empty_body` | Corpo ausente. |
| 400 | `missing_meeting_chapter` | `meeting_chapter` nao foi enviado ou ficou vazio. |
| 400 | `delay_not_supported` | `delay_seconds` foi enviado. |
| 400 | `missing_start` | Inicio ausente. |
| 400 | `invalid_start_utc` | `start_utc` nao esta em formato UTC valido. |
| 400 | `start_in_past` | Inicio igual ou anterior ao horario atual da placa. |
| 400 | `missing_end` | Fim ausente. |
| 400 | `invalid_end_utc` | `end_utc` nao esta em formato UTC valido. |
| 400 | `end_before_start` | Fim menor ou igual ao inicio. |
| 400 | `invalid_duration` | Duracao fora da faixa de 1 minuto a 24 horas. |
| 400 | `invalid_recurrence` | Recorrencia invalida. |
| 400 | `invalid_weekdays` | Dias de recorrencia invalidos. |
| 400 | `invalid_profiles` | Lista de participantes ausente ou invalida. |
| 400 | `invalid_profile_names` | Lista por nome/capitulo esta malformada. |
| 400 | `invalid_profile` | Indice de perfil inexistente. |
| 400 | `profile_without_cards` | Perfil existe, mas nao tem cartao cadastrado. |
| 400 | `unknown_profile_name` | Par `name` + `chapter` nao encontrado. |
| 400 | `ambiguous_profile_name` | Par `name` + `chapter` encontrou mais de um perfil. |
| 400 | `too_many_profiles` | Lista de participantes excedeu o limite aceito. |
| 400 | `invalid_profile_keys` | Nao foi possivel gerar a identidade estavel dos perfis. |
| 401 | `unauthorized` | Chave ausente ou incorreta. |
| 409 | `schedule_storage_unavailable` | Falha ao carregar a fila persistida de agendamentos. |
| 409 | `schedule_full` | Limite de 8 agendamentos pendentes atingido. |
| 500 | `schedule_save_failed` | Falha ao salvar `meeting.json` na QSPI. |

## POST /api/meeting/cancel

Cancela agendamentos pendentes.

Nao encerra uma reuniao que ja esta ativa. Para encerrar uma reuniao ativa, use a interface web em `/meeting_mode_stop`.

### Cancelar um agendamento especifico

```powershell
$body = '{"id":3}'

curl.exe -i -X POST "http://192.168.15.125/api/meeting/cancel" `
  -H "X-API-KEY: <API_KEY>" `
  -H "Content-Type: application/json" `
  --data-binary $body
```

Tambem e aceito por query string:

```powershell
curl.exe -i -X POST "http://192.168.15.125/api/meeting/cancel?id=3" `
  -H "X-API-KEY: <API_KEY>" `
  -H "Content-Length: 0"
```

### Cancelar todos os agendamentos pendentes

```powershell
curl.exe -i -X POST "http://192.168.15.125/api/meeting/cancel" `
  -H "X-API-KEY: <API_KEY>" `
  -H "Content-Length: 0"
```

### Resposta de sucesso

```json
{
  "ok": true,
  "canceled_count": 1,
  "pending_count": 0,
  "active": false
}
```

### Erros

| HTTP | `error` | Causa provavel |
| --- | --- | --- |
| 400 | `invalid_id` | `id` enviado em formato invalido ou zero. |
| 401 | `unauthorized` | Chave ausente ou incorreta. |
| 409 | `schedule_storage_unavailable` | Falha ao carregar a fila persistida de agendamentos. |
| 500 | `schedule_save_failed` | Falha ao salvar a fila atualizada em QSPI. |

## GET /api/meeting/active

Consulta rapida para saber se o modo reuniao esta ativo.

### Exemplo curl

```powershell
curl.exe -i "http://192.168.15.125/api/meeting/active" `
  -H "X-API-KEY: <API_KEY>"
```

### Resposta

```json
{
  "ok": true,
  "active": true,
  "time_synced": true,
  "now_utc": "2030-01-01T12:45:00Z",
  "now_unix": 1893501900,
  "meeting_chapter": "RAS",
  "end_utc": "2030-01-01T13:37:00Z",
  "end_unix": 1893505020,
  "remaining_seconds": 3120,
  "selected_profiles": 3,
  "allowed_cards": 5
}
```

Campos importantes:

- `active`: indica se o modo reuniao esta ativo.
- `time_synced`: indica se o horario UTC da placa esta sincronizado.
- `meeting_chapter`: texto exibido no LCD.
- `remaining_seconds`: tempo restante, quando ativo e com horario sincronizado.
- `selected_profiles`: quantidade de perfis selecionados.
- `allowed_cards`: quantidade de cartoes autorizados para a reuniao.

## GET /api/meeting/status

Consulta detalhada do estado do modo reuniao e dos agendamentos pendentes.

Use este endpoint para telas administrativas. Para aplicativo que so precisa saber se a reuniao esta ativa, prefira `/api/meeting/active`, que e menor.

### Exemplo curl

```powershell
curl.exe -i "http://192.168.15.125/api/meeting/status" `
  -H "X-API-KEY: <API_KEY>"
```

### Resposta

```json
{
  "ok": true,
  "active": false,
  "pending_count": 1,
  "time_synced": true,
  "now_utc": "2030-01-01T12:30:00Z",
  "now_unix": 1893501000,
  "active_meeting_chapter": "",
  "active_end_utc": "1970-01-01T00:00:00Z",
  "active_end_unix": 0,
  "active_selected_profiles": 0,
  "active_allowed_cards": 0,
  "last_id": 3,
  "last_start_utc": "2030-01-01T12:37:00Z",
  "last_start_unix": 1893501420,
  "last_end_utc": "2030-01-01T13:37:00Z",
  "last_end_unix": 1893505020,
  "last_selected_profiles": 0,
  "last_allowed_cards": 0,
  "last_status": "scheduled",
  "schedules": [
    {
      "id": 3,
      "start_utc": "2030-01-01T12:37:00Z",
      "start_unix": 1893501420,
      "end_utc": "2030-01-01T13:37:00Z",
      "end_unix": 1893505020,
      "meeting_chapter": "RAS",
      "profile_count": 3,
      "recurrence": "weekly",
      "weekdays_mask": 42
    }
  ]
}
```

### Possiveis `last_status`

| Valor | Significado |
| --- | --- |
| `idle` | Sem acao recente. |
| `loaded` | Agendamentos carregados da QSPI. |
| `scheduled` | Novo agendamento salvo. |
| `starting` | Reuniao pendente esta sendo iniciada. |
| `started` | Reuniao iniciada com sucesso. |
| `failed` | Tentativa de iniciar reuniao falhou. |
| `save_failed` | Falha ao salvar agendamentos. |
| `canceled` | Um ou mais agendamentos foram cancelados. |

### Erros

| HTTP | `error` | Causa provavel |
| --- | --- | --- |
| 401 | `unauthorized` | Chave ausente ou incorreta. |
| 409 | `schedule_storage_unavailable` | Falha ao carregar a fila persistida de agendamentos. |
| 500 | `status_too_large` | A resposta detalhada nao coube no buffer interno. Use `/api/meeting/active` ou reduza a quantidade de agendamentos pendentes. |

## Observacoes operacionais

- O firmware usa UTC internamente. Converta horario local para UTC antes de chamar a API.
- O NTP precisa sincronizar para que reunioes agendadas sejam executadas no horario correto.
- O limite atual e de 8 agendamentos pendentes.
- O limite atual e de 100 usuarios.
- Cada usuario pode ter ate 4 cartoes.
- Agendamentos novos sao persistidos em QSPI com `profile_key_hashes`, derivadas dos UIDs dos cartoes, para evitar dependencia da posicao do perfil na lista.
- Os campos legados `profile_keys` e `profiles` ainda sao aceitos na leitura dos arquivos persistidos; `profiles` por indice tambem segue aceito nas chamadas de agendamento.
- Logs de acesso e metricas sao gravados de forma incremental em QSPI e passam por retencao temporal de 7 dias.
- Administradores podem abrir a porta mesmo com modo reuniao ativo.
