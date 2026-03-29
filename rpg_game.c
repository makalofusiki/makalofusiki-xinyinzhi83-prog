/*
 * Final Fantasy Style RPG Game (C Language)
 * 类最终幻想风格的RPG游戏
 *
 * Features:
 *   - Character system with HP, MP, Attack, Defense, Level
 *   - Turn-based battle system with Attack and Magic actions
 *   - Experience and level-up system
 *   - Random enemy encounters
 *   - Simple console UI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ─────────────────────────────────────────
   Constants
   ───────────────────────────────────────── */
#define MAX_NAME_LEN      32
#define MAX_LEVEL         99
#define EXP_PER_LEVEL     100   /* base EXP needed to level up */
#define MAGIC_MP_COST     10    /* MP consumed per magic cast   */

/* ─────────────────────────────────────────
   Data structures
   ───────────────────────────────────────── */

typedef struct {
    char  name[MAX_NAME_LEN];
    int   hp;
    int   max_hp;
    int   mp;
    int   max_mp;
    int   attack;
    int   defense;
    int   level;
    int   exp;
    int   exp_to_next;   /* EXP needed to reach next level */
} Character;

typedef struct {
    char  name[MAX_NAME_LEN];
    int   hp;
    int   max_hp;
    int   attack;
    int   defense;
    int   exp_reward;
    int   gold_reward;
} Enemy;

/* ─────────────────────────────────────────
   Utility helpers
   ───────────────────────────────────────── */

/* Return a random integer in [lo, hi] */
static int rand_range(int lo, int hi)
{
    return lo + rand() % (hi - lo + 1);
}

/* Print a horizontal divider */
static void print_divider(void)
{
    printf("========================================\n");
}

