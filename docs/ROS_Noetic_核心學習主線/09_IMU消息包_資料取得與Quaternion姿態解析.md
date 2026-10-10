# 09_IMU消息包_資料取得與Quaternion姿態解析

## 1. 本章定位

本章延續 `08_LiDAR_LaserScan資料取得與基礎避障`，先不進 EKF、robot_localization、完整 Sensor Fusion，也不要求完整 TF 架構。

本章目標：

```text
1. 看懂 sensor_msgs/Imu 消息格式
2. 理解 /imu/data 的資料流
3. 從 /imu/data 取得 IMU 資料
4. 讀取 orientation / angular_velocity / linear_acceleration
5. 理解 covariance 的基本用途
6. 將 Quaternion 轉成 Roll / Pitch / Yaw
7. 用目前 Gazebo 的 simple_agv 做實際驗證
```

沿用既有套件：

```text
~/ros_learning_ws/src/ros_learning_pkg
```

不另外建立 `imu_pkg`。

---

## 2. IMU 是什麼

IMU = Inertial Measurement Unit = 慣性測量單元。

常見資料：

```text
orientation          → 姿態
angular_velocity     → 角速度
linear_acceleration  → 線加速度
```

`orientation` 常以 Quaternion 四元數表示。

---

## 3. sensor_msgs/Imu 消息格式

```bash
rosmsg show sensor_msgs/Imu
```

核心結構：

```text
std_msgs/Header header
geometry_msgs/Quaternion orientation
float64[9] orientation_covariance
geometry_msgs/Vector3 angular_velocity
float64[9] angular_velocity_covariance
geometry_msgs/Vector3 linear_acceleration
float64[9] linear_acceleration_covariance
```

可整理成：

```text
sensor_msgs/Imu
├── header
├── orientation
├── orientation_covariance
├── angular_velocity
├── angular_velocity_covariance
├── linear_acceleration
└── linear_acceleration_covariance
```

---

## 4. orientation

```text
geometry_msgs/Quaternion
```

欄位：

```text
x
y
z
w
```

注意：`orientation` 不是直接存 Roll / Pitch / Yaw，而是用 Quaternion 表示姿態。

---

## 5. angular_velocity

```text
geometry_msgs/Vector3
```

```text
angular_velocity.x
angular_velocity.y
angular_velocity.z
```

單位通常是 `rad/s`。

對平面 AGV 而言，最常注意：

```text
angular_velocity.z
```

因為車子左右轉向主要是繞 Z 軸旋轉。

---

## 6. linear_acceleration

```text
geometry_msgs/Vector3
```

```text
linear_acceleration.x
linear_acceleration.y
linear_acceleration.z
```

單位通常是 `m/s^2`。

可以先理解成：

```text
x → 前後方向加速度
y → 左右方向加速度
z → 上下方向加速度
```

---

## 7. Covariance 基本理解

IMU 內有三組 covariance：

```text
orientation_covariance
angular_velocity_covariance
linear_acceleration_covariance
```

每一組都是 `float64[9]`，也就是 3×3 矩陣。

先理解成：

```text
量測數值
→ 告訴你量到了多少

Covariance
→ 描述量測的不確定程度，以及各軸誤差間的關聯
```

若某項資料不存在，ROS IMU Message 常以 covariance 第一個元素為 `-1` 表示。

例如：

```cpp
if (msg.orientation_covariance[0] < 0)
{
    return;
}
```

代表沒有有效 orientation，就不要繼續處理它。

---

## 8. 常見 IMU Topics

```text
/imu/data_raw
/imu/data
/imu/mag
```

### /imu/data_raw
常見為原始或較基礎的 IMU 輸出，主要包含角速度與線加速度。

### /imu/data
常見以 `sensor_msgs/Imu` 提供姿態、角速度與線加速度；是否真的有有效 orientation 依 Driver 而定。

### /imu/mag
常見為磁力計資料，型別通常是 `sensor_msgs/MagneticField`。

實際系統一律先查：

```bash
rostopic list
rostopic info /imu/data
```

---

## 9. /imu/data 資料流

```text
IMU Sensor / Gazebo IMU
↓
IMU Driver / Gazebo Plugin
↓
sensor_msgs/Imu
↓
/imu/data
↓
imu_subscriber_node
↓
IMUCallback()
↓
讀取 IMU 資料
```

