# Relatório Institucional
## Projeto e Implementação de uma Plataforma Embarcada de Controle de Acesso com Ethernet baseada em Microcontrolador Renesas Synergy

**Data:** `2026-03-20`  
**Ramo:** `Ramo Estudantil IEEE UFJF`  
**Capítulos:** `IEEE Computer Society Universidade Federal de Juiz de Fora Student Branch Chapter e IEEE Robotics and Automation Society Universidade Federal de Juiz de Fora Student Branch`  
**Projeto:** `IoTRamoRenesas`  
**Plataforma:** `Renesas SK-S7G2`  

## Contexto

O projeto `IoTRamoRenesas` nasceu como uma proposta de sistema embarcado de controle de acesso. A ideia inicial era simples: usar a placa `Renesas SK-S7G2` para ler cartões RFID, mostrar o resultado em uma interface local e permitir algum nível de administração pela rede. Com o avanço do trabalho, esse escopo cresceu e o projeto deixou de ser apenas uma prova de conceito.

Hoje, a solução reúne autenticação por RFID, interface local em display com touch, painel web administrativo, persistência em memória QSPI, exibição de foto do usuário, controle de porta e registro de eventos. Em outras palavras, o que antes era uma base experimental passou a funcionar como um sistema embarcado completo, já em condição de demonstração.

## O que foi desenvolvido

Ao longo do desenvolvimento, o trabalho se concentrou em quatro frentes principais.

A primeira foi a estabilização do firmware. Foi necessário recuperar a leitura RFID, reorganizar a UI local, corrigir travamentos e construir uma base de execução confiável. Essa etapa foi importante porque o projeto tinha vários pontos sensíveis, especialmente em interface, rede e acesso à memória persistente.

A segunda frente foi a parte de rede. A interface web foi redesenhada para funcionar em páginas menores, mais leves, compatíveis com o servidor HTTP embarcado. Isso permitiu viabilizar o cadastro, a edição e a administração de perfis sem comprometer o restante do firmware. Também foi integrado o uso de DHCP para obtenção automática de IP e NTP para sincronização de horário.

A terceira frente foi a persistência. O sistema passou a salvar perfis, fotos e logs em QSPI com FileX, sem quebrar o boot da placa. Esse ponto exigiu bastante cuidado, porque a ordem e o momento de acesso à mídia influenciavam diretamente a estabilidade do sistema. O resultado foi um fluxo de persistência funcional para `users.json`, `access.log` e arquivos binários de foto.

Por fim, houve uma ampliação clara do modelo de usuário. O cadastro deixou de ser apenas um UID associado a um nome e passou a aceitar perfis com nome, cargo, capítulo IEEE, foto, múltiplos cartões e privilégios administrativos. Também foi adicionada a autenticação por PIN para perfis administradores, tanto na interface web quanto na interface local.

## Estado atual

Na data deste relatório, o projeto está funcional para demonstração e uso controlado. Os principais recursos operacionais são:

- leitura de cartão RFID e validação de acesso;
- abertura de porta em caso de autenticação autorizada;
- interface local com tela de espera, tela de PIN e tela de resultado;
- interface web administrativa para gerenciamento de perfis;
- cadastro, edição e remoção de usuários;
- upload e exibição de fotos de perfil;
- perfis administradores com PIN próprio;
- registro de logs de acesso com horário sincronizado por rede;
- persistência de perfis, fotos e logs em memória QSPI.

Também vale destacar que o sistema já consegue importar perfis por JSON e aproveitar dados oriundos de uma base externa, o que abre caminho para integração com fluxos já existentes no contexto do ramo.

## Relevância do projeto

O valor do `IoTRamoRenesas` está no fato de ele reunir, em um único sistema, várias camadas de engenharia que costumam aparecer separadas em projetos acadêmicos. Aqui, elas precisaram conviver ao mesmo tempo: firmware, interface gráfica, autenticação, rede, armazenamento persistente e operação física sobre a placa.

Para o Ramo Estudantil IEEE UFJF, isso significa ter uma base concreta para demonstrações técnicas, formação de novos membros e continuidade de desenvolvimento. Para o IEEE Computer Society, o projeto dialoga diretamente com áreas centrais como sistemas embarcados, redes embarcadas, software de baixo nível e integração entre hardware e software.

## Fechamento

O `IoTRamoRenesas` chegou a um ponto em que já pode ser apresentado como uma entrega técnica madura. Ainda há espaço para evolução, como em qualquer projeto vivo, mas o núcleo do sistema está construído, integrado e funcionando.

Mais do que um experimento isolado, o projeto se consolidou como uma plataforma embarcada real, capaz de demonstrar competência técnica, organização de desenvolvimento e potencial de continuidade dentro das atividades do IEEE UFJF.
