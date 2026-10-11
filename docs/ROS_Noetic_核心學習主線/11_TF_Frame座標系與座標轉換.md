# 11_TF_Frame座標系與座標轉換入門

## 1. 本章定位

本章延續：

```text
10_OccupancyGrid柵格地圖_訊息結構與自訂地圖發布
```

本章先不進 Hector Mapping、Gmapping，也不直接進 Cartographer。

這一章專心把 ROS 裡最重要的座標系概念弄懂：

```text
Frame
TF
parent / child
translation
rotation
map
odom
base_footprint
base_link
laser_link
imu_link
```

本章以一般 ROS 行動機器人常見的 Frame 結構與公開範例進行說明，不包含任何內部專案、客戶資訊、設備名稱、實際路徑或未公開架構。

---

## 2. 本章學習目標

完成本章後，要能理解：

```text
1. TF 是什麼
2. Frame 是什麼
3. 兩個 Frame 之間為什麼需要 Transform
4. parent frame / child frame 是什麼
5. translation 與 rotation 分別代表什麼
6. Quaternion 為什麼會再次出現在 TF
7. /tf 與 /tf_static 的差異
8. map / odom / base_footprint / base_link 的角色
9. laser_link / imu_link 為什麼通常掛在 base_link 底下
10. 如何在 RViz 看 TF
11. 如何用 tf_echo 看兩個 Frame 的相對關係
```

---

# 3. TF 是什麼

TF 可以理解成：

> ROS 用來描述「不同座標系 Frame 之間相對位置與相對姿態」的機制。

例如：

```text
base_link
↓
laser_link
```

代表 LiDAR 的座標系 `laser_link` 是相對於車體座標系 `base_link` 定義的。

TF 會告訴 ROS：

```text
laser_link 相對 base_link
平移多少
旋轉多少
```

所以 TF 傳遞的不是地圖本身，而是：

```text
Frame A
相對
Frame B
的位置與姿態關係
```

---

# 4. Frame 是什麼

Frame 就是一套座標系。

例如：

```text
map
odom
base_footprint
base_link
laser_link
imu_link
```

每一個 Frame 都有自己的：

```text
X 軸
Y 軸
Z 軸
原點
方向
```

不同 Frame 之間要靠 TF 才能互相知道位置關係。

---

# 5. TF 的核心結構

一個 TF 關係可以簡化成：

```text
parent frame
↓
child frame
```

例如：

```text
base_link
↓
laser_link
```

表示：

```text
laser_link 是 base_link 的 child
base_link 是 laser_link 的 parent
```

兩者之間會有：

```text
translation
rotation
```

---

# 6. translation 是什麼

`translation` = 平移。

通常包含：

```text
x
y
z
```

例如某個 LiDAR 相對車體中心：

```text
x = 0.2
y = 0.0
z = 0.15
```

可以理解成：

```text
往前 0.2 m
左右沒有偏移
往上 0.15 m
```

這些數值只是示意，實際數值應以各自機器人的 URDF / Xacro 設定為準。

---

# 7. rotation 是什麼

`rotation` = 旋轉。

ROS TF 裡常用 Quaternion 表示：

```text
x
y
z
w
```

也就是前面 IMU 已經學過的四元數。

所以：

```text
TF
=
translation
+
rotation
```

也就是：

```text
平移
+
旋轉
```

---

# 8. TransformStamped 訊息結構

TF 常見的資料結構：

```text
geometry_msgs/TransformStamped
```

核心欄位：

```text
header.frame_id
child_frame_id
transform.translation
transform.rotation
```

可以理解成：

```text
header.frame_id
→ parent frame

child_frame_id
→ child frame

translation
→ child 相對 parent 的位置

rotation
→ child 相對 parent 的姿態
```

---

# 9. /tf Topic

動態 TF 通常會發布到：

```text
/tf
```

訊息型別：

```text
tf2_msgs/TFMessage
```

`TFMessage` 裡可以一次包含多個：

```text
geometry_msgs/TransformStamped
```

資料流可以理解成：

```text
TF Publisher
↓
/tf
↓
RViz / 其他 ROS Node
```

---

# 10. /tf_static

固定不會改變的座標關係通常會放在：

```text
/tf_static
```

例如：

```text
base_link → laser_link
base_link → imu_link
```

因為 LiDAR、IMU 等 Sensor 安裝完成後，相對於機器人本體的位置通常不會持續改變。

這類關係就是 static transform。

---

# 11. 常見的行動機器人 Frame Tree

常見架構：

```text
map
↓
odom
↓
base_footprint
↓
base_link
├── laser_link
└── imu_link
```

這棵樹之後在 Localization、SLAM、Navigation 裡會一直看到。

---

# 12. map Frame

```text
map
```

代表全域地圖座標系。

通常由：

```text
SLAM
Localization
```

相關系統提供。

它描述機器人在整張地圖裡的位置。

---

# 13. odom Frame

```text
odom
```

代表里程計座標系。

特點：

```text
短時間連續
移動平滑
但誤差可能隨時間累積
```

下一章會正式進入 Odometry。

---

# 14. base_footprint

```text
base_footprint
```

通常代表：

```text
機器人投影到地面的 2D 基準座標
```

通常只關心：

```text
x
y
yaw
```

---

# 15. base_link

```text
base_link
```

代表機器人本體主要座標系。

LiDAR、IMU、Camera 等 Sensor 通常都會掛在 `base_link` 底下。

例如：

```text
base_link
├── laser_link
├── imu_link
└── camera_link
```

---

# 16. 為什麼 Sensor 一定需要 Frame

例如 LiDAR 發布：

