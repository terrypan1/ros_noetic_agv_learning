# 01 ROS Noetic 取得、安裝、環境、Master 與基本工具

## 目前狀態

已實作基本 ROS Noetic 環境、Turtlesim 與 Topic 觀察。

本章補上之前跳過的一層：**ROS Noetic 本身從哪裡來、怎麼進到 Ubuntu、為什麼會有 `/opt/ros/noetic`，以及之後才怎麼建立自己的 Catkin Workspace。**

這些安裝指令不要求背誦；真正需要重新建環境時，再依當下官方來源檢查並執行。這裡重點是理解來源與順序。

---

## 1. 這一章真正要理解什麼

完整順序不是直接從 `source /opt/ros/noetic/setup.bash` 開始，而是：

```text
Ubuntu 20.04
↓
加入 ROS 官方 APT 軟體來源
↓
APT 能找到 ros-noetic-* 套件
↓
安裝 ROS Noetic
↓
/opt/ros/noetic
↓
source /opt/ros/noetic/setup.bash
↓
ROS 指令與環境可使用
↓
建立自己的 Catkin Workspace
↓
建立自己的 Package / Node
```

真正需要記住的是這條關係，不是每一條安裝命令。

---

## 2. ROS Noetic 從哪裡來

目前學習環境使用：

```text
Ubuntu 20.04 (focal)
ROS 1 Noetic
```

ROS Noetic 一般不是先用 `git clone` 把整套 ROS 拉下來。

目前這台機器使用 ROS 官方 APT Repository：

```text
http://packages.ros.org/ros/ubuntu
```

現機已確認設定檔：

```text
/etc/apt/sources.list.d/ros-latest.list
```

內容指向：

```text
deb http://packages.ros.org/ros/ubuntu focal main
```

因此 Ubuntu 的 APT 才知道去哪裡取得：

```text
ros-noetic-...
```

這些 ROS binary packages。

---

## 3. 目前這台機器實際安裝的是什麼

已確認目前安裝：

```text
ros-noetic-ros-core
ros-noetic-ros-base
```

其中：

```text
ros-noetic-ros-core
→ ROS 最核心的通訊、Package、Message、Launch 等基礎能力

ros-noetic-ros-base
→ 在 ros-core 之上再加入一批常用的非 GUI ROS 基礎套件
```

它們屬於 metapackage，可以理解成「一組 ROS 套件的安裝入口」，不是所有 ROS 功能都塞在單一檔案裡。

目前不用繼續追完整 dependency tree；那是套件管理細節，不是現在學 ROS 專案組裝的主線。

---

## 4. 如果未來要從乾淨 Ubuntu 20.04 重建

概念流程：

```text
設定 ROS 官方 APT Repository
↓
更新 APT 套件索引
↓
安裝需要的 ROS Noetic metapackage
↓
確認 /opt/ros/noetic 存在
↓
載入 setup.bash
↓
確認 ROS_DISTRO=noetic
```

實際命令在重建當下依 ROS 官方文件與當時 repository signing 設定確認，不要求人工背誦舊版安裝指令。

目前學習環境已經完成安裝，所以不需要為了練習重新安裝一次 ROS Noetic。

---

## 5. `/opt/ros/noetic` 是什麼

ROS Noetic 透過系統套件安裝後，主要環境位於：

```text
/opt/ros/noetic
```

其中常用入口：

```text
/opt/ros/noetic/setup.bash
```

所以：

```bash
source /opt/ros/noetic/setup.bash
```

不是下載 ROS，也不是安裝 ROS。

它的作用是：

```text
把已經安裝好的 ROS Noetic 環境
載入目前這個 shell
```

載入後才容易直接使用：

```text
roscore
rosrun
roslaunch
rosnode
rostopic
catkin_make
```

---

## 6. ROS Noetic、Git Clone、Catkin Package 是三件不同的事

這一點之後必須一直分清楚。

### ROS Noetic 本體

```text
來源：ROS 官方 APT Repository
取得方式：apt 安裝 ros-noetic-* binary packages
主要位置：/opt/ros/noetic
```

### ROS 官方或第三方原始碼

```text
來源：Git Repository
取得方式：git clone
位置：自己決定，通常放進某個 Workspace 的 src
之後通常還要自己編譯
```

### 自己建立的 ROS Package

```text
來源：自己建立
工具：catkin_create_pkg
位置：自己的 Catkin Workspace/src
```

所以：

```text
apt install
≠ git clone
≠ catkin_create_pkg
```

---

## 7. ROS Master 與基本通訊概念

ROS 環境載入後，Node 可以透過 ROS 的機制互相找到並建立通訊。

目前先理解：

```text
Node
→ 向 Master 登記彼此位置
→ Publisher 與 Subscriber 建立連線
→ Topic 訊息在 Node 之間傳送
```

Master 主要負責發現與協調，不是所有 Topic 資料都必須經過 Master 轉送。

這個細節後續在 Publisher / Subscriber 章節再實際驗證。

---

## 8. 已完成的基本實作

已完成：

- 載入 ROS Noetic 環境。
- 啟動 `roscore`。
- 啟動 Turtlesim。
- 使用 `/turtle1/cmd_vel` 控制小烏龜。
- 使用 `rosnode list`、`rostopic list`、`rostopic info`、`rostopic echo` 觀察系統。
- 確認目前 ROS Noetic 來自 ROS 官方 APT Repository。
- 確認目前安裝 `ros-noetic-ros-core` 與 `ros-noetic-ros-base`。
- 確認 ROS Noetic 環境入口為 `/opt/ros/noetic/setup.bash`。

---

## 9. 目前不需要深入的東西

現在先不要花時間研究：

- `ros-noetic-ros-core` 完整 dependency tree。
- 每個 Debian package 內部如何拆分。
- APT / dpkg 的底層機制。
- ROS Noetic 整套原始碼如何自行從 source build。

目前只需要能回答：

```text
ROS Noetic 從哪裡取得？
→ ROS 官方 APT Repository

安裝後主要在哪裡？
→ /opt/ros/noetic

source 在做什麼？
→ 載入已安裝好的 ROS 環境

下一層是什麼？
→ 建立自己的 Catkin Workspace
```

---

## 10. 目前新的學習 Workspace

為了不依賴舊練習環境已經建立好的 `catkin_ws`，目前另外從零建立：

```text
/root/ros_learning_ws
```

目前已完成：

```text
/root/ros_learning_ws
├── build/
├── devel/
└── src/
```

這個 Workspace 後續會作為「自己從零重建 ROS 專案」的主要實驗環境。

原本的：

```text
/root/catkin_ws
```

保留作為舊練習環境，不直接當成新的核心學習起點。

---

## 11. 本章完成判定

不要求背安裝指令。

能用自己的話說明以下流程即可：

```text
Ubuntu 20.04
→ ROS 官方 APT Repository
→ 安裝 ROS Noetic
→ /opt/ros/noetic
→ source setup.bash
→ 建立自己的 Catkin Workspace
```

並能分辨：

```text
ROS 本體怎麼來
Git 專案怎麼來
自己的 Package 怎麼建立
```

到這裡就足夠進下一章。

---

## 12. 下一步

銜接 [02 Workspace、Package 與套件來源](02_Workspace_Package與套件來源.md)。

下一階段直接使用新的：

```text
/root/ros_learning_ws
```

從空的 `src/` 開始建立第一個自己的 ROS Package，不再沿用別人已經準備好的 Workspace 結構。
