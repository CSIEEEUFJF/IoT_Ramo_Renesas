# Relatório Técnico — Sistema Embarcado de Controle de Acesso

Data de emissão: `2026-03-20`

Projeto: `IoTRamoRenesas`  
Plataforma-alvo: `Renesas SK-S7G2`

## 1. Objetivo do projeto

O projeto `IoTRamoRenesas` foi desenvolvido para implementar um sistema embarcado de controle de acesso com foco em demonstração tecnológica, operação local e administração remota. O sistema integra:

- autenticação por cartão RFID;
- interface local em display TFT com touch;
- interface web administrativa;
- persistência de dados em memória QSPI;
- exibição de foto do usuário;
- controle de porta e iluminação;
- registro de logs de acesso com horário sincronizado por rede.

O objetivo principal foi transformar uma base experimental em uma plataforma funcional, demonstrável e tecnicamente reaproveitável pelo Ramo Estudantil IEEE UFJF e pelo IEEE Computer Society.

## 2. Resumo executivo

Ao longo do desenvolvimento, o sistema evoluiu de uma base parcial, com instabilidades em UI, rede e persistência, para uma plataforma integrada com:

- leitura RFID funcional;
- interface local refinada e orientada à experiência do usuário;
- painel web administrativo dividido em páginas leves;
- perfis de usuário com nome, cargo, capítulo, foto, múltiplos cartões e privilégios administrativos;
- suporte a upload e exibição de fotos;
- persistência de perfis, fotos e logs em QSPI/FileX;
- obtenção automática de IP via DHCP;
- sincronização de data e hora via NTP;
- autenticação administrativa por PIN de perfil.

O resultado atual atende tanto a uso demonstrativo quanto a futuras extensões acadêmicas e institucionais.

## 3. Principais fases de implementação

### 3.1 Estabilização inicial da aplicação

A primeira fase foi dedicada à recuperação de uma baseline confiável de execução. O foco esteve em:

- correção de travamentos na UI local;
- ajuste do touch e da navegação entre telas;
- recuperação da leitura RFID;
- consolidação do fluxo principal de autenticação;
- restauração de uma base estável de firmware.

Marco associado:

- `818dcf9` — `Stable app baseline without persistence worker`

### 3.2 Recuperação e estabilização da rede

Foi realizada uma investigação extensa da pilha de rede para restaurar o servidor HTTP sem comprometer o restante do sistema.

Entregas principais:

- restauração do servidor HTTP funcional;
- reorganização da inicialização de rede;
- isolamento de trechos que causavam travamento;
- divisão da interface web em páginas menores;
- redução de latência e respostas HTTP excessivamente pesadas.

Marcos associados:

- `b55418d` — `Restore working web network flow`
- `f633395` — `Ajustes na stack de rede`
- `0e06671` — `Ajustes na stack de rede`

### 3.3 Consolidação da UI local

A interface local passou por várias iterações até atingir um formato visualmente consistente e adequado para demonstração.

Entregas principais:

- tela de espera alinhada à identidade visual do projeto;
- tela de resultado de acesso com foto, nome e identificação do perfil;
- tela da engrenagem convertida em painel local de rede/IP;
- retorno automático após apresentação do resultado;
- remoção de interações desnecessárias para “voltar” manualmente;
- tratamento de nomes longos e melhor ocupação do frame da foto;
- redução do brilho visual para cerca de `30%` após `60s` de inatividade.

Marco associado:

- `eec1cb5` — `UI final pronta, partindo para foto de usuário`

### 3.4 Persistência em QSPI

Uma etapa central do trabalho foi tornar a QSPI utilizável com FileX sem comprometer o boot da placa.

Foram implementados:

- carregamento tardio e seguro da mídia;
- persistência automática de perfis em `users.json`;
- persistência de fotos em arquivos binários separados;
- persistência de logs de acesso em `access.log`;
- serialização de perfis com múltiplos cartões;
- estratégia de worker em background para evitar travamentos em threads sensíveis.

