# 05 Launch 與多 Node 啟動

本章接續：

```text
ROS_04Topic&Message
```

04 已經實際跑通：

```text
/speed_publisher
        ↓
    /agv_speed
        ↓
/speed_subscriber
```

本章沿用 04 的 AGV 速度案例，不另外設計遊戲式範例或無實際工程意義的 Topic 名稱。

本章分成兩個區塊：

```text
A. 核心
→ Launch 本身怎麼用
→ 一次啟動多個 Node
→ 看懂 XML、pkg、type、name、output

B. 實戰常用工具
→ 系統啟動後怎麼查 Node / Topic
→ 怎麼看 Topic 資料
→ 怎麼看頻率
→ 怎麼看拓樸
→ 怎麼錄 rosbag
```

兩個區塊分開整理。

Launch 是本章主題；實戰工具是本章額外補充，不要混成同一個概念。

---

# 一、目前正式基準

Workspace：

```text
/root/ros_learning_ws
```

Package：

```text
/root/ros_learning_ws/src/ros_learning_pkg
```

目前 Package 至少包含：

```text
ros_learning_pkg/
├── CMakeLists.txt
├── package.xml
└── src/
    ├── learning_node.cpp
    ├── speed_publisher_node.cpp
    └── speed_subscriber_node.cpp
```

04 已完成並實際驗證：

```text
Publisher
Subscriber
Topic
Message
Callback
rostopic info
rostopic echo
rqt_graph
```

Topic：

```text
/agv_speed
```

Message：

```text
std_msgs/Float32
```

目前兩個 executable：

```text
speed_publisher_node
speed_subscriber_node
```

目前 ROS Node 名稱：

```text
/speed_publisher
/speed_subscriber
```

本章不修改 04 已完成的 C++ 程式，沿用既有的 Publisher / Subscriber 行為。

---

# 二、本章核心目標

原本 04 的執行方式：

```text
Terminal 1
→ roscore

Terminal 2
→ rosrun ros_learning_pkg speed_publisher_node

Terminal 3
→ rosrun ros_learning_pkg speed_subscriber_node
```

05 改成：

```text
roslaunch
↓
一次啟動 Publisher + Subscriber
```

最後要能直接看到：

```text
Publisher 發布 AGV speed
↓
/agv_speed
↓
Subscriber 收到 AGV speed
```

本章不要提前加入：

```text
Service
Action
Parameter Server
TF
URDF
Gazebo
Navigation
```

---

# A. 核心｜Launch

# 三、建立 launch 資料夾

在：

```text
/root/ros_learning_ws/src/ros_learning_pkg
```

建立：

```text
launch/
```

最後 Package 結構至少應為：

```text
ros_learning_pkg/
├── CMakeLists.txt
├── package.xml
├── launch/
│   └── agv_speed.launch
└── src/
    ├── learning_node.cpp
    ├── speed_publisher_node.cpp
    └── speed_subscriber_node.cpp
```

---

# 四、建立 agv_speed.launch

建立：

```text
/root/ros_learning_ws/src/ros_learning_pkg/launch/agv_speed.launch
```

內容：

```xml
<launch>

  <!-- 啟動 AGV 速度 Publisher -->
  <!-- type 對應 CMake 建立的 executable；name 是執行後的 ROS Node 名稱。 -->
  <node
    pkg="ros_learning_pkg"
    type="speed_publisher_node"
    name="speed_publisher"
    output="screen" />

  <!-- 啟動 AGV 速度 Subscriber -->
  <node
    pkg="ros_learning_pkg"
    type="speed_subscriber_node"
    name="speed_subscriber"
    output="screen" />

</launch>
```

不要把 Node 名稱改成遊戲式名稱。

---

# 五、Launch XML 最小理解

本章只需要理解到能讀懂 ROS Launch，不延伸成完整 XML 語法教學。

## 1. `<launch>`

最外層標籤。

```text
一個 launch 檔
↓
描述要啟動哪些 ROS Node
```

例如：

```xml
<launch>
    ...
</launch>
```

可先理解成：

