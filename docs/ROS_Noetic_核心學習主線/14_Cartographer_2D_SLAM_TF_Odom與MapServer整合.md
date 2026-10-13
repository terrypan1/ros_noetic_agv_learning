# 14_Cartographer_2D_SLAM_TF_Odom與MapServer整合實戰

## 1. 本章目標

本章要把前面已經完成的 Gazebo 小房間、LiDAR、Odom、TF 真正整合到 Cartographer 2D SLAM，最後把建立完成的地圖保存成可重複使用的地圖檔。

整體流程：

```text
simple_house.world
↓
simple_robot
↓
/scan + /odom + TF
↓
Cartographer 2D SLAM
↓
/map
↓
RViz 驗證
↓
map_server / map_saver
↓
.pgm + .yaml
```

本章第一版先使用：

```text
/scan + /odom + TF
```

`/imu/data` 先不加入，等第一版穩定後再擴充。

---

## 2. 目前已經完成的前置條件

目前學習專案已經具備：

```text
/cmd_vel
/scan
/imu/data
/odom
/tf
/tf_static
/joint_states
```

Gazebo 小房間場景：

```text
worlds/simple_house.world
```

啟動檔：

```text
launch/gazebo_house.launch
```

目前已確認：

- Gazebo 可以正常載入小房間。
- simple_robot 可以正常生成。
- 車子可以使用 `/cmd_vel` 移動。
- LiDAR 可以發布 `/scan`。
- 差速驅動 Plugin 可以發布 `/odom`。
- TF 與 TF Static 已存在。

因此接下來不再修改房間場景，正式進入 Cartographer。

---

## 3. Phase 1：再次確認 Topic

啟動 Gazebo 小房間後：

```bash
rostopic list
```

本章主要關注：

```text
/scan
/odom
/tf
/tf_static
/cmd_vel
```

檢查 LiDAR：

```bash
rostopic info /scan
rostopic echo -n 1 /scan
```

預期型別：

```text
sensor_msgs/LaserScan
```

檢查 Odom：

```bash
rostopic info /odom
rostopic echo -n 1 /odom
```

預期型別：

```text
nav_msgs/Odometry
```

---

## 4. Phase 2：確認 TF Tree

Cartographer 需要知道感測器與車體之間的座標關係。

先確認：

```bash
rosrun tf tf_echo odom base_link
```

再確認：

```bash
rosrun tf tf_echo base_link laser_link
```

目前希望先具備：

```text
odom
└── base_link
    └── laser_link
```

其中：

- `odom -> base_link`：由 Gazebo 差速驅動 Plugin 發布。
- `base_link -> laser_link`：由 URDF + robot_state_publisher 發布。

Cartographer 加入後，完整概念會變成：

```text
map
↓
odom
↓
base_link
↓
laser_link
```

本章會真正把第 11 章 TF 與第 12 章 Odom 串進 SLAM。

---

## 5. Phase 3：安裝 Cartographer

目前 WSL Ubuntu 20.04 / ROS Noetic 環境尚未完成 Cartographer 安裝。

需要準備：

```text
cartographer
cartographer_ros
```

Noetic 環境若 apt 無完整可用版本，則以 source build 方式處理。

安裝完成後至少確認：

```bash
roscd cartographer_ros
```

可以正常找到 package。

也要確認 Cartographer ROS 的相關 node / launch 可以被 ROS 找到。

---

## 6. Phase 4：建立 Cartographer 設定檔

不另外建立新 package，沿用：

```text
ros_learning_pkg
```

新增：

```text
ros_learning_pkg/
├── config/
│   └── cartographer_2d.lua
└── launch/
    └── cartographer_2d.launch
```

`cartographer_2d.lua` 主要負責設定：

```text
tracking_frame
published_frame
odom_frame
provide_odom_frame
use_odometry
num_laser_scans
2D SLAM
```

第一版預計使用：

```text
tracking_frame     = base_link
published_frame    = odom
odom_frame         = odom
provide_odom_frame = false
use_odometry       = true
num_laser_scans    = 1
```

`published_frame` 是 Cartographer 要發布 `map -> 誰` 的那個 frame。

`odom -> base_link` 已經由 Gazebo 差速驅動 Plugin 發布，所以 Cartographer 只發布 `map -> odom`。

如果設成 `base_link`，`base_link` 會同時有 `map` 與 `odom` 兩個 parent，TF Tree 會衝突。

第一版不使用 IMU。

核心輸入：

```text
/scan
/odom
TF
```

---

## 7. Phase 5：建立 Cartographer Launch

建立：

```text
launch/cartographer_2d.launch
```

這個 launch 的目標是啟動：

```text
Cartographer Node
+
Occupancy Grid Node
```

Gazebo 小房間仍由：

```text
gazebo_house.launch
```

負責啟動。

第一階段可以分開啟動，方便 Debug：

Terminal 1：

```bash
roslaunch ros_learning_pkg gazebo_house.launch
```

Terminal 2：

```bash
roslaunch ros_learning_pkg cartographer_2d.launch
```

後續確認穩定後，再考慮整合成一個總 launch。

