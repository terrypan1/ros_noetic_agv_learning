# 14_Cartographer_2D_SLAM_TF_Odom整合實戰

## 1. 本章目標

本章開始正式進入 Cartographer 2D SLAM 實作，並把前面學過的 LiDAR、TF、Odometry、OccupancyGrid 與 Gazebo 模擬環境整合起來。

最終資料流：

```text
Gazebo 模擬場景
↓
模擬車移動
↓
/scan
/odom
TF
↓
Cartographer
↓
SLAM 建圖
↓
/map
↓
RViz
↓
保存地圖
```

本章不另外建立新的 ROS package，沿用目前的 `ros_learning_pkg`。

---

## 2. 本章實作原則

這一章不是只學「怎麼啟動 Cartographer」，而是要完整理解 Cartographer 需要哪些資料，以及這些資料如何串在一起。

本章整合的核心包括：

```text
LiDAR
TF
Odometry
Cartographer
OccupancyGrid
RViz
Gazebo
```

第一版先使用：

```text
LiDAR + Odom
```

等整體建圖流程穩定後，再視需要加入 IMU。

---

## 3. Phase 1：確認目前 ROS Topic

先啟動目前既有的 Gazebo 模擬車與小房間場景。

接著查看目前 Topic：

```bash
rostopic list
```

至少確認下列 Topic 是否存在：

```text
/scan
/odom
/cmd_vel
/tf
/tf_static
```

如果這些 Topic 已經存在，代表目前模擬環境已具備進入 Cartographer 的基本條件。

---

## 4. Phase 2：確認 LiDAR `/scan`

查看 Topic 型別：

```bash
rostopic info /scan
```

預期型別：

```text
sensor_msgs/LaserScan
```

再查看一筆實際資料：

```bash
rostopic echo -n 1 /scan
```

主要確認：

```text
header.frame_id
angle_min
angle_max
angle_increment
range_min
range_max
ranges[]
```

其中最重要的是：

```text
header.frame_id
```

它必須和 TF 中 LiDAR 所在的 Frame 對得起來，例如：

```text
laser_link
```

資料流：

```text
Gazebo LiDAR
↓
/scan
↓
sensor_msgs/LaserScan
↓
Cartographer
```

---

## 5. Phase 3：確認 Odometry `/odom`

查看 Topic：

```bash
rostopic info /odom
```

預期型別：

```text
nav_msgs/Odometry
```

查看一筆資料：

```bash
rostopic echo -n 1 /odom
```

主要確認：

```text
header.frame_id
child_frame_id
pose
pose.pose.position
pose.pose.orientation
twist
twist.twist.linear
twist.twist.angular
```

通常會看到類似：

```text
header.frame_id: odom
child_frame_id: base_link
```

代表 Odometry 描述的是：

```text
odom
↓
base_link
```

之間的相對移動關係。

---

## 6. Phase 4：確認 TF Tree

Cartographer 能不能正常運作，TF 是非常重要的一環。

先確認：

```bash
rosrun tf tf_echo odom base_link
```

如果持續看到 translation 與 rotation，代表：

```text
odom → base_link
```

存在。

再確認 LiDAR：

```bash
rosrun tf tf_echo base_link laser_link
```

如果可以正常取得轉換，代表：

```text
base_link → laser_link
```

存在。

此時基本 TF Tree 應該類似：

```text
odom
└── base_link
    └── laser_link
```

如果 IMU 已存在，也可能是：

```text
odom
└── base_link
    ├── laser_link
    └── imu_link
```

---

## 7. Cartographer 加入後的 Frame 關係

Cartographer 開始建圖後，會開始建立全域地圖座標系。

完整概念會變成：

```text
map
↓
odom
↓
base_link
↓
laser_link
```

各 Frame 的角色：

```text
map
→ SLAM 建立的全域地圖座標

odom
→ 短時間連續的里程計座標

base_link
→ 機器人本體座標

laser_link
→ LiDAR 座標
```

Cartographer 必須知道這些 Frame 之間的關係，才能把不同時間收到的 LaserScan 放到正確的位置。

---

## 8. Phase 5：安裝與確認 Cartographer