```text
開始標籤
內容
結束標籤
```

---

## 2. `<node>`

每一個 `<node>` 代表要啟動一個 ROS Node。

例如：

```xml
<node
  pkg="ros_learning_pkg"
  type="speed_publisher_node"
  name="speed_publisher"
  output="screen" />
```

---

## 3. `pkg`

```text
pkg="ros_learning_pkg"
```

代表：

```text
這個 Node 位在哪一個 ROS Package
```

---

## 4. `type`

```text
type="speed_publisher_node"
```

代表：

```text
要執行哪一個 executable
```

這裡對應 CMakeLists.txt 中：

```cmake
add_executable(speed_publisher_node src/speed_publisher_node.cpp)
```

所以要清楚區分：

```text
C++ 原始碼檔名
speed_publisher_node.cpp

CMake executable
speed_publisher_node

ROS Node name
/speed_publisher
```

Subscriber 同理：

```text
speed_subscriber_node.cpp
speed_subscriber_node
/speed_subscriber
```

---

## 5. `name`

```text
name="speed_publisher"
```

代表 Launch 啟動後的 ROS Node 名稱。

最後可以在：

```bash
rosnode list
```

看到：

```text
/speed_publisher
```

---

## 6. `output="screen"`

```xml
output="screen"
```

本章先理解為：

```text
讓 Node 的一般 ROS log
直接顯示在 roslaunch 所在 Terminal
```

因此執行後，可以在同一個 Terminal 直接看到：

```text
[Publisher] Publish AGV speed: ...
[Subscriber] Receive AGV speed: ...
```

本章不要深入 ROS logging 底層。

---

# 六、關於 launch-prefix

另一種常見寫法：

```xml
launch-prefix="gnome-terminal -e"
```

概念是：

```text
在 Node 啟動前額外加一層啟動指令
```

常見用途是讓 Node 各自開獨立 Terminal。

但目前正式學習環境是：

```text
Windows
+
WSL Ubuntu 20.04
+
VS Code
```

因此這一章不要把：

```xml
launch-prefix="gnome-terminal -e"
```

當成必要設定。

目前正式做法維持：

```xml
output="screen"
```

讓 Publisher / Subscriber 的輸出直接顯示在同一個 `roslaunch` Terminal。

不要為了這個功能額外安裝 GNOME Terminal。

---

# 七、確認 CMakeLists.txt

沿用 04 已經存在的：

```cmake
## 04 Publisher：發布 AGV 速度到 /agv_speed
add_executable(speed_publisher_node src/speed_publisher_node.cpp)

target_link_libraries(speed_publisher_node
  ${catkin_LIBRARIES}
)

## 04 Subscriber：訂閱 /agv_speed 並在 Callback 顯示速度
add_executable(speed_subscriber_node src/speed_subscriber_node.cpp)

target_link_libraries(speed_subscriber_node
  ${catkin_LIBRARIES}
)
```

原本註解要保留。

Launch XML 本身不需要新增 C++ build target。

編譯確認：

```bash
cd ~/ros_learning_ws
catkin_make
```

必須確認至少看到：

```text
Built target speed_publisher_node
Built target speed_subscriber_node
```

---

# 八、正式啟動 Launch

新的 Terminal：

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
roslaunch ros_learning_pkg agv_speed.launch
```

這一章正式驗證時，不要求另外先手動啟動：

```bash
roscore
```

`roslaunch` 會處理 ROS Master 啟動需求。

如果原本已有 ROS Master，也可以直接沿用。

---

# 九、預期啟動畫面

預期可以看到類似：

```text
process[speed_publisher-...]: started
process[speed_subscriber-...]: started

[Publisher] Publish AGV speed: 0.00 m/s
[Subscriber] Receive AGV speed: 0.00 m/s

