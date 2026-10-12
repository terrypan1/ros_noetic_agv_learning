# 13_SLAM原理_建圖定位與資料流概念

## 13.1 本章目標

這一章先不進入 Cartographer 實作，而是先把 SLAM 的核心概念整理清楚。

本章要理解的是：

- SLAM 是什麼
- 為什麼移動機器人需要 SLAM
- SLAM 如何同時做「定位」與「建圖」
- LiDAR、Odom、IMU、TF 在 SLAM 裡各自負責什麼
- 不同位置的雷射觀測，如何對齊成同一張地圖
- 為什麼 Odom 會累積誤差
- Scan Matching、Loop Closure、Pose Graph 在做什麼
- 為下一章 Cartographer 2D SLAM 做準備

---

## 13.2 SLAM 是什麼

SLAM 全名：

```text
Simultaneous Localization And Mapping
```

中文可以理解成：

```text
同時定位與地圖構建
```

它要同時回答兩個問題：

```text
1. 我現在在哪裡？
2. 周圍環境長什麼樣子？
```

如果機器人只有地圖、卻不知道自己在哪裡，就沒有辦法正確使用地圖。

如果機器人知道自己在哪裡、卻完全沒有環境地圖，也無法進一步做導航與路徑規劃。

因此 SLAM 的核心就是：

```text
機器人一邊移動
↓
一邊觀察環境
↓
一邊估算自己的位置
↓
一邊更新地圖
```

---

## 13.3 SLAM 的基本輸入

2D LiDAR SLAM 常見會使用以下資訊：

```text
LiDAR
Odom
IMU
TF
```

### LiDAR

LiDAR 用來觀察機器人周圍的環境。

在 ROS 裡常見的 Topic：

```text
/scan
```

Message：

```text
sensor_msgs/LaserScan
```

LiDAR 提供的是「從目前位置往不同角度量到多遠有物體」。

例如：

```text
機器人
   ↗ 2.1 m
→  1.5 m
   ↘ 3.0 m
```

這些距離資料組合起來，就會形成一圈環境輪廓。

---

### Odom

Odom 是 Odometry，也就是里程計資訊。

常見 Topic：

```text
/odom
```

Message：

```text
nav_msgs/Odometry
```

Odom 用來估算：

```text
機器人從上一個位置
移動了多少距離
旋轉了多少角度
```

它的優點是：

- 更新快
- 連續
- 短時間內平滑

但缺點也很重要：

```text
誤差會隨時間累積
```

例如輪胎打滑、地面不平、Encoder 誤差，都會讓 Odom 漸漸漂移。

---

### IMU

IMU 全名：

```text
Inertial Measurement Unit
```

中文：

```text
慣性測量單元
```

IMU 可以提供：

- 角速度
- 線性加速度
- 部分裝置也會提供姿態資訊

在 SLAM 中，IMU 可以幫助系統判斷機器人的旋轉與姿態變化。

但不同 SLAM 演算法、不同機器人配置，不一定都要求使用 IMU。

---

### TF

TF 用來描述不同座標系之間的關係。

例如：

```text
map
↓
odom
↓
base_link
↓
laser_link
```

SLAM 必須知道：

```text
LiDAR 裝在車體哪裡？
車體現在相對 odom 在哪裡？
最後整台機器人相對 map 在哪裡？
```

所以 TF 是 SLAM 資料能不能正確放進同一套座標系的關鍵。

---

## 13.4 SLAM 的基本資料流

SLAM 可以先理解成下面這條資料流：

```text
LiDAR /scan
+
Odom /odom
+
IMU（視需求）
+
TF
↓
SLAM
↓
位置估計
+
地圖更新
↓
目前機器人位置
+
完整環境地圖
```

也就是：

```text
感測器輸入
↓
位置估計
↓
環境對齊
↓
地圖更新
```

---

## 13.5 不同位置只會看到局部環境

機器人在位置 A 時，只能看到當下 LiDAR 掃描範圍內的環境。

移動到位置 B 後，又會得到另一組新的雷射掃描。

例如：

```text
位置 A
↓
看到左邊牆角與前方牆面

位置 B
↓
看到前方牆面與右側轉角
```

每一次觀測，都只能得到一部分局部資訊。

所以 SLAM 並不是「掃一次就得到完整地圖」。

而是：

```text
位置 A 的觀測
+
位置 B 的觀測
+
位置 C 的觀測
+
...
↓
逐步拼成完整地圖
```

---

## 13.6 不同觀測如何拼成同一張地圖

核心概念是：

```text
找出不同觀測之間相同或相似的幾何特徵
```

例如位置 A 與位置 B 都掃到同一個牆角：

