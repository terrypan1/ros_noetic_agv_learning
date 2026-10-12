# 08_LiDAR_LaserScan資料取得與基礎避障

## 1. 本章定位

本章延續目前已完成的：

```text
06_Gazebo模擬與cmd_vel速度控制
07_RViz從零操作與ROS資料視覺化
```

這一章不先進 Navigation、SLAM，也不要求完整 TF 架構。

目前目標只有三件事：

```text
1. 看懂 sensor_msgs/LaserScan 消息包格式
2. 從 /scan 取得雷射雷達資料
3. 根據雷達距離資料做最基礎的避障
```

而且本章直接沿用目前已經建立好的 Gazebo 小車，不另外建立新的 package。

目前套件：

```text
ros_learning_pkg/
├── CMakeLists.txt
├── README.md
├── package.xml
├── include/
│   └── ros_learning_pkg/
├── launch/
│   ├── robot_speed.launch
│   └── gazebo_robot.launch
├── src/
│   ├── cmd_vel_publisher_node.cpp
│   ├── learning_node.cpp
│   ├── speed_publisher_node.cpp
│   └── speed_subscriber_node.cpp
├── urdf/
│   └── simple_robot.urdf
└── worlds/
    └── simple_robot.world
```

WSL 實際 Workspace：

```text
~/ros_learning_ws/src/ros_learning_pkg
```

---

# 2. 本章最後要完成的資料流

本章最重要的是把這條資料流真正跑通：

```text
Gazebo LiDAR
↓
/scan
↓
sensor_msgs/LaserScan
↓
LiDAR Subscriber
↓
LidarCallback()
↓
讀取 ranges[]
↓
取得前方障礙物距離
↓
判斷是否太近
↓
geometry_msgs/Twist
↓
/cmd_vel
↓
Gazebo 小車前進 / 轉向 / 停止
```

先學會「雷達資料怎麼來」，最後才做避障。

---

# 3. LiDAR 是什麼

LiDAR：

```text
Light Detection and Ranging
```

中文可理解為：

```text
光達 / 雷射雷達
```

2D LiDAR 最直覺的概念：

```text
雷射往不同角度掃描
↓
量每個方向碰到物體的距離
↓
得到一圈距離資料
```

在 ROS 裡，這些掃描資料通常會透過：

```text
/scan
```

發布出去。

---

# 4. ROS 裡的 LaserScan 消息包

LiDAR 常用的 Message：

```text
sensor_msgs/LaserScan
```

可以在 ROS Noetic 裡直接查看：

```bash
rosmsg show sensor_msgs/LaserScan
```

主要欄位：

```text
std_msgs/Header header

float32 angle_min
float32 angle_max
float32 angle_increment
float32 time_increment
float32 scan_time
float32 range_min
float32 range_max
float32[] ranges
float32[] intensities
```

---

# 5. LaserScan 欄位理解

## 5.1 header

```text
Header header
```

主要包含：

```text
時間戳記
Frame 名稱
```

之後進 TF 時會再深入。

本章先知道 LiDAR 資料本身會帶著它所屬的座標 Frame 即可。

---

## 5.2 angle_min

```text
angle_min
```

代表：

```text
這次掃描開始的角度
```

單位是：

```text
rad（弧度）
```

---

## 5.3 angle_max

```text
angle_max
```

代表：

```text
這次掃描結束的角度
```

---

## 5.4 angle_increment

```text
angle_increment
```

代表：

```text
相鄰兩筆雷達距離資料之間差多少角度
```

例如：

```text
angle_increment = 1°
```

就可以理解成每隔 1 度有一筆距離資料。

實際 ROS 裡使用的是弧度。

---

## 5.5 range_min

```text
range_min
```

代表 LiDAR 能有效量到的最近距離。

例如：

```text
range_min = 0.12
```

代表小於約 0.12 m 的值可能不屬於有效量測範圍。

---

## 5.6 range_max

```text
range_max
```

代表 LiDAR 能有效量到的最遠距離。

例如：

```text
range_max = 10.0
```

代表有效範圍最大約 10 m。

---

## 5.7 ranges[]

這是本章最重要的欄位：

```text
float32[] ranges
```

它就是：

```text
每一個掃描角度所量到的距離
```

例如概念上：

```text
ranges[0]
ranges[1]
ranges[2]
...
ranges[n]
```

每一筆都對應一個角度。

所以可以理解成：

