# 07 RViz 從零操作與 ROS 資料視覺化

## 一、本章定位

本章接續：

```text
06 Gazebo 模擬與 /cmd_vel 速度控制
```

目前已經完成：

```text
Gazebo World
↓
simple_agv URDF
↓
Gazebo differential drive plugin
↓
/cmd_vel
↓
rqt_robot_steering
↓
Gazebo AGV 可以前進 / 後退 / 左轉 / 右轉
```

本章開始進入 RViz。

本章先不要加入 LiDAR、Map、Navigation、SLAM。

目前只專注理解：

```text
RViz 是什麼？
RobotModel 為什麼會出現？
robot_description 是什麼？
robot_state_publisher 在做什麼？
Frame / TF 是什麼？
Fixed Frame 為什麼會影響畫面？
為什麼 Gazebo 裡的車會動，但 RViz 一開始看起來不動？
為什麼 Fixed Frame 改成 odom 後，RViz 的車就會移動？
為什麼 RViz 與 Gazebo 畫面位置不一定相同？
```

本章目標不是把 RViz 所有功能學完。

本章只建立「ROS 模型資料 + TF + RViz 顯示」的完整基本關係。

---

# 二、本章核心目標

最後要真正理解這兩條資料流。

## 1. 模型資料

```text
simple_agv.urdf
↓
/robot_description
↓
RViz RobotModel
↓
知道機器人長什麼樣
```

## 2. 座標資料

```text
robot_state_publisher / Gazebo Plugin
↓
TF
↓
RViz
↓
知道各 Link 在哪裡
```

最後組合成：

```text
robot_description
        +
       TF
        ↓
RViz RobotModel
```

---

# 三、RViz 是什麼

RViz 是 ROS 常用的 3D 資料視覺化工具。

先用一句話理解：

```text
RViz 不負責模擬機器人。
RViz 負責把 ROS 裡的資料畫出來。
```

常見可以顯示：

```text
RobotModel
TF
LaserScan
PointCloud2
Map
Odometry
Path
Marker
Image
```

目前只處理：

```text
RobotModel
TF
Fixed Frame
```

其他內容留到後續章節。

---

# 四、RViz 與 Gazebo 的差別

## Gazebo

Gazebo 目前負責：

```text
模擬世界
+
模擬機器人
+
模擬物理
+
碰撞
+
輪子運動
```

例如現在的 `simple_agv`：

```text
rqt_robot_steering
↓
/cmd_vel
↓
Gazebo differential drive plugin
↓
左右輪
↓
Gazebo 裡的 AGV 移動
```

## RViz

RViz 負責：

```text
讀 ROS 模型資料
+
讀 TF / Frame
↓
把機器人與 ROS 資料畫出來
```

所以：

```text
Gazebo
→ 看「模擬世界裡真的發生什麼」

RViz
→ 看「ROS 現在知道什麼」
```

這兩個工具角色不同，不要混在一起。

---

# 五、目前使用的 Gazebo Launch

目前使用：

```text
~/ros_learning_ws/src/ros_learning_pkg/launch/gazebo_agv.launch
```

內容：

```xml
<launch>

  <!-- 啟動 Gazebo，並載入我們自己的 world -->
  <include file="$(find gazebo_ros)/launch/empty_world.launch">
    <arg name="world_name"
         value="$(find ros_learning_pkg)/worlds/simple_agv.world"/>
  </include>

  <!-- 把 URDF 內容放進 ROS Parameter Server -->
  <param name="robot_description"
         textfile="$(find ros_learning_pkg)/urdf/simple_agv.urdf"/>

  <!-- 根據 robot_description 發布機器人各 Link 的 TF -->
  <node pkg="robot_state_publisher"
        type="robot_state_publisher"
        name="robot_state_publisher"/>

  <!-- 把 URDF 機器人放進 Gazebo -->
  <node pkg="gazebo_ros"
        type="spawn_model"
        name="spawn_simple_agv"
        args="-urdf -param robot_description -model simple_agv -x 0 -y 0 -z 0.2"/>

</launch>
```

本章最重要的是其中兩段。

---

# 六、robot_description 是什麼

Launch 裡有：

```xml
<param name="robot_description"
       textfile="$(find ros_learning_pkg)/urdf/simple_agv.urdf"/>
```

代表：

```text
simple_agv.urdf
↓
讀取 URDF 內容
↓
放進 ROS Parameter Server
↓
參數名稱 = /robot_description
```

目前已實際驗證：

```bash
rosparam get /robot_description
```