Arquivos persistidos atualmente:

- `users.json`;
- `access.log`;
- `photo_XXXXXXXX.bin`.

Marcos associados:

- `6fee797` — `Leitura, display, salvamento e rede funcional, pré foto de usuário`
- `aa58035` — `Fotos ficando persistentes na QSPI`

### 3.5 Perfis de usuário

O modelo de usuário deixou de ser apenas `nome + UID` e passou a suportar um perfil expandido.

Campos suportados atualmente:

- nome;
- cargo;
- capítulo IEEE;
- foto;
- indicador de administrador;
- PIN administrativo;
- até 4 cartões por usuário.

Também foram implementados:

- listagem paginada de perfis;
- criação de perfil;
- edição de perfil;
- remoção de perfil;
- persistência automática após salvar.

### 3.6 Fotos de usuário

Foi adicionada a capacidade de associar fotos aos usuários e exibi-las na UI local após autenticação.

Entregas desta fase:

- importação de fotos pré-processadas por fluxo offline;
- upload web em blocos pequenos, para não derrubar o servidor embarcado;
- exibição da foto na tela local após leitura do cartão;
- fallback visual quando não há foto disponível;
- persistência da foto enviada pela web na QSPI.

Marco associado:

- `2d8db02` — `Foto em alta res(160x160) funcionando.`

### 3.7 Importação de usuários e dados externos

Foi criada uma ponte de importação offline para trazer perfis de sistemas externos, sem criar dependência em tempo de execução.

Capacidades entregues:

- importação de perfis por JSON via interface web;
- processamento de importação em background;
- suporte a dados exportados de Firebase/Firestore por fluxo offline;
- preparação de imagens externas para o formato usado na placa.

Isso permite aproveitar bases de usuários já existentes, inclusive para demonstrações e migração de dados.

### 3.8 DHCP, NTP e data/hora real

Após a estabilização da rede, foram integrados:

- DHCP para obtenção automática de IP;
- exibição do IP atual na UI local;
- sincronização de horário por NTP;
- uso do tempo sincronizado no log de acesso.

Com isso, os registros passaram a ter timestamp absoluto em vez de depender apenas do tick do sistema.

Marco associado:

- `5b233db` — `TUDO FUNCIONANDO!!!!!(só falta RTC)`

### 3.9 Log de acesso

O sistema passou a manter um log de acesso utilizável para auditoria e demonstração.

Entregas:

- log em RAM para operação em tempo real;
- página web dedicada `/access_log`;
- timestamps reais após sincronização NTP;
- persistência do log em QSPI;
- remoção do evento de fechamento automático de porta do log, mantendo apenas abertura.

### 3.10 Administração por perfis

Na fase mais recente, foi adicionada uma camada de administração baseada em perfis cadastrados, sem perder o fallback legado.

Entregas:

- perfis marcados como administradores;
- PIN próprio por perfil administrador;
- login web por PIN de administrador;
- autenticação local da engrenagem por PIN de administrador;
- persistência desse PIN no `users.json`;
- manutenção do PIN padrão `1234` como fallback seguro.

Marco associado:

- `38dfc4b` — `Add admin profiles with persisted PIN auth`

## 4. Funcionalidades implementadas

### 4.1 RFID e autenticação

- leitura de UID por RC522;
- suporte a UIDs de tamanhos diferentes;
- consulta ao cadastro persistido;
- autorização e negação de acesso;
- associação de mais de um cartão ao mesmo perfil.

### 4.2 Interface local

- tela de espera personalizada;
- tela de PIN;
- tela de IP/rede;
- tela de acesso autorizado/negado;
- exibição de foto, nome e `(CARGO-CAPÍTULO)`;
- brilho reduzido por inatividade.

### 4.3 Interface web

Rotas administrativas principais:

- `/`
- `/login`
- `/admin_profiles`
- `/profile_form`
- `/upload_photo`
- `/import`
- `/access_log`
- `/door`