這和 08 LiDAR 的資料流概念相同，只是 `/scan` 換成 `/imu/data`。

---

## 10. 本章沿用 ros_learning_pkg

常見做法是另建 `imu_pkg`，本學習專案不另外拆 Package。

新增：

```text
src/imu_subscriber_node.cpp
```

修改：

```text
urdf/simple_agv.urdf
CMakeLists.txt
package.xml
README.md
```

---

## 11. Gazebo simple_agv 加入 IMU

目標：

```text
simple_agv
↓
Gazebo IMU Sensor
↓
/imu/data
```

在 `simple_agv.urdf` 加入：

```text
imu_link
imu_joint
Gazebo IMU Sensor
Gazebo ROS IMU Plugin
```

概念：

```text
base_link
↓ fixed joint
imu_link
↓
Gazebo IMU Sensor
↓
/imu/data
```

本章先專注 IMU Message 與資料取得，不要求先把完整 TF 教學補完。

---

## 12. 驗證 /imu/data

啟動 Gazebo 後：

```bash
rostopic list
rostopic info /imu/data
rostopic echo /imu/data
```

確認 Type：

```text
sensor_msgs/Imu
```

並能看到：

```text
orientation
angular_velocity
linear_acceleration
```

先證明：

```text
Gazebo IMU → /imu/data
```

已經打通。

---

## 13. 建立 imu_subscriber_node.cpp

基本 Include：

```cpp
#include <ros/ros.h>
#include <sensor_msgs/Imu.h>
#include <tf/tf.h>
```

Node 責任：

```text
訂閱 /imu/data
↓
進入 IMUCallback
↓
讀取 sensor_msgs/Imu
↓
顯示需要的數值
```

---

## 14. 訂閱 /imu/data

```cpp
ros::Subscriber imu_sub =
    n.subscribe("/imu/data", 10, IMUCallback);
```

```text
Node
↓
訂閱 /imu/data
↓
收到新資料
↓
呼叫 IMUCallback
```

最後：

```cpp
ros::spin();
```

持續等待 Callback。

---

## 15. IMUCallback

```cpp
void IMUCallback(const sensor_msgs::Imu& msg)
{
    // 讀取 IMU 資料
}
```

本章先關注：

```text
orientation
angular_velocity
linear_acceleration
```

---

## 16. 讀取 Quaternion

```cpp
tf::Quaternion quaternion(
    msg.orientation.x,
    msg.orientation.y,
    msg.orientation.z,
    msg.orientation.w
);
```

這一步只是把 ROS Message 裡的四元數交給 TF 工具處理。

---

## 17. Quaternion 轉 Roll / Pitch / Yaw

```cpp
double roll, pitch, yaw;
tf::Matrix3x3(quaternion).getRPY(roll, pitch, yaw);
```

此時通常是弧度 `rad`。

---

## 18. 弧度轉角度

```cpp
roll  = roll  * 180 / M_PI;
pitch = pitch * 180 / M_PI;
yaw   = yaw   * 180 / M_PI;
```

例如：

```text
Roll  = 0°
Pitch = 0°
Yaw   = 90°
```

表示沒有側傾、沒有抬頭低頭，但車頭方向已旋轉約 90°。

---

## 19. Roll / Pitch / Yaw

```text
Roll  → 橫滾 → 繞 X 軸
Pitch → 俯仰 → 繞 Y 軸
Yaw   → 偏航 → 繞 Z 軸
```

對目前平面 AGV 最容易觀察的是 `Yaw`。

---

## 20. ROS_INFO 顯示姿態

```cpp
ROS_INFO(
    "Roll = %.2f deg, Pitch = %.2f deg, Yaw = %.2f deg",
    roll,
    pitch,
    yaw
);
```

---

## 21. 觀察 angular_velocity

```cpp
msg.angular_velocity.x
msg.angular_velocity.y
msg.angular_velocity.z
```

用 rqt 控制小車旋轉時，特別觀察：

```text
angular_velocity.z
```

---

## 22. 觀察 linear_acceleration

```cpp
msg.linear_acceleration.x
msg.linear_acceleration.y
msg.linear_acceleration.z
```

小車開始前進、停止、加速、減速時，線加速度會產生變化。實際值以 Gazebo Plugin 輸出為準。

---

## 23. CMakeLists.txt

確認相依：

```text
roscpp
sensor_msgs
tf
```

加入 executable：

