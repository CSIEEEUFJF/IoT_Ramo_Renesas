# Projeto e Implementação de uma Plataforma Embarcada de Controle de Acesso com Ethernet baseada em Microcontrolador Renesas Synergy

Documentacao principal do projeto `IoTRamoRenesas`.

Ultima revisao desta documentacao: `2026-05-09`

## 1. Visao geral

Este projeto implementa um sistema de controle de acesso para a placa **Renesas SK-S7G2**, com:

- leitura de cartoes RFID por RC522
- interface local no display TFT com touch
- autenticacao de usuarios e abertura de porta
- administracao por interface web local
- persistencia de perfis, fotos e logs em QSPI/FileX
- sincronizacao de horario por NTP para registrar logs com timestamp real

O projeto foi evoluido em cima de uma base sensivel a memoria, boot e FileX. Por isso, varias decisoes atuais priorizam estabilidade e isolamento de responsabilidades.

## 2. Estado atual

Funciona hoje:

- RFID com autenticacao e cadastro local basico
- display, touch e UI local
- painel web administrativo
- perfis com nome, cargo, capitulo, foto e multiplos cartoes
- upload web de foto em alta resolucao runtime com persistencia em QSPI
- persistencia automatica dos perfis na QSPI
- log de acesso persistido na QSPI
- DHCP para obter IP automaticamente
- sincronizacao de relogio por NTP
- API HTTP protegida por token para abertura remota da porta

Pontos importantes do estado atual:

- a rede opera em modo `HTTP only`
- a autenticacao admin local e web aceita PINs de perfis administradores
- o PIN `1234` continua ativo como fallback padrao
- o fluxo de porta registra somente abertura, nao fechamento
- a tela reduz o brilho visual para cerca de `30%` apos `60s` de inatividade
- os reles de porta e luz sao configurados explicitamente como GPIO no boot

## 3. Hardware e perifericos

### 3.1 Plataforma

- Placa: `Renesas SK-S7G2`
- RTOS: `ThreadX`
- Rede/IP/HTTP: `NetX / NetX Duo`
- Sistema de arquivos: `FileX`
- Armazenamento persistente: `QSPI`

### 3.2 RFID

O RC522 esta ligado por SPI por software.

Mapeamento atual em [`src/rfid.c`](./src/rfid.c):

- `D10 -> SS`  (`P507`)
- `D9  -> RST` (`P506`)
- `D11 -> MOSI` (`P105`)
- `D12 -> MISO` (`P104`)
- `D13 -> SCK`  (`P106`)

Observacoes:

- o SPI do RC522 eh bit-banged
- ha tratamento para UIDs de 4, 7 e 10 bytes
- o leitor trabalha em polling

### 3.3 Display e touch

Display:

- controlador inicializado em [`src/hardware/lcd_setup.c`](./src/hardware/lcd_setup.c)
- painel baseado em sequencia de inicializacao `ILI9341`
- area logica usada pela UI: `240x320`

Touch:

- controlador `SX8654`
- pinos definidos em [`src/ui.c`](./src/ui.c):
  - `SDA -> P511`
  - `SCL -> P512`
  - `IRQ -> P004`
  - `RESET -> P609`

### 3.4 Porta, luz e botoes

Mapeamento atual em [`src/gpio.c`](./src/gpio.c):

- rele da porta: `D6 -> P613`
- rele da luz: `D5 -> P608`
- botao da porta: `P008`
- botao externo da porta: `D4 -> P112`
- botao da luz: `P009`

Comportamento:

- a porta abre por pulso temporizado de `2s`
- o fechamento automatico nao gera evento de log
- o botao em `D4` atua como disparo externo adicional da porta em modo ocioso
- os botoes locais tambem podem disparar eventos de UI
- os pinos dos reles sao configurados manualmente em `gpio_init()`, pois nao dependem do `pin_data.c`

## 4. Estrutura do repositorio

Principais diretorios:

- [`src`](./src): codigo da aplicacao
- [`src/assets`](./src/assets): logo, fontes e assets de fotos
- [`src/hardware`](./src/hardware): inicializacao especifica de hardware do display
- [`src/synergy_gen`](./src/synergy_gen): codigo gerado pelo e2 studio/SSP
- [`synergy`](./synergy): BSP e componentes Synergy/Azure RTOS
- [`synergy_cfg`](./synergy_cfg): configuracoes auxiliares do projeto
- [`script`](./script): scripts de importacao, conversao e geracao de assets
- [`Debug`](./Debug): build de desenvolvimento

Arquivos de apoio:

- [`CURRENT_STATE.md`](./CURRENT_STATE.md): snapshot operacional rapido
- [`NETWORK_NOTES.md`](./NETWORK_NOTES.md): historico da investigacao de rede
- [`configuration.xml`](./configuration.xml): stack/configuracao do projeto no e2 studio

## 5. Arquitetura de software

## 5.1 Modulos principais

- [`src/main.c`](./src/main.c)
  - estado global da aplicacao
  - fila de eventos
  - log de acesso em RAM
  - relogio de software baseado em UTC sincronizado por NTP
  - criacao das threads da aplicacao

- [`src/rfid.c`](./src/rfid.c)
  - driver bit-banged do RC522
  - leitura do UID
  - autenticacao local
  - cadastro local por aproximacao

- [`src/ui.c`](./src/ui.c)
  - renderizacao da interface local
  - tratamento do touch
  - tela de espera
  - tela de PIN
  - tela de IP/rede
  - tela de resultado de acesso
  - cache de perfil/foto
  - fotos runtime carregadas ou enviadas pela web

- [`src/net.c`](./src/net.c)
  - servidor HTTP embarcado
  - login admin web
  - dashboard
  - CRUD de perfis
  - upload de foto por tiles
  - importacao de perfis por JSON
  - pagina de log de acesso
  - controle web de porta e luz
  - DHCP e NTP

- [`src/storage.c`](./src/storage.c)
  - base de usuarios em RAM
  - thread de persistencia
  - gravacao/leitura de `users.json`
  - gravacao/leitura de `access.log`
  - gravacao/leitura de fotos em binarios separados

- [`src/gpio.c`](./src/gpio.c)
  - rele da porta
  - rele da luz
  - botoes fisicos

## 5.2 Threads da aplicacao

Definidas em [`src/main.c`](./src/main.c):

- `ui`
- `rfid`
- `gpio`
- `net`

Threads desativadas pela configuracao atual:

- `tunnel`
- `status`

Flags relevantes:

- `NETWORK_ISOLATION_MODE = 0`
- `NETWORK_HTTP_ONLY_MODE = 1`

## 5.3 Fila de eventos

Os modulos se comunicam por `TX_QUEUE` em [`src/main.c`](./src/main.c).

Eventos relevantes:

- `EVENT_RFID_AUTH_OK`
- `EVENT_RFID_AUTH_FAIL`
- `EVENT_DOOR_OPEN`
- `EVENT_LIGHT_ON`
- `EVENT_LIGHT_OFF`
- `EVENT_USER_ADDED`
- `EVENT_USER_REMOVED`
- `EVENT_CARD_REGISTERED`
- `EVENT_CARD_ALREADY_REGISTERED`
- `EVENT_CARD_REGISTRATION_FAILED`
- eventos de navegacao de UI

Observacao:

- `EVENT_DOOR_CLOSE` ainda existe no enum, mas o fechamento automatico da porta nao gera mais esse evento para log

## 6. Fluxos principais

## 6.1 Fluxo normal de autenticacao

1. A placa fica na tela `AGUARDANDO USUARIO / APROXIME O CARTAO`.
2. O RC522 detecta um UID.
3. [`src/rfid.c`](./src/rfid.c) consulta `storage_check_uid()`.
4. Se o cartao existir:
   - atualiza identidade atual
   - abre a porta
   - gera `EVENT_RFID_AUTH_OK`