可以看到完整 URDF 內容。

包含：

```text
base_link
left_wheel_link
right_wheel_link
left_wheel_joint
right_wheel_joint
Gazebo differential drive plugin
```

因此可以確認：

```text
simple_agv.urdf
↓
/robot_description
```

這條已經成立。

---

# 七、RobotModel 為什麼會出現

RViz 中加入：

```text
Add
↓
RobotModel
```

RobotModel 會讀取：

```text
/robot_description
```

因此它知道：

```text
有哪些 Link
有哪些 Joint
車體是什麼形狀
輪子是什麼形狀
各零件尺寸
```

可以先理解：

```text
/robot_description
→ 告訴 RViz「這台機器人長什麼樣」
```

但光知道模型長什麼樣還不夠。

RViz 還需要知道：

```text
每個 Link 現在在哪裡？
```

這就需要 TF。

---

# 八、robot_state_publisher 在做什麼

Launch 裡有：

```xml
<node pkg="robot_state_publisher"
      type="robot_state_publisher"
      name="robot_state_publisher"/>
```

目前已實際驗證：

```bash
rosnode list
```

可以看到：

```text
/robot_state_publisher
```

代表這個 Node 已經啟動。

它會根據：

```text
robot_description
+
URDF 裡的 Link / Joint 關係
```

產生機器人各座標 Frame 之間的關係。

例如目前：

```text
base_link
├── left_wheel_link
└── right_wheel_link
```

先不用深入 TF 數學。

目前只需要理解：

```text
robot_description
→ 模型長什麼樣

TF
→ 各零件在哪裡
```

---

# 九、Frame 是什麼

Frame 可以先理解成：

```text
座標系
```

目前會碰到：

```text
odom
base_link
left_wheel_link
right_wheel_link
```

其中：

```text
base_link
→ AGV 車體主要座標系
```

```text
left_wheel_link
→ 左輪自己的座標系
```

```text
right_wheel_link
→ 右輪自己的座標系
```

```text
odom
→ ROS 里程計參考座標系
```

之後還會看到：

```text
map
laser
imu_link
```

但本章先不要全部展開。

---

# 十、TF 是什麼

TF 可以先理解成：

```text
座標系之間的轉換關係
```

例如：

```text
odom
↓
base_link
↓
left_wheel_link
```

可以理解成：

```text
左輪在哪裡？
↓
先知道左輪相對 base_link 在哪裡
↓
再知道 base_link 相對 odom 在哪裡
```

RViz 才能把左輪畫到正確位置。

目前不要深入：

```text
TF broadcaster
TF listener
Quaternion
Transformation Matrix
```

那些留到下一個 TF2 專章。

---

# 十一、啟動 RViz

另外開一個 Terminal：

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
rviz
```

開啟後先認識：

```text
中央 3D View
左側 Displays
Global Options
Fixed Frame
Add
```

本章不要一次加入太多 Display。

---

# 十二、Fixed Frame 是什麼

Fixed Frame 是 RViz 最重要的概念之一。

先用一句話理解：

```text
RViz 要選一個座標系，當作所有資料顯示時的共同基準。
```

例如：

```text
Fixed Frame = base_link
```

可以理解成：

```text
站在車體座標系裡看世界
```

如果車自己移動：

```text
你跟著車一起移動
```

所以 RViz 裡的 RobotModel 可能看起來一直留在原地。

這不是車沒有動。

只是因為：

```text
你把 base_link 當成觀察基準
```

---

# 十三、為什麼 Fixed Frame = base_link 時車看起來不動

目前用 `rqt_robot_steering`：

```text
rqt_robot_steering
↓
/cmd_vel
↓
Gazebo
↓
AGV 移動
```

Gazebo 裡確實看到車在動。

但是如果 RViz：

```text
Fixed Frame = base_link
```

就等於：

```text
攝影機的座標基準跟著車一起走
```

因此車本身看起來不會離開原點。

直覺可以想成：

```text
站在車上看車
→ 車永遠在自己腳下
```

---

# 十四、/odom 是什麼

目前 Gazebo differential drive plugin 已設定：

```xml
<odometryTopic>odom</odometryTopic>
<robotBaseFrame>base_link</robotBaseFrame>
<publishOdomTF>true</publishOdomTF>
```

因此 Gazebo 會產生：

```text
/odom
```

可以查看：

```bash
rostopic echo /odom
```

使用 `rqt_robot_steering` 控車時，可以看到：

```text
pose.pose.position.x
pose.pose.position.y
```

等數值持續改變。

這代表：

```text
Gazebo AGV 移動
↓
里程計資料更新
↓
/odom 更新
```

---

# 十五、odom → base_link TF

Gazebo Plugin 目前有：

```xml
<publishOdomTF>true</publishOdomTF>
```

因此會發布：

```text
odom
↓
base_link
```

可以驗證：

```bash
rosrun tf tf_echo odom base_link
```

控制 AGV 移動時，如果數值持續變化，就代表：

```text
odom → base_link
```

這條 TF 正常。

目前核心關係：

```text
Gazebo AGV 移動
↓
/odom 更新
↓
odom → base_link TF 更新
↓
RViz 可以知道車相對 odom 移動到哪裡
```

---

# 十六、讓 RViz 看到車移動

RViz 中把：

```text
Fixed Frame = odom
```

再使用：

```text
rqt_robot_steering
```

控制車輛。

這時 RViz RobotModel 就會跟著移動。

原因是：

```text
Fixed Frame = odom
↓
RViz 站在 odom 座標系裡
↓
base_link 相對 odom 持續改變
↓
RobotModel 在 RViz 中移動
```

直覺可以理解成：

```text
Fixed Frame = base_link
→ 站在車上看車

