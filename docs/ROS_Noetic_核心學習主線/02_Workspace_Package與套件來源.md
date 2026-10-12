# 02 Workspace、Package 與套件來源

## 目前狀態

已完成 ROS Noetic 本體來源與環境理解，並另外建立一個全新的 Catkin Workspace：

```text
/root/ros_learning_ws
```

目前這個 Workspace 還沒有建立自己的 ROS Package。

這一章的目的，就是從這個乾淨 Workspace 開始理解：

```text
ROS Noetic 系統環境
↓
Catkin Workspace
↓
src
↓
ROS Package
↓
C++ Node
```

這些建立與同步指令不要求背誦；需要時再查即可。重點是知道每一層在做什麼、檔案從哪裡來。

---

## 1. 先分清楚 ROS 系統與自己的 Workspace

上一章已確認 ROS Noetic 系統主要位於：

```text
/opt/ros/noetic
```

這是透過 ROS 官方 APT Repository 安裝進 Ubuntu 的 ROS 系統環境。

我們現在自己建立的是：

```text
/root/ros_learning_ws
```

兩者不是同一個東西：

```text
/opt/ros/noetic
→ ROS Noetic 系統環境

/root/ros_learning_ws
→ 自己開發 ROS Package 的 Catkin Workspace
```

目前核心學習不再直接沿用舊的：

```text
/root/catkin_ws
```

舊 Workspace 保留作為之前的練習與實驗紀錄，新主線改用 `/root/ros_learning_ws` 從零重建。

---

## 2. 目前新的 Catkin Workspace 怎麼建立

本次實際執行：

```bash
source /opt/ros/noetic/setup.bash
mkdir -p ~/ros_learning_ws/src
cd ~/ros_learning_ws
catkin_make
```

成功後得到：

```text
/root/ros_learning_ws
├── build/
├── devel/
└── src/
```

這幾條命令不用背。

真正需要理解的是：

```text
先載入 ROS Noetic
↓
建立 Workspace 與 src
↓
catkin_make 初始化／編譯 Workspace
↓
產生 build 與 devel
```

---

## 3. Workspace 是什麼

可以先把 Catkin Workspace 理解成：

> 一個用來集中管理、編譯與載入多個 ROS Package 的開發工作區。

目前：

```text
/root/ros_learning_ws
```

就是我們自己的 ROS 開發工作區。

其中最重要的是：

```text
src/
```

因為真正需要閱讀、修改、版本管理的 Package 原始碼主要放在這裡。

---

## 4. `src`、`build`、`devel` 分別做什麼

### `src/`

```text
/root/ros_learning_ws/src
```

放 ROS Package 原始碼。

未來可能看到：

```text
src/
├── 自己建立的 Package
├── ROS 官方原始碼 Package
└── 第三方 ROS Package
```

這裡才是開發時最需要關心的區域。

### `build/`

```text
/root/ros_learning_ws/build
```

Catkin / CMake 編譯過程使用的中間檔案。

目前先理解成：

```text
編譯工作區
```

不需要手動在裡面寫程式，也不應當成原始碼保存區。

### `devel/`

```text
/root/ros_learning_ws/devel
```

編譯成功後產生的開發環境。

其中很重要的是：

```text
/root/ros_learning_ws/devel/setup.bash
```

之後：

```bash
source ~/ros_learning_ws/devel/setup.bash
```

可以讓目前 Terminal 認得這個 Workspace 編譯出的 Package。

---

## 5. `catkin_make` 到底在做什麼

目前不用研究 CMake 內部細節。

先理解：

```text
catkin_make
↓
查看 Workspace/src 裡有哪些 Catkin Package
↓
依 Package 的 CMakeLists.txt / package.xml 處理相依與編譯
↓
產生 build
↓
產生 devel
```

所以：

```text
catkin_make
≠ 建立 ROS Noetic
≠ 下載 Package
≠ 建立某一個 Node
```

它主要負責的是：

> 編譯目前這個 Catkin Workspace。

---

## 6. ROS Package 是什麼

