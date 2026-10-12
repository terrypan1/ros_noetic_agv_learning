# 06 Gazebo 模擬與 cmd_vel 速度控制

## 一、本章定位

本章接續前面的 ROS Noetic 基礎：

```text
ROS Noetic
↓
Workspace / Package
↓
C++ Node
↓
Publisher / Subscriber / Topic
↓
Launch
↓
Gazebo + /cmd_vel
```

目前正式 Workspace：

```text
~/ros_learning_ws
```

目前正式 Package：

```text
~/ros_learning_ws/src/ros_learning_pkg
```

本章不直接使用現成的完整模擬 Package。

目標是自己從零建立一台最小機器人，真正理解：

```text
World
↓
URDF
↓
Link / Joint / Axis
↓
Gazebo Plugin
↓
/cmd_vel
↓
機器人移動
```

---

# 二、本章完成目標

本章最後要完整建立：

```text
控制端
↓
geometry_msgs/Twist
↓
/cmd_vel
↓
Gazebo Differential Drive Plugin
↓
left_wheel_joint / right_wheel_joint
↓
左右輪轉動
↓
機器人在 Gazebo 中移動
```

最重要的觀念：

```text
/cmd_vel 本身不會讓車移動
```

真正讓車動的是：

```text
有一個 Subscriber
↓
收到 /cmd_vel
↓
把 Twist 轉成底盤 / 輪子控制
```

在本章模擬環境中，這個 Subscriber 就是 Gazebo 的差速驅動 Plugin。

---

# 三、Gazebo 是什麼

Gazebo 可以先理解成：

```text
機器人模擬器
```

它可以模擬：

```text
世界
地面
障礙物
機器人
物理
碰撞
輪子
感測器
```

直接執行：

```bash
gazebo
```

只會開一個預設空世界。

如果要載入自己建立的世界：

```bash
gazebo ~/ros_learning_ws/src/ros_learning_pkg/worlds/simple_robot.world
```

---

# 四、World 是什麼

World 可以理解成：

```text
Gazebo 的模擬場景
```

它負責描述：

```text
地板
光源
牆壁
障礙物
環境物件
```

目前建立：

```text
~/ros_learning_ws/src/ros_learning_pkg/worlds/simple_robot.world
```

目前最小內容包含：

```text
ground_plane
sun
box_obstacle
```

其中方塊障礙物用來確認：

```text
.world 檔
↓
真的決定 Gazebo 場景裡有哪些東西
```

---

# 五、World 與 Robot 要分清楚

目前先建立這個概念：

```text
World
= 模擬環境

Robot
= 放進模擬環境裡的機器人
```

例如目前：

```text
simple_robot.world
→ 地板、光源、方塊障礙物

simple_robot.urdf
→ 機器人車體與輪子
```

---

# 六、URDF 是什麼

URDF 全名：

```text
Unified Robot Description Format
```

中文可以理解成：

```text
統一機器人描述格式
```

目前檔案：

```text
~/ros_learning_ws/src/ros_learning_pkg/urdf/simple_robot.urdf
```

URDF 主要描述：

```text
機器人有哪些零件
各零件尺寸
各零件位置
零件之間怎麼連
零件能不能旋轉
```

本章先使用一般 `.urdf`。

Xacro 留到後面的 URDF / Xacro 專章再深入。

---

# 七、Link 是什麼

在一般英文中 `link` 常有「連結」的意思。

但在 URDF 裡，`link` 是專有名詞，可以先理解成：

```text
一個機器人零件 / 剛體
```

目前最小機器人：

```text
base_link
→ 主車體

left_wheel_link
→ 左輪

right_wheel_link
→ 右輪
```

所以目前模型可先看成：

```text
simple_robot
├── base_link
├── left_wheel_link
└── right_wheel_link
```

---

# 八、base_link 是什麼

`base_link` 是目前這台機器人的主車體 Link。

可以先理解成：

```text
機器人本體
+
機器人的主要參考座標
```

之後其他零件通常都會以 `base_link` 為主要父層，例如：

