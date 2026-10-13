# ROS Noetic 機器人學習與實作

本專案記錄 ROS Noetic 從基礎通訊、Gazebo 模擬、RViz、LiDAR、IMU，
一路延伸到 TF、Odometry、SLAM 與 Navigation 的學習與實作過程。

重點不是單純執行既有套件，而是透過自己建立 Node、Topic、Message、
Gazebo 小車與 Sensor，理解移動機器人系統中的實際資料流與控制流程。

目前已完成：

- 自己撰寫的 ROS C++ Node（Publisher / Subscriber、`/cmd_vel` 控制、LiDAR 避障、IMU 姿態解析、自訂 `/map` 發布、Odom timestamp 過濾）
- 自己用 URDF 建立的 Gazebo 差速小車 `simple_robot`
- 自己在小車上加入 LiDAR（`/scan`）與 IMU（`/imu/data`）Sensor
- 自己建立的 Gazebo 多房間小屋場景 `simple_house.world`
- 用 Cartographer 2D SLAM 在小屋裡開車建圖，並用 `map_saver` 存成 `.pgm` + `.yaml`

後續會延伸到 Localization 與 Navigation。

## 畫面預覽

| Gazebo：自己建立的 simple_robot | RViz：RobotModel 與 TF |
|---|---|
| ![Gazebo simple_robot](docs/images/gazebo_simple_robot.jpg) | ![RViz RobotModel](docs/images/rviz_robot_model.jpg) |

| Gazebo：自己建立的小屋場景 simple_house | RViz：Cartographer 建出來的地圖 |
|---|---|
| ![Gazebo simple_house](docs/images/gazebo_simple_house.jpg) | ![Cartographer 建圖結果](docs/images/cartographer_house_map.jpg) |

右圖是 RViz 俯視，紅點是當下的 LaserScan。

---

## 目前學習內容

- [x] ROS Noetic 環境與基本工具
- [x] Workspace / Package
- [x] C++ Node
- [x] Publisher / Subscriber / Topic
- [x] Launch
- [x] Gazebo 小車模擬
- [x] /cmd_vel 速度控制
- [x] RViz 資料視覺化
- [x] LiDAR / LaserScan
- [x] 基礎避障
- [x] IMU / sensor_msgs/Imu
- [x] Quaternion → Roll / Pitch / Yaw
- [x] OccupancyGrid 柵格地圖 / 自訂 /map 發布
- [x] TF / Frame
- [x] Odometry
- [x] SLAM 原理
- [x] Cartographer 2D SLAM 建圖
- [x] map_server / map_saver 地圖保存與載入
- [ ] Localization
- [ ] Navigation

---

## 核心資料流

### 小車控制

```text
控制 Node / rqt_robot_steering
↓
/cmd_vel
↓
Gazebo Diff Drive Plugin
↓
左右輪
↓
機器人移動
```

![cmd_vel 速度控制架構](docs/images/cmd_vel_control.jpg)

`/cmd_vel` 使用 `geometry_msgs/Twist`，平面機器人主要使用 `linear.x`（前進速度）與 `angular.z`（轉向角速度）：

![Twist 訊息結構](docs/images/twist_message.jpg)

### LiDAR

```text
Gazebo LiDAR
↓
/scan
↓
sensor_msgs/LaserScan
↓
Subscriber
↓
ranges[]
↓
障礙物距離判斷
```

<img src="docs/images/gazebo_lidar_scan.jpg" alt="Gazebo LiDAR 360° 掃描" width="480">

Gazebo 中的 360° LiDAR 掃描（藍色雷射線），前方方塊為測試用障礙物。

### IMU

```text
Gazebo IMU
↓
/imu/data
↓
sensor_msgs/Imu
↓
Quaternion
↓
Roll / Pitch / Yaw
```

### 地圖

```text
nav_msgs/OccupancyGrid
↓
/map
↓
RViz Map
```

![OccupancyGrid 訊息結構](docs/images/occupancy_grid_message.jpg)

### TF 與 Odometry

```text
map
↓  Cartographer
odom
↓  Gazebo Diff Drive Plugin（同時發布 /odom）
base_link
↓  URDF + robot_state_publisher
laser_link
```

每一段 TF 只能有一個發布者。`odom -> base_link` 已經由 Gazebo 發布，所以 Cartographer 只發布 `map -> odom`。

![TF Tree 與各 Frame 說明](docs/images/tf_tree_frames.jpg)

上圖是通用的 TF Tree 說明，`simple_robot` 沒有 `base_footprint` 這一層。

![Odom 里程計原理](docs/images/odom_principle.jpg)

### Cartographer 2D SLAM

```text
Gazebo Diff Drive Plugin
↓
/odom
↓
odom_timestamp_filter_node
↓
/odom_filtered

/scan + /odom_filtered + TF
↓
Cartographer
↓
/map
↓
map_saver
↓
simple_house_map.pgm + simple_house_map.yaml
```