Capacidades:

- login administrativo;
- CRUD de perfis;
- upload de foto;
- importação por JSON;
- visualização do log;
- controle de porta e luz.

### 4.4 Persistência

- perfis persistidos em `users.json`;
- fotos persistidas em arquivos binários;
- logs persistidos em `access.log`;
- carregamento seguro após boot;
- persistência automática de perfil novo.

### 4.5 Rede

- servidor HTTP funcional;
- páginas leves para reduzir travamentos;
- DHCP;
- NTP.

### 4.6 Controle físico

- acionamento da porta por pulso;
- acionamento da luz;
- botões físicos locais;
- LEDs onboard desligados no boot.

## 5. Arquitetura atual

Módulos centrais:

- [`src/main.c`](./src/main.c)
- [`src/rfid.c`](./src/rfid.c)
- [`src/ui.c`](./src/ui.c)
- [`src/net.c`](./src/net.c)
- [`src/storage.c`](./src/storage.c)
- [`src/gpio.c`](./src/gpio.c)

Tecnologias e componentes:

- `ThreadX`
- `NetX / NetX Duo`
- `FileX`
- `QSPI`
- `RC522`
- `SX8654`
- `ILI9341`

## 6. Estado funcional atual

No estado atual do projeto, estão operacionais:

- autenticação por cartão;
- abertura de porta;
- UI local;
- rede com DHCP;
- log web com timestamp absoluto;
- cadastro, edição e remoção de perfis;
- upload de foto;
- persistência de perfis, fotos e logs;
- autenticação administrativa por perfis.

## 7. Impacto técnico do trabalho

Do ponto de vista de engenharia, o projeto resultou em:

- integração completa entre hardware embarcado e interface web;
- transformação de uma prova de conceito em sistema demonstrável;
- criação de uma base extensível para futuras funções;
- organização do firmware em módulos separados;
- documentação técnica em português e inglês.

## 8. Próximos passos recomendados

Embora o sistema esteja funcional, algumas evoluções futuras são naturais:

- opcionalmente remover o fallback `1234`;
- fortalecer políticas de PIN administrativo;
- ampliar exportação/importação de perfis;
- melhorar o tratamento de fotos em lote;
- adicionar filtros e exportação no log de acesso;
- consolidar testes de regressão de web e storage.

## 9. Referências internas

Documentos relacionados no repositório:

- [`README.md`](./README.md)
- [`README_EN.md`](./README_EN.md)
- [`CURRENT_STATE.md`](./CURRENT_STATE.md)
- [`CURRENT_STATE_EN.md`](./CURRENT_STATE_EN.md)
- [`NETWORK_NOTES.md`](./NETWORK_NOTES.md)
- [`NETWORK_NOTES_EN.md`](./NETWORK_NOTES_EN.md)

## 10. Histórico resumido por marcos

- `818dcf9` — baseline estável inicial;
- `b55418d` — restauração do fluxo web/rede;
- `6fee797` — leitura, display, salvamento e rede funcional;
- `eec1cb5` — UI final pronta;
- `2d8db02` — foto em alta resolução funcionando;
- `aa58035` — fotos persistentes na QSPI;
- `7ed5264` — código principal funcionando, exceto DHCP;
- `5b233db` — sistema quase completo, faltando RTC/NTP;
- `38cfd68` — consolidação de estado pronto;
- `f633395` — ajustes na stack de rede;
- `0e06671` — novos ajustes na stack de rede;
- `38dfc4b` — perfis administradores com PIN persistido.

## 11. Conclusão

O desenvolvimento entregou um sistema embarcado de controle de acesso completo, com identidade visual própria, gerenciamento web, persistência em memória flash, suporte a fotos e registro de eventos com horário real.

O resultado final atende tanto a demonstrações técnicas quanto a extensões futuras pelo Ramo Estudantil IEEE UFJF e pelo IEEE Computer Society.