```text
base_link
├── left_wheel_link
├── right_wheel_link
├── laser_link
└── camera_link
```

本章先只使用車體與左右輪。

---

# 九、Joint 是什麼

`joint` 可以理解成：

```text
兩個 Link 之間的連接規則
```

例如：

```text
base_link
↓
left_wheel_joint
↓
left_wheel_link
```

Joint 會描述：

```text
parent
child
origin
axis
joint type
```

目前左右輪使用：

```text
type="continuous"
```

代表輪子可以持續旋轉。

---

# 十、Axis 是什麼

`axis` 就是：

```text
Joint 的旋轉軸
```

例如：

```xml
<axis xyz="0 1 0"/>
```

表示：

```text
繞 Y 軸旋轉
```

常見直覺：

```text
1 0 0
→ X 軸

0 1 0
→ Y 軸

0 0 1
→ Z 軸
```

本章左右輪 Joint 使用 Y 軸作為旋轉軸。

---

# 十一、目前 ROS Package 結構

目前整理成：

```text
ros_learning_pkg/
├── CMakeLists.txt
├── package.xml
├── README.md
├── include/
├── launch/
│   ├── robot_speed.launch
│   └── gazebo_robot.launch
├── worlds/
│   └── simple_robot.world
├── urdf/
│   └── simple_robot.urdf
└── src/
    ├── learning_node.cpp
    ├── speed_publisher_node.cpp
    ├── speed_subscriber_node.cpp
    └── cmd_vel_publisher_node.cpp
```

目前不需要建立大量額外資料夾。

---

# 十二、如何把 URDF 車子放進 Gazebo

建立：

```text
~/ros_learning_ws/src/ros_learning_pkg/launch/gazebo_robot.launch
```

目前 Launch 主要負責：

```text
啟動 Gazebo
↓
載入 simple_robot.world
↓
把 simple_robot.urdf 放進 robot_description
↓
啟動 robot_state_publisher
↓
使用 spawn_model
↓
把 simple_robot 放進 Gazebo
```

目前 Launch：

```xml
<launch>

  <!-- 啟動 Gazebo，並載入我們自己的 world -->
  <include file="$(find gazebo_ros)/launch/empty_world.launch">
    <arg name="world_name"
         value="$(find ros_learning_pkg)/worlds/simple_robot.world"/>
  </include>

  <!-- 把 URDF 內容放進 ROS Parameter Server -->
  <param name="robot_description"
         textfile="$(find ros_learning_pkg)/urdf/simple_robot.urdf"/>

  <!-- 根據 robot_description 發布機器人各 Link 的 TF -->
  <node pkg="robot_state_publisher"
        type="robot_state_publisher"
        name="robot_state_publisher"/>

  <!-- 把 URDF 機器人放進 Gazebo -->
  <node pkg="gazebo_ros"
        type="spawn_model"
        name="spawn_simple_robot"
        args="-urdf -param robot_description -model simple_robot -x 0 -y 0 -z 0.2"/>

</launch>
```

注意：

```text
gazebo_robot.launch
目前不要自動啟動 cmd_vel_publisher_node
```

原因是之後可能改由 `rqt_robot_steering` 控制 `/cmd_vel`。

這樣可以避免兩個 Publisher 同時搶控制權。

---

# 十三、Gazebo Differential Drive Plugin

只有 URDF 車體與輪子，車還不會自己動。

還需要 Gazebo 差速驅動 Plugin：

```text
Differential Drive Plugin
= 差速驅動外掛
```

它負責：

```text
Subscribe /cmd_vel
↓
收到 geometry_msgs/Twist
↓
計算左右輪速度
↓
控制 left_wheel_joint
控制 right_wheel_joint
↓
機器人移動
```

目前在 `simple_robot.urdf` 的 `</robot>` 前加入：

