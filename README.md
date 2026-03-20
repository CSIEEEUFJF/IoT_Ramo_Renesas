# IoT Ramo Renesas

Resumo operacional mais recente:

- [CURRENT_STATE.md](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/CURRENT_STATE.md)

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

## Importação offline

Para trazer usuários já existentes de um app em Firebase sem criar dependência em runtime, existe o conversor:

- [firebase_export_to_profiles.py](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/script/firebase_export_to_profiles.py)
- [export_firebase_profiles.py](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/script/export_firebase_profiles.py)
- [download_firebase_profile_photos.py](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/script/download_firebase_profile_photos.py)
- [generate_card_bindings_template.py](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/script/generate_card_bindings_template.py)
- [build_users_json_from_bundle.py](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/script/build_users_json_from_bundle.py)
- [generate_profile_photo_assets.py](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/script/generate_profile_photo_assets.py)

Uso esperado:

```bash
python script/firebase_export_to_profiles.py export_firebase.json users_importados.json
```

Ele converte campos comuns como:

- `name` / `nome`
- `role` / `cargo`
- `chapter` / `capitulo`
- `uid`, `cards`, `rfids`, `tags`

Saída gerada:

- JSON no formato de perfis do firmware, com `name`, `uid`, `role`, `chapter`, `photo_id` e `cards`

Se a origem estiver no Firestore, o caminho recomendado é:

```bash
python script/export_firebase_profiles.py --service-account service-account.json --collection users --output users_importados.json --storage-bucket seu-bucket.appspot.com --storage-prefix fotos/
```

Observação:

- Firestore não oferece um `Export JSON` simples como o Realtime Database
- esse script faz a exportação offline a partir da service account, sem criar dependência no firmware

Fluxo recomendado completo:

1. Exportar os perfis do Firestore para um bundle offline:

```bash
python script/export_firebase_profiles.py --service-account service-account.json --collection users --output script/firebase_bundle/firebase_profiles.json --storage-bucket seu-bucket.firebasestorage.app
```

2. Baixar e reduzir as fotos para uso futuro no firmware:

```bash
python script/download_firebase_profile_photos.py --service-account service-account.json --bundle script/firebase_bundle/firebase_profiles.json --output-dir script/firebase_bundle/photos --size 80 --quality 60
```

3. Gerar um CSV para preencher os cartões RFID:

```bash
python script/generate_card_bindings_template.py --bundle script/firebase_bundle/firebase_profiles.json --output script/firebase_bundle/card_bindings.csv
```

4. Depois de preencher o CSV com os cartões, gerar um `users.json` compatível com a placa:

```bash
python script/build_users_json_from_bundle.py --bundle script/firebase_bundle/firebase_profiles.json --bindings script/firebase_bundle/card_bindings.csv --output script/firebase_bundle/users.json
```

Se quiser importar os perfis primeiro pela interface web e vincular os cartões depois:

- gere ou use [profiles_import.json](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/script/firebase_bundle/profiles_import.json)
- abra a página da placa
- cole o conteúdo no bloco `Importar perfis offline`
- clique em `Importar perfis`
- depois clique em `Persistir cadastros`

5. Para integrar as fotos importadas na UI da placa como assets estáticos:

```bash
python script/generate_profile_photo_assets.py --bundle script/firebase_bundle/firebase_profiles.json --manifest script/firebase_bundle/photos/photo_manifest.json --photos-dir script/firebase_bundle/photos/downscaled --output src/assets/profile_photo_assets.h
```

Observações do bundle atual:

- os perfis do Firestore vieram com `name`, `chapterRoles`, `phoneNumber`, `email` e às vezes `profilePictureUrl`
- não há campos de cartão/RFID no Firestore atual, então os cartões precisam ser vinculados depois
- as fotos reduzidas ficam em [script/firebase_bundle/photos/downscaled](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/script/firebase_bundle/photos/downscaled)
- o header gerado para a UI fica em [profile_photo_assets.h](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/src/assets/profile_photo_assets.h)

## Histórico de rede

As anotações específicas da investigação de Ethernet estão em:

- [NETWORK_NOTES.md](/C:/Users/killl/e2_studio/workspace/IoTRamoRenesas/NETWORK_NOTES.md)
