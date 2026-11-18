// Renders game scenes for the level guide PDF with the game's own code.
#include <stdio.h>
#include "game.h"
#include "screen.h"

static const Actions NONE = {};
static Actions FIRE = {};
static const char* OUT;

static void run(Game& game, const Actions& a, float seconds) {
    for (int i = 0; i < (int)(seconds * 60 + 0.5f); i++) updateGame(game, &a, 1.0f / 60);
}

static void save(Game& game, const char* name) {
    for (Player& p : game.players) p.invincibleTimer = 0;
    game.trauma = 0;
    game.flashTimer = 0;
    drawGame(game);
    for (Player& p : game.players) p.invincibleTimer = 1e9f;
    char path[256];
    snprintf(path, sizeof path, "%s/%s.bmp", OUT, name);
    if (!screenSaveBmp(path)) printf("could not write %s\n", path);
}

static void saveBackground(Game& game, const char* name) {
    drawBackground(game);
    char path[256];
    snprintf(path, sizeof path, "%s/%s.bmp", OUT, name);
    screenSaveBmp(path);
}

static void startAt(Game& game, int level, int wave, int weapon = 3) {
    startGame(game, 1, 7 + level * 13 + wave, level);
    game.players[0].invincibleTimer = 1e9f;
    game.players[0].weaponLevel = weapon;
    game.wave = wave - 1;
    for (Alien& alien : game.aliens) alien.alive = false;
    run(game, NONE, 3.5f);
    for (Boss& boss : game.bosses) boss.attackTimer = 100;
}

static void attackNow(Boss& boss, int next) {
    boss.nextAttack = next;
    boss.attackTimer = 0.01f;
}

