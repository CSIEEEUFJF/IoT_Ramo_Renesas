# Estado Atual do Projeto

Data de referencia: `2026-03-19`

Este arquivo resume o ponto atual do projeto para retomada segura, especialmente porque rede e persistencia ja mostraram sensibilidade a mudancas agressivas.

## Base estavel conhecida

Hoje o projeto compila e sobe com:

- leitura RFID funcionando
- display e touch funcionando
- UI local com splash/logo ajustada
- rede HTTP funcionando em IP estatico
- interface web admin funcionando em paginas separadas
- persistencia QSPI/Flash existente e ativa

Endereco atual da web:

- `http://192.168.15.180/`

PIN admin local e web:

- `1234`

## Modelo atual de usuario

O projeto ja nao trabalha mais so com `nome + uid`. O perfil atual suporta:

- `name`
- `role`
- `chapter`
- `photo_id`
- ate `4` cartoes por usuario

Arquivos principais:

- [storage.h](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/storage.h)
- [storage.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/storage.c)

## Estado atual da interface web

Para reduzir a lentidao do servidor HTTP, a interface foi quebrada em paginas menores.

Rotas principais:

- `/`
- `/login`
- `/profiles`
- `/profile_form`
- `/upload_photo`
- `/import`
- `/access_log`
- `/door`

Objetivos dessa divisao:

- reduzir o HTML por request
- evitar trabalho pesado no callback HTTP
- separar cadastro, importacao, upload e controle da porta
- exigir autenticacao admin antes das acoes criticas

Arquivo principal:

- [net.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/net.c)

### Estado funcional atual da web

Ja existe:

- login admin por PIN
- criacao e edicao de perfil
- remocao de perfil
- importacao offline por JSON
- upload de foto em runtime
- log de acessos
- controle de porta/lampada

### Ponto sensivel atual da web

Apesar da divisao em paginas, ainda houve relato de:

- lentidao alta na interface
- travamento ao importar JSON em algumas versoes

Para mitigar isso, a importacao de perfis foi movida para processamento em background na thread de rede, em vez de rodar inteira dentro da resposta HTTP.

## Estado atual da persistencia

### O que esta implementado

A persistencia automatica agora foi religada de forma incremental:

- salvar/editar perfil agenda persistencia automatica
- remover perfil agenda persistencia automatica
- cadastro local por RFID agenda persistencia automatica
- importacao offline agenda persistencia automatica depois de processar o JSON

O botao manual `Persistir cadastros` continua existindo como fallback.

Arquivos principais:

- [storage.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/storage.c)
- [net.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/net.c)

### O que ainda e sensivel

O ponto mais sensivel continua sendo:

- persistencia pos-reboot de perfis com multiplos cartoes

Ja houve historico de:

- o segundo cartao sumir depois de reboot
- regressões ao mexer no parser/load do `users.json`

Entao, mesmo com auto-save ligado, essa area ainda merece teste real na placa sempre que o schema do perfil mudar.

## Estado atual das fotos

### Fotos embutidas no firmware

Ja existem fotos convertidas para assets locais, e a UI ja consegue exibi-las ao autenticar um cartao cujo perfil tenha `photo_id`.

Arquivos principais:

- [ui.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/ui.c)
- [profile_photo_assets.h](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/assets/profile_photo_assets.h)
- [profile_photo_ids.h](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/assets/profile_photo_ids.h)

### Upload de foto pela interface web

Ja existe um fluxo de upload em:

- `/upload_photo`

Esse fluxo:

- recorta e reduz a imagem no navegador
- envia em RGB565
- armazena a foto em RAM
- vincula o `photo_id` ao perfil

Limitacao importante:

- a foto enviada por upload web e apenas runtime
- ela nao persiste apos reboot nesta versao

Ou seja:

- o perfil persiste
- a foto enviada pela web ainda nao

## Importacao offline do Firebase

Existe um fluxo offline pronto para importar dados do Firestore e baixar fotos do Storage, sem criar dependencia do Firebase em runtime.

Scripts principais:

- [export_firebase_profiles.py](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/script/export_firebase_profiles.py)
- [download_firebase_profile_photos.py](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/script/download_firebase_profile_photos.py)
- [build_users_json_from_bundle.py](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/script/build_users_json_from_bundle.py)

Bundle local:

- [firebase_bundle](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/script/firebase_bundle)

O bundle ja contem:

- perfis exportados
- fotos reduzidas
- manifesto das fotos
- template de vinculo de cartoes

## Log de acesso

Foi adicionada uma trilha em RAM dos eventos principais:

- acesso liberado
- acesso negado
- porta aberta/fechada
- eventos de cadastro

Arquivos principais:

- [main.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/main.c)
- [main.h](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/main.h)
- [net.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/net.c)

Pagina web:

- `/access_log`

## Restricoes importantes para nao quebrar o que ja funciona

Nao mexer sem muito cuidado em:

- configuracao de rede
- bring-up Ethernet
- configuracao do FileX/QSPI no boot
- abertura precoce da midia
- generation do projeto no e2 studio sem necessidade

O pedido recorrente do usuario continua sendo:

- nao quebrar a rede atual
- nao quebrar a persistencia atual
- preferir mudancas pequenas e isoladas

## Arquivos mais importantes agora

- [src/net.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/net.c)
- [src/storage.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/storage.c)
- [src/storage.h](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/storage.h)
- [src/ui.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/ui.c)
- [src/main.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/main.c)
- [src/rfid.c](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/rfid.c)

## Build mais recente

Ultimo build local bem-sucedido:

- alvo: [Debug](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/Debug)
- binario: `IoTRamoRenesas.elf`

## Proximo passo recomendado

O proximo passo mais seguro agora e validar na placa:

1. auto-save depois de criar/editar/remover perfil
2. auto-save depois de importar perfis por JSON
3. persistencia pos-reboot de multiplos cartoes
4. confirmar que a web ficou estavel com a importacao em background

Se algo falhar, a prioridade deve ser:

- corrigir `save/load` de perfis e cartoes

antes de mexer mais em fotos persistentes ou novos fluxos web.