5. Se o cartao nao existir:
   - atualiza identidade atual como nao cadastrada
   - gera `EVENT_RFID_AUTH_FAIL`
6. [`src/ui.c`](./src/ui.c) mostra a tela de resultado por `10s`.
7. A UI retorna automaticamente para a tela de espera.

## 6.2 Fluxo admin local

1. Na tela de espera, o usuario toca na engrenagem.
2. A UI abre o teclado numerico.
3. A UI aceita:
   - o PIN de um perfil administrador cadastrado
   - o PIN fallback padrao `1234`
4. Se o PIN estiver correto, a UI entra na tela de rede/IP.
5. Essa tela mostra:
   - estado de DHCP
   - IP atual da placa
   - botao `Voltar`

Observacao:

- a tela da engrenagem nao eh mais uma tela de cadastro local; ela eh uma tela de informacao de rede

## 6.3 Fluxo admin web

1. O usuario acessa a interface HTTP da placa.
2. Faz login com:
   - o PIN de um perfil administrador cadastrado
   - ou o PIN fallback `1234`
3. A sessao admin fica valida por `10 minutos`.
4. A partir da sessao, pode:
   - listar perfis
   - criar/editar/remover perfil
   - importar perfis por JSON
   - fazer upload de foto
   - ver log de acesso
   - acionar porta e luz

## 7. Interface web

## 7.1 Rede

A interface usa DHCP.

Bring-up atual em [`src/net.c`](./src/net.c):

1. inicializa `packet_pool`, `ip` e `http_server`
2. cria cliente DHCP manual em `g_net_dhcp_client`
3. solicita lease
4. aguarda IP valido
5. sobe HTTP
6. tenta sincronizar horario via NTP

Observacao importante:

- o projeto inclui a implementacao local de DHCP em [`src/nxd_dhcp_client.c`](./src/nxd_dhcp_client.c)
- esse arquivo faz parte do build e nao deve ser removido

## 7.2 Rotas principais

Rotas visiveis ao usuario:

- `/` - dashboard/home
- `/login` - login admin web
- `/logout` - encerra sessao admin
- `/admin_profiles` - lista de perfis paginada
- `/admin_profiles/<pagina>` - paginas adicionais
- `/profile_form` - criar perfil
- `/profile_form/<id>` - editar perfil
- `/upload_photo` - upload de foto para um perfil
- `/import` - importar perfis por JSON
- `/access_log` - visualizar log de acesso
- `/door` - controle de porta e luz

Rotas de acao:

- `/add_user`
- `/remove_user`
- `/save_users`
- `/import_profiles`
- `/portaon`
- `/lampadatoggle`
- `POST /api/door/open`
- `/upload_photo_begin`
- `/upload_photo_chunk`
- `/upload_photo_commit`

## 7.3 Caracteristicas da interface web

- separada em paginas pequenas para reduzir carga no servidor HTTP embarcado
- uso de redirects e flash messages para evitar respostas pesadas
- pagina de perfis com paginacao
- importacao JSON processada em background
- upload de foto em blocos pequenos

## 7.4 Login admin web

Parametros atuais:

- PIN: `1234`
- validade da sessao: `10 minutos`

Implementado em [`src/net.c`](./src/net.c) com:

- validacao por perfis administradores persistidos
- fallback `WEB_ADMIN_PIN` somente enquanto nenhum PIN de administrador estiver configurado nos perfis e a lista de usuários tiver sido carregada sem erro
- timeout de sessao em `WEB_ADMIN_SESSION_TICKS`

## 7.5 API HTTP para abertura da porta

Existe uma rota dedicada para integracoes externas:

- `POST /api/door/open`

Autenticacao aceita um destes headers:

- `X-API-KEY: <chave>`
- `Authorization: Bearer <chave>`

