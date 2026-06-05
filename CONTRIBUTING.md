# 团队协作指南

> **⚠️ master 分支已开启保护，禁止直接推送。所有改动必须通过 Pull Request 合并！**
>
> **⚠️ 每次提交代码更新后，必须在 `DEVLOG.md` 中记录本次改动！**

---

## 零、规则速览

```
                    ❌ 已禁止                             ✅ 唯一路径

              git push origin master           git checkout -b feature/xxx
                      ↓                       git add -A && git commit -m "..."
                 直接改 master                git push -u origin feature/xxx
                                              去 GitHub 网页 → Create Pull Request
                                              Review 通过 → Merge
```

---

## 一、环境准备

### 1.1 安装 Git

- Windows: 下载 [Git for Windows](https://git-scm.com/download/win)
- 安装时选择默认选项即可
- 安装完成后打开终端，配置身份：

```bash
git config --global user.name "你的姓名"
git config --global user.email "你的邮箱（必须和 GitHub 账户的已验证邮箱一致！）"
```

### 1.2 克隆仓库

```bash
git clone https://github.com/WuWingKit/Inverted-pendulum.git
cd Inverted-pendulum
```

---

## 二、完整工作流

### 流程图

```
 git pull              git checkout -b         写代码 &              git push
 同步最新              feature/xxx             频繁 commit           推送到 GitHub
    │                      │                       │                     │
    ▼                      ▼                       ▼                     ▼
 ┌──────┐             ┌──────────┐            ┌──────────┐          ┌──────────┐
 │master│             │ 功能分支  │            │ git add  │          │  GitHub  │
 │ 最新  │──────────▶│ 草稿开发  │──────────▶│ git      │──────────▶│  创建 PR │
 └──────┘             └──────────┘            │ commit   │          └────┬─────┘
                                              └──────────┘               │
                                                                        ▼
                                                                 ┌──────────────┐
                                                                 │ 同事 Review  │
                                                                 │ 通过 → Merge │
                                                                 └──────┬───────┘
                                                                        │
                                                  git checkout master    │
                                                  git pull origin master │
                                                  删除本地分支 ◀─────────┘
```

### 第 1 步：每天开始工作前 — 同步 master

```bash
git checkout master
git pull origin master
```

### 第 2 步：创建功能分支

```bash
git checkout -b feature/你的功能描述

# 命名示例：
#   feature/pid-tuning        — PID 参数调试
#   feature/mpu6050-driver    — 添加 MPU6050 驱动
#   feature/balance-control   — 直立平衡控制
#   fix/encoder-overflow      — 修复编码器溢出 bug
```

**分支命名规范：**

| 前缀 | 用途 | 示例 |
|------|------|------|
| `feature/` | 新功能开发 | `feature/imu-driver` |
| `fix/` | Bug 修复 | `fix/motor-direction` |
| `refactor/` | 代码重构 | `refactor/pi-controller` |
| `docs/` | 文档更新 | `docs/pinout-table` |

### 第 3 步：写代码 & 本地提交

```bash
git status                 # 看改了什么
git add -A                 # 添加所有修改
git commit -m "做了什么"    # 本地提交

# ✅ 好的提交信息：
git commit -m "修复编码器D电机方向反转问题"
git commit -m "调整PI参数: Kp=2.5, Ki=0.8"

# ❌ 不要这样写：
git commit -m "update"
git commit -m "111"
```

**提交粒度：** 每完成一个可工作的小改动就提交，不要攒一堆。

### 第 4 步：推送到 GitHub

```bash
# 第一次推送
git push -u origin feature/你的功能描述

# 后续推送
git push
```

### 第 5 步：创建 Pull Request（核心！）

推送后打开 https://github.com/WuWingKit/Inverted-pendulum，页面顶部会出现：

```
┌─────────────────────────────────────────────────────────┐
│  feature/xxx had recent pushes  [Compare & pull request]│  ← 点这里
└─────────────────────────────────────────────────────────┘
```

然后：
1. **base** 保持 `master`，**compare** 选你的功能分支
2. 写标题和描述（说明改了什么、为什么这样改）
3. 点 **「Create pull request」**
4. 把 PR 链接发到工作群，让同事 Review

### 第 6 步：审查 & 合并

- **如果你是 Reviewer（审查者）：** 打开 PR 页面看代码改动，没问题在评论区发 "LGTM" 或点 **「Approve」**
- **如果你是作者：** 收到 Approve 后，点 **「Merge pull request」** → **「Confirm merge」**
- 合并完后可以点 **「Delete branch」** 删远程分支（不删也不影响）

### 第 7 步：清理本地

```bash
git checkout master
git pull origin master
git branch -d feature/你的功能描述    # 删除本地分支
```

---

## 三、日常速查卡片

```bash
# === 每天开始 ===
git checkout master && git pull origin master

# === 开分支干活 ===
git checkout -b feature/你的功能

# === 提交代码 ===
git add -A
git commit -m "描述你的改动"

# === 推送 ===
git push -u origin feature/你的功能

# === 推送完 → 打开 GitHub 网页 → 点 Compare & pull request ===

# === 合并完成后 ===
git checkout master && git pull origin master
git branch -d feature/你的功能
```

---

## 四、冲突处理

当两个人修改了同一个文件的同一行时，Git 无法自动合并，就会产生冲突。

### 4.1 场景：你的 PR 与 master 有冲突

```bash
# 先更新本地 master
git checkout master
git pull origin master

# 切回功能分支
git checkout feature/你的功能描述

# 把 master 合并进来
git merge master
```

此时 Git 会提示冲突文件，打开冲突文件会看到：

```c
<<<<<<< HEAD
int TargetVelocity = 500;  // 你的版本
=======
int TargetVelocity = 800;  // master 上的版本
>>>>>>> master
```

**解决步骤：**
1. 和同事沟通，决定保留哪个版本（或合并两者）
2. 手动编辑文件，删除 `<<<<<<<`、`=======`、`>>>>>>>` 标记
3. 保留最终想要的代码，保存
4. 执行：

```bash
git add 冲突文件名
git commit -m "解决与 master 的合并冲突"
git push
```

### 4.2 预防冲突

- 开发前先 `git pull` 获取最新代码
- 不要两个人在同一个函数里同时改
- 经常推送，不要积压太多本地修改
- 不确定时先和同事沟通谁改哪部分

---

## 五、常见操作速查

| 操作 | 命令 |
|------|------|
| 查看当前分支 | `git branch` |
| 切换分支 | `git checkout 分支名` |
| 创建并切换分支 | `git checkout -b 分支名` |
| 查看提交历史 | `git log --oneline` |
| 撤销未提交的修改 | `git checkout -- 文件名` |
| 撤销 git add | `git reset HEAD 文件名` |
| 查看远程仓库地址 | `git remote -v` |
| 暂存当前工作 | `git stash` |
| 恢复暂存的工作 | `git stash pop` |
| 查看某次提交详情 | `git show 提交哈希` |

---

## 六、Keil 工程特别说明

### 6.1 编译产物不要提交

`.gitignore` 已配置忽略以下文件，**不要用 `git add -f` 强制添加**：
- `*.axf` `*.hex` `*.bin` — 烧录文件
- `*.o` `*.d` `*.crf` — 编译中间文件
- `*.lst` `*.map` — 链接输出
- `JLinkSettings.ini` — 调试器个人配置
- `*.uvguix.*` `*.scvd` — Keil 个人配置

### 6.2 工程文件注意事项

`USER/Tb6612demo.uvprojx` 是 Keil 工程文件，如果改了包含路径、编译宏、链接脚本，请在提交信息中说明。

---

## 七、开发文档更新规则（必须遵守！）

> ⚠️ **每次提交代码更新后，必须同步更新 `DEVLOG.md`！**

每条记录按照以下格式：

```markdown
### YYYY-MM-DD — 改动简述

- **改动者：** 姓名
- **类型：** 新增功能 / Bug修复 / 参数调整 / 文档更新
- **改动文件：** `HAREWER/XXX/xxx.c`
- **内容：** 详细描述
- **验证：** 如何验证
```

**操作流程：**
1. 写完代码，`git commit`
2. 编辑 `DEVLOG.md`，在 `## 开发日志` 下方添加记录
3. `git add DEVLOG.md && git commit -m "docs: 更新开发日志"`
4. `git push`

---

## 八、问题反馈

遇到 Git 问题或编译问题，在 GitHub Issues 中提出，或在工作群沟通。
