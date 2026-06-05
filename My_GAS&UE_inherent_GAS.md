# 我的自定义 GAS 系统 vs UE 原生 Gameplay Ability System

## 系统的对应关系

| 我构建的系统 | UE 原生 GAS 等价物 | 我的版本做了什么 |
|---|---|---|
| `USAction` (UObject) | `UGameplayAbility` | 可激活的技能，带 GameplayTags 管理、状态阻断、网络复制 |
| `USActionComponent` | `UAbilitySystemComponent` | 管理技能生命周期、复制 ActiveGameplayTags、RPC 调用 |
| `USActionEffect` | `UGameplayEffect` | 基于时长（Duration）和周期（Period）的 Buff/Debuff 系统 |
| `USAttributeComponent` | `UAttributeSet` | 生命值、怒气值，服务端权威钳制，多播 Delegate 通知 UI |
| `SGameplayFunctionLibrary::ApplyDamage` | GE Execution | 统一伤害入口：施加生命变化量 + 物理冲量 |
| `FGameplayTagContainer`（已在用） | GameplayTags | 状态标记（眩晕/格挡）、技能阻断、技能赋予、动画状态查询 |

---

## 我的系统更优的地方

### 1. 可理解性

整个系统大约 **4 个核心类，500+ 行 C++**。新组员花 10 分钟就能通读全部源码，理解整个技能体系的运作方式。

GAS 是**几十个类、数千行代码**，拥有深层次的继承链：
- `UGameplayAbility` 继承自 `UGameplayAbilityBase` -> `UObject`
- `UAbilitySystemComponent` 继承自 `UGameplayTasksComponent` -> `UActorComponent`
- `UGameplayEffect` 本身就是一个复杂的配置系统
- 加上 Modifier、Execution、Cue、Task 等子系统的交叉耦合

**结论：** 小团队、快速迭代的项目，简洁压倒一切。

### 2. 可调试性

当 `StartAction` 没触发时，我的调用链是：

```
StartActionByName → CanStart（Tag 检查）→ StartAction（设置状态、广播事件）
```

3 层调用，断点一打，问题立刻定位。

GAS 的调用链：

```
输入 → ASC → TryActivateAbility → 网络策略检查 → 拥有者/Avatar 检查
→ GameplayTag 阻挡检查 → 消耗 GE 检查 → 冷却 GE 检查
→ 网络授权判断 → 激活策略（按标签替换/取消其他技能）
→ PreActivate → ActivateAbility → CommitAbility（消耗+冷却）
→ 实际的技能逻辑执行
```

排查一个"技能为什么没放出来"的问题，需要逐层确认十几处可能被拦截的地方。

### 3. 蓝图友好

```cpp
// 我的 USAction
UCLASS(Blueprintable)
class USAction : public UObject
{
    UFUNCTION(BlueprintNativeEvent)
    void StartAction(AActor* Instigator);
    
    UFUNCTION(BlueprintNativeEvent)
    void StopAction(AActor* Instigator);
};
```

策划和设计师可以直接在编辑器中建蓝图子类，重写 `StartAction` / `StopAction`，在蓝图中写技能逻辑。不需要任何额外包装。

GAS 的方式：
- C++ 中写 `UGameplayAbility` 子类
- 用 `UAbilityTask` 在 C++ 中编排逻辑
- 或者用 `GameplayAbilityBlueprint`（有自身限制）
- 或者包装一层 `BlueprintCallable` 函数

**结论：** 我的方案在蓝图迭代速度上远超 GAS。

### 4. 没有不必要的复杂度

我不需要的 GAS 功能：

| GAS 功能 | 我的替代方案 | 为什么不需要 |
|---|---|---|
| `GameplayEffect` 数值修饰器系统（加法/乘法/覆写/系数/前后乘） | 直接改 `USAttributeComponent` 的值 | 只有 2 个属性，不需要复杂数值运算 |
| `PreAttributeChange` / `PostGameplayEffectExecute` 回调链 | `OnHealthChanged` / `OnRageChanged` Delegate | 直接、可读、够用 |
| `GameplayCue` 独立通知系统 + `GameplayCueManager` 池化 | 在投掷物/技能代码里直接 `SpawnEmitter` / `PlaySound` | 每种技能就 1-2 个特效，不需要独立的 Cue 系统 |
| `AbilityTask` 异步节点系统 | 直接用 FTimer + Delegate | 简单、无 GC 隐患 |
| 客户端预测 | 服务端权威 + 客户端纯表现先行播放 | Roguelike 的 PvE 场景对延迟容忍度高 |