```text
角度
↓
對應 ranges[index]
↓
該方向距離
```

---

## 5.8 intensities[]

```text
float32[] intensities
```

代表雷射回波強度。

不是所有 LiDAR 都一定會提供有意義的 intensity 資料。

本章避障先不使用。

---

# 6. ranges[index] 為什麼可以代表某個方向

最簡單的寫法直接使用：

```cpp
msg.ranges[180]
```

來取得前方距離。

但這只是建立在該範例雷達資料排列方式上的簡化寫法。

不能死背：

```text
ranges[180] = 前方
```

真正的概念是：

```text
angle_min
+
index × angle_increment
=
這個 index 對應的角度
```

所以之後面對不同 LiDAR、不同模擬器或不同掃描範圍時，都應該先確認：

```text
angle_min
angle_max
angle_increment
ranges.size()
```

再判斷哪個 index 是正前方。

進一步的寫法：

```cpp
int nNum = msg.ranges.size();
int nMid = nNum / 2;
float fMidDist = msg.ranges[nMid];
```

比直接寫死 `180` 更具有通用性。

但也仍然要先確認「陣列中間」是否真的就是該 LiDAR 定義的正前方。

---

# 7. 本章第一個實作：讓 Gazebo 小車產生 /scan

目前 `simple_robot` 已經可以在 Gazebo 裡移動。

本章要在現有模型上增加：

```text
laser_link
+
Gazebo LiDAR Sensor
+
/scan Topic
```

預期資料流：

```text
simple_robot.urdf
↓
加入 laser_link
↓
加入 Gazebo ray / laser sensor
↓
Gazebo 模擬雷射掃描
↓
發布 /scan
```

這一階段不需要先完成整套 TF 教學。

只要先讓 Gazebo 小車真正產生 `/scan`，就足夠進入本章練習。

---

# 8. 驗證 /scan 是否存在

啟動 Gazebo：

```bash
roslaunch ros_learning_pkg gazebo_robot.launch
```

確認 Topic：

```bash
rostopic list
```

預期可以看到：

```text
/scan
```

再看資訊：

```bash
rostopic info /scan
```

應該可以看到 Message Type 類似：

```text
sensor_msgs/LaserScan
```

再直接看資料：

```bash
rostopic echo /scan
```

這時要先觀察：

```text
angle_min
angle_max
angle_increment
range_min
range_max
ranges
```

---

# 9. 本章第二個實作：C++ 訂閱 /scan

本章不建立新的 `lidar_pkg`。

直接在現有：

```text
ros_learning_pkg
```

新增：

```text
src/lidar_scan_subscriber_node.cpp
```

目的只有：

```text
訂閱 /scan
↓
取得 LaserScan
↓
印出雷達資料
```

---

# 10. lidar_scan_subscriber_node.cpp 核心概念

需要：

```cpp
#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
```

Callback 概念：

```cpp
void lidarCallback(const sensor_msgs::LaserScan::ConstPtr& msg)
{
    // 在這裡讀取 LaserScan 資料
}
```

訂閱：

```cpp
ros::Subscriber lidar_sub =
    nh.subscribe("/scan", 10, lidarCallback);
```

最後：

```cpp
ros::spin();
```

整體概念：

```text
/scan 有新資料
↓
ROS 呼叫 lidarCallback()
↓
msg 裡就是最新 LaserScan
```

---

# 11. 先印出 LaserScan 基本資訊

第一階段不要急著避障。

先確認真的讀得到：

```text
ranges.size()
angle_min
angle_max
angle_increment
range_min
range_max
```

例如概念：

```cpp
ROS_INFO("ranges size = %zu", msg->ranges.size());
ROS_INFO("angle_min = %f", msg->angle_min);
ROS_INFO("angle_max = %f", msg->angle_max);
ROS_INFO("angle_increment = %f", msg->angle_increment);
```

這一段重點不是漂亮，而是先確認：

```text
Subscriber 真的收到 /scan
```

---

# 12. 取得正前方距離

核心做法：

```cpp
int nNum = msg->ranges.size();
int nMid = nNum / 2;
float fMidDist = msg->ranges[nMid];
```

再印出：

```cpp
ROS_INFO("前方測距 ranges[%d] = %f 米", nMid, fMidDist);
```

這一步完成後，要能理解：

```text
LiDAR
↓
/scan
↓
LaserScan
↓
ranges[]
↓
某個 index
↓
某方向的障礙物距離
```

---

