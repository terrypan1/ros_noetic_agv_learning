# ros_learning_pkg 快速執行指南

這個 Package 目前包含：

- `speed_publisher_node`：發布機器人速度到 `/robot_speed`
- `speed_subscriber_node`：訂閱 `/robot_speed`
- `robot_speed.launch`：一次啟動 Publisher 與 Subscriber

## 目錄結構

```text
ros_learning_pkg/
├── CMakeLists.txt                      # 編譯設定，建立 8 個 Node 執行檔
├── README.md                           # 本說明文件
├── package.xml                         # Package 資訊與相依套件
├── include/
│   └── ros_learning_pkg/               # 標頭檔資料夾（目前是空的）
├── launch/
│   ├── robot_speed.launch              # 一次啟動速度 Publisher 與 Subscriber
│   ├── gazebo_house.launch             # 開 Gazebo、載入自製小屋場景並放入機器人模型
│   ├── gazebo_robot.launch             # 開 Gazebo、載入場景並放入機器人模型
│   └── lidar_avoidance.launch          # Gazebo + LiDAR 基礎避障 Node 一次啟動
├── src/
│   ├── cmd_vel_publisher_node.cpp      # 以 10 Hz 發布 /cmd_vel（前進 0.10 m/s，不轉彎）
│   ├── imu_subscriber_node.cpp         # 訂閱 /imu/data，Quaternion 轉 Roll / Pitch / Yaw
│   ├── learning_node.cpp               # 最基本的 Node 範例，啟動後待命
│   ├── lidar_avoidance_node.cpp        # 訂閱 /scan，車道內 < 1.0 m 轉向、> 1.5 m 才恢復前進（發布 /cmd_vel）
│   ├── lidar_scan_subscriber_node.cpp  # 訂閱 /scan，印出 LaserScan 欄位與正前方距離
│   ├── map_publisher_node.cpp          # 以 1 Hz 發布 /map（nav_msgs/OccupancyGrid，4×2 自訂地圖）
│   ├── speed_publisher_node.cpp        # 發布機器人速度到 /robot_speed
│   └── speed_subscriber_node.cpp       # 訂閱 /robot_speed 並顯示收到的速度
├── urdf/
│   └── simple_robot.urdf               # 機器人模型：車體 + 左右兩輪 + LiDAR（發布 /scan），差速驅動 Plugin 接收 /cmd_vel
└── worlds/
    ├── simple_house.world              # Gazebo 場景：自製 8 m × 6 m 多房間小屋（牆與家具都是靜態方塊）
    └── simple_robot.world              # Gazebo 場景：地面、光源、前方 2 m 一個方塊障礙物
```

## 最快啟動方式

開啟 WSL Terminal，執行：

```bash
cd ~/ros_learning_ws
source /opt/ros/noetic/setup.bash
source devel/setup.bash
roslaunch ros_learning_pkg robot_speed.launch
```

執行後，同一個 Terminal 會持續顯示 Publisher 發布和 Subscriber 收到的機器人速度。

停止程式：

```text
Ctrl+C
```

`roslaunch` 會在需要時自動啟動 ROS Master，因此不用另外先執行 `roscore`。

## 修改程式後重新編譯

如果修改過 `.cpp` 或 `CMakeLists.txt`，先執行：

```bash
cd ~/ros_learning_ws
source /opt/ros/noetic/setup.bash
catkin_make
source devel/setup.bash
roslaunch ros_learning_pkg robot_speed.launch
```

只修改 `.launch` 檔通常不需要重新執行 `catkin_make`。

## 檢查 Node 與 Topic

保持 `roslaunch` 執行，另外開一個 WSL Terminal：

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
```

查看目前 Node：

```bash
rosnode list
```

預期至少看到：

```text
/rosout
/speed_publisher
/speed_subscriber
```

查看 Topic 的 Publisher 與 Subscriber：

```bash
rostopic info /robot_speed
```

查看 Topic 傳送的即時資料：

```bash
rostopic echo /robot_speed
```

查看通訊關係圖：

```bash
rqt_graph
```

## 分開啟動兩個 Node（第 04 章方式）

Terminal 1：

```bash
source /opt/ros/noetic/setup.bash
roscore
```

Terminal 2：

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
rosrun ros_learning_pkg speed_publisher_node
```

Terminal 3：

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
rosrun ros_learning_pkg speed_subscriber_node
```

一般練習建議直接使用 `roslaunch`，一次啟動兩個 Node。
