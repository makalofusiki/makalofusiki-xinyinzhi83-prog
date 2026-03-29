# 类最终幻想 RPG 游戏 (Final Fantasy Style RPG in C)

一个用 C 语言编写的基础类最终幻想风格 RPG 游戏。  
A basic Final Fantasy-style RPG game written in C.

## 功能 (Features)

- **角色系统 (Character System)** – HP、MP、攻击力、防御力、等级属性
- **回合制战斗 (Turn-based Battle)** – 普通攻击 + 魔法技能
- **经验升级系统 (EXP & Level-up)** – 击败敌人获得经验，升级提升属性
- **随机敌人 (Random Enemies)** – 5 种不同敌人，难度随等级提升
- **简洁控制台 UI (Console UI)** – 中英双语提示

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

1. 输入你的角色名字开始游戏
2. 主菜单选择：
   - **[1] 开始战斗** – 遭遇随机敌人进行回合制战斗
   - **[2] 查看状态** – 查看角色当前属性
   - **[3] 退出游戏**
3. 战斗中选择：
   - **[1] 攻击** – 普通物理攻击
   - **[2] 魔法** – 消耗 MP 造成更高伤害

## 扩展方向 (Ideas for Extension)

- 多个职业（战士、魔法师、弓手）
- 道具和装备系统
- 多个地图/场景
- 存档读档功能
- BOSS 战