### 5. 怒气作为资源的处理方式

我把怒气直接放在 `USAttributeComponent` 里，和生命值并列：

```cpp
UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly)
float Rage;    // 和 Health 平级
float RageMax;
```

受击时自动涨怒：
```cpp
void ASCharacter::OnHealthChanged(AActor* Instigator, USAttributeComponent* Comp, 
    float NewHealth, float Delta)
{
    if (Delta < 0)
    {
        AttributeComp->ApplyRage(Instigator, FMath::Abs(Delta));
    }
}
```

GAS 里实现同样的效果需要：
1. 创建独立的 `URageAttributeSet`（或把 Rage 和 Health 放同一个但用不同的 GE 驱动）
2. 写一个监听 `PostGameplayEffectExecute` 的 C++ 逻辑
3. 或者在 GameplayEffect 里用 `AttributeBasedMagnitude` 把承受伤害映射为怒气获取

**结论：** 简单资源系统的维护成本远低于 GAS。

---

## GAS 会更好的地方

### 1. 客户端预测

如果需要"按键立刻看到效果"的体验，GAS 内置了完整的预测框架：
- 客户端本地立即执行
- 服务端验证后校正
- 回滚机制

我目前是服务端权威模型：客户端调用 `ServerStartActionByName` RPC，等服务器确认后才真正执行。表现层可以先行播放（你已经在做了），但逻辑层是纯服务端驱动。

**想法：** Roguelike PvE 场景这个不是瓶颈，不需要。

### 2. 冷却系统

GAS 的冷却系统很成熟：
- 一个 `UGameplayEffect` 赋予 `Cooldown` 标签
- 冷却时长基于属性值可动态调整
- UI 直接查标签剩余时长

我目前 **没有冷却系统**。`USAction_ProjectileAttack` 没有 CD 限制。

**想法：** 值得加，但不需要 GAS。一个 `CooldownDuration` + `GetWorld()->GetTimeSeconds()` 足够。

### 3. 消耗/资源系统

GAS 的消耗检查是内置的：
- 激活前校验 Attribute 是否足够
- 自动扣除
- 支持多资源（法力、耐力、怒气）

我目前的 `RageCost` 只在蓝图里声明了字段，C++ 层没有强校验。

**想法：** 值得在 `CanStart()` 里补一刀怒气检查。

### 4. 等级与数值缩放

GAS 的 GameplayEffect 支持：
- 曲线表（`UCurveTable`）按等级计算数值
- `SetByCaller` 动态传入数值
- `AttributeBasedMagnitude` 属性驱动数值

我的伤害是硬编码常量 `DamageAmount = 20`。

**想法：** Roguelike 里通常是固定数值 + 道具叠加，不需要复杂的曲线拟合。

### 5. 网络复制优化

GAS 有 `FGameplayEffectSpec` 结构，可以**批量打包复制**多个 Effect 的激活和移除，避免逐帧逐对象复制。

我目前通过重写 `ReplicateSubobjects` 把数组中每个 `USAction` 逐一复制。对 4-6 个技能的角色来说没问题，但结构上不够紧凑。

**想法：** 当前项目规模不需要优化这一步。

### 6. 引擎级集成

GAS 原生支持：
| 子系统 | 用途 |
|---|---|
| `UAbilityTask` | 异步等待动画蒙太奇、根运动、输入、碰撞 |
| `GameplayCue` | 服务端驱动的表现层通知，多端同步 |
| `UAbilityTask_WaitTargetData` | 通用瞄准/目标选择 |
| `UAbilityTask_PlayMontageAndWait` | 技能动画 + 根运动 |

这些我如果要用，得从零写。但目前我实际用到的功能用 Timer + Delegate 已经满足。

