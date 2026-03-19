# IoT Ramo Renesas

Projeto para a placa **Renesas SK-S7G2** com:

- leitura de RFID RC522
- interface gráfica no display TFT com touch
- cadastro e autenticação de usuários
- salvamento de usuários em **QSPI/FileX**
- interface web local para administração

Estado desta versão: **leitura, display, salvamento e rede funcional**, ainda **antes da integração de foto real do usuário**.

## Estado atual

Funcional hoje nesta pasta:

- leitura de cartão/tag RFID com RC522
- autenticação de usuários cadastrados
- tela local com logo, splash, espera de cartão e fluxo de PIN/admin
- tela touch para entrar no modo de cadastro local
- cadastro de usuários em runtime
- cadastro e remoção de usuários pela interface web
- persistência explícita dos usuários em QSPI
- carregamento tardio dos usuários salvos após o boot
- servidor HTTP local acessível na rede

Ainda não está pronto ou pode mudar:

- foto real por usuário na UI
- fluxo completo de assets/imagens por usuário
- uso de DHCP
- túnel/status remotos

## Arquitetura resumida

Principais módulos:

- [main.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/main.c): criação das threads e estado global da aplicação
- [ui.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/ui.c): interface gráfica, touch e fluxo local de PIN/cadastro
- [rfid.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/rfid.c): driver RC522 em SPI por software e fluxo de autenticação/cadastro
- [storage.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/storage.c): lista de usuários em RAM + persistência em QSPI/FileX
- [net.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/net.c): servidor HTTP e página de administração
- [gpio.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/gpio.c): porta, iluminação e I/O auxiliares

Threads principais:

- `ui`
- `rfid`
- `gpio`
- `net`

Nesta versão, a rede está em modo **HTTP only**.

## RFID

O RC522 está ligado no header Arduino da SK-S7G2 via SPI por software:

- `D10` -> `SS`
- `D9` -> `RST`
- `D11` -> `MOSI`
- `D12` -> `MISO`
- `D13` -> `SCK`

Referência: [rfid.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/rfid.c)

Comportamento atual:

- modo normal: autentica o UID contra a base de usuários
- modo cadastro: cadastra o próximo cartão lido

## Interface local

A UI roda no display TFT e usa touch.

Fluxo atual:

- splash / espera com a logo do Ramo
- aproximação do cartão
- tela de resultado com nome do usuário e status de acesso
- entrada em modo admin por engrenagem
- PIN admin local
- tela de cadastro local

Referência: [ui.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/ui.c)

PIN admin atual:

- `1234`

## Interface web

A placa sobe com IP estático:

- IP: `192.168.15.180`
- Máscara: `255.255.255.0`
- Gateway: `192.168.15.1`

Página principal:

- [http://192.168.15.180/](http://192.168.15.180/)

Health check:

- [http://192.168.15.180/health](http://192.168.15.180/health)

A página permite:

- cadastrar usuário por `nome + UID`
- remover usuário
- persistir cadastros na QSPI
- visualizar último cartão lido
- abrir a porta

Referência: [net.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/net.c)

## Persistência

O cadastro funciona em duas etapas:

1. atualização imediata em RAM
2. persistência explícita para `users.json` na QSPI

Comportamento atual:

- o botão web `Persistir cadastros` solicita a gravação
- uma thread de storage grava o snapshot em background
- após o boot, o carregamento da base é tardio para evitar travamentos na inicialização

Referência: [storage.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/storage.c)

Observações importantes:

- existe um atraso de segurança antes de acessar a mídia logo após o boot
- o projeto usa `FileX` sobre a QSPI
- esta versão evita tocar na mídia cedo demais durante a inicialização

## Flags sensíveis

Alguns pontos foram deixados explícitos no código para manutenção:

- em [main.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/main.c), `NETWORK_HTTP_ONLY_MODE` está em `1`
- em [storage.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/storage.c), `FORMAT_DRIVE_NOW` está em `0`
- em [main.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/main.c), `WIPE_QSPI_NOW` está em `0`

Esses flags não devem ser alterados sem cuidado, porque afetam boot e persistência.

## Build

Ambiente esperado:

- Renesas e2 studio
- SSP/Synergy configurado para a placa
- projeto `IoTRamoRenesas`

Fluxo recomendado:

1. abrir o projeto no e2 studio
2. usar `Generate Project Content` apenas quando necessário
3. fazer build em `Debug`
4. gravar na placa

## Limitações atuais

- ainda não há foto real do usuário na UI
- a rede está em IP estático, não DHCP
- a persistência está focada em `users.json`; ainda não há pipeline de imagens
- há bastante telemetria de debug no código para facilitar bring-up e manutenção

## Histórico de rede

As anotações específicas da investigação de Ethernet estão em:

- [NETWORK_NOTES.md](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/NETWORK_NOTES.md)