```xml
  <!-- Gazebo 差速驅動 Plugin：接收 /cmd_vel，控制左右輪 -->
  <gazebo>
    <plugin name="differential_drive_controller" filename="libgazebo_ros_diff_drive.so">

      <leftJoint>left_wheel_joint</leftJoint>
      <rightJoint>right_wheel_joint</rightJoint>

      <wheelSeparation>0.55</wheelSeparation>
      <wheelDiameter>0.20</wheelDiameter>

      <commandTopic>cmd_vel</commandTopic>
      <odometryTopic>odom</odometryTopic>

      <robotBaseFrame>base_link</robotBaseFrame>

      <publishWheelTF>true</publishWheelTF>
      <publishOdomTF>true</publishOdomTF>

    </plugin>
  </gazebo>
```

其中先理解：

```text
leftJoint
→ 左輪 Joint

rightJoint
→ 右輪 Joint

wheelSeparation
→ 左右輪距離

wheelDiameter
→ 輪子直徑

commandTopic
→ 接收 cmd_vel

robotBaseFrame
→ 主要車體 base_link
```

---

# 十四、geometry_msgs/Twist

本章速度命令 Message：

```text
geometry_msgs/Twist
```

結構：

```text
linear
├── x
├── y
└── z

angular
├── x
├── y
└── z
```

對目前差速機器人，先只使用：

```text
linear.x
→ 前進 / 後退速度

angular.z
→ 左轉 / 右轉角速度
```

最基本命令：

```text
linear.x = 0.10
angular.z = 0.00
```

表示：

```text
向前 0.10 m/s
不旋轉
```

---

# 十五、/cmd_vel 是什麼

`/cmd_vel` 是：

```text
ROS Topic
```

常用來傳：

```text
geometry_msgs/Twist
```

目前控制鏈：

```text
Publisher
↓
/cmd_vel
↓
Gazebo
↓
Differential Drive Plugin
↓
左右輪
```

重要：

```text
/cmd_vel
不是 Node
不是 Message
不是 Driver
```

它只是 Topic。

---

# 十六、cmd_vel_publisher_node

目前自己建立：

```text
src/cmd_vel_publisher_node.cpp
```

Node：

```text
/cmd_vel_publisher
```

它負責固定發布：

```text
/cmd_vel
geometry_msgs/Twist
```

目前測試值：

```text
linear.x = 0.10
angular.z = 0.00
```

啟動：

```bash
rosrun ros_learning_pkg cmd_vel_publisher_node
```

這個 Node 的用途是：

```text
用自己寫的 C++ 程式
持續發布速度命令
```

---

# 十七、驗證 Publisher / Subscriber

先啟動 Gazebo：

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
roslaunch ros_learning_pkg gazebo_robot.launch
```

再啟動：

```bash
rosrun ros_learning_pkg cmd_vel_publisher_node
```

另外開 Terminal：

```bash
rostopic info /cmd_vel
```

目前已實際驗證：

```text
Type:
geometry_msgs/Twist

Publishers:
/cmd_vel_publisher

Subscribers:
/gazebo
```

這證明：

```text
cmd_vel_publisher
↓
/cmd_vel
↓
Gazebo
```

資料流已成立。

而且 Gazebo 中的機器人已經實際向前移動並撞到方塊障礙物。

---

# 十八、rqt_robot_steering：GUI 手動控制機器人

如果不想每次自己打 `rostopic pub`，可以使用：

```text
rqt_robot_steering
```

它可以理解成：

```text
目前模擬機器人的 GUI 遙控器
```

它本身會成為 `/cmd_vel` 的 Publisher。

控制鏈變成：

```text
rqt_robot_steering
↓
geometry_msgs/Twist
↓
/cmd_vel
↓
Gazebo
↓
機器人
```

---

# 十九、安裝 rqt_robot_steering

如果尚未安裝：

```bash
sudo apt install ros-noetic-rqt-robot-steering
```

目前環境已確認套件存在。

---

# 二十、正確啟動 rqt_robot_steering

目前這台環境直接輸入：

```bash
rqt_robot_steering
```

可能出現：

```text
command not found
```

目前實際可用的啟動方式是：

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
rosrun rqt_robot_steering rqt_robot_steering
```

開啟 GUI 後，Topic 設為：

```text
/cmd_vel
```

---

# 二十一、rqt_robot_steering 控制方式