Package 是 ROS 專案最基本的功能組織單位。

可以先想成：

```text
Workspace
└── src
    ├── Package A
    ├── Package B
    └── Package C
```

一台機器人不會只有一個巨大程式。

之後可能依功能拆成：

```text
感測器 Package
控制 Package
定位 Package
導航 Package
安全 Package
Bringup Package
```

---

## 7. Package 最小組成

一個 C++ ROS Package 常會看到：

```text
my_package/
├── CMakeLists.txt
├── package.xml
├── include/
└── src/
```

目前先理解三個最重要的部分。

### `package.xml`

```text
描述 Package 本身
+ 宣告 ROS 相依套件
```

例如這個 Package 需要 `roscpp`、`std_msgs`，就會在這裡留下相依資訊。

### `CMakeLists.txt`

```text
告訴 Catkin / CMake 怎麼編譯這個 Package
```

之後建立 C++ Node 時，會在這裡加入 executable、link library 等設定。

### `src/`

```text
放 C++ 原始碼
```

例如之後建立：

```text
src/my_node.cpp
```

---

## 8. Package 不一定都是自己寫的

這是這一章最重要的觀念之一。

Workspace 裡的 Package 可能有不同來源。

### 來源 A：ROS 系統 binary package

例如：

```text
ros-noetic-roscpp
ros-noetic-std-msgs
```

通常透過：

```text
apt install ros-noetic-...
```

安裝到 ROS 系統環境，主要在：

```text
/opt/ros/noetic
```

這些通常不需要複製到自己的 `src` 才能使用。

### 來源 B：ROS 官方原始碼

例如之前接觸過的：

```text
ros_tutorials
```

它可以從 ROS 官方 Git Repository 取得原始碼，再放進 Workspace 的 `src` 自己編譯。

### 來源 C：第三方 ROS 專案

例如之前練習使用：

```text
wpr_simulation
```

這類通常來自第三方 Git Repository。

精確上游、版本與修改內容需要另外確認，不能因為它能在 ROS 裡執行就直接當成「ROS 官方」。

### 來源 D：自己建立的 Package

例如接下來我們會在：

```text
/root/ros_learning_ws/src
```

建立自己的 ROS Package。

這才是真正開始「自己組 ROS 專案」。

---

## 9. 三種常見取得方式一定要分清楚

```text
apt install
→ 安裝已經編好的 ROS binary package
→ 通常進 /opt/ros/noetic


git clone
→ 取得某個 Repository 的原始碼
→ 可以放進 Workspace/src 再自己編譯


catkin_create_pkg
→ 自己建立一個新的 Catkin Package 骨架
→ 放在自己的 Workspace/src
```

所以：

```text
apt install
≠ git clone
≠ catkin_create_pkg
```

這三種東西最後都可能參與同一個 ROS 系統，但來源完全不同。

---

## 10. 為什麼別人的 Git 專案看起來「下載後就有很多東西」

如果別人直接：

```text
git clone 某個 ROS Repository
```

下載後裡面已經可能包含：

```text
Package
Node 原始碼
launch
config
URDF
Gazebo 設定
第三方程式
```

這不代表這些東西都是 ROS Noetic 自帶的。

正確的判斷方式是逐層問：

```text
這個 Package 是誰寫的？
↓
ROS 官方？
第三方？
設備廠商？
範例／教材作者？
公司自己？

它怎麼取得？
↓
apt？
git clone？
自己建立？
```

這也是目前核心學習主線刻意要補回來的能力。

---

## 11. 同名 Package 為什麼可能有兩份

之前已經遇過這種情況：

```text
/opt/ros/noetic/share/turtlesim
```

同時又可能有：

```text
/root/某個_workspace/src/ros_tutorials/turtlesim
```

也就是：

```text
APT 安裝的 binary 版本
+
自己下載原始碼後編譯的版本
```

可以同時存在。

需要確認 ROS 實際找到哪一份時，可使用：

```bash
rospack find <package_name>
```

這個工具的目的不是要求背路徑，而是：

> 不要猜 ROS 現在用哪一份，直接查。