[Publisher] Publish AGV speed: 0.10 m/s
[Subscriber] Receive AGV speed: 0.10 m/s
```

不要求 log 排列順序逐字完全相同。

真正要驗證的是：

```text
一個 roslaunch
↓
同時啟動兩個 Node
↓
Publisher 持續發布
↓
Subscriber 持續收到
```

---

# 十、Launch 最核心的理解

04：

```text
rosrun
→ 一次執行一個 Node
```

05：

```text
roslaunch
→ 用一份 launch 組織並啟動多個 Node
```

但資料傳遞方式完全沒有改變。

仍然是：

```text
/speed_publisher
        ↓
    /agv_speed
        ↓
/speed_subscriber
```

Launch 解決的是：

```text
如何啟動與組織 Node
```

不是：

```text
Launch 負責傳送 Topic 資料
```

這一點一定要寫清楚。

---

# B. 實戰補充｜系統啟動後怎麼查 ROS

下面這一區不是 Launch XML 語法。

這一區是：

```text
ROS 系統已經啟動之後
工程上最常用的檢查工具
```

這些指令之後到真實 AGV、Driver、LiDAR、Navigation 系統都會一直用到。

建議把它獨立標成「實戰常用工具」，不要跟 Launch 語法混在一起。

---

# 十一、實戰流程 1：先看 Node

保持：

```bash
roslaunch ros_learning_pkg agv_speed.launch
```

持續執行。

另外開一個 Terminal：

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
rosnode list
```

預期至少看到：

```text
/rosout
/speed_publisher
/speed_subscriber
```

實戰理解：

```text
rosnode list
→ 現在有哪些 ROS Node 活著
```

如果系統說某個 Node 應該有啟動，但 `rosnode list` 看不到，這就是第一個要查的地方。

---

# 十二、實戰流程 2：列出目前所有 Topic

執行：

```bash
rostopic list
```

至少應該能找到：

```text
/agv_speed
```

實戰理解：

```text
rostopic list
→ 現在 ROS 系統有哪些 Topic
```

之後到真車上，不知道 Topic 名稱時，通常先從這裡開始找。

---

# 十三、實戰流程 3：查 Topic 型別、Publisher、Subscriber

執行：

```bash
rostopic info /agv_speed
```

預期：

```text
Type: std_msgs/Float32

Publishers:
 * /speed_publisher

Subscribers:
 * /speed_subscriber
```

實戰理解：

```text
rostopic info
→ 這個 Topic 傳什麼 Message
→ 誰在發布
→ 誰在訂閱
```

這是查 ROS 資料鏈非常重要的指令。

---

# 十四、實戰流程 4：直接看 Topic 正在傳什麼

執行：

```bash
rostopic echo /agv_speed
```

預期看到：

```text
data: 0.0
---
data: 0.10000000149011612
---
data: 0.20000000298023224
---
```

`std_msgs/Float32` 出現類似：

```text
0.10000000149011612
```

屬於浮點數表示精度現象，不代表 Publisher 發錯。

實戰理解：

```text
rostopic echo
→ 直接看 Topic 目前真正流動的 Message 內容
```

停止：

```text
Ctrl + C
```

---

# 十五、實戰流程 5：看 Topic 頻率

執行：

```bash
rostopic hz /agv_speed
```

目前 Publisher 使用：

```cpp
ros::Rate loop_rate(1);
```

因此理論上應接近：

```text
1 Hz
```

實戰理解：

```text
rostopic hz
→ 看 Topic 實際每秒大約收到幾筆資料
```

這個指令之後查感測器特別重要。

例如：

```bash
rostopic hz /scan
```

可以快速確認 LiDAR Topic 實際發布頻率。

停止：

```text
Ctrl + C
```

---

# 十六、實戰流程 6：用 rqt_graph 看完整資料鏈

如果尚未安裝：

```bash
apt install ros-noetic-rqt-graph
```

已安裝過就不用重複安裝。

執行：

```bash
rqt_graph
```

目前應看到類似：

```text
/speed_publisher
        |
        | /agv_speed
        v
/speed_subscriber
```

實戰理解：

```text
rqt_graph
→ 看 Node 與 Topic 的整體連線關係
```

注意：

```text
rqt_graph
```

主要是看拓樸關係，不是拿來錄 Topic 資料。

---

