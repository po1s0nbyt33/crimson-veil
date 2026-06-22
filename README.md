# 🦇 Crimson Veil

> Um **roguelite de sobrevivência por hordas** em **pixel art**, nativo para o **PlayStation Vita**.
> Sobreviva a ondas infinitas de criaturas da noite, evolua suas armas e enfrente chefes implacáveis.

![Platform](https://img.shields.io/badge/platform-PS%20Vita-blue)
![Language](https://img.shields.io/badge/language-C-orange)
![SDK](https://img.shields.io/badge/built%20with-VitaSDK%20%2B%20vita2d-green)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

---

## ⚖️ Aviso

Este é um **homebrew original e independente**, feito como projeto pessoal/educacional.
É **inspirado no gênero** "bullet-heaven / horde survival", mas **não é afiliado, patrocinado ou endossado**
por nenhum jogo comercial. Todo o código é original e **todos os sprites e sons são gerados proceduralmente
em tempo de execução** (não há assets de terceiros embutidos). O nome "Crimson Veil" é original.

---

## ✨ Funcionalidades

- **Gameplay estilo bullet-heaven**: ataque automático, mundo infinito com câmera que segue o jogador.
- **4 personagens** jogáveis, cada um com arma inicial, atributos e estilo próprios (compráveis com moedas).
- **5 armas base + 5 evoluções (fusões)** — funda uma arma de alto nível com uma passiva certa para criar uma versão muito mais forte.
- **11 atributos passivos** e tela de melhoria a cada nível.
- **Inimigos variados**: enxames, corredores, brutamontes, atiradores e magos (com projéteis teleguiados).
- **5 chefes** com padrões distintos: investida, invocação de hordas, leques de projéteis, *bullet-hell* radial e baque em área.
- **Progressão permanente** entre partidas: loja de melhorias, **sistema de missões** com recompensas e **coleção** de tudo que foi desbloqueado.
- **Áudio procedural**: música chiptune para menu/gameplay/chefe e efeitos sonoros sintetizados — zero arquivos de áudio.
- **Save persistente** em `ux0:data/CrimsonVeil/` (binário com versão + checksum).

## 🎮 Controles

| Ação | Botão |
|------|-------|
| Mover | Analógico esquerdo / D-Pad |
| Confirmar | **✕** |
| Cancelar / Voltar | **◯** |
| Pausar | **START** |
| Iniciar rápido (na loja) | **△** |

> O ataque é **automático** — concentre-se em se posicionar, coletar XP e escolher as melhorias.

## 📥 Instalação (no PS Vita)

Requer um Vita com homebrew habilitado (HENkaku / h-encore / Enso).

1. Baixe o **[`CrimsonVeil.vpk`](CrimsonVeil.vpk)** (está na raiz deste repositório).
2. Copie para o cartão/memória do Vita.
3. Abra o **VitaShell**, navegue até o arquivo, pressione **✕** e confirme a instalação.
4. O app **Crimson Veil** aparecerá no LiveArea.

## 🛠️ Compilando do código-fonte

Requisitos: **[VitaSDK](https://vitasdk.org/)** instalado, com `vita2d` e dependências
(`zlib bzip2 libpng libjpeg-turbo freetype harfbuzz libvita2d` via `vdpm`), além de `cmake` e `make`.

```bash
export VITASDK=/usr/local/vitasdk
export PATH=$VITASDK/bin:$PATH

git clone https://github.com/<seu-usuario>/crimson-veil.git
cd crimson-veil
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake ..
make            # gera CrimsonVeil.vpk
```

Também há um `Makefile` alternativo (sem CMake): basta `make` na raiz.

## 📂 Estrutura do projeto

```
crimson-veil/
├── CMakeLists.txt        # build principal (CMake + VitaSDK)
├── Makefile              # build alternativo
├── CrimsonVeil.vpk       # pacote pré-compilado (pronto para instalar)
├── README.md · LICENSE · .gitignore
├── sce_sys/
│   └── icon0.png         # ícone do app (128x128)
└── src/
    ├── config.h          # constantes de balanceamento e limites de pool
    ├── types.{h,c}       # Vec2, utilitários, RNG (xorshift32)
    ├── input.{h,c}       # leitura do controle (SceCtrl)
    ├── render.{h,c}      # camada sobre vita2d: fonte PGF, texto, formas
    ├── sprite.{h,c}      # sprites pixel-art gerados proceduralmente (texturas)
    ├── audio.{h,c}       # sintetizador chiptune + SFX (thread sceAudioOut)
    ├── player.{h,c}      # movimento, vida, XP/nível, atributos
    ├── weapon.{h,c}      # armas, evoluções e projéteis
    ├── enemy.{h,c}       # inimigos, IA e projéteis inimigos
    ├── boss.{h,c}        # chefes e seus padrões
    ├── pickup.{h,c}      # gemas de XP, moedas, cura, ímã, bomba
    ├── wave.{h,c}        # diretor de ondas e dificuldade
    ├── upgrade.{h,c}     # sorteio/aplicação de melhorias e fusões
    ├── fx.{h,c}          # números de dano, partículas, tremor de tela
    ├── character.{h,c}   # personagens jogáveis
    ├── fusion.{h,c}      # receitas de fusão de armas
    ├── mission.{h,c}     # missões e recompensas
    ├── collection.{h,c}  # coleção de itens desbloqueáveis
    ├── save.{h,c}        # save/load binário + migração de versão
    ├── menu.{h,c}        # menus (inicial, seleção, loja, coleção, missões, etc.)
    ├── game.{h,c}        # struct central + máquina de estados + HUD
    └── main.c            # init e loop principal (timing por dt)
```

## 🧠 Destaques técnicos

- **Sprites procedurais**: criaturas pixel-art simétricas e ícones de armas são gerados em texturas no boot
  (filtro POINT para pixels nítidos). Nada é carregado do disco em runtime → carregamento instantâneo e baixo uso de memória.
- **Áudio sintetizado**: um pequeno sintetizador (ondas quadrada/triangular/ruído/seno) roda numa thread própria
  via `sceAudioOut`, tocando música e efeitos gerados em tempo real.
- **Sem `malloc` no loop**: todas as entidades usam pools de tamanho fixo (estáticos), ideal para o hardware do Vita.
- **Renderização otimizada**: *culling* de tudo fora da tela e pool de vértices do vita2d ampliado.

## 🗺️ Roadmap (ideias futuras)

- LiveArea customizada (background/startup) em PNG indexado.
- Mais personagens, armas, fusões e chefes.
- Sprites desenhados à mão (substituindo os procedurais).
- Mais faixas musicais.

## 📜 Licença

Distribuído sob a licença **MIT** — veja [`LICENSE`](LICENSE).

Feito com 🩸 para o PlayStation Vita.
