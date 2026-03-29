/*
 * Final Fantasy Style RPG Game (C Language)
 * 类最终幻想风格的RPG游戏
 *
 * Features:
 *   - Character system with HP, MP, Attack, Defense, Level
 *   - Multi-class system: Warrior, Mage, Archer (多职业系统)
 *   - Turn-based battle system with Attack, Magic, and Class Skill actions
 *   - Experience and level-up system with class-based stat growth
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
   Job class definitions  (多职业系统)
   ───────────────────────────────────────── */

typedef enum {
    JOB_WARRIOR = 0,   /* 战士 */
    JOB_MAGE    = 1,   /* 魔法师 */
    JOB_ARCHER  = 2,   /* 弓手 */
    JOB_COUNT   = 3
} JobClass;

/*
 * Per-job configuration:
 *   base_*       – initial stat at Lv.1
 *   hp/mp/atk/def_lo, hi – random growth range per level-up
 *   skill_name   – unique class skill name (Chinese + English)
 *   skill_desc   – short description shown in battle menu
 *   skill_mp     – MP cost of the class skill (0 = free)
 */
typedef struct {
    const char *name;          /* job display name */
    const char *desc;          /* one-line flavour text */
    int  base_hp;
    int  base_mp;
    int  base_atk;
    int  base_def;
    int  hp_lo,  hp_hi;       /* HP growth per level */
    int  mp_lo,  mp_hi;       /* MP growth per level */
    int  atk_lo, atk_hi;      /* ATK growth per level */
    int  def_lo, def_hi;      /* DEF growth per level */
    const char *skill_name;
    const char *skill_desc;
    int  skill_mp;
} JobTemplate;

static const JobTemplate JOB_TABLE[JOB_COUNT] = {
    /* ── 战士 Warrior ── */
    {
        "战士 (Warrior)",
        "力量超群的近战斗士，拥有厚重护甲。",
        /* HP   MP   ATK  DEF */
        150,  20,  18,  15,
        /* HP±    MP±    ATK±   DEF± */
        15, 25,  2,  5,  3,  6,  2,  4,
        "猛击 (Power Strike)",
        "全力一击，造成2倍物理伤害",
        0       /* 猛击 costs 0 MP */
    },
    /* ── 魔法师 Mage ── */
    {
        "魔法师 (Mage)",
        "精通元素魔法，攻击力强但体质虚弱。",
        /* HP   MP   ATK  DEF */
        70,  80,  20,   6,
        /* HP±    MP±     ATK±   DEF± */
        8, 12,  8, 15,  4,  7,  1,  2,
        "火球术 (Fireball)",
        "召唤火球，无视防御造成2.5倍魔法伤害",
        20      /* Fireball costs 20 MP */
    },
    /* ── 弓手 Archer ── */
    {
        "弓手 (Archer)",
        "身手敏捷的远程射手，技能穿透防御。",
        /* HP   MP   ATK  DEF */
        100,  40,  17,   9,
        /* HP±    MP±    ATK±   DEF± */
        10, 18,  4,  8,  3,  6,  1,  3,
        "穿透射击 (Piercing Shot)",
        "穿透射击，完全无视敌人防御",
        15      /* Piercing Shot costs 15 MP */
    }
};

/* ─────────────────────────────────────────
   Data structures
   ───────────────────────────────────────── */