---

## 8. Phase 6：啟動 RViz

啟動：

```bash
rviz
```

RViz 主要觀察：

```text
TF
LaserScan
Map
RobotModel
Cartographer Submaps
```

Fixed Frame 依 Cartographer 實際 TF 設定調整，最終應以 `map` 為主要全域座標系。

觀察重點：

```text
機器人移動
↓
/scan 持續更新
↓
Cartographer 進行 Scan Matching
↓
Submap 逐步建立
↓
/map 逐漸形成
```

---

## 9. Phase 7：用 rqt_robot_steering 控車建圖

啟動：

```bash
rosrun rqt_robot_steering rqt_robot_steering
```

Topic：

```text
/cmd_vel
```

建議以低速慢慢繞完整個小房間。

建圖路線可以先採：

```text
中央出生點
→ 左側房間
→ 左上區域
→ 回中央
→ 右上區域
→ 右側區域
→ 右下區域
→ 回中央
```

不要一開始高速移動。

SLAM 初期重點是讓 LiDAR 掃描之間有足夠重疊。

### 9.1 `/odom` timestamp 重複，Cartographer 當掉

第一次開車建圖到模擬時間 603 秒時，`cartographer_node` 當掉：

```text
F map_by_time.h:43] Check failed: data.time > std::prev(trajectory.end())->first
(621355974033840000 vs. 621355974033840000)
... OptimizationProblem2D::AddOdometryData()
[cartographer_node-1] process has died [exit code -6]
```

原因：

```text
Gazebo 差速驅動 Plugin 偶爾會發出兩筆 timestamp 相同的 /odom
↓
Cartographer 要求 Odometry 的時間必須嚴格遞增
↓
遇到重複就 Check failed，整個 Node 中止
```

這不是操作錯誤，也不是 `.lua` 寫錯。

看 Node 當掉的原因：在跑 launch 的 Terminal 往上找 `F` 開頭的那行。

### 9.2 修法：Odom Timestamp Filter Node

新增：

```text
src/odom_timestamp_filter_node.cpp
```

資料流：

```text
Gazebo diff_drive
↓
/odom
↓
odom_timestamp_filter_node
↓
/odom_filtered
↓
Cartographer
```

規則：

- `stamp > last_stamp`：原封不動發布到 `/odom_filtered`，更新 `last_stamp`。
- `stamp <= last_stamp`：丟掉，用 `ROS_WARN` 印出被丟掉的 timestamp。
- 不修改原始 timestamp。
- 原本的 `/odom` 不改名、不刪除。

`CMakeLists.txt` 加上：

```cmake
add_executable(odom_timestamp_filter_node src/odom_timestamp_filter_node.cpp)

target_link_libraries(odom_timestamp_filter_node
  ${catkin_LIBRARIES}
)
```

`cartographer_2d.launch` 的 `cartographer_node` 加上 remap：

```xml
<remap from="odom" to="/odom_filtered"/>
```

`<remap>` 寫在 `<node>` 裡面，所以只有 Cartographer 改收 `/odom_filtered`。

啟動順序變成：

```bash
roslaunch ros_learning_pkg gazebo_house.launch
rosrun ros_learning_pkg odom_timestamp_filter_node
roslaunch ros_learning_pkg cartographer_2d.launch
```

確認接線：

```bash
rostopic info /odom
rostopic info /odom_filtered
```

預期：

```text
/odom          的 Subscribers 只有 /odom_timestamp_filter_node
/odom_filtered 的 Subscribers 有   /cartographer_node
```

### 9.3 加上 Filter 後的結果

連續建圖到模擬時間 1162 秒，Cartographer 沒有再當掉。

Filter Node 在 486～487 秒擋掉 3 筆重複的 `/odom`：

```text
[Odom Filter] drop /odom, stamp: 486.128000000 <= last_stamp: 486.128000000
[Odom Filter] drop /odom, stamp: 486.445000000 <= last_stamp: 486.445000000
[Odom Filter] drop /odom, stamp: 487.711000000 <= last_stamp: 487.711000000
```

三筆都是 `stamp` 與 `last_stamp` 完全相等，就是原本會讓 Cartographer 當掉的那種資料。

---

## 10. Phase 8：觀察 Cartographer 建圖

建圖過程中注意：

### 10.1 地圖是否重疊

如果同一面牆被畫成兩條：

```text
可能原因：
- Odom 漂移過大
- TF 不正確
- 車移動太快
- Cartographer 參數不適合
```

### 10.2 轉彎後地圖是否歪掉

如果轉彎後新地圖與舊地圖對不齊，要檢查：

```text
/odom
TF
LaserScan frame
wheelSeparation
wheelDiameter
```

### 10.3 回到原點附近

讓車繞一圈回到曾經走過的區域，用來觀察：

```text
Loop Closure
```

也就是：

```text
我又回到以前來過的位置
↓
Cartographer 發現掃描特徵相似
↓
修正 Pose Graph
↓
地圖整體重新對齊
```

---

## 11. Phase 9：確認 `/map`

Cartographer Occupancy Grid Node 正常後，確認：

