# 团队协作指南

> **重要：每次提交更新后，必须在 `DEVLOG.md` 中记录本次改动内容！**

---

## 一、环境准备

### 1.1 安装 Git

- Windows: 下载 [Git for Windows](https://git-scm.com/download/win)
- 安装时选择默认选项即可
- 安装完成后打开 Git Bash 或 CMD，配置身份：

```bash
git config --global user.name "你的姓名"
git config --global user.email "你的邮箱"
```

### 1.2 克隆仓库

```bash
git clone https://github.com/WuWingKit/Inverted-pendulum.git
cd Inverted-pendulum
```

---

## 二、日常工作流（Feature Branch 模式）

> 核心原则：**永远不要在 master 分支上直接修改代码！** 所有开发工作都在功能分支上进行。

### 2.1 每天开始工作前 — 同步最新代码

```bash
# 切换到 master 分支
git checkout master

# 拉取远程最新代码
git pull origin master
```

### 2.2 开始新功能 — 创建功能分支

```bash
# 从最新的 master 创建你的功能分支
git checkout -b feature/你的功能描述

# 命名示例：
#   feature/pid-tuning        — PID参数调试
#   feature/mpu6050-driver    — 添加MPU6050驱动
#   feature/balance-control   — 直立平衡控制
#   fix/encoder-overflow      — 修复编码器溢出bug
```

分支命名规范：
| 前缀 | 用途 | 示例 |
|------|------|------|
| `feature/` | 新功能开发 | `feature/imu-driver` |
| `fix/` | Bug 修复 | `fix/motor-direction` |
| `refactor/` | 代码重构 | `refactor/pi-controller` |
| `docs/` | 文档更新 | `docs/pinout-table` |

### 2.3 开发过程 — 频繁提交

```bash
# 查看当前修改状态
git status

# 查看具体改了什么
git diff

# 将修改添加到暂存区
git add 文件名            # 添加指定文件
git add -A               # 添加所有修改

# 提交（写好提交信息）
git commit -m "修改了XXX功能，因为XXX原因"

# 好的提交信息示例：
#   git commit -m "增加MPU6050角度读取函数"
#   git commit -m "修复编码器D电机方向反转问题"
#   git commit -m "调整PI参数: Kp=2.5, Ki=0.8"

# 不好的提交信息（不要这样写）：
#   git commit -m "update"
#   git commit -m "修改"
#   git commit -m "111"
```

**提交粒度建议：** 每完成一个小的、可工作的改动就提交一次，不要攒一大堆一起提交。

### 2.4 推送分支到 GitHub

```bash
# 第一次推送这个分支
git push -u origin feature/你的功能描述

# 后续推送（已经关联过远程分支）
git push
```

### 2.5 发起 Pull Request (PR)

1. 打开 https://github.com/WuWingKit/Inverted-pendulum
2. 点击 **"Pull requests"** 标签 → **"New pull request"**
3. base 选 `master`，compare 选你的功能分支
4. 填写 PR 标题和描述，说明你做了什么改动
5. 点击 **"Create pull request"**
6. 在 PR 页面通知其他成员进行 Code Review
7. Review 通过后由仓库管理员合并

### 2.6 合并后 — 清理本地分支

```bash
# 切回 master
git checkout master

# 拉取合并后的最新代码
git pull origin master

# 删除已合并的本地分支
git branch -d feature/你的功能描述
```

---

## 三、冲突处理

当两个人修改了同一个文件的同一行时，Git 无法自动合并，就会产生冲突。

### 3.1 场景：你的 PR 与 master 有冲突

```bash
# 先更新本地 master
git checkout master
git pull origin master

# 切回你的功能分支
git checkout feature/你的功能描述

# 把 master 的最新代码合并进来
git merge master
```

此时 Git 会提示冲突文件，打开这些文件会看到类似标记：

```c
<<<<<<< HEAD
int TargetVelocity = 500;  // 你的版本
=======
int TargetVelocity = 800;  // master 上的版本
>>>>>>> master
```

**解决步骤：**
1. 和修改了同一段代码的同事沟通，决定保留哪个版本（或合并两者）
2. 手动编辑文件，删除 `<<<<<<<`、`=======`、`>>>>>>>` 这些标记
3. 保留最终想要的代码
4. 保存文件后：

```bash
git add 冲突文件名          # 标记冲突已解决
git commit -m "解决与master的合并冲突"
git push
```

### 3.2 预防冲突

- 开发前先 `git pull` 获取最新代码
- 不要修改同一个函数/同一段逻辑
- 经常推送，不要积压太多本地修改
- 不确定时先和同事沟通谁改哪部分

---

## 四、常见操作速查

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
| 丢弃所有本地修改 | `git checkout .` (谨慎使用!) |

---

## 五、Keil 工程特别说明

### 5.1 编译产物不要提交

`.gitignore` 已经配置忽略以下文件，不要用 `git add -f` 强制添加：
- `*.axf` `*.hex` `*.bin` — 烧录文件
- `*.o` `*.d` `*.crf` — 编译中间文件
- `*.lst` `*.map` — 链接输出
- `JLinkSettings.ini` — 调试器个人配置
- `*.uvguix.*` `*.scvd` — Keil 个人配置

### 5.2 工程文件注意事项

`USER/Tb6612demo.uvprojx` 是 Keil 工程文件，如果改了：
- 包含路径（新增了 `#include` 目录）
- 编译宏定义
- 链接脚本

请在提交信息中说明，方便其他同事同步更新。

---

## 六、开发文档更新规则（必须遵守！）

> ⚠️ **每次提交代码更新后，必须同步更新 `DEVLOG.md`！**

`DEVLOG.md` 是项目的开发日志，记录了所有重要改动。每条记录包含：

```markdown
### YYYY-MM-DD — 改动简述

- **改动者：** 你的名字
- **类型：** 新增功能 / Bug修复 / 参数调整 / 文档更新
- **改动文件：** `HAREWER/XXX/xxx.c`, `USER/main.c`
- **内容：** 详细描述改了什么、为什么这样改
- **验证：** 如何验证改动正确（编译通过 / 实际运行OK / 串口输出正常）
```

**具体流程：**
1. 写完代码，提交 commit
2. 打开 `DEVLOG.md`
3. 在文件顶部（`## 开发日志` 下方）添加你的改动记录
4. 再提交一次 `git commit -m "docs: 更新开发日志"`
5. 推送

这样做的好处：
- 每个同事都能快速了解项目进展
- 出问题时可以回溯谁改了什么
- 方便写周报/总结

---

## 七、问题反馈

遇到 Git 问题或工程编译问题，请在 GitHub Issues 中提出，或在工作群中沟通。