```text
位置 A 掃到：
┌────
│
│

位置 B 掃到：
    ────┐
        │
        │
```

SLAM 會嘗試找出：

```text
這兩組掃描裡
哪些幾何輪廓其實是同一個地方
```

接著估算兩次觀測之間的：

```text
平移 Translation
+
旋轉 Rotation
```

最後把兩次局部觀測對齊。

---

## 13.7 Scan Matching 是什麼

Scan Matching 可以先理解成：

```text
把這次 LiDAR 掃描
和前一次 / 現有地圖的掃描結果相比
↓
找出最合理的平移與旋轉
```

目標是回答：

```text
「我這一次的掃描，應該放在地圖的哪裡？」
```

它不是在做一般影像辨識，也不是在判斷：

```text
這是桌子
這是椅子
這是一面牆
```

2D LiDAR SLAM 更常關心的是：

```text
幾何輪廓
牆面
轉角
障礙物邊界
空間結構
```

例如：

```text
上一幀的牆角輪廓
和
這一幀的牆角輪廓
```

如果兩者很接近，就可以估算機器人的位移與旋轉。

---

## 13.8 從真實環境到柵格地圖

SLAM 建立 2D 地圖時，常見輸出會使用 Occupancy Grid。

也就是：

```text
nav_msgs/OccupancyGrid
```

真實空間會被切成一格一格的 Grid。

常見數值：

```text
0   = Free / 可通行
100 = Occupied / 障礙物
-1  = Unknown / 未知區域
```

因此流程可以理解成：

```text
真實環境
↓
LiDAR 掃描
↓
位置估計
↓
把量測轉到共同座標系
↓
更新 Grid
↓
逐步建立 OccupancyGrid
```

---

## 13.9 為什麼 Odom 不能直接當 SLAM

Odom 可以告訴我們：

```text
「我大概移動了多少」
```

但它無法保證長時間都準確。

假設每次移動只有一點點誤差：

```text
第 1 次：誤差 1 cm
第 2 次：再多 1 cm
第 3 次：再多一些
...
```

時間一長，誤差就可能累積成：

```text
10 cm
20 cm
50 cm
甚至更大
```

這就是 Drift。

因此：

```text
Odom
→ 適合提供短時間連續移動估計

SLAM
→ 利用環境觀測持續修正位置
```

---

## 13.10 SLAM 如何修正位置

可以先用非常簡化的方式理解：

```text
Odom 說：
「我應該往前走了 1.0 m」

LiDAR Scan Matching 發現：
「按照牆的位置來看，實際比較像 0.96 m」

SLAM
↓
重新修正目前位姿
```

所以 SLAM 不會完全相信單一資料來源。

而是會根據感測器與環境資訊持續估算更合理的位置。

---

## 13.11 Loop Closure 是什麼

假設機器人從起點出發：

```text
A
↓
B
↓
C
↓
D
↓
最後又回到 A 附近
```

因為一路移動時有誤差，系統可能原本以為：

```text
現在的位置不是原本的 A
```

但 LiDAR 發現：

```text
目前看到的環境
和之前 A 點附近的環境非常相似
```

這時就可能判斷：

```text
「我回到以前來過的地方了。」
```

這就是 Loop Closure 的核心概念。

Loop Closure 可以幫助 SLAM 修正長距離移動產生的累積誤差。

---

## 13.12 Pose Graph 是什麼

可以把機器人走過的路程想成很多個節點：

```text
Pose 1
↓
Pose 2
↓
Pose 3
↓
Pose 4
```

每一個 Pose 代表某個時間點的機器人位置與方向。

節點之間會有各種約束，例如：

```text
Odom 約束
Scan Matching 約束
Loop Closure 約束
```

形成一張 Graph：

```text
Pose 1 ─ Pose 2 ─ Pose 3 ─ Pose 4
  └─────────────────────────┘
           Loop Closure
```

SLAM 可以透過這些約束，重新最佳化整條軌跡與地圖。

因此：

```text
Loop Closure
↓
發現「這裡以前來過」
↓
加入新的約束
↓
Pose Graph Optimization
↓
修正過去累積的誤差
↓
整張地圖重新對齊得更合理
```

---

## 13.13 Local SLAM 與 Global SLAM

很多現代 SLAM 系統可以簡單拆成兩個層次。

### Local SLAM

處理比較近、比較即時的資料。

主要工作：

```text
接收目前感測器資料
↓
Scan Matching
↓
估算目前位姿
↓
建立局部地圖
```

它重視的是：

```text
即時、連續、短時間準確
```

---

### Global SLAM

處理長時間與大範圍的整體一致性。