A chave usada hoje e a constante `API_KEY`, definida em [`src/main.c`](./src/main.c). Antes de usar em producao, o ideal e trocar o valor padrao por uma chave propria.

Exemplo em JavaScript:

```js
await fetch("http://192.168.11.2/api/door/open", {
  method: "POST",
  headers: {
    "X-API-KEY": "<sua-chave-da-placa>"
  }
});
```

Resposta de sucesso:

```json
{"ok":true,"message":"Door open command sent."}
```

## 7.6 API HTTP para agendamento do modo reunião

O modo reunião também pode ser agendado por API. As rotas usam a mesma autenticação da abertura remota da porta:

- `POST /api/meeting/schedule`
- `POST /api/meeting/cancel`
- `GET /api/meeting/status`

O agendamento aceita JSON ou formulário `application/x-www-form-urlencoded`. Para iniciar por atraso relativo, envie `delay_seconds` e a lista `profile_indices` com os índices dos perfis autorizados:

```js
await fetch("http://192.168.11.2/api/meeting/schedule", {
  method: "POST",
  headers: {
    "Content-Type": "application/json",
    "X-API-KEY": "<sua-chave-da-placa>"
  },
  body: JSON.stringify({
    delay_seconds: 300,
    profile_indices: [0, 4, 12]
  })
});
```

Para horário absoluto, use `start_utc` em UTC, no formato ISO `YYYY-MM-DDTHH:MM:SSZ`. Nesse caso, o agendamento depende do NTP estar sincronizado antes do horário chegar:

```json
{"start_utc":"2030-01-01T00:00:00Z","profile_indices":[0,4,12]}
```

Também é possível enviar os participantes por `name` + `chapter`, quando o sistema externo já faz a filtragem da reunião. Nesse modo, o firmware resolve cada par para o perfil cadastrado e salva internamente os índices, mantendo o mesmo formato persistido em QSPI:

```json
{
  "start_utc": "2030-01-01T00:00:00Z",
  "profile_names": [
    {"name": "Rafael Lago", "chapter": "CS"},
    {"name": "Maria Eduarda de Sá", "chapter": "RAS"}
  ]
}
```

O campo `profiles` também aceita a mesma lista de objetos, mas `profile_indices` continua sendo o formato mais direto quando os índices já são conhecidos. A comparação por `name` + `chapter` ignora apenas espaços no início e no fim; acentos, letras e pontuação precisam bater com o cadastro. Se o par não existir, for ambíguo ou apontar para um perfil sem cartão, a API rejeita o agendamento.

Para recorrência diária, adicione `recurrence: "daily"`:

```json
{"start_utc":"2030-01-01T00:00:00Z","profile_indices":[0,4,12],"recurrence":"daily"}
```

Para recorrência semanal, adicione `recurrence: "weekly"` e, opcionalmente, `weekdays`. Os dias usam `0=domingo`, `1=segunda`, ..., `6=sábado`:

```json
{"start_utc":"2030-01-01T00:00:00Z","profile_indices":[0,4,12],"recurrence":"weekly","weekdays":[1,3,5]}
```

Se `recurrence` for `"weekly"` e `weekdays` não for enviado, o firmware usa automaticamente o dia da semana de `start_utc`.

Cada chamada de agendamento retorna um `id`. Esse `id` pode ser usado para cancelar apenas uma reunião pendente:

```js
await fetch("http://192.168.11.2/api/meeting/cancel", {
  method: "POST",
  headers: {
    "Content-Type": "application/json",
    "X-API-KEY": "<sua-chave-da-placa>"
  },
  body: JSON.stringify({ id: 3 })
});
```

Se `POST /api/meeting/cancel` for chamado sem `id`, todos os agendamentos pendentes são removidos.

Regras importantes:

- cada perfil selecionado precisa existir e ter ao menos um cartão cadastrado
- quando a seleção vier por nome, cada item precisa ter `name` e `chapter`
- `start_utc` deve estar em UTC, por exemplo `2030-01-01T00:00:00Z`
- `start_unix` ainda é aceito apenas por compatibilidade
- `delay_seconds` aceita até 24 horas
- `delay_seconds` precisa de NTP sincronizado, pois o firmware converte o atraso para o horário absoluto antes de salvar
- até 8 agendamentos pendentes podem ficar salvos ao mesmo tempo
- os agendamentos pendentes são persistidos em QSPI no arquivo `meeting.json`
- agendamentos recorrentes mantêm o mesmo `id` e atualizam o próximo horário após cada execução
- no horário marcado, o firmware chama a mesma lógica de `storage_meeting_mode_start(...)` usada pela página web

## 8. Modelo de dados de usuario

Definido em [`src/storage.h`](./src/storage.h):

```c
typedef struct
{
    char name[NAME_MAX_LEN];
    char role[STORAGE_ROLE_MAX_LEN];
    char chapter[STORAGE_CHAPTER_MAX_LEN];
    char photo_id[STORAGE_PHOTO_ID_MAX_LEN];
    bool is_admin;
    char admin_pin[STORAGE_ADMIN_PIN_MAX_LEN];
    unsigned int card_count;
    char cards[STORAGE_MAX_CARDS_PER_USER][UID_MAX_LEN];
} storage_user_profile_t;
```

Limites atuais:

- maximo de usuarios: `50`
- maximo de cartoes por usuario: `4`
- foto runtime: ate `160x160`

## 9. Persistencia em QSPI/FileX

## 9.1 Estrategia geral

O projeto evita tocar cedo demais na QSPI durante o boot.

Por isso:

- o storage so acessa a midia apos um atraso seguro
- load e save pesados ficam numa thread de persistencia
- a aplicacao usa RAM como estado operacional principal

Constantes relevantes em [`src/storage.c`](./src/storage.c):

- `STORAGE_MEDIA_SAFE_DELAY_TICKS = 2s`
- `USERS_JSON_CHUNK_SIZE = 512`
- `USERS_JSON_OBJECT_SIZE = 1024`
- `ACCESS_LOG_BUFFER_SIZE = 4096`
- `ACCESS_LOG_ROTATE_SIZE = 64 KB`

## 9.2 Arquivos persistidos

Arquivos principais:

- `users.json`
- `access.log`
- `meeting.json`
- `photo_XXXXXXXX.bin`

### `users.json`

Guarda os perfis persistidos.

O arquivo e gravado em partes pequenas, perfil por perfil, para nao depender de um buffer unico de 8 KB. A gravacao usa `users.tmp` e `users.bak` para evitar que uma falha no meio da escrita destrua o ultimo arquivo valido. Na leitura do boot, o parser tambem processa objetos em blocos e mantem o limite operacional de `STORAGE_MAX_USERS`.

Formato atual aproximado:

```json
[
  {
    "name": "Nome",
    "uid": "UID_PRIMARIO",
    "role": "CARGO",
    "chapter": "CAPITULO",
    "photo_id": "foto_ou_asset",
    "is_admin": true,
    "admin_pin": "1234",
    "cards_csv": "UID1,UID2",
    "cards": ["UID1", "UID2"]
  }
]
```

Observacoes:

- `uid` existe por compatibilidade com fluxos antigos
- `is_admin` marca se o perfil pode autenticar como administrador
- `admin_pin` guarda o PIN numerico de 4 digitos do administrador
- `cards_csv` foi mantido para reforcar compatibilidade do parser
- `cards` eh o formato mais rico

### `access.log`

Guarda o log de acesso persistido.

Cada linha e serializada como:

```text
unix_utc|tipo_evento|dado|usuario
```

Observacoes:

- a gravacao e incremental, em append
- o arquivo ativo gira por tamanho e usa `access.bak` como arquivo de rotacao
- o boot recarrega o final do log persistido, lendo `access.bak` antes de `access.log` para preservar a ordem dos eventos mais recentes

### `meeting.json`