確認 ROS Noetic 環境：

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
```

確認是否已有 Cartographer ROS：

```bash
rospack find cartographer_ros
```

或：

```bash
roscd cartographer_ros
```

如果系統尚未安裝，再進行 Cartographer ROS 安裝與設定。

本章後續會使用：

```text
cartographer_node
cartographer_occupancy_grid_node
```

Cartographer 本身負責 SLAM，`cartographer_occupancy_grid_node` 則負責把 Submap 轉成一般 RViz / Navigation 可使用的 OccupancyGrid。

---

## 9. Phase 6：建立 Cartographer 設定資料夾

在目前的 `ros_learning_pkg` 內加入：

```text
ros_learning_pkg/
├── config/
│   └── cartographer_2d.lua
├── launch/
│   └── cartographer_2d.launch
├── src/
├── urdf/
└── worlds/
```

本章不另外建立新 package。

---

## 10. `cartographer_2d.lua` 的角色

`cartographer_2d.lua` 是 Cartographer 的主要設定檔。

它會決定：

```text
使用 2D 還是 3D SLAM
tracking_frame 是誰
published_frame 是誰
odom_frame 是誰
是否使用 Odom
是否使用 IMU
是否使用 LaserScan
Submap 如何建立
Scan Matching 如何處理
Pose Graph 如何最佳化
```

第一版實作方向：

```text
2D SLAM
LiDAR = 使用
Odom = 使用
IMU = 暫時不使用
```

先讓最小系統成功運作，再逐步增加資料來源。

---

## 11. 第一版 Cartographer 資料流

第一版先建立最簡單的：

```text
/scan
+
/odom
+
TF
↓
Cartographer
↓
Submap
↓
Pose Estimation
↓
/map
```

不要一開始同時加入過多資料來源。

如果第一版就同時加入：

```text
/scan
/odom
/imu/data
多組 TF
```

出現錯誤時會比較難判斷是哪一個來源造成。

---

## 12. Phase 7：建立 Cartographer Launch

建立：

```text
launch/cartographer_2d.launch
```

Launch 的責任是把需要的 Node 串起來。

整體概念：

```text
Gazebo
↓
Robot / Sensor
↓
Cartographer Node
↓
Occupancy Grid Node
↓
RViz
```

最終希望可以使用一個指令啟動 Cartographer：

```bash
roslaunch ros_learning_pkg cartographer_2d.launch
```

---

## 13. Cartographer Node 的輸入

Cartographer 主要接收：

```text
LaserScan
Odometry
TF
```

第一版：

```text
/scan
/odom
```

其中 Frame 必須可以透過 TF 查詢。

例如 LaserScan：

```text
header.frame_id = laser_link
```

Cartographer 必須可以查到：

```text
base_link → laser_link
```

如果查不到，Cartographer 就不知道這批雷射資料相對於車體的位置。

---

## 14. Phase 8：啟動 RViz

Cartographer 成功運作後，在 RViz 中觀察：

```text
TF
LaserScan
RobotModel
Map
```

如果有 Cartographer 對應 Display，也可以觀察 Submap。

RViz 的 Fixed Frame 之後通常會設定成：

```text
map
```

此時可以看到：

```text
機器人移動
↓
LiDAR 持續掃描
↓
SLAM 持續更新位置
↓
地圖逐步建立
```

---

## 15. Phase 9：用 rqt 控制模擬車

使用：

```bash
rosrun rqt_robot_steering rqt_robot_steering
```

Topic 設定：

```text
/cmd_vel
```

建議一開始速度不要太快。

實際操作方式：

```text
慢速前進
↓
轉彎
↓
沿牆走
↓
走完整個房間
↓
回到曾經經過的位置
```

觀察 RViz 中的地圖是否逐步完整。

---

## 16. SLAM 實際運作流程

當車子移動時：

```text
LiDAR 掃描
↓
取得當下局部環境
↓
Odom 提供移動估計
↓
TF 提供座標轉換
↓
Cartographer Scan Matching
↓
估計目前位姿
↓
建立 / 更新 Submap
↓
Pose Graph 最佳化
↓
產生完整地圖
```

這就是第 13 章 SLAM 原理真正進入實作後的資料流。

---

## 17. Phase 10：觀察 Topic

Cartographer 運作後，可以再次查看：

```bash
rostopic list
```

重點觀察 Cartographer 新增的 Topic。

其中最重要的是最後的：

```text
/map
```

確認：

```bash
rostopic info /map
```

預期型別：

```text
nav_msgs/OccupancyGrid
```

這就和第 10 章自己建立的 `map_publisher_node.cpp` 接起來了。

差別是：

```text
第 10 章
自己手動建立 OccupancyGrid