例如：

```text
Loop Closure
Pose Graph
Global Optimization
```

它主要處理：

```text
長時間累積誤差
地圖整體變形
回到曾經走過的位置
```

可以先記成：

```text
Local SLAM
→ 現在走得準不準

Global SLAM
→ 整張地圖最後有沒有對齊
```

---

## 13.14 SLAM 完整概念流程

整個 2D LiDAR SLAM 可以整理成：

```text
機器人移動
↓
LiDAR 持續發布 /scan
+
Odom 持續發布 /odom
+
IMU（視系統需求）
+
TF 提供座標關係
↓
SLAM 接收感測器資料
↓
Scan Matching
↓
估算目前 Pose
↓
更新 Local Map / Submap
↓
持續移動與觀測
↓
偵測 Loop Closure
↓
Pose Graph Optimization
↓
修正軌跡與地圖
↓
產生完整 2D 地圖
```

---

## 13.15 三張概念圖對應的意義

### 圖 1：不同位置觀測，如何拼成同一張地圖

建議搭配圖片：

```text
SLAM：從觀測拼成完整地圖(1).png
```

這張圖主要表達：

```text
不同位置觀測
↓
找到共同幾何特徵
↓
地圖對齊
↓
合併成完整地圖
```

適合對應：

- Scan Matching
- 地圖對齊
- 局部觀測合併

---

### 圖 2：SLAM 同時定位與建圖

建議搭配圖片：

```text
SLAM機器人定位與建圖流程(1).png
```

這張圖主要表達：

```text
LiDAR + Odom
↓
SLAM
↓
目前位置 + 柵格地圖
```

適合放在本章最前面，作為 SLAM 整體概念圖。

---

### 圖 3：真實環境如何變成柵格地圖

建議搭配圖片：

```text
SLAM：從環境到完整地圖(1).png
```

這張圖主要表達：

```text
真實空間
↓
柵格化
↓
不同位置觀測
↓
對齊與合併
↓
OccupancyGrid
```

適合對應：

- Occupancy Grid
- 局部地圖
- 地圖更新

---

## 13.16 常見 2D SLAM 演算法

ROS Noetic 常見的 2D SLAM 包含：

```text
Hector Mapping
Gmapping
Cartographer
```

本學習路線不打算把每一套 SLAM 都完整實作一次。

重點是先理解共通概念，再選 Cartographer 深入實作。

### Hector Mapping

概念上偏重 LiDAR Scan Matching。

可以在較弱依賴 Wheel Odom 的情況下進行建圖。

這裡只需要理解它的定位即可，不做深入實作。

---

### Gmapping

ROS1 很經典的 2D SLAM。

常見輸入：

```text
LiDAR
+
Odom
```

課程裡雖然有 Gmapping，但本學習路線不打算花大量時間做 Gmapping 調參。

---

### Cartographer

後續主要實作 SLAM。

會整合：

```text
LiDAR
TF
Odom
IMU（視設定）
```

並會接觸：

```text
Scan Matching
Submap
Pose Graph
Loop Closure
Optimization
```

所以下一章正式進入 Cartographer。

---

## 13.17 本章先不用做什麼

這一章先不要急著：

- 安裝 Cartographer
- 寫 Cartographer Lua config
- 調 Scan Matching 參數
- 調 Pose Graph 參數
- 深入比較所有 SLAM 演算法
- 同時做 Hector、Gmapping、Cartographer

這章只做一件事情：

```text
把 SLAM 的整體資料流與核心概念看懂
```

---

## 13.18 本章最後要真正理解的事情

如果最後能理解下面這段，本章就完成了：

```text
機器人不是掃一次就得到地圖。

它會在不同位置持續取得 LiDAR 觀測，
利用 Odom、IMU 與 TF 等資訊估算自身移動，
再透過 Scan Matching 把不同時間的局部觀測對齊。

隨著機器人持續移動，
SLAM 一邊修正機器人的位置，
一邊更新環境地圖。

當機器人再次回到曾經走過的位置，
Loop Closure 可以提供新的全域約束，
再透過 Pose Graph Optimization 修正累積誤差，
讓整條軌跡與地圖重新變得一致。
```

一句話整理：

```text
SLAM = 一邊估算自己在哪裡，一邊把多次局部觀測對齊並建立完整地圖。
```

---

# 下一章

```text
14_Cartographer_2D_SLAM_TF_Odom整合實戰
```

下一章開始正式把：

```text
Gazebo
LiDAR /scan
Odom /odom
TF
Cartographer
RViz
OccupancyGrid
```

全部串起來，完成第一套完整的 2D SLAM 模擬實作。
