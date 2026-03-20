# Estado Atual do Projeto

Data de referencia: `2026-03-20`

Este arquivo e um resumo rapido. A documentacao principal e mais completa esta em [`README.md`](./README.md).

## Base funcional atual

Hoje a base esta operando com:

- RFID funcionando
- UI local funcionando
- autenticacao por cartao funcionando
- painel web administrativo funcionando
- perfis com nome, cargo, capitulo, foto e multiplos cartoes
- DHCP ativo
- NTP ativo
- persistencia de perfis em QSPI
- persistencia de fotos enviadas pela web em QSPI
- persistencia do log de acesso em QSPI

## Caracteristicas importantes

- PIN admin local e web: `1234`
- retorno automatico da tela de resultado: `10s`
- dimming da tela apos `60s`
- brilho reduzido para cerca de `30%`
- log registra abertura de porta, mas nao fechamento automatico
- web opera em modo `HTTP only`

## Fluxos considerados estaveis

- leitura de cartao autorizado/negado
- criar, editar e remover perfil pela web
- paginacao em `/admin_profiles/<pagina>`
- importacao de perfis por JSON
- upload de foto por tiles
- visualizacao do log de acesso
- controle de porta e luz pela web

## Pontos ainda sensiveis

- qualquer aumento agressivo do peso das paginas HTML
- uploads HTTP muito grandes
- mudancas no boot do FileX/QSPI
- alteracoes grandes no parser/load do `users.json`
- alteracoes grandes no bring-up da rede

## Arquivos mais importantes

- [`src/main.c`](./src/main.c)
- [`src/net.c`](./src/net.c)
- [`src/storage.c`](./src/storage.c)
- [`src/storage.h`](./src/storage.h)
- [`src/ui.c`](./src/ui.c)
- [`src/rfid.c`](./src/rfid.c)
- [`src/gpio.c`](./src/gpio.c)
- [`src/nxd_dhcp_client.c`](./src/nxd_dhcp_client.c)

## Build atual

Saida principal:

- [`Debug/IoTRamoRenesas.elf`](./Debug/IoTRamoRenesas.elf)

## Recomendacao pratica

Antes de mexer em rede, FileX, upload ou parser de perfis, releia:

- [`README.md`](./README.md)
- [`NETWORK_NOTES.md`](./NETWORK_NOTES.md)
