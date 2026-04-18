# Network Notes

Data de referencia: `2026-03-20`

Este arquivo registra o estado atual da rede e os aprendizados mais importantes do bring-up HTTP/Ethernet.

## Estado atual

- Ethernet funcionando
- DHCP funcionando
- HTTP funcionando
- NTP funcionando
- interface web administrativa funcionando
- modo atual do app: `HTTP only`

## O que hoje faz parte da base estavel

- cliente DHCP manual no codigo da aplicacao
- implementacao local em [`src/nxd_dhcp_client.c`](./src/nxd_dhcp_client.c)
- `packet_pool`, `ip` e `http_server` inicializados antes do uso da web
- paginas web pequenas, em vez de uma pagina unica pesada
- buffers `static` nas rotas mais pesadas
- serializacao do callback HTTP para evitar reuso concorrente de buffers
- importacao JSON em background
- upload de foto em tiles pequenos

## Achados historicos importantes

Os problemas de rede encontrados ao longo do projeto tiveram mais relacao com:

- uso excessivo de stack na thread HTTP
- paginas HTML grandes demais
- respostas HTTP grandes ou mal fechadas
- operacoes pesadas de FileX dentro da requisicao web
- reuso inadequado de buffers no callback

Do que com:

- cabo de rede
- switch
- ajuste fino de PHY

## Bring-up atual

Resumo do fluxo em [`src/net.c`](./src/net.c):

1. inicializa `packet_pool`, `ip` e `http_server`
2. cria o cliente DHCP
3. solicita lease e espera IP valido
4. marca a rede como pronta
5. sobe HTTP
6. tenta sincronizar horario por NTP
7. faz ressincronizacao periodica de NTP

## NTP

Prioridade atual:

1. servidor NTP anunciado no DHCP (opcao 42)
2. fallback:
   - `129.6.15.28`
   - `129.6.15.29`

## Rotas mais sensiveis

Historicamente, estas foram as mais sensiveis:

- listagem de perfis
- importacao de JSON
- upload de foto

Mitigacoes atuais:

- `/admin_profiles` paginada
- `/import` processa em background
- `/upload_photo` usa preprocessamento no navegador e envio em tiles

## Cuidados ao mexer em rede

Evite mudar sem necessidade:

- tamanho dos buffers HTML
- estrategia de resposta HTTP
- parsing do corpo HTTP dentro do callback
- ordem de inicializacao de rede
- bring-up manual do DHCP
- stack da thread de rede

## Arquivos principais

- [`src/net.c`](./src/net.c)
- [`src/net.h`](./src/net.h)
- [`src/nxd_dhcp_client.c`](./src/nxd_dhcp_client.c)
- [`src/main.c`](./src/main.c)
- [`src/storage.c`](./src/storage.c)
- [`src/synergy_gen/common_data.c`](./src/synergy_gen/common_data.c)

## Referencia

Para a documentacao completa do projeto, use:

- [`README.md`](./README.md)