---

## 12. 新 Workspace 的環境載入

ROS 系統環境：

```bash
source /opt/ros/noetic/setup.bash
```

可以先理解成：

```text
我要使用 ROS Noetic
```

自己的 Workspace 編譯成功後：

```bash
source ~/ros_learning_ws/devel/setup.bash
```

可以理解成：

```text
我要再讓 ROS 認得 ros_learning_ws 裡的 Package
```

概念上是：

```text
ROS Noetic 系統
/opt/ros/noetic
        ↓
自己的 Workspace
/root/ros_learning_ws
```

目前不需要深入研究所有環境變數。

---

## 13. Windows、WSL 與 Git 怎麼對應

這次學習不把 WSL 的 `build/`、`devel/` 當成正式 Git 原始資料。

目前原則：

```text
Windows Git 專案

↓ 保存

Markdown 學習筆記
+ 自己真正要保留的 ROS Package 原始碼
```

WSL 則負責真正編譯與執行：

```text
/root/ros_learning_ws/src
```

也就是：

```text
Windows
→ Git / 長期保存 / 查看修改

WSL
→ Linux ROS 執行環境 / catkin_make / rosrun
```

之後自己建立的 Package 原始碼，在 Windows 正式保存一份，再同步到 WSL 的 `ros_learning_ws/src` 測試。

不需要把：

```text
build/
devel/
```

同步回 Windows Git。

---

## 14. 目前舊 Workspace 與新 Workspace 的定位

### 舊環境

```text
/root/catkin_ws
```

用途：

```text
保留之前練習操作
wpr_simulation
ros_tutorials
ssr_pkg
其他既有實驗
```

目前不刪除、不重構。

### 新核心學習環境

```text
/root/ros_learning_ws
```

用途：

```text
從零開始
↓
自己建 Package
↓
自己建 C++ Node
↓
自己編譯
↓
自己驗證
```

這一條才是目前核心學習主線。

---

## 15. 來源分類規則

之後每遇到一個 ROS Package，至少要能回答：

```text
Package 名稱是什麼？
來源是什麼？
怎麼取得？
版本／branch 是什麼？
它提供什麼功能？
有沒有被別人另外修改？
```

可以標示成：

```text
[ROS 官方 binary package]
[ROS 官方原始碼]
[第三方開源]
[設備廠商]
[自己建立]
[來源待確認]
```

來源不知道時不要猜，直接標示「待確認」。

---

## 16. 已踩過的坑

過去實作已遇過：

- 新 shell 忘記 `source`。
- 缺 `image_transport`、`xacro`、`joint_state_publisher` 等 dependency。
- Gazebo plugin 出現 `.so` undefined symbol／link 問題。
- 同名 Package 有多份時，需要用 `rospack find` 確認實際來源。
- 已存在的 Workspace 很容易讓人誤以為某些 Package、設定或 dependency 是 ROS 自動產生的。

本章只保留架構理解，不在這裡展開除錯細節。

---

## 17. 本章完成判定

這章不要求背 `catkin_make`、Git 或同步命令。

能說清楚下面這張圖就夠：

```text
ROS Noetic
/opt/ros/noetic
        ↓

自己的 Catkin Workspace
/root/ros_learning_ws
        ↓
src/
        ↓
ROS Package
        ↓
自己的 C++ Node
```

並且知道 Package 可能來自：

```text
APT binary
Git 官方原始碼
Git 第三方原始碼
自己建立
公司專案
```

到這裡就可以正式開始建立第一個自己的 Package。

---

## 18. 下一步

銜接：

[03 C++ Node 從零建立](03_C++Node從零建立.md)

但在進入 C++ Node 前，先在新的：

```text
/root/ros_learning_ws/src
```

建立第一個自己的 ROS Package。

下一階段只處理：

```text
catkin_create_pkg
↓
看懂新 Package 產生哪些檔案
↓
catkin_make
↓
source devel/setup.bash
↓
確認 ROS 找得到自己的 Package
```

完成後才進入第一個 C++ Node。
