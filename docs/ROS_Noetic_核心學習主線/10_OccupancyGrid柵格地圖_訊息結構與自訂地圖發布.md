# ROS_10_OccupancyGrid柵格地圖_訊息結構與自訂地圖發布

## 1. 本章定位

本章延續前面的 ROS 基礎、RViz、LiDAR 與 IMU，開始進入 ROS 地圖資料的表示方式。

本章重點不是先做 SLAM，而是先搞懂：

```text
ROS 裡「一張 2D 地圖」到底是怎麼存的？
```

本章核心主線：

```text
OccupancyGrid 訊息格式
↓
header / info / data
↓
設定 resolution / width / height / origin
↓
建立每一格的佔據值
↓
發布 /map
↓
RViz 顯示地圖
```

---

## 2. Grid Map / OccupancyGrid 是什麼

### 2.1 中文名稱

```text
Grid Map
→ 柵格地圖 / 網格地圖

OccupancyGrid
→ 佔據柵格地圖
```

在 ROS 導航與 SLAM 裡，`OccupancyGrid` 是很常見的 2D 地圖資料格式。

它的概念就是：

```text
把真實世界切成很多小格子
↓
每一格記錄「這裡能不能走」
```

---

## 3. /map Topic 與 map_server

常見資料流：

```text
map_server
↓
nav_msgs/OccupancyGrid
↓
/map
↓
RViz
```

也就是：

```text
/map
= ROS 裡常見的地圖 Topic

Message Type
= nav_msgs/OccupancyGrid
```

RViz 可以訂閱 `/map`，把這些格子資料畫成地圖。

---

## 4. OccupancyGrid 每一格代表什麼

常見值：

```text
0
→ Free
→ 自由空間
→ 可通行

100
→ Occupied
→ 已佔據
→ 障礙物

-1
→ Unknown
→ 未知區域
→ 尚未確定
```

可以先記成：

```text
0   = 可以走
100 = 不可以走
-1  = 不知道
```

RViz 常見顯示：

```text
白色 → 0
黑色 → 100
灰色 → -1
```

---

## 5. nav_msgs/OccupancyGrid 訊息結構

核心結構：

```text
std_msgs/Header header
nav_msgs/MapMetaData info
int8[] data
```

可以理解成：

```text
OccupancyGrid
├── header
│   ├── stamp
│   └── frame_id
│
├── info
│   ├── map_load_time
│   ├── resolution
│   ├── width
│   ├── height
│   └── origin
│
└── data[]
    └── 每一格的佔據值
```

---

## 6. Header

```text
header.stamp
→ 這張地圖資料的時間

header.frame_id
→ 這張地圖屬於哪個座標系
```

常見：

```cpp
msg.header.frame_id = "map";
msg.header.stamp = ros::Time::now();
```

也就是：

```text
這張地圖是以 map Frame 為基準
```

---

## 7. MapMetaData

### 7.1 resolution

```text
resolution
→ 每一格代表真實世界多大
```

例如：

```cpp
msg.info.resolution = 1.0;
```

表示：

```text
1 格 = 1 公尺 × 1 公尺
```

如果：

```text
resolution = 0.05
```

則表示：

```text
1 格 = 0.05 m = 5 cm
```

---

### 7.2 width

```text
width
→ 地圖有幾欄
→ X 方向格子數量
```

例如：

```cpp
msg.info.width = 4;
```

代表一列有 4 格。

---

### 7.3 height

```text
height
→ 地圖有幾列
→ Y 方向格子數量
```

例如：

```cpp
msg.info.height = 2;
```

代表共有 2 列。

所以：

```text
width = 4
height = 2

總格數 = 4 × 2 = 8
```

---

## 8. origin 是什麼

```text
origin
→ 地圖格子座標 (0,0)
   在真實 map 世界座標中的位置與方向
```

它的型別是：

```text
geometry_msgs/Pose
```

所以包含：

```text
position
→ x y z

orientation
→ Quaternion x y z w
```

例如：

```cpp
msg.info.origin.position.x = 0;
msg.info.origin.position.y = 0;
```

代表地圖原點直接放在 map 座標的 `(0,0)`。

---

## 9. data 為什麼是一維陣列

OccupancyGrid 看起來是二維地圖：

```text
□ □ □ □
□ □ □ □
```

但 ROS 裡實際存的是：

```text
int8[] data
```

也就是一維陣列。

例如：

```text
width = 4
height = 2
```

總共有 8 格：

```cpp
msg.data.resize(4 * 2);
```

---

## 10. row-major 是什麼

ROS 的 `OccupancyGrid.data` 依照 row-major 順序排列。

可以理解成：

```text
先存第 0 列
再存第 1 列
再存第 2 列
...
```

例如：

```text
第 1 列：100 100 0 -1
第 2 列：0   0   0  0
```

實際 data：

```text
[100, 100, 0, -1, 0, 0, 0, 0]
```

索引公式：

```text
index = y * width + x
```

其中：

```text
x = 第幾欄
y = 第幾列
```

例如 width = 4：

```text
(x=0, y=0)
index = 0 * 4 + 0 = 0

(x=3, y=0)
index = 0 * 4 + 3 = 3

(x=0, y=1)
index = 1 * 4 + 0 = 4
```