第 14 章
Cartographer 根據真實 / 模擬感測資料自動建立 OccupancyGrid
```

---

## 18. Phase 11：保存地圖

當地圖建立完成後，可以保存：

```bash
rosrun map_server map_saver -f my_map
```

會產生：

```text
my_map.pgm
my_map.yaml
```

其中：

```text
my_map.pgm
→ 地圖影像

my_map.yaml
→ 地圖參數
```

這代表完整流程：

```text
實際環境
↓
LiDAR / Odom / TF
↓
Cartographer
↓
OccupancyGrid
↓
/map
↓
PGM + YAML
```

---

## 19. Phase 12：第一版驗收項目

第一版 Cartographer 成功標準：

```text
[ ] Gazebo 小房間可以正常啟動
[ ] 模擬車可以透過 /cmd_vel 移動
[ ] /scan 正常
[ ] /odom 正常
[ ] odom → base_link TF 正常
[ ] base_link → laser_link TF 正常
[ ] Cartographer 可以啟動
[ ] RViz 可以看到 LaserScan
[ ] RViz 可以看到 TF
[ ] Cartographer 可以建立地圖
[ ] /map 為 nav_msgs/OccupancyGrid
[ ] 車移動時地圖會持續更新
[ ] 可以使用 map_saver 保存地圖
```

以上全部完成，代表第一版 Cartographer 2D SLAM 整合成功。

---

## 20. 常見問題方向

如果 Cartographer 無法正常運作，優先依序檢查：

```text
1. /scan 是否有資料
2. /odom 是否有資料
3. LaserScan frame_id 是否正確
4. odom → base_link 是否存在
5. base_link → laser_link 是否存在
6. Cartographer 設定中的 Frame 名稱是否一致
7. use_odometry 設定是否符合目前系統
8. Topic 名稱是否一致
9. RViz Fixed Frame 是否正確
```

不要一開始就直接亂調 Cartographer 演算法參數。

先把：

```text
Topic
Message
Frame
TF
```

確認正確，再處理 SLAM 參數。

---

## 21. 第二階段：加入 IMU

第一版 LiDAR + Odom 穩定後，再考慮加入：

```text
/imu/data
```

資料流變成：

```text
LiDAR
+
Odom
+
IMU
+
TF
↓
Cartographer
```

此時再比較：

```text
只有 LiDAR + Odom
```

和：

```text
LiDAR + Odom + IMU
```

在旋轉、轉彎與定位上的差異。

IMU 不需要為了「資料越多越好」而強制加入，必須確認 Frame、方向與資料品質都正確。

---

## 22. 本章與前面章節的關係

第 8 章：

```text
LiDAR / LaserScan
```

第 9 章：

```text
IMU
```

第 10 章：

```text
OccupancyGrid
```

第 11 章：

```text
TF
```

第 12 章：

```text
Odometry
```

第 13 章：

```text
SLAM 原理
```

第 14 章：

```text
把前面的資料正式整合成 Cartographer 2D SLAM
```

因此第 14 章不是新的獨立知識，而是前面所有基礎第一次真正整合起來。

---

## 23. 本章最重要的觀念

Cartographer 並不是：

```text
LiDAR → 地圖
```

而是：

```text
LiDAR
+
機器人移動資訊
+
座標系關係
↓
估計位置
↓
對齊不同時間的觀測
↓
建立 Submap
↓
修正誤差
↓
建立完整地圖
```

因此真正要理解的是整條資料流：

```text
Sensor
↓
Topic
↓
Message
↓
TF / Odom
↓
SLAM
↓
Map
```

這也是本章最主要的學習目標。

---

## 24. 本章完成後

完成本章後，目前 ROS Noetic 學習專案已經具備：

```text
ROS Node
Publisher / Subscriber
Topic / Message
Launch
Gazebo
RViz
/cmd_vel
LiDAR
IMU
OccupancyGrid
TF
Odometry
SLAM
Cartographer
地圖保存
```

下一階段再開始進入：

```text
Navigation
AMCL
Costmap
Global Planner
Local Planner
move_base
Action Goal
```

