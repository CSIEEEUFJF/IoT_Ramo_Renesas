# Ethernet Notes

Estado atual:
- O projeto compila e inicia normalmente.
- A aplicacao usa IP fixo em `192.168.15.180` em `src/net.c`.
- A versao atual do workspace voltou a responder na interface web.
- O maior achado foi de software/stack da thread de rede, nao de PHY.

O que ja foi testado:
- `sf_el_nx` no canal 1 (`ETHERC1/EDMAC1`).
- Reset do PHY em `P806`.
- `P806` subindo em nivel alto no boot.
- Variacoes de DHCP.
- Variacoes de bring-up do PHY/MAC.
- Teste com cabo e switch diferentes.
- Tentativa de forcar modo fixo no PHY.
- Comparacao com a versao funcional do proprio workspace.

O que parece ter destravado a rede:
- `render_home_page()` em `src/net.c` usando buffers `static` para HTML e linhas da tabela, evitando estouro de stack.
- `packet_pool`, `ip` e `http_server` com `init.enable` no gerado.
- Prioridades geradas mais conservadoras:
  - `g_ip0`: prioridade 10
  - `Main Thread`: prioridade 5
  - `HTTP server`: prioridade 8
- `g_common_init()` chamando `packet_pool_init0()` e `ip_init0()` automaticamente.

Leitura atual:
- A rede funcional atual parece depender mais da ordem de inicializacao e do uso de memoria na thread HTTP do que de ajuste de PHY.

Arquivos principais relacionados:
- `src/net.c`
- `src/synergy_gen/common_data.c`
- `src/synergy_gen/pin_data.c`
- `synergy/ssp/src/framework/sf_el_nx/nx_hw_init.c`
- `synergy/ssp/src/framework/sf_el_nx/phy/ether_phy.c`
