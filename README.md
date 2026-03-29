# 类最终幻想 RPG 游戏 (Final Fantasy Style RPG in C)

一个用 C 语言编写的基础类最终幻想风格 RPG 游戏。  
A basic Final Fantasy-style RPG game written in C.

## 功能 (Features)

- **多职业系统 (Multi-Class System)** – 战士、魔法师、弓手，各有独特属性与专属技能
- **角色系统 (Character System)** – HP、MP、攻击力、防御力、等级属性
- **回合制战斗 (Turn-based Battle)** – 普通攻击 + 魔法 + 职业专属技能
- **职业成长 (Class-based Growth)** – 升级属性成长依职业不同而有差异
- **经验升级系统 (EXP & Level-up)** – 击败敌人获得经验，升级提升属性
- **随机敌人 (Random Enemies)** – 5 种不同敌人，难度随等级提升
- **简洁控制台 UI (Console UI)** – 中英双语提示

## 职业介绍 (Classes)

| 职业 | HP | MP | ATK | DEF | 专属技能 | 说明 |
|------|----|----|-----|-----|---------|------|
| 战士 (Warrior)  | 150 | 20 | 18 | 15 | 猛击 (Power Strike, 0 MP)     | 2× 物理伤害，减少防御减免 |
| 魔法师 (Mage)   |  70 | 80 | 20 |  6 | 火球术 (Fireball, 20 MP)      | 2.5× 魔法伤害，无视防御 |
| 弓手 (Archer)   | 100 | 40 | 17 |  9 | 穿透射击 (Piercing Shot, 15 MP) | 1.5× 伤害，完全无视防御 |

## 构建方法 (Build)

```bash
make
```

> 需要 GCC 和 Make。(Requires GCC and Make.)

## 运行 (Run)

```bash
./rpg_game
```

## 游戏操作 (How to Play)

1. 输入你的角色名字
2. 选择职业（战士 / 魔法师 / 弓手）
3. 主菜单选择：
   - **[1] 开始战斗** – 遭遇随机敌人进行回合制战斗
   - **[2] 查看状态** – 查看角色当前属性与技能
   - **[3] 退出游戏**
4. 战斗中选择：
   - **[1] 攻击** – 普通物理攻击
   - **[2] 魔法** – 消耗 10 MP 造成更高伤害
   - **[3] 职业技能** – 消耗对应 MP 使用职业专属强力技能

## 扩展方向 (Ideas for Extension)

- 道具和装备系统
- 多个地图/场景
- 存档读档功能
- BOSS 战