Fixed Frame = odom
→ 站在地面看車
```

這是本章非常重要的概念。

---

# 十七、RViz 與 Gazebo 為什麼位置看起來不同

目前兩邊使用的基準不同。

Gazebo：

```text
Gazebo world 座標
```

RViz：

```text
Fixed Frame = odom
```

因此目前不是：

```text
world
↓
odom
↓
base_link
```

完整統一的一棵 ROS TF 樹。

目前主要建立的是：

```text
odom
↓
base_link
↓
wheel links
```

Gazebo 自己還有自己的 `world` 座標。

所以：

```text
Gazebo world
≠
ROS odom
```

不要直接假設它們一定完全重合。

另外兩個工具的攝影機：

```text
視角
縮放
中心點
```

也完全獨立。

所以畫面看起來不同是正常的。

---

# 十八、Gazebo 裡的障礙物為什麼 RViz 看不到

目前 `simple_agv.world` 裡的箱子是：

```text
Gazebo World 物件
```

RViz 不會自動知道 Gazebo 裡有哪些牆、箱子、柱子。

因此現在：

```text
Gazebo
→ 車 + 地面 + 箱子

RViz
→ RobotModel + TF
```

這是正常狀態。

之後加入 LiDAR 後才會形成：

```text
Gazebo 箱子
↓
Gazebo LiDAR 掃到箱子
↓
/scan
↓
RViz LaserScan
↓
看到雷射點
```

但這留到後面再做。

---

# 十九、目前實際操作流程

## Terminal 1：roscore

```bash
roscore
```

## Terminal 2：Gazebo

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
roslaunch ros_learning_pkg gazebo_agv.launch
```

## Terminal 3：rqt_robot_steering

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
rosrun rqt_robot_steering rqt_robot_steering
```

Topic：

```text
/cmd_vel
```

## Terminal 4：RViz

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
rviz
```

RViz：

```text
Fixed Frame = odom
Add → RobotModel
```

這時可以看到：

```text
rqt_robot_steering
↓
/cmd_vel
↓
Gazebo AGV 移動
↓
/odom / TF 更新
↓
RViz RobotModel 移動
```

---

# 二十、目前完整資料流

## 控制線

```text
rqt_robot_steering
        ↓
geometry_msgs/Twist
        ↓
     /cmd_vel
        ↓
Gazebo differential drive plugin
        ↓
左右輪
        ↓
Gazebo AGV 移動
```

## 里程計 / TF 線

```text
Gazebo differential drive plugin
        ↓
      /odom
        ↓
odom → base_link TF
        ↓
       RViz
```

## 模型線

```text
simple_agv.urdf
        ↓
/robot_description
        ↓
RViz RobotModel
```

三條線合起來：

```text
                  rqt_robot_steering
                          ↓
                       /cmd_vel
                          ↓
                      Gazebo
                          ↓
                      AGV 移動
                          ↓
                 /odom + TF 更新
                          ↓
                         RViz
                          ↑
                  /robot_description
                          ↑
                   simple_agv.urdf
```

這就是目前 06 → 07 最重要的整體關係。

---

# 二十一、實戰驗證指令

## 1. 確認 robot_description

```bash
rosparam get /robot_description
```

## 2. 確認 robot_state_publisher

```bash
rosnode list
```

應看到：

```text
/robot_state_publisher
```