# 13. 不要只看單一點

為了簡單，入門範例可能只抓：

```text
ranges[mid]
```

但真實避障如果只看一條雷射線，容易漏掉障礙物。

所以本章可以在理解基本原理後，再稍微改成：

```text
看正前方一小段角度範圍
↓
找其中最近距離
```

例如概念：

```text
前方 -15° ~ +15°
↓
全部掃描
↓
取最小距離
```

這樣會比只讀一個 index 更合理。

但本章先把單點版本跑通，再做這個改善。

---

# 14. 注意無效雷達距離

LaserScan 裡可能出現：

```text
inf
NaN
小於 range_min
大於 range_max
```

所以不能假設每一筆 `ranges[]` 都一定是正常數字。

實作避障前要先知道：

```text
讀到距離
≠
一定是有效距離
```

之後做距離判斷時，要先過濾無效值。

---

# 15. 本章第三個實作：基礎避障

新增：

```text
src/lidar_avoidance_node.cpp
```

功能：

```text
訂閱 /scan
+
發布 /cmd_vel
```

所以這個 Node 同時是：

```text
Subscriber
+
Publisher
```

---

# 16. 避障 Node 的資料流

```text
Gazebo LiDAR
↓
/scan
↓
lidar_avoidance_node
↓
LidarCallback()
↓
讀取前方距離
↓
判斷距離
├── 安全 → 往前
└── 太近 → 轉向
↓
geometry_msgs/Twist
↓
/cmd_vel
↓
Gazebo diff drive plugin
↓
小車移動
```

這就是最基礎的 Reactive Obstacle Avoidance。

---

# 17. /cmd_vel Message

前面已經學過：

```text
geometry_msgs/Twist
```

常用：

```text
linear.x
angular.z
```

例如：

```cpp
geometry_msgs::Twist vel_cmd;
```

往前：

```cpp
vel_cmd.linear.x = 0.10;
vel_cmd.angular.z = 0.0;
```

轉向：

```cpp
vel_cmd.linear.x = 0.0;
vel_cmd.angular.z = 0.30;
```

---

# 18. 最簡單的避障邏輯

概念先保持非常單純：

```text
如果前方距離 < 安全距離
→ 不再往前
→ 開始轉向

否則
→ 繼續往前
```

例如：

```text
安全距離 = 1.0 m
```

邏輯：

```cpp
if (front_distance < 1.0)
{
    vel_cmd.linear.x = 0.0;
    vel_cmd.angular.z = 0.3;
}
else
{
    vel_cmd.linear.x = 0.1;
    vel_cmd.angular.z = 0.0;
}
```

最後：

```cpp
vel_pub.publish(vel_cmd);
```

---

# 19. 基礎避障概念

基本概念是：

```cpp
geometry_msgs::Twist vel_cmd;

if (fMidDist < 1.5f)
{
    vel_cmd.angular.z = 0.3;
}
else
{
    vel_cmd.linear.x = 0.05;
}
```

也就是：

```text
前方太近
→ 轉彎

前方沒有太近
→ 前進
```

這不是 Navigation Stack 的完整避障。

它只是最基本的：

```text
Sensor
→ 判斷
→ 控制
```

但這一段非常重要，因為可以把 ROS 的資料流真正串起來。

---

# 20. 為什麼這已經算避障

LiDAR 本身不會讓車避障。

LiDAR 只負責：

```text
量距離
```

真正的避障是：

```text
雷達資料
↓
程式判斷
↓
產生新的速度命令
↓
小車改變運動
```

所以：

```text
LiDAR ≠ 避障
```

正確理解是：

```text
LiDAR
+
避障演算法
+
/cmd_vel 控制
=
基礎避障
```

---

# 21. 本章與 Navigation 避障的差別

本章只做：

```text
看到前方障礙物
↓
轉彎
```

它不知道：

```text
目的地在哪裡
全域路徑在哪裡
哪條路最合理
地圖長什麼樣子
```

所以這不是完整 Navigation。

Navigation 之後會再加入：

```text
Map
Localization
Global Planner
Local Planner
Costmap
```

本章現在不用碰。

---

# 22. Gazebo 測試場景

目前：

```text
worlds/simple_robot.world
```

已經有固定障礙物：

```text
box_obstacle
```

這剛好可以直接拿來測本章 LiDAR 與避障。

測試方式：