| 建圖過程：RViz 中逐步長出來的地圖 | 建圖結果：map_saver 存出來的 simple_house_map.pgm |
|---|---|
| ![Cartographer 建圖過程](docs/images/cartographer_mapping_progress.jpg) | ![map_saver 保存的地圖](docs/images/simple_house_map.png) |

左圖邊緣呈放射狀的區域，是雷射掃過但車子還沒走近的地方。右圖白色是空地、黑色是障礙物、灰色是未知。

Gazebo 差速驅動 Plugin 偶爾會發出兩筆 timestamp 相同的 `/odom`，Cartographer 要求時間嚴格遞增，遇到重複就會當掉。

`odom_timestamp_filter_node` 把 timestamp 重複或倒退的 `/odom` 丟掉，再交給 Cartographer。

---

## 專案結構

```text
ros_noetic_robot_learning/
├── ros_learning_pkg/
│   ├── config/      # Cartographer 設定檔
│   ├── launch/      # 啟動 Gazebo、Cartographer 與 ROS Node
│   ├── maps/        # map_saver 保存的地圖
│   ├── src/         # C++ Node
│   ├── urdf/        # simple_robot 與 Sensor 定義
│   ├── worlds/      # Gazebo 測試場景
│   ├── CMakeLists.txt
│   └── package.xml
│
└── docs/
    └── ROS_Noetic_核心學習主線/   # 各章節學習筆記：概念、指令、資料流與實作紀錄
```

---

## 執行環境

```text
Windows + WSL2
Ubuntu 20.04
ROS Noetic
Gazebo Classic 11
RViz
C++
Catkin
Cartographer（source build，獨立 workspace）
map_server
```

Workspace：

```text
~/ros_learning_ws
```

將 `ros_learning_pkg` 放到 `~/ros_learning_ws/src/` 底下即可編譯。

Cartographer 的安裝方式見第 14 章筆記。

---

## 基本執行方式

### 編譯

```bash
cd ~/ros_learning_ws
catkin_make
source devel/setup.bash
```

### 啟動 Gazebo

```bash
roslaunch ros_learning_pkg gazebo_robot.launch
```

### 手動控制

```bash
rosrun rqt_robot_steering rqt_robot_steering
```

### 查看 LiDAR

```bash
rostopic echo /scan
```

### 查看 IMU

```bash
rostopic echo /imu/data
```

### IMU 姿態解析 Node

```bash
rosrun ros_learning_pkg imu_subscriber_node
```

### Cartographer 建圖

各開一個 Terminal，依序啟動：

```bash
roslaunch ros_learning_pkg gazebo_house.launch
rosrun ros_learning_pkg odom_timestamp_filter_node
roslaunch ros_learning_pkg cartographer_2d.launch
rviz
rosrun rqt_robot_steering rqt_robot_steering
```

RViz 的 Fixed Frame 設成 `map`，加入 `Map`（`/map`）、`LaserScan`（`/scan`）、`RobotModel`、`TF`。

開車建議線速度 0.2～0.3 m/s、轉向 0.3～0.5 rad/s。

### 保存地圖

Cartographer 還在執行時：

```bash
cd ~/ros_learning_ws/src/ros_learning_pkg/maps
rosrun map_server map_saver -f simple_house_map
```

### 載入保存的地圖

```bash
cd ~/ros_learning_ws/src/ros_learning_pkg/maps
rosrun map_server map_server simple_house_map.yaml
```

---

## 學習筆記

`docs/ROS_Noetic_核心學習主線/` 依章節記錄每個主題，逐步整理成：

```text
概念
↓
ROS Message / Topic
↓
程式實作
↓
Gazebo / RViz 驗證
↓
實戰對照
↓
學習結論
```

| 章節 | 主題 |
|---|---|
| 01 | ROS Noetic 安裝、環境、Master 與基本工具 |
| 02 | Workspace、Package 與套件來源 |
| 03 | C++ Node 從零建立 |
| 04 | Publisher / Subscriber 與 Topic |
| 05 | Launch 與多 Node 啟動 |
| 06 | Gazebo 模擬與 cmd_vel 速度控制 |
| 07 | RViz 操作與 ROS 資料視覺化 |
| 08 | LiDAR LaserScan 資料取得與基礎避障 |
| 09 | IMU 資料取得與 Quaternion 姿態解析 |
| 10 | OccupancyGrid 柵格地圖訊息結構與自訂地圖發布 |
| 11 | TF Frame 座標系與座標轉換 |
| 12 | Odometry 里程計訊息結構與 odom 座標系 |
| 13 | SLAM 原理：建圖、定位與資料流概念 |
| 14 | Cartographer 2D SLAM：TF、Odom 與 map_server 整合 |