## 3. 查看 odom

```bash
rostopic echo /odom
```

## 4. 查看 odom → base_link

```bash
rosrun tf tf_echo odom base_link
```

## 5. 查看 /cmd_vel

```bash
rostopic info /cmd_vel
```

使用 `rqt_robot_steering` 時，Publisher 應為 rqt 相關 Node。

Subscriber 應包含 Gazebo。

---

# 二十二、常見錯誤觀念

## 1. RViz 是模擬器

錯。

```text
Gazebo
→ 模擬

RViz
→ 視覺化
```

## 2. RobotModel 出現代表 Gazebo 車存在

錯。

RobotModel 的主要來源是：

```text
robot_description
+
TF
```

不代表 Gazebo 一定有車。

## 3. Gazebo 車會動，RViz 就一定會動

錯。

還要看：

```text
TF 有沒有更新
Fixed Frame 選什麼
```

## 4. Fixed Frame = base_link 時車不動，是 TF 壞掉

不一定。

可能只是：

```text
你站在車體座標系裡看車
```

## 5. Gazebo world 等於 ROS odom

不要直接這樣理解。

目前：

```text
Gazebo world
```

和：

```text
ROS odom
```

是不同系統中的參考座標概念。

---

# 二十三、本章完成條件

```text
[ ] 知道 RViz 是 ROS 資料視覺化工具
[ ] 知道 RViz 與 Gazebo 的差別
[ ] 可以自行啟動 RViz
[ ] 知道 Displays / Add / RobotModel 在做什麼
[ ] 知道 /robot_description 來自 simple_agv.urdf
[ ] rosparam get /robot_description 可以看到 URDF
[ ] 知道 robot_state_publisher 的基本用途
[ ] rosnode list 可以看到 /robot_state_publisher
[ ] 可以初步解釋 Frame 是座標系
[ ] 可以初步解釋 TF 是座標系之間的轉換關係
[ ] 可以解釋 Fixed Frame 是 RViz 的顯示基準座標系
[ ] 知道 Fixed Frame = base_link 時車可能看起來不動
[ ] 知道 /odom 會隨 Gazebo 車移動更新
[ ] 可以使用 tf_echo 查看 odom → base_link
[ ] Fixed Frame = odom 時可以看到 RobotModel 跟著移動
[ ] 知道 Gazebo world 與 ROS odom 目前不是同一個概念
[ ] 知道 Gazebo 的障礙物不會自動出現在 RViz
```

---

# 二十四、本章最後必須能回答

```text
1. RViz 是什麼？

2. RViz 與 Gazebo 的差別是什麼？

3. RobotModel 的模型資料從哪裡來？

4. /robot_description 是什麼？

5. robot_state_publisher 在做什麼？

6. Frame 是什麼？

7. TF 是什麼？

8. Fixed Frame 是什麼？

9. 為什麼 Fixed Frame = base_link 時車看起來不動？

10. 為什麼改成 Fixed Frame = odom 後，車會在 RViz 裡移動？

11. /odom 在目前系統中從哪裡來？

12. odom → base_link 代表什麼？

13. 為什麼 Gazebo 與 RViz 畫面位置看起來不一定一樣？

14. 為什麼 Gazebo 裡的箱子目前不會直接出現在 RViz？
```

---

# 二十五、本章核心模型

最後只要牢記：

```text
URDF
↓
/robot_description
↓
RobotModel
```

以及：

```text
Gazebo / robot_state_publisher
↓
TF
↓
RViz
```

以及：

```text
Fixed Frame = base_link
→ 站在車上看車

Fixed Frame = odom
→ 站在里程計座標系看車
```

目前完整關係：

```text
rqt_robot_steering
↓
/cmd_vel
↓
Gazebo
↓
AGV 移動
↓
/odom + odom→base_link TF
↓
RViz

simple_agv.urdf
↓
/robot_description
↓
RViz RobotModel
```

---

# 二十六、本章先停在這裡

07 目前做到：

```text
RViz 基本操作
+
RobotModel
+
robot_description
+
robot_state_publisher
+
Frame / TF 基本概念
+
Fixed Frame
+
odom → base_link
+
Gazebo 與 RViz 的角色差異
```

先不要在本章加入：

```text
LiDAR
/scan
LaserScan
Map
AMCL
Navigation
SLAM
Cartographer
Rosbag
```

下一階段再正式進入：

```text
08 TF2 / Frame 與座標系
```

把目前只用直覺理解的：

```text
odom
base_link
TF
Fixed Frame
```

再完整拆開學習。