```text
小車前方有 Box
↓
LiDAR 掃描到 Box
↓
/scan 的前方距離變小
↓
Callback 收到數值
↓
距離低於門檻
↓
發布轉向 /cmd_vel
↓
小車不再直接撞上 Box
```

這就是本章最重要的驗證。

---

# 23. 建議新增的檔案

本章完成後，`ros_learning_pkg` 可以增加：

```text
ros_learning_pkg/
├── launch/
│   ├── robot_speed.launch
│   ├── gazebo_robot.launch
│   └── lidar_avoidance.launch
├── src/
│   ├── cmd_vel_publisher_node.cpp
│   ├── learning_node.cpp
│   ├── speed_publisher_node.cpp
│   ├── speed_subscriber_node.cpp
│   ├── lidar_scan_subscriber_node.cpp
│   └── lidar_avoidance_node.cpp
├── urdf/
│   └── simple_robot.urdf
└── worlds/
    └── simple_robot.world
```

不建立新的 `lidar_pkg`。

---

# 24. CMakeLists.txt 要補的觀念

新增 C++ Node 後，要記得加入可執行檔。

概念：

```cmake
add_executable(lidar_scan_subscriber_node
  src/lidar_scan_subscriber_node.cpp
)

target_link_libraries(lidar_scan_subscriber_node
  ${catkin_LIBRARIES}
)

add_executable(lidar_avoidance_node
  src/lidar_avoidance_node.cpp
)

target_link_libraries(lidar_avoidance_node
  ${catkin_LIBRARIES}
)
```

如果 `sensor_msgs` 尚未加入 package 相依，也要補進：

```text
CMakeLists.txt
package.xml
```

實際修改時以目前現有內容為準，不整份亂改。

---

# 25. 編譯

回到 Workspace：

```bash
cd ~/ros_learning_ws
catkin_make
```

再：

```bash
source devel/setup.bash
```

---

# 26. 第一階段測試：只看雷達

Terminal 1：

```bash
roscore
```

Terminal 2：

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
roslaunch ros_learning_pkg gazebo_robot.launch
```

Terminal 3：

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
rosrun ros_learning_pkg lidar_scan_subscriber_node
```

要確認：

```text
前方障礙物距離會隨小車 / 障礙物相對位置改變
```

---

# 27. 第二階段測試：啟動基礎避障

確認沒有其他 Node 同時在發布 `/cmd_vel`。

例如不要同時執行：

```text
cmd_vel_publisher_node
rqt_robot_steering
lidar_avoidance_node
```

三個一起發布速度命令。

因為它們會互相搶控制權。

測試時保留：

```text
lidar_avoidance_node
```

作為 `/cmd_vel` 的主要 Publisher。

可以確認：

```bash
rostopic info /cmd_vel
```

---

# 28. RViz 可以拿來看什麼

如果目前 TF 條件允許，可以在 RViz 加入：

```text
LaserScan
```

Topic 指定：

```text
/scan
```

就可以看到雷射點。

如果因為 TF / Fixed Frame 尚未完整，RViz 暫時顯示不出來，也不影響本章核心學習。

本章真正必要的是：

```text
/scan 有資料
Subscriber 收得到
ranges[] 讀得到
避障邏輯會控制 /cmd_vel
```

TF 會在後面的專門章節再補完整。

---

# 29. 本章不要做的事

目前先不要把範圍擴大到：

```text
Navigation Stack
move_base
AMCL
SLAM
Cartographer
完整 Costmap
完整 TF 教學
多 Sensor Fusion
```

08 只專心完成：

```text
LaserScan
+
/scan
+
Subscriber
+
ranges[]
+
基礎避障
```

---

# 30. 本章完成條件

完成 08 時，要能自己說清楚：

```text
sensor_msgs/LaserScan 是什麼
```

知道：

```text
/scan 裡面裝的是 LaserScan
```

知道：

```text
ranges[] 是每個掃描方向的距離資料
```

知道：

```text
angle_min
angle_max
angle_increment
```

如何跟 `ranges[]` 對應。

可以寫出：

```text
Subscriber → /scan → Callback
```

可以從 Callback 取得前方障礙物距離。

最後可以完成：

```text
/scan
↓
距離判斷
↓
Twist
↓
/cmd_vel
↓
Gazebo 機器人避開 Box
```

---

# 31. 本章最重要的一句話

```text
LiDAR 負責「看見距離」；
避障程式負責「根據距離決定怎麼走」。
```

本章就是把這兩件事情真正接起來。