typedef struct {
    char      name[MAX_NAME_LEN];
    JobClass  job;             /* character's job class */
    int       hp;
    int       max_hp;
    int       mp;
    int       max_mp;
    int       attack;
    int       defense;
    int       level;
    int       exp;
    int       exp_to_next;   /* EXP needed to reach next level */
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

static void init_character(Character *c, const char *name, JobClass job)
{
    const JobTemplate *jt = &JOB_TABLE[job];

    strncpy(c->name, name, MAX_NAME_LEN - 1);
    c->name[MAX_NAME_LEN - 1] = '\0';
    c->job         = job;
    c->level       = 1;
    c->exp         = 0;
    c->exp_to_next = EXP_PER_LEVEL;
    c->max_hp      = jt->base_hp;
    c->hp          = c->max_hp;
    c->max_mp      = jt->base_mp;
    c->mp          = c->max_mp;
    c->attack      = jt->base_atk;
    c->defense     = jt->base_def;
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
    printf("  [%s]  职业: %s  Lv.%d\n",
           c->name, JOB_TABLE[c->job].name, c->level);
    printf("  HP: %d/%d   MP: %d/%d\n", c->hp, c->max_hp, c->mp, c->max_mp);
    printf("  ATK: %d   DEF: %d   EXP: %d/%d\n",
           c->attack, c->defense, c->exp, c->exp_to_next);
    printf("  专属技能: %s (消耗 %d MP)\n",
           JOB_TABLE[c->job].skill_name, JOB_TABLE[c->job].skill_mp);
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

/*
 * Class skill damage:
 *   Warrior – Power Strike:   2× physical, reduced defense mitigation (def/4)
 *   Mage    – Fireball:       2.5× magic damage ignoring defense
 *   Archer  – Piercing Shot:  1.5× physical but fully ignores defense
 */
static int calc_skill_damage(const Character *c, const Enemy *e)
{
    int base, variance, dmg;
    switch (c->job) {
    case JOB_WARRIOR:
        base     = c->attack * 2 - e->defense / 4;
        if (base < 1) base = 1;
        variance = rand_range(-base / 5, base / 5);
        dmg      = base + variance;
        return (dmg < 1) ? 1 : dmg;

    case JOB_MAGE:
        base     = (int)(c->attack * 2.5 + 0.5);   /* 2.5× rounded */
        variance = rand_range(-base / 4, base / 4);
        dmg      = base + variance;
        return (dmg < 5) ? 5 : dmg;

    case JOB_ARCHER:
        base     = (int)(c->attack * 1.5 + 0.5);   /* 1.5× rounded, no defense */
        variance = rand_range(-base / 5, base / 5);
        dmg      = base + variance;
        return (dmg < 1) ? 1 : dmg;

    default:
        return calc_physical_damage(c->attack, e->defense);
    }
}

/* ─────────────────────────────────────────
   Level-up logic
   ───────────────────────────────────────── */

static void try_level_up(Character *c)
{
    const JobTemplate *jt = &JOB_TABLE[c->job];

    while (c->exp >= c->exp_to_next && c->level < MAX_LEVEL) {
        c->exp        -= c->exp_to_next;
        c->level      += 1;
        c->exp_to_next = EXP_PER_LEVEL + (c->level - 1) * 20;

        /* Stat growth on level-up – uses job-specific ranges */
        int hp_gain  = rand_range(jt->hp_lo,  jt->hp_hi);
        int mp_gain  = rand_range(jt->mp_lo,  jt->mp_hi);
        int atk_gain = rand_range(jt->atk_lo, jt->atk_hi);
        int def_gain = rand_range(jt->def_lo, jt->def_hi);

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
    const JobTemplate *jt = &JOB_TABLE[c->job];
    int choice = 0;
    printf("\n  行动指令 (Action):\n");
    printf("  [1] 攻击 (Attack)\n");
    printf("  [2] 魔法 (Magic)  -- 消耗 %d MP\n", MAGIC_MP_COST);
    printf("  [3] %s  -- %s (消耗 %d MP)\n",
           jt->skill_name, jt->skill_desc, jt->skill_mp);
    printf("  选择 (Choose): ");

    if (scanf("%d", &choice) != 1) choice = 1;
    /* Consume trailing newline */
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;

    /* Validate MP for Magic (option 2) */
    if (choice == 2 && c->mp < MAGIC_MP_COST) {
        printf("\n  ❌ MP 不足！(Not enough MP!) 改为普通攻击。\n");
        choice = 1;
    }
    /* Validate MP for class skill (option 3) */
    if (choice == 3 && c->mp < jt->skill_mp) {
        printf("\n  ❌ MP 不足！(Not enough MP!) 改为普通攻击。\n");
        choice = 1;
    }

    if (choice == 2) {
        c->mp -= MAGIC_MP_COST;
        int dmg = calc_magic_damage(c->attack);
        e->hp -= dmg;
        printf("\n  %s 施放魔法！(casts magic!)\n", c->name);
        printf("  造成 %d 点魔法伤害！(Magic damage: %d)\n", dmg, dmg);
    } else if (choice == 3) {
        c->mp -= jt->skill_mp;
        int dmg = calc_skill_damage(c, e);
        e->hp -= dmg;
        printf("\n  %s 使用了 %s！\n", c->name, jt->skill_name);
        printf("  造成 %d 点伤害！(Skill damage: %d)\n", dmg, dmg);
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
   Job selection screen  (职业选择界面)
   ───────────────────────────────────────── */

static JobClass select_job(void)
{
    print_divider();
    printf("  ★ 请选择你的职业 (Choose Your Class) ★\n");
    print_divider();
    for (int i = 0; i < JOB_COUNT; i++) {
        const JobTemplate *jt = &JOB_TABLE[i];
        printf("  [%d] %s\n", i + 1, jt->name);
        printf("      %s\n", jt->desc);
        printf("      HP:%-4d  MP:%-4d  ATK:%-4d  DEF:%-4d\n",
               jt->base_hp, jt->base_mp, jt->base_atk, jt->base_def);
        printf("      专属技能: %s (消耗 %d MP)\n", jt->skill_name, jt->skill_mp);
        if (i < JOB_COUNT - 1) printf("\n");
    }
    print_divider();

    int choice = 0;
    for (;;) {
        int ch;
        printf("  选择职业 (Choose): ");
        if (scanf("%d", &choice) == 1 && choice >= 1 && choice <= JOB_COUNT) {
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            break;
        }
        /* Clear bad input */
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
        printf("  无效选择，请输入 1-%d。(Invalid choice, enter 1-%d.)\n",
               JOB_COUNT, JOB_COUNT);
    }

    JobClass job = (JobClass)(choice - 1);
    printf("\n  你选择了 %s！\n", JOB_TABLE[job].name);
    return job;
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
    JobClass job = select_job();
    init_character(&player, player_name, job);

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