Guarda a fila de agendamentos pendentes do modo reunião.

Formato atual aproximado:

```json
[
  {
    "id": 1,
    "start_unix": 1893456000,
    "recurrence": 2,
    "weekdays_mask": 42,
    "profiles": [0, 4, 12]
  }
]
```

Observacoes:

- a gravacao usa `meeting.tmp` e `meeting.bak` para evitar perda do arquivo anterior durante uma falha
- o limite operacional e `STORAGE_MEETING_SCHEDULE_MAX_ITEMS`, atualmente `8`
- `recurrence` usa `0=unico`, `1=diario` e `2=semanal`
- `weekdays_mask` usa bits de domingo a sabado; por exemplo, `42` representa segunda, quarta e sexta
- agendamentos unicos sao removidos da fila quando chegam ao horario de inicio
- agendamentos recorrentes permanecem na fila e avancam para a proxima ocorrencia futura

### `photo_XXXXXXXX.bin`

Cada foto persistida usa um nome derivado do `photo_id`.

Conteudo:

- cabecalho customizado com magic, largura, altura e tamanho do payload
- payload RGB565 bruto

## 9.3 Persistencia automatica

Hoje o projeto ja solicita persistencia automaticamente em varios fluxos:

- salvar perfil
- remover perfil
- cadastro local por RFID
- importacao de perfis
- upload de foto
- gravacao de log de acesso

APIs principais em [`src/storage.h`](./src/storage.h):

- `storage_persist_now()`
- `storage_persist_wait()`
- `storage_access_log_enqueue()`
- `storage_access_log_persist_now()`
- `storage_photo_persist_now()`
- `storage_photo_ensure_loaded()`

Estados da persistencia:

- `0 = idle`
- `1 = pending`
- `2 = success`
- `3 = failed`

## 10. Fotos de usuario

O projeto trabalha com dois tipos de foto:

### 10.1 Fotos embutidas no firmware

Assets em:

- [`src/assets/profile_photo_assets.h`](./src/assets/profile_photo_assets.h)
- [`src/assets/profile_photo_ids.h`](./src/assets/profile_photo_ids.h)

Uso:

- bom para fotos estaveis
- qualidade alta
- exige recompilar o firmware

### 10.2 Fotos enviadas pela web

Fluxo atual:

1. O navegador corta a imagem em formato quadrado.
2. Faz downscale para `160x160`.
3. Converte para RGB565.
4. Divide em `16` tiles reais de `40x40`.
5. Envia por:
   - `/upload_photo_begin`
   - `/upload_photo_chunk`
   - `/upload_photo_commit`
6. A UI remonta a imagem em RAM.
7. O perfil recebe o `photo_id`.
8. A foto e o perfil sao persistidos na QSPI.

Constantes relevantes em [`src/net.c`](./src/net.c):

- `UPLOAD_IMAGE_DIM = 160`
- `UPLOAD_TILE_DIM = 40`

Constantes relevantes em [`src/ui.c`](./src/ui.c) e [`src/storage.h`](./src/storage.h):

- `UI_UPLOADED_PHOTO_SLOTS = 1`
- `STORAGE_RUNTIME_PHOTO_MAX_DIM = 160`

Observacoes:

- existe apenas `1` slot runtime de foto carregada para economizar RAM
- fotos persistidas podem ser recarregadas sob demanda pela UI
- `photo_id` e o elo entre perfil, UI, QSPI e assets

## 11. UI local

## 11.1 Telas principais

Estados principais em [`src/ui.c`](./src/ui.c):

- `UI_VIEW_IDLE`
- `UI_VIEW_RESULT`
- `UI_VIEW_PIN`
- `UI_VIEW_ENROLL_WAIT`

### Tela de espera

- usa a logo do Ramo
- mostra:
  - `AGUARDANDO USUARIO`
  - `APROXIME O CARTAO`
- possui engrenagem para entrar no fluxo admin local

### Tela de resultado