---

## 11. 自訂地圖範例

範例建立：

```text
map_pub_node
```

發布：

```text
/map
```

Message：

```text
nav_msgs/OccupancyGrid
```

核心概念：

```cpp
ros::Publisher pub =
    n.advertise<nav_msgs::OccupancyGrid>("/map", 10);
```

然後建立：

```cpp
nav_msgs::OccupancyGrid msg;
```

設定：

```cpp
msg.header.frame_id = "map";
msg.header.stamp = ros::Time::now();

msg.info.origin.position.x = 0;
msg.info.origin.position.y = 0;
msg.info.resolution = 1.0;
msg.info.width = 4;
msg.info.height = 2;
```

配置 8 格：

```cpp
msg.data.resize(4 * 2);
```

再指定格子：

```cpp
msg.data[0] = 100;
msg.data[1] = 100;
msg.data[2] = 0;
msg.data[3] = -1;
```

最後：

```cpp
pub.publish(msg);
```

---

## 12. 本專案怎麼做

本章不要另外建立 `map_pkg`。

沿用目前：

```text
ros_learning_pkg
```

新增：

```text
src/map_publisher_node.cpp
```

並修改：

```text
CMakeLists.txt
package.xml
```

加入：

```text
nav_msgs
```

依賴。

---

## 13. 預計實作步驟

### Step 1：新增 map_publisher_node.cpp

目標：

```text
建立 OccupancyGrid
↓
設定 header
↓
設定 info
↓
設定 data
↓
發布 /map
```

---

### Step 2：修改 CMakeLists.txt

加入：

```text
nav_msgs
```

並加入：

```text
map_publisher_node
```

executable。

---

### Step 3：修改 package.xml

加入：

```xml
<depend>nav_msgs</depend>
```

---

### Step 4：編譯

```bash
cd ~/ros_learning_ws
catkin_make
source devel/setup.bash
```

---

### Step 5：執行 Node

```bash
rosrun ros_learning_pkg map_publisher_node
```

確認：

```bash
rostopic info /map
```

Message Type 應該是：

```text
nav_msgs/OccupancyGrid
```

---

### Step 6：RViz 顯示

啟動：

```bash
rviz
```

設定：

```text
Fixed Frame = map
```

Add：

```text
Map
```

Topic：

```text
/map
```

就可以看到自己用 C++ 建立的柵格地圖。

---

### Step 7：加上 Axes 確認原點與方向

只看地圖，看不出原點在哪、哪邊是 +X、哪邊是 +Y。

Add：

```text
Axes
```

Reference Frame：

```text
<Fixed Frame>
→ 也就是 map
```

Axes 會在 Frame 的原點畫出三根軸：

```text
紅 → X
綠 → Y
藍 → Z

RGB = XYZ
```

對照本章的地圖：

```text
origin = (0,0)
→ 地圖左下角剛好在 Axes 的位置

data[0]、data[1]（黑色）
→ 從原點沿著紅色 X 軸排出去

y = 1 那一列
→ 在綠色 Y 軸方向，所以畫在上面
```

也就是：

```text
上面（y=1）：白 白 白 白
下面（y=0）：黑 黑 白 灰
            ↑
          原點
```

所以 `data` 的第 0 列在 RViz 裡是「下面」那一列，不是上面。

這張地圖是手寫的假資料，和 Gazebo 世界無關，不需要旋轉對齊。

後面 TF 章節會用 `TF` Display，等於一次幫每個 Frame 都畫上 Axes。

---

## 14. 本章最重要的理解

不要把 `/map` 想成一張 PNG 圖片。

ROS 裡的地圖真正存的是：

```text
地圖尺寸
+
每格解析度
+
原點位置
+
每格佔據值
```

也就是：

```text
OccupancyGrid
= 地圖參數
+ int8[] data
```

RViz 只是把這些資料「畫出來」。

---

## 15. 與後續 Cartographer 的關係

目前這章是自己手動建立：

```text
OccupancyGrid
```

後面進 Cartographer 時，概念會變成：

```text
LiDAR /scan
+
TF
+
Odometry / Localization
↓
Cartographer
↓
建立地圖
↓
轉成 ROS 地圖資料
↓
/map
```

所以這章不是在學「怎麼 SLAM」，而是在先理解：

> SLAM 最後做出來的地圖，在 ROS 裡到底是什麼資料格式。

---

# 16. 本章完成標準

```text
□ 知道 OccupancyGrid 是什麼
□ 知道 0 / 100 / -1 代表什麼
□ 知道 resolution / width / height
□ 知道 origin 的用途
□ 知道 data 是 int8[]
□ 知道 row-major
□ 知道 index = y * width + x
□ 可以自己發布 /map
□ RViz 可以顯示自己建立的地圖
□ 會用 Axes 確認地圖原點與 X / Y 方向
```

---

# 17. 一句話總結

```text
OccupancyGrid
= 把 2D 世界切成一格一格
+ 每格記錄 Free / Occupied / Unknown
+ 用 /map 發布給 ROS 其他節點與 RViz 使用
```
