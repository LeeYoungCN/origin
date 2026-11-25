# Windows 沙箱工作区写权限（完整性标签）

## 1. 现象

在 Windows 上用 DSH 之类的沙箱助手工作时，会出现"文件写不进去"，而且往往分三层：

1. 连列目录都失败，报 `SetNamedSecurityInfoW failed (Win32 5): grantWrite(...)`；
2. 修好之后，工作区根目录能写，`src/`、`out/` 等子目录仍被拒绝；
3. 根目录下新建的子目录却能正常写。

这三种表现指向同一件事：**Windows 的强制完整性控制（MIC）**，与 ACL 无关。

## 2. 原理

### 2.1 沙箱进程跑在低完整性级别

沙箱进程的令牌是 **Low 完整性**（`whoami /all` 里的 `Mandatory Label\Low Mandatory Level`，SID `S-1-16-4096`）。完整性策略默认是"禁止向上写"（no-write-up）：低完整性进程**不能写中等完整性对象**，即使 DACL 里给足了权限。

所以"容器/文件权限看着是 `Modify`，写进去还是拒绝访问"是正常现象：ACL 只是第一道门。

### 2.2 授权动作包含三件事

沙箱给一个目录授权时实际做了三件事，缺一不可：

| 动作 | 作用 |
|---|---|
| 加 capability SID（如 `S-1-4-...`）的可写 ACE | 让沙箱令牌在 DACL 上过第一道检查 |
| 加一条 Everyone 的"禁止删除子项"ACE | 限制沙箱进程删整棵子树 |
| **把目录标成 Low** | 让低完整性检查放行 |

只有第三件事决定"能不能写"。第一、二件是配套。

### 2.3 为什么子目录不继承

完整性标签和继承型 ACL 一样，只在对象**创建时**从父对象继承。要让已存在的子对象也更新，写父对象 ACL 时必须带上 auto-inherit 标志 —— `icacls` 和资源管理器会带，直接调 `SetSecurityInfo` 不一定带。

沙箱通常只给**工作区根目录**授权，于是：

- 根目录是 Low → 可写；
- 已有的 `src/`、`out/` 仍是 Medium → 不可写；
- 之后新建的目录继承 Low → 又可写了。

这就是"根目录能写、子目录不能写、新建目录又能写"的全部原因，和仓库内容无关，任何已有内容的仓库在 Windows 上第一次使用都会踩到。

### 2.4 为什么第一次连列目录都失败

授权要调 `SetNamedSecurityInfoW`，它需要 `WRITE_DAC` + **`WRITE_OWNER`**。常见目录的权限是从上级继承的 `Authenticated Users: 修改`，不含这两项；对象所有者虽然有隐含的 `WRITE_DAC`，但没有 `WRITE_OWNER`（UAC 过滤后的令牌更不会有）。授权调用直接 `ERROR_ACCESS_DENIED`，沙箱拿不到工作区控制权，干脆不启动受限进程。

## 3. 脚本用法

`script/sandbox_label.sh` 负责改完整性标签。它只动标签和权限项，不改文件内容、不改所有者，以目录所有者身份运行即可，**不需要管理员**。

```bash
./script/sandbox_label.sh status              # 只读：打印当前标签
./script/sandbox_label.sh dry-run             # 只统计对象数，不做改动
./script/sandbox_label.sh apply               # 打 Low，沙箱内可写
./script/sandbox_label.sh restore             # 回 Medium，沙箱内恢复只读
./script/sandbox_label.sh apply -p /e/other   # 处理其他目录
```

默认目标是仓库根目录；`status` 会附带打印两层内几个子目录的标签，便于判断哪一层没继承到。

`status` 输出示例（`NW` 是"禁止向上写"策略标记，不受系统语言影响）：

```
目标 E:\code\origin，共 12270 个对象，标签行如下：
    Mandatory Label\Low Mandatory Level:(OI)(CI)(NW)
```

### 3.1 前置条件

- 必须在**沙箱之外**运行（普通终端）。低完整性进程改不了安全标签；msys2 的 bash 在沙箱里连启动都会失败（`couldn't create signal pipe, Win32 error 5`）。
- 需要 msys2 / Git-Bash 环境（脚本用了 `cygpath`、`icacls.exe`）。
- 脚本内部已设置 `MSYS2_ARG_CONV_EXCL='*'`，否则 `/grant` 会被 MSYS 当成路径改写，报 `Invalid parameter`。

### 3.2 执行步骤

1. 给目录树补 `(WO)`（取得所有权）权限项 —— 这是设置标签的前提；
2. `icacls <path> /setintegritylevel "(OI)(CI)<L|M>" /T /C /Q` 递归设置标签；
3. 重新读取并打印标签行，便于确认。

12270 个对象约 9 秒。重复执行是幂等的，`icacls` 会合并同类 ACE，不会产生重复权限项。

## 4. 排查顺序

遇到"权限看着没问题但写不进去"，建议按这个顺序：

1. `whoami /all` —— 先看完整性级别，Low 就优先怀疑标签，而不是 ACL；
2. `icacls <目录>` —— 看最后一行 `Mandatory Label\...` 是什么级别；
3. `Get-Acl <目录>` —— 确认 capability SID 的 ACE 在不在；
4. 对比"根目录 / 已有子目录 / 新建子目录"三者的结果，就能定位到继承问题。

反过来，如果一上来就按 ACL 思路修，会出现"权限项补齐了、写入依旧失败"的假象。

## 5. 回滚与安全提示

```bash
./script/sandbox_label.sh restore
```

- **Low 标签意味着任何低完整性进程都能改这些文件**（浏览器渲染进程、其他沙箱应用）。这是沙箱助手对工作区根目录的既有策略，铺到整棵树是一个取舍。
- 不接受该取舍时：`restore` 后把会话切到"完全权限"（full access）模式，而不是继续靠标签。
- 脚本补的 `(WO)` 权限项会保留（`icacls` 无法只删除某一项权限位），对所有者无实际影响。