Ao autenticar ou negar acesso:

- usa fonte estilo pixelada, alinhada com a tela de espera
- tenta quebrar nomes grandes em ate 2 linhas
- mostra `(CARGO-CAPITULO)` abaixo do nome quando houver dados
- retorna automaticamente apos `10s`

### Tela de PIN

- PIN local: `1234`
- teclado numerico touch
- botoes `limpa`, `0`, `entrar`

### Tela de rede/IP

Chamada internamente de `UI_VIEW_ENROLL_WAIT`, mas hoje seu uso real e:

- mostrar estado da rede
- mostrar IP atual
- mostrar `AGUARDANDO DHCP` se ainda nao houver lease

## 11.2 Dimming de tela

Apos `60s` de inatividade:

- a tela nao apaga
- o framebuffer e re-renderizado com brilho aproximado de `30%`

Importante:

- isso nao e PWM real no backlight
- e um dimming por software sobre o framebuffer
- foi escolhido assim para evitar mudancas mais arriscadas em timer/backlight

## 12. RTC por software e NTP

## 12.1 Como funciona

O projeto nao usa RTC dedicado.

Em vez disso:

1. A rede sobe via DHCP.
2. [`src/net.c`](./src/net.c) tenta obter servidor NTP da opcao DHCP 42.
3. Se nao houver servidor no lease, tenta fallback:
   - `129.6.15.28`
   - `129.6.15.29`
4. O horario UTC sincronizado e entregue a [`src/main.c`](./src/main.c) via `app_time_set_utc()`.
5. A partir dai o projeto mantem um relogio de software usando `tx_time_get()`.

## 12.2 Formato de timestamp

O log de acesso usa:

- horario absoluto se NTP ja tiver sincronizado
- fallback `T+hh:mm:ss` se ainda nao houver hora valida

O timestamp absoluto atual e formatado como horario local em [`src/main.c`](./src/main.c), com deslocamento fixo de `UTC-3`.

Formato exibido:

```text
YYYY-MM-DD:HHhMMminSSs
```

Exemplo:

```text
2026-03-19:20h03min05s
```

## 13. Log de acesso

Capacidade em RAM:

- `ACCESS_LOG_SIZE = 32`

Eventos registrados atualmente:

- acesso autorizado
- acesso negado
- abertura de porta
- cadastro de cartao
- cartao ja cadastrado
- falha no cadastro

Importante:

- o fechamento automatico da porta nao entra mais no log

Pagina web:

- `/access_log`

Persistencia:

- salva em `access.log` na QSPI com append incremental
- gira para `access.bak` quando o arquivo ativo atinge o limite configurado
- recarrega no boot com atraso seguro, lendo o final dos arquivos persistidos

## 14. Rede

## 14.1 Modo atual

- Ethernet ativa
- HTTP local ativo
- DHCP ativo
- NTP ativo
- `NETWORK_HTTP_ONLY_MODE = 1`

Nao esta ativo neste estado:

- tunel remoto
- thread de status remoto

## 14.2 Informacoes exibidas ao usuario

Na web e na UI local, o projeto mostra:

- IP atual
- mascara de rede
- estado do link
- informacao de rede via DHCP

## 14.3 Observacao importante sobre DHCP

O cliente DHCP foi integrado manualmente ao projeto com fonte local:

- [`src/nxd_dhcp_client.c`](./src/nxd_dhcp_client.c)

Esse arquivo precisa continuar no build.

## 15. Importacao offline do Firebase

O projeto suporta importar dados de Firebase sem dependencia em runtime.

Scripts principais em [`script`](./script):

- [`script/export_firebase_profiles.py`](./script/export_firebase_profiles.py)
- [`script/download_firebase_profile_photos.py`](./script/download_firebase_profile_photos.py)
- [`script/firebase_export_to_profiles.py`](./script/firebase_export_to_profiles.py)
- [`script/generate_card_bindings_template.py`](./script/generate_card_bindings_template.py)
- [`script/build_users_json_from_bundle.py`](./script/build_users_json_from_bundle.py)
- [`script/generate_profile_photo_assets.py`](./script/generate_profile_photo_assets.py)
- [`script/regenerate_profile_photo_assets.ps1`](./script/regenerate_profile_photo_assets.ps1)