```text
/scan
```

資料裡會包含：

```text
header.frame_id
```

假設是：

```text
laser_link
```

意思是：

> 這份掃描資料，是以 `laser_link` 這個座標系為基準量到的。

如果後面要拿這些資料去做地圖、定位、避障或 Navigation，就必須知道：

```text
laser_link
相對
base_link
在哪裡
```

這就是 TF 的作用。

---

# 17. IMU 為什麼也需要 Frame

IMU 發布：

```text
/imu/data
```

資料通常也會包含：

```text
header.frame_id
```

例如：

```text
imu_link
```

代表這份姿態、角速度與線加速度，是以 `imu_link` 為基準。

後面如果要把 IMU 資料跟其他 Sensor 一起使用，就需要知道：

```text
imu_link
相對
base_link
的位置與方向
```

---

# 18. TF 跟 URDF 的關係

URDF / Xacro 描述機器人的結構：

```text
Link
Joint
Parent
Child
Origin
```

例如 fixed joint：

```text
base_link
↓
laser_link
```

`robot_state_publisher` 可以根據 URDF 的 joint 關係發布 TF。

概念上：

```text
URDF / Xacro
↓
robot_description
↓
robot_state_publisher
↓
TF
```

所以 URDF 不只是拿來畫模型，也定義了機器人各個 Link 之間的座標關係。

---

# 19. 在 RViz 看 TF

啟動自己的 ROS / Gazebo 測試環境後：

```bash
rviz
```

在 RViz：

```text
Add
→ TF
```

可以看到 Frame Tree。

常見會看到：

```text
odom
↓
base_link
├── laser_link
└── imu_link
```

如果目前還沒有 `map`，不用急著補。

`map` 會在後面的 SLAM / Localization 階段正式進來。

---

# 20. tf_echo

ROS Noetic 可以使用：

```bash
rosrun tf tf_echo base_link laser_link
```

用來查看：

```text
laser_link
相對
base_link
```

的位置與旋轉。

也可以看：

```bash
rosrun tf tf_echo base_link imu_link
```

---

# 21. TF Tree 工具

可以使用：

```bash
rosrun tf view_frames
```

產生 TF Tree 圖。

也可以用：

```bash
rosrun rqt_tf_tree rqt_tf_tree
```

直接查看目前 ROS 系統的 TF 關係。

---

# 22. /tf 與 /tf_static 的差異

```text
/tf
→ 會隨時間改變
→ 例如 odom → base_link

/tf_static
→ 固定不變
→ 例如 base_link → laser_link
```

先記成：

```text
會動的關係 → /tf
固定安裝關係 → /tf_static
```

---

# 23. Fixed Frame 再複習一次

RViz 的：

```text
Fixed Frame
```

意思是：

> RViz 要站在哪一個座標系來看所有資料。

例如：

```text
Fixed Frame = base_link
```

就像站在機器人本體上看世界。

如果：

```text
Fixed Frame = odom
```

就像站在里程計世界座標裡看機器人移動。

後面有 `map` 之後：

```text
Fixed Frame = map
```

就可以站在全域地圖座標系看整台機器人。

---

# 24. 本章不要急著做的東西

本章先不深入：

```text
Hector Mapping
Gmapping
Cartographer
AMCL
Navigation
EKF
Sensor Fusion
```

原因：

```text
TF 本身就是所有這些系統的基礎
```

先把 TF 弄懂，再進 Odometry 與 Cartographer。

---

# 25. Hector Mapping / Gmapping 在本章的定位

課程可能使用 Hector Mapping 或 Gmapping 來展示：

```text
map
odom
base_link
laser
```

這些 Frame 的關係。

本學習主線不需要完整實作每一套 SLAM。

本章只保留它們的概念：

```text
Hector Mapping
→ LiDAR scan matching 型 SLAM

Gmapping
→ 經典 2D LiDAR + Odometry SLAM
```

後續主線以 Cartographer 為主要 SLAM 實作。

---

# 26. 與 Cartographer 的關係

Cartographer 之後也會依賴 TF。

典型資料會包含：

```text
/scan
/imu/data（視設定）
/odom（視設定）
TF
```

最後會涉及：

```text
map
↓
odom
↓
base_link
↓
laser_link
```

所以本章不是額外的知識，而是 Cartographer 前的重要基礎。

---

# 27. 本章建議驗證

本章只需要完成以下幾項：

```text
□ 知道 TF 與 Frame 的差別
□ 知道 parent / child
□ 知道 translation / rotation
□ 知道 Quaternion 在 TF 裡的角色
□ RViz 能加入 TF Display
□ 能使用 tf_echo
□ 能看懂 base_link → laser_link
□ 能看懂 base_link → imu_link
□ 知道 /tf 與 /tf_static 的差別
□ 初步理解 map → odom → base_link
```

---

# 28. 本章核心資料流

```text
URDF / Xacro
↓
robot_state_publisher
↓
TF
↓
/tf 或 /tf_static
↓
RViz / ROS Node / SLAM / Navigation
```

---

# 29. 本章最重要的一句話

```text
Frame 是座標系；
TF 是不同 Frame 之間的位置與旋轉關係。
```

再往後：

```text
map
↓
odom
↓
base_link
↓
Sensor Frames
```

會成為 ROS Localization、SLAM 與 Navigation 的核心骨架。

---

# 30. 下一步

下一章：

```text
12_Odometry里程計與odom座標系
```

下一章再正式理解：

```text
/odom
nav_msgs/Odometry
odom → base_link
位置
速度
Yaw
里程誤差累積
```

把 TF 與 Odometry 串起來後，再正式進入 Cartographer。