```bash
rostopic info /map
```

預期型別：

```text
nav_msgs/OccupancyGrid
```

也可以讀一筆：

```bash
rostopic echo -n 1 /map
```

這裡就會真正接回第 10 章學過的：

```text
nav_msgs/OccupancyGrid
```

也就是：

```text
SLAM
↓
OccupancyGrid
↓
/map
```

---

## 12. Phase 10：安裝 map_server

目前 WSL 尚未處理 `map_server`。

安裝：

```bash
sudo apt update
sudo apt install ros-noetic-map-server
```

確認：

```bash
roscd map_server
```

`map_server` 主要有兩個用途：

```text
map_saver
→ 把目前 /map 保存成檔案

map_server
→ 之後重新把保存好的地圖發布出來
```

本章先使用 `map_saver`。

---

## 13. Phase 11：保存 Cartographer 地圖

Cartographer 建圖完成，而且 `/map` 正常後執行：

```bash
mkdir -p ~/ros_learning_ws/src/ros_learning_pkg/maps
cd ~/ros_learning_ws/src/ros_learning_pkg/maps
rosrun map_server map_saver -f simple_house_map
```

地圖存在 package 的 `maps/` 資料夾。

存檔時 Cartographer 不能關，地圖只存在它的記憶體裡。

會產生：

```text
simple_house_map.pgm
simple_house_map.yaml
```

其中：

### `.pgm`

```text
實際的 2D 黑白 / 灰階柵格地圖
```

### `.yaml`

保存：

```text
image
resolution
origin
occupied_thresh
free_thresh
negate
```

這兩個檔案要一起使用。

這次存出來的 `simple_house_map.yaml`：

```yaml
image: simple_house_map.pgm
resolution: 0.050000
origin: [-4.425061, -3.505073, 0.000000]
negate: 0
occupied_thresh: 0.65
free_thresh: 0.196
```

地圖大小 178 × 139 格，也就是 8.9 m × 6.95 m。

---

## 14. Phase 12：重新載入保存地圖

之後可以停止 SLAM，直接用：

```bash
cd ~/ros_learning_ws/src/ros_learning_pkg/maps
rosrun map_server map_server simple_house_map.yaml
```

重新發布：

```text
/map
```

確認 `/map` 的 Publisher 已經從 `/cartographer_occupancy_grid_node` 變成 `/map_server_...`：

```bash
rostopic info /map
```

`map_server` 只發布地圖，不發布 `map -> odom`。

Cartographer 關掉後，RViz 的 `RobotModel`、`LaserScan`、`TF` 會找不到 `map` 到車子的 TF，這要到定位章節才會補上。

這時候就不再是「正在建圖」，而是：

```text
已有地圖
↓
載入地圖
↓
定位 / Navigation 使用
```

這會銜接後面的導航章節。

---

## 15. 本章完整資料流

本章做完後，完整資料流應該可以理解成：

```text
Gazebo 小房間
↓
LiDAR Plugin
↓
/scan

Gazebo Diff Drive
↓
/odom
↓
odom → base_link

/odom
↓
odom_timestamp_filter_node
↓
/odom_filtered

URDF + robot_state_publisher
↓
base_link → laser_link

/scan + /odom_filtered + TF
↓
Cartographer
↓
Submap
↓
Scan Matching
↓
Pose Graph
↓
Loop Closure
↓
OccupancyGrid
↓
/map
↓
map_saver
↓
.pgm + .yaml
```

---

## 16. 本章完成條件

以下全部完成，才算第 14 章完成：

- [ ] Gazebo 小房間正常啟動。
- [ ] 車子可以使用 `/cmd_vel` 移動。
- [ ] `/scan` 正常。
- [ ] `/odom` 正常。
- [ ] `odom -> base_link` TF 正常。
- [ ] `base_link -> laser_link` TF 正常。
- [ ] Cartographer 安裝完成。
- [ ] `cartographer_2d.lua` 建立完成。
- [ ] `cartographer_2d.launch` 建立完成。
- [ ] Cartographer 可以正常接收 `/scan`。
- [ ] Cartographer 可以使用 `/odom`。
- [ ] RViz 可以看到建圖結果。
- [ ] `/map` 為 `nav_msgs/OccupancyGrid`。
- [ ] `map_server` 安裝完成。
- [ ] `map_saver` 可以保存地圖。
- [ ] 產生 `.pgm` 與 `.yaml`。
- [ ] 保存後的地圖可以重新載入。

---

## 17. 本章定位

第 10～14 章的關係：

```text
10 OccupancyGrid
→ 地圖資料長什麼樣

11 TF
→ 不同座標系怎麼連接

12 Odom
→ 機器人怎麼估算移動

13 SLAM
→ 同時定位與建圖的原理

14 Cartographer
→ 把以上全部真正串起來
```

第 14 章完成後，目前這個 ROS Noetic 學習專案就會第一次具備完整的：

```text
模擬環境
+
機器人
+
感測器
+
TF
+
Odom
+
SLAM
+
地圖保存
```

這也會成為後續 Navigation、Costmap、Planner 的基礎。