---

## 结论：继续用我的系统

我的系统本质上是 **GAS-lite：一个精简版的 Gameplay Ability System**。

### 为什么不必切换

**1. 规模匹配**

这是一个 Roguelike 动作游戏：
- 每个角色 ~4 个主动技能
- 2 个属性（生命、怒气）
- 伤害计算简单（固定值 ± 百分比反射）

GAS 是为 MOBA / 大型 RPG 设计的：
- 20+ 技能 + 被动
- 复合属性系统（力量→攻击力，智力→法强）
- 复杂的数值修饰链

**2. 最重要的 GAS 特性已经在用了**

`GameplayTag` 是 GAS 的灵魂。我已经把它用对了：
- `Status.Stunned` — 状态查询
- `Status.Parrying` — 技能交互判定
- `GrantsTags` / `BlockedTags` — 技能赋予与阻断
- 动画蓝图查 `ActiveGameplayTags` 驱动动画状态机

**3. 迁移成本 vs 收益**

| 迁移要做的事 | 代价 | 收益 |
|---|---|---|
| 重写所有技能类 | 高 | 微 |
| 改网络复制模型 | 高 | 微 |
| 重测全部联机玩法 | 高 | 无 |
| 获得冷却系统 | — | 自己加只要 10 行代码 |
| 获得客户端预测 | — | PvE 不需要 |

**4. 架构是干净的**

```
USAction（做什么）
  └── USActionComponent（管理生命周期、标签）
USAttributeComponent（状态数据）
  └── OnHealthChanged / OnRageChanged Delegate（通知层）
SGameplayFunctionLibrary（伤害统一入口）
FGameplayTagContainer（状态标记与阻断）
```

这个分解已经和 GAS 的核心思路一致，只是去掉了你不需要的部分。

### 一句话总结

> 我用 10% 的复杂度拿到了 GAS 80% 的核心价值。除非出现明确需求（客户端预测、大规模数值体系、数十个属性的复合计算），没有理由迁移到原生 GAS。

---

## 后续改进方向（按优先级）

### 1. 给 `USAction` 加冷却机制

```cpp
// SAction.h
UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
float CooldownDuration = 0.0f;

float LastActivatedTime = -1e9f;  // 非复制，仅在服务端

// CanStart() 中
bool USAction::CanStart(AActor* Instigator)
{
    if (IsRunning()) return false;
    if (ActionComp && ActionComp->ActiveGameplayTags.HasAny(BlockedTags)) return false;
    
    if (CooldownDuration > 0.0f)
    {
        float TimeSinceLast = GetWorld()->GetTimeSeconds() - LastActivatedTime;
        if (TimeSinceLast < CooldownDuration) return false;
    }
    return true;
}
```

### 2. 在 C++ 层校验怒气消耗

`USAction_ProjectileAttack` 的 `RageCost` 是声明了字段的，把它引入 `CanStart()`：

```cpp
// SAction.h
UPROPERTY(EditDefaultsOnly, Category = "Cost")
float RageCost = 0.0f;

// CanStart() 中加上
if (RageCost > 0.0f)
{
    USAttributeComponent* AttrComp = USAttributeComponent::GetAttributes(Instigator);
    if (AttrComp && AttrComp->GetRage() < RageCost) return false;
}
```

### 3. 冷却期间打上标签，供 UI 查询

```cpp
// 技能激活时
ActiveGameplayTags.AddTag(FGameplayTag::RequestGameplayTag("Action.Cooldown.PrimaryAttack"));

// UI Widget 绑定
ActionComp->ActiveGameplayTags.HasTag("Action.Cooldown.PrimaryAttack") → 显示 CD 遮罩
```

### 4. 使用标签类别做统一阻断

与其每个 Action 各自配 `BlockedTags`，不如按类别控制：

```
Action.Type.Attack     → 被 Status.Stunned 统一阻断
Action.Type.Movement   → 被 Status.Stunned 统一阻断（Dash、Sprint）
Action.Type.Buff       → 不会被 Stunned 阻断
```

这样新增一个眩晕来源时，不需要改动每个 Action。
