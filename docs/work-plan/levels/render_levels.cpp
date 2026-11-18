// Draws scenes of levels 3-5 with the game's own code into BMPs (three-levels-plan.md, step 9). Run from the repo root:
//   c++ -std=c++14 -Isource docs/work-plan/levels/render_levels.cpp source/game.cpp source/screen.cpp -o build/render_levels
//   mkdir -p build/levels && build/render_levels && for f in build/levels/*.bmp; do sips -s format png "$f" --out docs/work-plan/levels/$(basename "${f%.bmp}").png; done
#include <stdio.h>
#include "game.h"
#include "screen.h"

static const Actions NONE = {};

static void run(Game& game, const Actions& a, float seconds) {
    for (int i = 0; i < (int)(seconds * 60 + 0.5f); i++) updateGame(game, &a, 1.0f / 60);
}

static void save(Game& game, const char* name) {
    for (Player& p : game.players) p.invincibleTimer = 0;   // a huge timer would draw the ship blinked off
    drawGame(game);
    for (Player& p : game.players) p.invincibleTimer = 1e9f;
    char path[128];
    snprintf(path, sizeof path, "build/levels/%s.bmp", name);
    if (!screenSaveBmp(path)) printf("could not write %s\n", path);
}

// A run at `level` whose ship can't die, started at `wave` (its aliens flown in, or its bosses hovering, attacks held).
static void startAt(Game& game, int level, int wave) {
    startGame(game, 1, 7, level);
    game.players[0].invincibleTimer = 1e9f;
    game.wave = wave - 1;
    for (Alien& alien : game.aliens) alien.alive = false;
    run(game, NONE, 3.5f);
    for (Boss& boss : game.bosses) boss.attackTimer = 100;
}

int main() {
    Actions fire = {};
    fire.fire = true;
    for (int level = 3; level <= 5; level++) {
        const char* names[] = {"", "", "", "glacier", "volcano", "sea"};
        char name[64];
        Game game;
        startAt(game, level, 8);   // a formation wave with a heavy
        run(game, fire, 1.2f);
        snprintf(name, sizeof name, "%s-wave", names[level]);
        save(game, name);

        startAt(game, level, 10);   // twin small bosses mid-attack
        game.bosses[0].attackTimer = 0.01f;
        game.bosses[1].attackTimer = 0.4f;
        game.bosses[0].nextAttack = game.bosses[1].nextAttack = 1;
        run(game, fire, 0.7f);
        Player& p = game.players[0];
        if (level == 3) p.frozenTimer = 1;   // show the ice cube
        if (level == 4) p.scorchTimer = 2.9f;   // and the scorch
        snprintf(name, sizeof name, "%s-twins", names[level]);
        save(game, name);

        startAt(game, level, 15);   // the big boss's signature attack
        Boss& boss = game.bosses[0];
        if (level == 3) {
            for (int i = 0; i < 6; i++) game.thrown.push_back({20.0f + i * 50, 0, 0, 140, THROWN_ICICLE, 0.5f, 0});
            game.thrown.push_back({150, 60, 0, 80, THROWN_ICE_BLOCK, 0, 0});
            boss.windupTimer = 0;
        } else if (level == 4) {
            boss.ballX = p.x + 40;
            boss.ballTimer = 0.4f;
            game.thrown.push_back({100, 90, 0, 60, THROWN_LAVA_DROP, 0, 0});
            game.thrown.push_back({200, 70, 20, 90, THROWN_FIREBALL, 0, 0});
        } else {
            boss.hookState = HOOK_DOWN;
            boss.hookX = p.x + 8;
            boss.hookY = boss.y + 1;
            run(game, NONE, 0.9f);
            game.thrown.push_back({60, 80, 0, 45, THROWN_BIG_BUBBLE, 0, 0});
            game.thrown.push_back({220, 100, 0, 60, THROWN_BUBBLE, 0, 0});
            game.thrown.push_back({240, 120, 0, 60, THROWN_BOOT, 0, 0});
        }
        run(game, fire, 0.3f);
        snprintf(name, sizeof name, "%s-boss", names[level]);
        save(game, name);
    }
    Game game;   // the yeti's sneeze
    startAt(game, 3, 5);
    game.bosses[0].nextAttack = 2;
    game.bosses[0].attackTimer = 0.01f;
    run(game, NONE, 0.62f);
    save(game, "glacier-sneeze");
    return 0;
}
