cmake_minimum_required(VERSION 3.10)

## ---------------------------------------------------------------------------
## VitaSDK toolchain bootstrap
## ---------------------------------------------------------------------------
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake"
        CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Defina a variavel de ambiente VITASDK apontando para o seu SDK!")
  endif()
endif()

project(CrimsonVeil C)

include("${VITASDK}/share/vita.cmake" REQUIRED)

## ---------------------------------------------------------------------------
## Metadados do pacote (.vpk)
## ---------------------------------------------------------------------------
set(VITA_APP_NAME  "Crimson Veil")
set(VITA_TITLEID   "CRMV00001")
set(VITA_VERSION   "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -O3 -std=c11")

include_directories(src)

## Todos os modulos do jogo.
add_executable(${PROJECT_NAME}
  src/main.c
  src/types.c
  src/input.c
  src/render.c
  src/sprite.c
  src/save.c
  src/player.c
  src/weapon.c
  src/enemy.c
  src/boss.c
  src/pickup.c
  src/wave.c
  src/upgrade.c
  src/fx.c
  src/character.c
  src/fusion.c
  src/mission.c
  src/collection.c
  src/audio.c
  src/menu.c
  src/game.c
)

target_link_libraries(${PROJECT_NAME}
  vita2d
  SceDisplay_stub
  SceGxm_stub
  SceSysmodule_stub
  SceCtrl_stub
  SceTouch_stub
  SceAppMgr_stub
  SceCommonDialog_stub
  SceLibKernel_stub
  SceKernelThreadMgr_stub
  SceAudio_stub
  ScePgf_stub
  ## freetype e harfbuzz tem dependencia circular -> grupo de link
  -Wl,--start-group
  freetype
  harfbuzz
  png
  jpeg
  z
  bz2
  -Wl,--end-group
  m
  c
)

## ---------------------------------------------------------------------------
## Geracao do .self e do .vpk
## ---------------------------------------------------------------------------
vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})

## IMPORTANTE: todas as imagens em sce_sys/ DEVEM ser PNG indexado de 8 bits
## (color type 3). PNG RGBA faz o instalador do Vita falhar com erro 0x8010113D.
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