# 十七、實戰流程 7：最小 rosbag 錄製

這裡只做最基本的 rosbag 錄製概念。

完整 rosbag 回放、篩選、多 Topic、大量資料分析留到後面的 Rosbag 專章再深入。

保持 Launch 執行。

另外開 Terminal，切到自己要存 bag 的位置，例如：

```bash
cd ~/ros_learning_ws
```

開始錄 `/agv_speed`：

```bash
rosbag record -O agv_speed_test.bag /agv_speed
```

讓它錄幾秒後：

```text
Ctrl + C
```

應產生：

```text
agv_speed_test.bag
```

查看 bag 基本資訊：

```bash
rosbag info agv_speed_test.bag
```

要能看到裡面包含：

```text
/agv_speed
```

以及 Message 型別：

```text
std_msgs/Float32
```

實戰理解：

```text
rostopic echo
→ 現在直接看

rosbag record
→ 把 Topic 錄下來，之後再分析
```

這兩個用途要分清楚。

---

# 十八、實戰常用工具快速對照

```text
想知道目前有哪些 Node
→ rosnode list

想知道目前有哪些 Topic
→ rostopic list

想知道 Topic 型別、誰發布、誰訂閱
→ rostopic info /Topic名稱

想直接看 Topic 正在傳什麼資料
→ rostopic echo /Topic名稱

想知道 Topic 實際發布頻率
→ rostopic hz /Topic名稱

想看 Node / Topic 整體關係
→ rqt_graph

想把 Topic 資料錄下來
→ rosbag record

想看 bag 裡錄了什麼
→ rosbag info
```

這一區可以直接當 ROS 現場查問題的小抄。

---

# 十九、建議的現場 ROS 排查順序

之後遇到一套陌生 ROS 系統，可以先照這個順序：

```text
1. 系統有沒有正常啟動
↓
rosnode list

2. 我要找的資料有沒有 Topic
↓
rostopic list

3. 這個 Topic 是誰送、誰收、什麼型別
↓
rostopic info

4. Topic 裡真的有沒有資料
↓
rostopic echo

5. 資料頻率正常嗎
↓
rostopic hz

6. 整體資料鏈怎麼接
↓
rqt_graph

7. 現場問題需要帶回去分析
↓
rosbag record
```

這是「實戰排查流程」，不是 Launch 語法流程。

---

# 二十、本章完成條件

## A. Launch 核心完成條件

```text
[ ] ros_learning_pkg/launch 已建立
[ ] agv_speed.launch 已建立
[ ] launch XML 格式正確
[ ] pkg 指向 ros_learning_pkg
[ ] type 使用 speed_publisher_node
[ ] type 使用 speed_subscriber_node
[ ] name 使用 speed_publisher
[ ] name 使用 speed_subscriber
[ ] 兩個 Node 都有 output="screen"
[ ] catkin_make 成功
[ ] roslaunch 可以成功啟動
[ ] Publisher 有持續輸出
[ ] Subscriber 有持續收到資料
```

## B. 實戰工具完成條件

```text
[ ] rosnode list 可看到兩個 Node
[ ] rostopic list 可找到 /agv_speed
[ ] rostopic info /agv_speed 關係正確
[ ] rostopic echo /agv_speed 可看到資料
[ ] rostopic hz /agv_speed 可看到約 1 Hz
[ ] rqt_graph 可看到 Publisher → /agv_speed → Subscriber
[ ] rosbag record 可錄下 /agv_speed
[ ] rosbag info 可確認 bag 內容
```

---

# 二十一、本章最後必須理解的兩層概念

## 第一層：Launch

```text
Launch
→ 管理「怎麼啟動多個 Node」
```

## 第二層：ROS 實戰工具

```text
rosnode / rostopic / rqt_graph / rosbag
→ 管理「系統啟動後怎麼觀察、檢查、記錄」
```

不要混成：

```text
Launch 在傳 Topic
```

真正的資料流仍然是：

```text
Publisher
↓
Topic
↓
Subscriber
```

Launch 只是負責把這些 Node 啟動起來。