```cmake
add_executable(imu_subscriber_node src/imu_subscriber_node.cpp)
target_link_libraries(imu_subscriber_node ${catkin_LIBRARIES})
```

只在目前 `CMakeLists.txt` 既有架構上補需要內容，不重寫整份檔案。

---

## 24. package.xml

補上缺少的相依：

```text
roscpp
sensor_msgs
tf
```

同樣不重寫整份檔案。

---

## 25. 編譯

```bash
cd ~/ros_learning_ws
catkin_make
source devel/setup.bash
```

```text
catkin_make
→ 編譯 Workspace

source devel/setup.bash
→ 讓目前 Terminal 認得剛編好的 Workspace / Package / Node
```

---

## 26. 執行 imu_subscriber_node

Gazebo 已啟動且 `/imu/data` 正常後：

```bash
rosrun ros_learning_pkg imu_subscriber_node
```

預期：

```text
/imu/data
↓
IMUCallback
↓
Roll / Pitch / Yaw 持續輸出
```

---

## 27. 搭配 rqt_robot_steering 測試

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
rosrun rqt_robot_steering rqt_robot_steering
```

Topic：

```text
/cmd_vel
```

測試：

```text
車子直走
→ 看 linear_acceleration

車子左轉 / 右轉
→ 看 angular_velocity.z
→ 看 yaw
```

不要同時啟動其他持續發布 `/cmd_vel` 的 Node，避免多個 Publisher 搶控制。

---

## 28. 本章最重要的資料流

```text
Gazebo IMU
↓
/imu/data
↓
sensor_msgs/Imu
↓
imu_subscriber_node
↓
IMUCallback
├── orientation
│   ↓
│   Quaternion
│   ↓
│   Roll / Pitch / Yaw
│
├── angular_velocity
│   ↓
│   三軸角速度
│
└── linear_acceleration
    ↓
    三軸線加速度
```

---

## 29. 不要混淆的幾件事

### Quaternion 不是四個角度

`x / y / z / w` 不是 Roll / Pitch / Yaw 加另一個角度，而是另一種姿態表示法。

### orientation 不是 Sensor Fusion 本身

```text
orientation
→ 姿態結果
```

它可能來自 IMU 內部演算法，也可能來自外部姿態估計或融合，但欄位本身不是融合演算法。

### /imu/data 不是所有硬體都完全一樣

Topic 名稱、orientation 是否有效、資料是否已融合，都依 Driver 與硬體而定。

實機永遠先查：

```bash
rostopic list
rostopic info /imu/data
rostopic echo /imu/data
```

---

## 30. 本章完成條件

```text
[ ] simple_agv 已加入 Gazebo IMU Sensor
[ ] ROS 可以看到 /imu/data
[ ] /imu/data Type 是 sensor_msgs/Imu
[ ] rostopic echo /imu/data 有資料
[ ] imu_subscriber_node 可以正常編譯
[ ] Node 能收到 IMUCallback
[ ] 能讀 orientation
[ ] 能讀 angular_velocity
[ ] 能讀 linear_acceleration
[ ] Quaternion 可以轉成 Roll / Pitch / Yaw
[ ] rqt 控車旋轉時 yaw 能看到變化
[ ] angular_velocity.z 能看到旋轉變化
```

---

## 31. 建議實作順序

### Step 1：Gazebo 加 IMU

修改：

```text
urdf/simple_agv.urdf
```

先只驗證：

```bash
rostopic info /imu/data
rostopic echo /imu/data
```

### Step 2：建立 IMU Subscriber

新增：

```text
src/imu_subscriber_node.cpp
```

修改：

```text
CMakeLists.txt
package.xml
```

先做到成功訂閱 `/imu/data` 並印出原始 IMU 資料。

### Step 3：Quaternion 轉 RPY

加入：

```text
tf::Quaternion
tf::Matrix3x3(...).getRPY(...)
```

輸出 Roll / Pitch / Yaw。

### Step 4：Gazebo 動態驗證

使用 `rqt_robot_steering` 控制車子前進與旋轉，觀察：

```text
Yaw
angular_velocity.z
linear_acceleration
```

### Step 5：README 收尾

最後再補回 `README.md`。

---

## 32. 一句話總結

```text
Gazebo IMU
→ /imu/data
→ sensor_msgs/Imu
→ Callback
→ orientation / angular_velocity / linear_acceleration
→ Quaternion 轉 Roll / Pitch / Yaw
```