GUI 主要兩個控制：

```text
Linear X
→ 前進 / 後退

Angular Z
→ 左轉 / 右轉
```

直覺：

```text
Linear X > 0
→ 前進

Linear X < 0
→ 後退

Angular Z > 0
→ 左轉

Angular Z < 0
→ 右轉

Linear X = 0
Angular Z = 0
→ 停止
```

如果：

```text
Linear X = 0
Angular Z != 0
```

差速底盤通常會原地旋轉。

---

# 二十二、cmd_vel_publisher_node 與 rqt_robot_steering 不要同時跑

這一點非常重要。

兩者都可以成為：

```text
/cmd_vel Publisher
```

如果同時啟動：

```text
cmd_vel_publisher_node ─┐
                       ├→ /cmd_vel → Gazebo
rqt_robot_steering ─────┘
```

兩邊可能持續發不同速度命令。

結果可能：

```text
GUI 拉回 0
↓
另一個 Publisher 又發 linear.x = 0.10
↓
車繼續動
```

所以手動 GUI 控制時：

```text
先停止 cmd_vel_publisher_node
```

使用：

```text
Ctrl + C
```

---

# 二十三、目前推薦的手動控制流程

## Terminal 1：ROS Master

如果沒有其他 launch 自動啟動 Master，可以先：

```bash
source /opt/ros/noetic/setup.bash
roscore
```

## Terminal 2：Gazebo + 機器人

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
roslaunch ros_learning_pkg gazebo_robot.launch
```

## Terminal 3：GUI 遙控器

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
rosrun rqt_robot_steering rqt_robot_steering
```

GUI Topic：

```text
/cmd_vel
```

控制鏈：

```text
rqt_robot_steering
↓
/cmd_vel
↓
Gazebo
↓
機器人
```

---

# 二十四、如何確認目前是誰在控制 /cmd_vel

執行：

```bash
rostopic info /cmd_vel
```

如果使用自己寫的 C++ Node：

```text
Publishers:
/cmd_vel_publisher
```

如果使用 GUI：

```text
Publishers:
rqt_robot_steering 相關 Node
```

Subscriber 應看到 Gazebo：

```text
Subscribers:
/gazebo
```

這個工具非常重要。

不要猜現在誰在控制車，直接查。

---

# 二十五、直接看 /cmd_vel 資料

執行：

```bash
rostopic echo /cmd_vel
```

可以看到：

```text
linear:
  x: ...
  y: ...
  z: ...
angular:
  x: ...
  y: ...
  z: ...
```

拖動 `rqt_robot_steering` 時，可以觀察數值同步改變。

也就是：

```text
GUI 滑桿
↓
Twist 數值改變
↓
/cmd_vel
↓
Gazebo 車輛運動改變
```

---

# 二十六、強制停止

如果控制來源混亂或 GUI 無法正常停止，可以暫時強制發布零速度：

```bash
rostopic pub -1 /cmd_vel geometry_msgs/Twist \
"{linear: {x: 0.0, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}"
```

但真正要解決的仍然是：

```text
確認現在有幾個 /cmd_vel Publisher
```

使用：

```bash
rostopic info /cmd_vel
```

---

# 二十七、World、URDF、Launch、Plugin 的角色

這四個東西一定要分清楚。

```text
World
→ 模擬世界
→ 地面 / 障礙物 / 環境

URDF
→ 機器人長什麼樣
→ Link / Joint / Axis

Launch
→ 把整個系統啟動起來
→ Gazebo / World / robot_description / spawn_model

Gazebo Plugin
→ 賦予模擬機器人行為
→ 接 /cmd_vel
→ 控制左右輪
```

---

# 二十八、目前與真實機器人的對照

Gazebo：

```text
rqt_robot_steering / 控制 Node
↓
/cmd_vel
↓
Gazebo Differential Drive Plugin
↓
模擬左右輪
↓
模擬機器人
```

真實機器人：

```text
Navigation / 控制器
↓
/cmd_vel 或廠商自己的速度 Topic
↓
真實底盤 Driver
↓
CAN / EtherCAT / Modbus / Serial / 廠商協定
↓
馬達控制器
↓
真正輪子
```