Bundle local atual:

- [`script/firebase_bundle`](./script/firebase_bundle)

Fluxo tipico:

1. Exportar perfis do Firestore.
2. Baixar fotos do Storage.
3. Reduzir/converter fotos.
4. Gerar manifestos ou assets.
5. Preencher cartoes RFID offline, se necessario.
6. Gerar `users.json` ou importar perfis pela interface web.

## 16. Build e gravacao

Ambiente esperado:

- Renesas e2 studio
- toolchain GNU ARM configurado
- pacotes SSP/Synergy do projeto

Passos usuais:

1. Abrir o projeto no e2 studio.
2. Evitar `Generate Project Content` sem necessidade.
3. Compilar em `Debug`.
4. Gravar o firmware na placa.

Saida principal:

- [`Debug/IoTRamoRenesas.elf`](./Debug/IoTRamoRenesas.elf)

Observacao:

- o projeto possui codigo gerado em `src/synergy_gen`
- mudancas manuais em arquivos gerados ou makefiles devem ser feitas com cuidado

## 17. Arquivos mais importantes para manutencao

- [`src/main.c`](./src/main.c)
- [`src/main.h`](./src/main.h)
- [`src/net.c`](./src/net.c)
- [`src/net.h`](./src/net.h)
- [`src/storage.c`](./src/storage.c)
- [`src/storage.h`](./src/storage.h)
- [`src/ui.c`](./src/ui.c)
- [`src/ui.h`](./src/ui.h)
- [`src/rfid.c`](./src/rfid.c)
- [`src/gpio.c`](./src/gpio.c)
- [`src/nxd_dhcp_client.c`](./src/nxd_dhcp_client.c)

## 18. Limitacoes e cuidados

### 18.1 Memoria

O projeto e sensivel a:

- buffers HTML grandes
- respostas HTTP pesadas
- parsing de corpo grande no callback HTTP
- uso excessivo de stack nas threads

Por isso:

- a web foi dividida em paginas leves
- importacao JSON roda em background
- upload de foto usa tiles
- varias paginas usam buffers `static`

### 18.2 FileX/QSPI

Historicamente, esta foi a area mais sensivel do projeto.

Cuidados:

- nao abrir a midia cedo demais no boot
- nao mover save/load pesado para dentro do callback HTTP
- manter a thread de persistencia separada

### 18.3 Rede

Mesmo com DHCP/NTP estaveis hoje:

- mudancas grandes em `net.c` podem reintroduzir timeouts
- uploads HTTP grandes tendem a quebrar mais facilmente

### 18.4 Fotos

Para a qualidade atual do upload web:

- o navegador faz preprocessamento
- a placa recebe tiles RGB565
- aumentar muito a resolucao runtime aumenta muito o risco de RAM e timeout

## 19. Checklist rapido de validacao

Depois de mudancas importantes, vale validar:

1. Boot da placa.
2. Leitura RFID autorizada e negada.
3. Abertura fisica da porta.
4. Pagina `/login`.
5. Pagina `/admin_profiles`.
6. Criacao/edicao/remocao de perfil.
7. Upload de foto.
8. Reinicio da placa e confirmacao de persistencia.
9. Pagina `/access_log`.
10. Sincronizacao de horario por NTP.

## 20. Documentos auxiliares

Para contexto adicional:

- [`CURRENT_STATE.md`](./CURRENT_STATE.md): resumo rapido de retomada
- [`NETWORK_NOTES.md`](./NETWORK_NOTES.md): historico da investigacao da pilha de rede

Este `README.md` deve ser tratado como a documentacao principal e mais completa do projeto.