int main(int argc, char** argv) {
    OUT = argv[1];
    FIRE.fire = true;
    const char* names[] = {"", "space", "jungle", "glacier", "volcano", "sea"};
    char name[64];
    for (int level = 1; level <= 5; level++) {
        Game game;
        startAt(game, level, level == 1 ? 13 : 8);   // a busy formation wave (with a roamer after level 1)
        run(game, FIRE, 1.4f);
        snprintf(name, sizeof name, "%s-wave", names[level]);
        save(game, name);
        game.sceneryTime += 3;
        snprintf(name, sizeof name, "%s-bg", names[level]);
        saveBackground(game, name);
        game.sceneryTime += 4;
        snprintf(name, sizeof name, "%s-bg2", names[level]);
        saveBackground(game, name);

        Game early;
        startAt(early, level, 2, 1);
        run(early, FIRE, 0.8f);
        snprintf(name, sizeof name, "%s-early", names[level]);
        save(early, name);
    }

    // Level 1: classic fan, twin classics, mage charging, mage beam, twin mages.
    { Game g; startAt(g, 1, 5); attackNow(g.bosses[0], 1); run(g, FIRE, 0.45f); save(g, "space-classic"); }
    { Game g; startAt(g, 1, 10); attackNow(g.bosses[0], 0); attackNow(g.bosses[1], 1); run(g, FIRE, 0.5f); save(g, "space-twins"); }
    { Game g; startAt(g, 1, 15); Boss& b = g.bosses[0];
      b.ballX = b.x + 16; b.ballY = b.y + 1; b.ballTimer = 1.5f; run(g, FIRE, 0.9f); save(g, "space-mage-charge");
      run(g, FIRE, 0.75f); save(g, "space-mage-beam"); }
    { Game g; startAt(g, 1, 20); attackNow(g.bosses[0], 1); attackNow(g.bosses[1], 0); run(g, FIRE, 0.5f); save(g, "space-mages"); }

    // Jungle: monkeys with bananas and a peel, gorilla pound and coconuts, stink mist and laugh.
    { Game g; startAt(g, 2, 5); attackNow(g.bosses[0], 1); run(g, FIRE, 0.5f); save(g, "jungle-monkey"); }
    { Game g; startAt(g, 2, 10); attackNow(g.bosses[0], 0); attackNow(g.bosses[1], 1); run(g, FIRE, 1.6f); save(g, "jungle-monkeys"); }
    { Game g; startAt(g, 2, 15); Boss& b = g.bosses[0]; b.windupTimer = 0.2f; run(g, NONE, 0.5f); save(g, "jungle-pound"); }
    { Game g; startAt(g, 2, 15); Boss& b = g.bosses[0]; b.windupTimer = 0.4f; run(g, NONE, 0.2f); save(g, "jungle-pound-windup"); }
    { Game g; startAt(g, 2, 20); Player& p = g.players[0];
      g.mists.push_back({p.x + 8, p.y + 7, 1.0f}); g.mists.push_back({60, 140, 2.0f});
      g.peels.push_back({240, 150, 4}); run(g, FIRE, 0.6f); g.bosses[0].laughTimer = 0.5f; save(g, "jungle-gorillas"); }
    { Game g; startAt(g, 2, 15); Player& p = g.players[0]; p.stunTimer = 0.8f; g.peels.push_back({60, 150, 4});
      run(g, NONE, 0.1f); save(g, "jungle-stun"); }

    // Glacier.
    { Game g; startAt(g, 3, 5); attackNow(g.bosses[0], 2); run(g, NONE, 0.62f); save(g, "glacier-sneeze"); }
    { Game g; startAt(g, 3, 5); attackNow(g.bosses[0], 1); run(g, FIRE, 0.5f); g.players[0].frozenTimer = 1; save(g, "glacier-yeti"); }
    { Game g; startAt(g, 3, 10); attackNow(g.bosses[0], 0); attackNow(g.bosses[1], 1); run(g, FIRE, 0.6f); save(g, "glacier-twins"); }
    { Game g; startAt(g, 3, 15); for (int i = 0; i < 6; i++) g.thrown.push_back({20.0f + i * 50, 0, 0, 140, THROWN_ICICLE, 0.5f, 0});
      run(g, FIRE, 0.3f); save(g, "glacier-golem"); }
    { Game g; startAt(g, 3, 20); g.thrown.push_back({150, 60, 0, 80, THROWN_ICE_BLOCK, 0, 0});
      g.thrown.push_back({60, 50, -10, 80, THROWN_ICE_BLOCK, 0, 0}); run(g, FIRE, 0.4f); save(g, "glacier-golems"); }

    // Volcano.
    { Game g; startAt(g, 4, 5); attackNow(g.bosses[0], 1); run(g, FIRE, 0.7f); save(g, "volcano-imp"); }
    { Game g; startAt(g, 4, 10); attackNow(g.bosses[0], 2); attackNow(g.bosses[1], 1); run(g, FIRE, 0.6f);
      g.players[0].scorchTimer = 2.9f; run(g, FIRE, 0.15f); save(g, "volcano-twins"); }
    { Game g; startAt(g, 4, 15); Boss& b = g.bosses[0]; Player& p = g.players[0];
      b.ballX = p.x + 50; b.ballY = 180; b.ballTimer = 0.5f; run(g, FIRE, 0.3f); save(g, "volcano-warn");
      run(g, FIRE, 0.4f); save(g, "volcano-eruption"); }
    { Game g; startAt(g, 4, 20); float lane = 315 / 5.0f;
      for (int i = 0; i < 5; i++) g.thrown.push_back({i * lane + 20, -6, 0, 70, THROWN_LAVA_DROP, 0, 0});
      run(g, FIRE, 0.9f); save(g, "volcano-lords"); }

    // Deep sea.
    { Game g; startAt(g, 5, 5); attackNow(g.bosses[0], 0); run(g, FIRE, 0.65f); save(g, "sea-angler"); }
    { Game g; startAt(g, 5, 5); g.bosses[0].nextAttack = 1; g.bosses[0].attackTimer = 0.01f; run(g, FIRE, 1.2f); save(g, "sea-bigbubble"); }
    { Game g; startAt(g, 5, 10); attackNow(g.bosses[0], 2); attackNow(g.bosses[1], 1); run(g, FIRE, 0.6f); save(g, "sea-twins"); }
    { Game g; startAt(g, 5, 15); Boss& b = g.bosses[0]; Player& p = g.players[0];
      b.hookState = HOOK_DOWN; b.hookX = p.x + 8; b.hookY = b.y + 1; run(g, NONE, 1.0f);
      g.thrown.push_back({240, 110, 0, 60, THROWN_BOOT, 0, 0}); run(g, NONE, 0.3f); save(g, "sea-hook"); }
    { Game g; startAt(g, 5, 20); attackNow(g.bosses[0], 0); run(g, FIRE, 0.5f);
      g.thrown.push_back({200, 90, -20, 60, THROWN_BOOT, 0, 0}); save(g, "sea-fishermen"); }
    return 0;
}