兩者上層觀念是一樣的：

```text
速度命令
↓
底盤接收端
↓
輪子 / 馬達
```

---

# 二十九、目前不要做的事情

本章先不要加入：

```text
LiDAR
/scan
完整 TF 教學
Xacro 深入
AMCL
Navigation
SLAM
Cartographer
Rosbag
```

這些後面分章學。

本章只把：

```text
Gazebo
+
最小機器人
+
/cmd_vel
+
差速底盤
+
GUI 手動控制
```

學完整。

---

# 三十、本章完成條件

目前應確認：

```text
[x] 自己建立 worlds/
[x] 自己建立 simple_robot.world
[x] World 中加入地板與障礙物
[x] 知道 World 是模擬環境
[x] 自己建立 urdf/
[x] 自己建立 simple_robot.urdf
[x] 知道 URDF 是機器人描述格式
[x] 知道 Link 是零件 / 剛體
[x] 知道 Joint 是 Link 的連接規則
[x] 知道 Axis 是旋轉軸
[x] 知道 base_link 是主車體 Link
[x] 自己建立 gazebo_robot.launch
[x] Gazebo 可以載入 simple_robot
[x] 建立 Differential Drive Plugin
[x] Plugin 訂閱 /cmd_vel
[x] cmd_vel_publisher_node 可以發布 Twist
[x] geometry_msgs/Twist 已理解基本用途
[x] linear.x 可以控制前後
[x] angular.z 可以控制左右旋轉
[x] rostopic info /cmd_vel 可看 Publisher / Subscriber
[x] Gazebo 已實際成為 /cmd_vel Subscriber
[x] 機器人已實際在 Gazebo 中移動
[x] rqt_robot_steering 已安裝
[x] 使用 rosrun rqt_robot_steering rqt_robot_steering 成功啟動
[x] rqt_robot_steering 可以控制 Gazebo 機器人
[x] 知道不要同時啟動兩個 /cmd_vel Publisher
```

---

# 三十一、本章最後必須能回答

```text
1. Gazebo 是什麼？

2. World 是什麼？

3. URDF 是什麼？

4. Link 是什麼？

5. Joint 是什麼？

6. Axis 是什麼？

7. base_link 是什麼？

8. /cmd_vel 是 Node、Topic 還是 Message？

9. geometry_msgs/Twist 是什麼？

10. linear.x 控制什麼？

11. angular.z 控制什麼？

12. 為什麼只 Publish /cmd_vel 不一定會讓車動？

13. Gazebo 裡是誰 Subscribe /cmd_vel？

14. Differential Drive Plugin 在做什麼？

15. rqt_robot_steering 在做什麼？

16. cmd_vel_publisher_node 和 rqt_robot_steering 有什麼差別？

17. 為什麼兩個 /cmd_vel Publisher 不應該同時控制？

18. 怎麼查現在 /cmd_vel 是誰發布、誰訂閱？
```

---

# 三十二、本章核心模型

最後牢記：

```text
World
= 世界

URDF
= 機器人

Link
= 零件

Joint
= 零件連接

Axis
= 旋轉軸

Gazebo Plugin
= 模擬底盤控制器

rqt_robot_steering / 控制 Node
        ↓
geometry_msgs/Twist
        ↓
     /cmd_vel
        ↓
Gazebo Differential Drive Plugin
        ↓
左右輪 Joint
        ↓
機器人移動
```

---

# 三十三、下一章銜接

06 完成後再進入：

```text
07 RViz 從零操作與 ROS 資料視覺化
```

07 先學：

```text
RViz
↓
Displays
↓
Fixed Frame
↓
Frame
↓
TF Display
```

之後再依序進入：

```text
TF2 / Frame
↓
URDF / Xacro 深入
↓
LaserScan / Odometry / IMU
↓
Map / AMCL
↓
Navigation
↓
SLAM / Cartographer
```

LiDAR、`/scan` 與 RViz LaserScan 不在本章提前加入。
