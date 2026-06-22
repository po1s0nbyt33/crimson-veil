#include "game.h"
#include <string.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>

const char *meta_name(MetaType m) {
    switch (m) {
        case META_MAXHP:    return "Vitalidade";
        case META_MIGHT:    return "Forca";
        case META_SPEED:    return "Velocidade";
        case META_ARMOR:    return "Armadura";
        case META_REGEN:    return "Regeneracao";
        case META_GROWTH:   return "Sabedoria";
        case META_GREED:    return "Ganancia";
        case META_PICKUP:   return "Magnetismo";
        case META_AMOUNT:   return "Municao";
        case META_COOLDOWN: return "Agilidade";
        default:            return "?";
    }
}

const char *meta_desc(MetaType m) {
    switch (m) {
        case META_MAXHP:    return "+20 de vida maxima inicial";
        case META_MIGHT:    return "+10% de dano";
        case META_SPEED:    return "+5% de velocidade";
        case META_ARMOR:    return "+1 de armadura";
        case META_REGEN:    return "+0.4 de vida por segundo";
        case META_GROWTH:   return "+5% de XP ganho";
        case META_GREED:    return "+6% de moedas";
        case META_PICKUP:   return "+12% de raio de coleta";
        case META_AMOUNT:   return "+1 projetil inicial";
        case META_COOLDOWN: return "-4% de recarga";
        default:            return "";
    }
}

int meta_cost(const SaveData *s, MetaType m) {
    int cur = s->meta[m];
    if (cur >= META_MAX_LEVEL) return 0;
    return 60 * (cur + 1) + cur * cur * 30;
}

static uint32_t save_checksum(const SaveData *s) {
    const uint8_t *b = (const uint8_t *)s;
    size_t n = sizeof(SaveData) - sizeof(uint32_t);   /* exclui o proprio checksum */
    uint32_t h = 2166136261u;                          /* FNV-1a */
    for (size_t i = 0; i < n; ++i) {
        h ^= b[i];
        h *= 16777619u;
    }
    return h;
}

/* Prefixo do save v2 (campos antes de 'run'), para migracao de progresso. */
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint8_t  meta[META_COUNT];
    uint32_t coins;
    uint32_t unlocks;
    uint32_t best_time;
    uint32_t total_kills;
    uint32_t runs;
    uint8_t  opt_shake;
    uint8_t  opt_damage_numbers;
    uint8_t  opt_show_fps;
    uint8_t  opt_start_weapon;
    uint8_t  has_run;
    uint8_t  _pad[3];
} LegacyV2Head;

void save_defaults(SaveData *s) {
    memset(s, 0, sizeof(*s));
    s->magic = SAVE_MAGIC;
    s->version = SAVE_VERSION;
    s->coins = 0;
    s->weapon_unlocks = (1u << WEAPON_BOLT) | (1u << WEAPON_KNIFE) | (1u << WEAPON_WHIP);
    s->selected_char = 0;
    s->opt_shake = 1;
    s->opt_damage_numbers = 1;
    s->opt_show_fps = 0;
    s->has_run = 0;
}

bool save_load(SaveData *s) {
    save_defaults(s);

    SceUID fd = sceIoOpen(SAVE_PATH, SCE_O_RDONLY, 0777);
    if (fd < 0) return false;

    SaveData tmp;
    int rd = sceIoRead(fd, &tmp, sizeof(tmp));
    sceIoClose(fd);

    if (rd < (int)(2 * sizeof(uint32_t))) return false;
    if (tmp.magic != SAVE_MAGIC) return false;

    if (tmp.version == SAVE_VERSION) {
        if (rd != (int)sizeof(tmp)) return false;
        if (save_checksum(&tmp) != tmp.checksum) return false;
        *s = tmp;
        return true;
    }

    /* migracao do v2: preserva moedas, melhorias, desbloqueios e estatisticas */
    if (tmp.version == 2 && rd >= (int)sizeof(LegacyV2Head)) {
        const LegacyV2Head *h = (const LegacyV2Head *)&tmp;
        for (int i = 0; i < META_COUNT; ++i) s->meta[i] = h->meta[i];
        s->coins          = h->coins;
        if (h->unlocks)   s->weapon_unlocks = h->unlocks;
        s->best_time      = h->best_time;
        s->total_kills    = h->total_kills;
        s->runs           = h->runs;
        s->opt_shake          = h->opt_shake;
        s->opt_damage_numbers = h->opt_damage_numbers;
        s->opt_show_fps       = h->opt_show_fps;
        save_write(s);          /* regrava ja no formato v3 */
        return true;
    }

    return false;   /* versao desconhecida -> mantem defaults */
}

bool save_write(const SaveData *s) {
    sceIoMkdir(SAVE_DIR, 0777);                          /* ok se ja existir */

    SaveData tmp = *s;
    tmp.magic = SAVE_MAGIC;
    tmp.version = SAVE_VERSION;
    tmp.checksum = save_checksum(&tmp);

    SceUID fd = sceIoOpen(SAVE_PATH, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd < 0) return false;

    int wr = sceIoWrite(fd, &tmp, sizeof(tmp));
    sceIoClose(fd);
    return wr == (int)sizeof(tmp);
}

void save_wipe(SaveData *s) {
    save_defaults(s);
    save_write(s);
}