/* Pause and wait for the player to press Enter */
static void press_enter(void)
{
    printf("[ Press Enter to continue... ]\n");
    /* Flush any leftover newline before waiting */
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

/* ─────────────────────────────────────────
   Character / Enemy initialisation
   ───────────────────────────────────────── */

static void init_character(Character *c, const char *name)
{
    strncpy(c->name, name, MAX_NAME_LEN - 1);
    c->name[MAX_NAME_LEN - 1] = '\0';
    c->level       = 1;
    c->exp         = 0;
    c->exp_to_next = EXP_PER_LEVEL;
    c->max_hp      = 100;
    c->hp          = c->max_hp;
    c->max_mp      = 50;
    c->mp          = c->max_mp;
    c->attack      = 15;
    c->defense     = 10;
}

/*
 * Enemy table – add more rows to expand the bestiary.
 * Fields: name, max_hp, attack, defense, exp_reward, gold_reward
 */
typedef struct {
    const char *name;
    int max_hp;
    int attack;
    int defense;
    int exp_reward;
    int gold_reward;
} EnemyTemplate;

static const EnemyTemplate ENEMY_TABLE[] = {
    { "哥布林 (Goblin)",       40,  8,  4,  20, 10 },
    { "骷髅兵 (Skeleton)",     55, 10,  6,  30, 15 },
    { "毒蜘蛛 (Poison Spider)", 35, 12,  3,  25, 12 },
    { "石像鬼 (Gargoyle)",     70, 14,  9,  45, 20 },
    { "暗影狼 (Shadow Wolf)",   60, 16,  7,  40, 18 },
};
#define ENEMY_COUNT (int)(sizeof(ENEMY_TABLE) / sizeof(ENEMY_TABLE[0]))

static void spawn_enemy(Enemy *e, int player_level)
{
    /* Pick a random template, scaled slightly with player level */
    int idx = rand_range(0, ENEMY_COUNT - 1);
    const EnemyTemplate *t = &ENEMY_TABLE[idx];

    strncpy(e->name, t->name, MAX_NAME_LEN - 1);
    e->name[MAX_NAME_LEN - 1] = '\0';

    /* Scale stats with player level (soft scaling) */
    float scale = 1.0f + (player_level - 1) * 0.15f;
    e->max_hp      = (int)(t->max_hp      * scale);
    e->hp          = e->max_hp;
    e->attack      = (int)(t->attack      * scale);
    e->defense     = (int)(t->defense     * scale);
    e->exp_reward  = (int)(t->exp_reward  * scale);
    e->gold_reward = (int)(t->gold_reward * scale);
}

/* ─────────────────────────────────────────
   Status display
   ───────────────────────────────────────── */

static void show_character_status(const Character *c)
{
    printf("  [%s]  Lv.%d\n", c->name, c->level);
    printf("  HP: %d/%d   MP: %d/%d\n", c->hp, c->max_hp, c->mp, c->max_mp);
    printf("  ATK: %d   DEF: %d   EXP: %d/%d\n",
           c->attack, c->defense, c->exp, c->exp_to_next);
}

static void show_enemy_status(const Enemy *e)
{
    printf("  [%s]\n", e->name);
    printf("  HP: %d/%d   ATK: %d   DEF: %d\n",
           e->hp, e->max_hp, e->attack, e->defense);
}

static void show_battle_status(const Character *c, const Enemy *e)
{
    print_divider();
    printf("  ★ 你的状态 (Your Status)\n");
    show_character_status(c);
    printf("\n  ★ 敌人状态 (Enemy Status)\n");
    show_enemy_status(e);
    print_divider();
}

/* ─────────────────────────────────────────
   Damage calculation
   ───────────────────────────────────────── */

/* Physical damage: attacker_atk vs defender_def, with ±20 % random variance */
static int calc_physical_damage(int atk, int def)
{
    int base = atk - def / 2;
    if (base < 1) base = 1;
    int variance = rand_range(-base / 5, base / 5);
    int dmg = base + variance;
    return (dmg < 1) ? 1 : dmg;
}

/* Magic damage: fixed power ignoring defense, higher variance */
static int calc_magic_damage(int atk)
{
    int base = atk * 3 / 2;
    int variance = rand_range(-base / 4, base / 4);
    int dmg = base + variance;
    return (dmg < 5) ? 5 : dmg;
}

/* ─────────────────────────────────────────
   Level-up logic
   ───────────────────────────────────────── */

static void try_level_up(Character *c)
{
    while (c->exp >= c->exp_to_next && c->level < MAX_LEVEL) {
        c->exp        -= c->exp_to_next;
        c->level      += 1;
        c->exp_to_next = EXP_PER_LEVEL + (c->level - 1) * 20;

        /* Stat growth on level-up */
        int hp_gain  = rand_range(10, 20);
        int mp_gain  = rand_range(5,  10);
        int atk_gain = rand_range(2,   5);
        int def_gain = rand_range(1,   3);

        c->max_hp  += hp_gain;
        c->max_mp  += mp_gain;
        c->attack  += atk_gain;
        c->defense += def_gain;

        /* Fully restore HP/MP on level-up (classic FF behaviour) */
        c->hp = c->max_hp;
        c->mp = c->max_mp;

        printf("\n  ✨ 升级了！(Level Up!) Lv.%d → Lv.%d ✨\n",
               c->level - 1, c->level);
        printf("  HP +%d  MP +%d  ATK +%d  DEF +%d\n",
               hp_gain, mp_gain, atk_gain, def_gain);
        printf("  HP 和 MP 已完全恢复！(HP and MP fully restored!)\n");
    }
}

/* ─────────────────────────────────────────
   Battle system
   ───────────────────────────────────────── */

typedef enum {
    BATTLE_ONGOING,
    BATTLE_WIN,
    BATTLE_LOSE
} BattleResult;

/* Player's turn – returns BATTLE_WIN if enemy dies, else BATTLE_ONGOING */
static BattleResult player_turn(Character *c, Enemy *e)
{
    int choice = 0;
    printf("\n  行动指令 (Action):\n");
    printf("  [1] 攻击 (Attack)\n");
    printf("  [2] 魔法 (Magic)  -- 消耗 %d MP\n", MAGIC_MP_COST);
    printf("  选择 (Choose): ");

    if (scanf("%d", &choice) != 1) choice = 1;
    /* Consume trailing newline */
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;

    if (choice == 2) {
        if (c->mp < MAGIC_MP_COST) {
            printf("\n  ❌ MP 不足！(Not enough MP!) 改为普通攻击。\n");
            choice = 1;
        }
    }

    if (choice == 2) {
        c->mp -= MAGIC_MP_COST;
        int dmg = calc_magic_damage(c->attack);
        e->hp -= dmg;
        printf("\n  %s 施放魔法！(casts magic!)\n", c->name);
        printf("  造成 %d 点魔法伤害！(Magic damage: %d)\n", dmg, dmg);
    } else {
        int dmg = calc_physical_damage(c->attack, e->defense);
        e->hp -= dmg;
        printf("\n  %s 发动攻击！(attacks!)\n", c->name);
        printf("  造成 %d 点伤害！(Damage: %d)\n", dmg, dmg);
    }

    if (e->hp <= 0) {
        e->hp = 0;
        printf("\n  ⚔️  %s 被击败了！(Enemy defeated!)\n", e->name);
        return BATTLE_WIN;
    }
    return BATTLE_ONGOING;
}

/* Enemy's turn – returns BATTLE_LOSE if player dies, else BATTLE_ONGOING */
static BattleResult enemy_turn(Character *c, const Enemy *e)
{
    int dmg = calc_physical_damage(e->attack, c->defense);
    c->hp -= dmg;
    if (c->hp < 0) c->hp = 0;

    printf("\n  %s 攻击了你！(Enemy attacks!)\n", e->name);
    printf("  你受到 %d 点伤害！(You took %d damage)\n", dmg, dmg);

    if (c->hp <= 0) {
        printf("\n  💀 你被击败了！(You were defeated...)\n");
        return BATTLE_LOSE;
    }
    return BATTLE_ONGOING;
}

/* Main battle loop */
static BattleResult run_battle(Character *c, Enemy *e)
{
    printf("\n");
    print_divider();
    printf("  ⚔️  战斗开始！(Battle Start!) ⚔️\n");
    printf("  遭遇了 %s！(You encountered %s!)\n", e->name, e->name);
    print_divider();
    press_enter();

    for (;;) {
        show_battle_status(c, e);

        /* --- Player turn --- */
        BattleResult r = player_turn(c, e);
        press_enter();
        if (r == BATTLE_WIN)  return BATTLE_WIN;

        /* --- Enemy turn --- */
        r = enemy_turn(c, e);
        press_enter();
        if (r == BATTLE_LOSE) return BATTLE_LOSE;
    }
}

/* Post-battle rewards */
static void give_rewards(Character *c, const Enemy *e)
{
    printf("\n  🎉 胜利！(Victory!)\n");
    printf("  获得 %d 经验值 和 %d 金币！\n",
           e->exp_reward, e->gold_reward);
    printf("  (Gained %d EXP and %d Gold!)\n",
           e->exp_reward, e->gold_reward);

    c->exp += e->exp_reward;
    try_level_up(c);
}

/* ─────────────────────────────────────────
   Main menu
   ───────────────────────────────────────── */

static void show_main_menu(void)
{
    print_divider();
    printf("  ★ 主菜单 (Main Menu) ★\n");
    print_divider();
    printf("  [1] 开始战斗 (Battle)\n");
    printf("  [2] 查看状态 (Status)\n");
    printf("  [3] 退出游戏 (Quit)\n");
    print_divider();
    printf("  选择 (Choose): ");
}

/* ─────────────────────────────────────────
   Entry point
   ───────────────────────────────────────── */

int main(void)
{
    srand((unsigned int)time(NULL));

    /* ── Title screen ── */
    print_divider();
    printf("  ★★★  类最终幻想 RPG 游戏  ★★★\n");
    printf("  ★★★  Final Fantasy Style RPG  ★★★\n");
    print_divider();
    printf("\n  请输入你的角色名字 (Enter your character name): ");

    char player_name[MAX_NAME_LEN];
    if (fgets(player_name, sizeof(player_name), stdin) == NULL) {
        strncpy(player_name, "勇者", MAX_NAME_LEN - 1);
        player_name[MAX_NAME_LEN - 1] = '\0';
    } else {
        /* Strip trailing newline */
        size_t len = strlen(player_name);
        if (len > 0 && player_name[len - 1] == '\n')
            player_name[len - 1] = '\0';
        if (player_name[0] == '\0')
            strncpy(player_name, "勇者", MAX_NAME_LEN - 1);
    }

    Character player;
    init_character(&player, player_name);

    printf("\n  欢迎，%s！冒险开始了！(Welcome, %s! The adventure begins!)\n\n",
           player.name, player.name);
    press_enter();

    /* ── Game loop ── */
    int running = 1;
    while (running) {
        show_main_menu();

        int choice = 0;
        if (scanf("%d", &choice) != 1) choice = 3;
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;

        switch (choice) {
        case 1: {
            /* Spawn a random enemy and fight */
            Enemy enemy;
            spawn_enemy(&enemy, player.level);
            BattleResult result = run_battle(&player, &enemy);

            if (result == BATTLE_WIN) {
                give_rewards(&player, &enemy);
            } else {
                /* Defeat: restore a small amount of HP and continue */
                printf("\n  你在黑暗中醒来... (You wake up in the darkness...)\n");
                player.hp = player.max_hp / 4;  /* survive with 25% HP */
                printf("  HP 恢复至 %d。(HP restored to %d)\n",
                       player.hp, player.hp);
            }
            press_enter();
            break;
        }
        case 2:
            printf("\n");
            print_divider();
            printf("  角色状态 (Character Status)\n");
            print_divider();
            show_character_status(&player);
            print_divider();
            press_enter();
            break;

        case 3:
            printf("\n  感谢游玩！再见！(Thanks for playing! Goodbye!)\n");
            running = 0;
            break;

        default:
            printf("  无效输入，请重试。(Invalid input, please try again.)\n");
            break;
        }
    }

    return 0;
}
