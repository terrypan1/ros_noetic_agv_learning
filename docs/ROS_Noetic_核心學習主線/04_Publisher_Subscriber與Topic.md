# 04 Publisher、Subscriber 與 Topic

## 一、這一章的學習方式

這一章重新整理。

常見入門範例常使用遊戲式文字與不具實際工程意義的 Topic 名稱，不利於後續理解真正 ROS 專案的資料流。

這次改成一個最小、正常，而且可以直接對應 AGV 系統的案例：

```text
speed_publisher
        ↓
    /agv_speed
  std_msgs/Float32
        ↓
speed_subscriber
```

這一章的教學方式改成：

```text
先把完整 Publisher + Subscriber 準備好
↓
第一次執行就直接看到完整效果
↓
再一步一步拆 Publisher
↓
再理解 Topic / Message
↓
再拆 Subscriber / Callback
↓
最後用 ROS 工具與 rqt_graph 驗證
```

學習時仍然一次只做一步，不一次把整章全部操作完。

---

## 二、這一章真正要學會什麼

這一章只處理 ROS 最核心的資料流：

```text
Publisher
→ Topic
→ Message
→ Subscriber
→ Callback
```

完成後要能回答：

1. 哪一個 Node 在發布資料？
2. Topic 名稱是什麼？
3. Topic 傳送的 Message 型別是什麼？
4. 哪一個 Node 在訂閱？
5. Subscriber 收到資料後做了什麼？
6. `advertise()` 在做什麼？
7. `publish()` 在做什麼？
8. `subscribe()` 在做什麼？
9. Callback 為什麼會被執行？
10. `ros::spin()` 為什麼 Subscriber 需要？
11. `rostopic list`、`rostopic info`、`rostopic echo` 各自能看到什麼？
12. ROS Master 在 Publisher / Subscriber 建立連線時扮演什麼角色？

---

## 三、本章實作案例

目前 Workspace：

```text
/root/ros_learning_ws
```

目前 Package：

```text
ros_learning_pkg
```

這一章建立兩個 Node。

### 1. Publisher Node

檔案：

```text
/root/ros_learning_ws/src/ros_learning_pkg/src/speed_publisher_node.cpp
```

Node 名稱：

```text
speed_publisher
```

工作：

```text
模擬 AGV 速度
↓
發布到 /agv_speed
```

---

### 2. Topic

Topic 名稱：

```text
/agv_speed
```

Message 型別：

```text
std_msgs/Float32
```

資料內容模擬：

```text
0.00
0.10
0.20
0.30
0.20
0.10
0.00
```

單位約定：

```text
m/s
```

這只是學習用的模擬速度，不代表真車速度來源。

---

### 3. Subscriber Node

檔案：

```text
/root/ros_learning_ws/src/ros_learning_pkg/src/speed_subscriber_node.cpp
```

Node 名稱：

```text
speed_subscriber
```

工作：

```text
訂閱 /agv_speed
↓
收到 std_msgs/Float32
↓
進入 Callback
↓
輸出收到的 AGV 速度
```

---

## 四、完整資料流

```text
speed_publisher
      |
      | publish
      v
  /agv_speed
std_msgs/Float32
      |
      | subscribe
      v
speed_subscriber
      |
      v
speedCallback()
```

這裡要先建立一個很重要的觀念：

```text
不是 speed_publisher 直接呼叫 speed_subscriber
```

而是：

```text
speed_publisher
→ 發布到 /agv_speed

speed_subscriber
→ 訂閱 /agv_speed

ROS
→ 根據 Topic 把 Publisher 與 Subscriber 接起來
```

---

# 五、完整程式先準備好

## 5.1 Publisher：speed_publisher_node.cpp

```cpp
#include <ros/ros.h>
#include <std_msgs/Float32.h>
#include <vector>

int main(int argc, char **argv)
{
    // 初始化 ROS，Node 名稱為 speed_publisher
    ros::init(argc, argv, "speed_publisher");

    // 建立 NodeHandle
    ros::NodeHandle nh;

    // 建立 Publisher
    // 發布 Topic：/agv_speed
    // Message 型別：std_msgs/Float32
    // Queue Size：10
    ros::Publisher speed_pub = nh.advertise<std_msgs::Float32>("/agv_speed", 10);

    // 每秒發布一次資料
    ros::Rate loop_rate(1);

    // 模擬 AGV 速度
    std::vector<float> speeds = {
        0.00f,
        0.10f,
        0.20f,
        0.30f,
        0.20f,
        0.10f,
        0.00f
    };

    std::size_t index = 0;

    while (ros::ok())
    {
        std_msgs::Float32 msg;
        msg.data = speeds[index];

        // 將速度資料發布到 /agv_speed
        speed_pub.publish(msg);

        ROS_INFO("[Publisher] Publish AGV speed: %.2f m/s", msg.data);

        index++;

        if (index >= speeds.size())
        {
            index = 0;
        }

        loop_rate.sleep();
    }

    return 0;
}
```

---

## 5.2 Subscriber：speed_subscriber_node.cpp

```cpp
#include <ros/ros.h>
#include <std_msgs/Float32.h>

// 當 /agv_speed 有新資料進來時，ROS 會呼叫這個 Callback
void speedCallback(const std_msgs::Float32::ConstPtr &msg)
{
    ROS_INFO("[Subscriber] Receive AGV speed: %.2f m/s", msg->data);
}

int main(int argc, char **argv)
{
    // 初始化 ROS，Node 名稱為 speed_subscriber
    ros::init(argc, argv, "speed_subscriber");

    // 建立 NodeHandle
    ros::NodeHandle nh;

    // 訂閱 /agv_speed
    // Message 型別會由 Callback 參數決定為 std_msgs/Float32
    // Queue Size：10
    ros::Subscriber speed_sub = nh.subscribe("/agv_speed", 10, speedCallback);

    ROS_INFO("speed_subscriber started, waiting for /agv_speed ...");

    // 讓 Node 持續運行並處理 Callback
    ros::spin();

    return 0;
}
```

---

# 六、CMakeLists.txt

目前原本的 `learning_node` 保留，不刪除。

在 `CMakeLists.txt` 的 Build 區塊加入 Publisher 與 Subscriber。

整理後相關內容應包含：

```cmake
###########
## Build ##
###########

## 指定 C++ 編譯時要去哪裡尋找 Header 檔案
## ${catkin_INCLUDE_DIRS} 會包含 roscpp、std_msgs 等 Catkin 相依套件的 include 路徑
include_directories(
  ${catkin_INCLUDE_DIRS}
)

## 03 章建立的 learning_node
add_executable(learning_node src/learning_node.cpp)

target_link_libraries(learning_node
  ${catkin_LIBRARIES}
)

## 04 章 Publisher
add_executable(speed_publisher_node src/speed_publisher_node.cpp)

target_link_libraries(speed_publisher_node
  ${catkin_LIBRARIES}
)

## 04 章 Subscriber
add_executable(speed_subscriber_node src/speed_subscriber_node.cpp)

target_link_libraries(speed_subscriber_node
  ${catkin_LIBRARIES}
)
```

這裡先只需要知道：

```text
speed_publisher_node.cpp
↓
add_executable
↓
speed_publisher_node executable
```

以及：

```text
speed_subscriber_node.cpp
↓
add_executable
↓
speed_subscriber_node executable
```

---

# 七、第一次完整執行效果

這一章第一次執行時，先直接把完整 Publisher + Subscriber 跑起來。

暫時不用 `roslaunch`。

原因：

```text
Launch 是後面的章節
```

這裡刻意使用三個 Terminal，讓 Publisher 與 Subscriber 是兩個獨立程序這件事看得更清楚。

---

## Terminal 1：ROS Master

```bash
source /opt/ros/noetic/setup.bash
roscore
```

保持 Terminal 1 不關閉。

---

## Terminal 2：Publisher

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
rosrun ros_learning_pkg speed_publisher_node
```

預期會持續看到：

```text
[Publisher] Publish AGV speed: 0.00 m/s
[Publisher] Publish AGV speed: 0.10 m/s
[Publisher] Publish AGV speed: 0.20 m/s
[Publisher] Publish AGV speed: 0.30 m/s
[Publisher] Publish AGV speed: 0.20 m/s
[Publisher] Publish AGV speed: 0.10 m/s
[Publisher] Publish AGV speed: 0.00 m/s
```

然後重新循環。

---

## Terminal 3：Subscriber

```bash
source /opt/ros/noetic/setup.bash
source ~/ros_learning_ws/devel/setup.bash
rosrun ros_learning_pkg speed_subscriber_node
```

預期會看到：

```text
speed_subscriber started, waiting for /agv_speed ...
[Subscriber] Receive AGV speed: 0.10 m/s
[Subscriber] Receive AGV speed: 0.20 m/s
[Subscriber] Receive AGV speed: 0.30 m/s
[Subscriber] Receive AGV speed: 0.20 m/s
[Subscriber] Receive AGV speed: 0.10 m/s
[Subscriber] Receive AGV speed: 0.00 m/s
```

Publisher 與 Subscriber 兩邊的數值應該可以互相對上。

例如：

```text
Publisher
Publish AGV speed: 0.20 m/s

          ↓ /agv_speed

Subscriber
Receive AGV speed: 0.20 m/s
```

這就是本章第一次最重要的實際效果。

---

# 八、04-1：先看完整結果

第一階段暫時不要急著背程式。

只確認：

```text
Terminal 2
speed_publisher
持續產生速度

        ↓

/agv_speed

        ↓

Terminal 3
speed_subscriber
持續收到同樣速度
```

這一步真正要理解的是：

```text
Publisher 與 Subscriber 是兩個獨立 Node
```

兩者不是直接互相呼叫。

中間是靠：

```text
Topic
```

進行資料交換。

---

# 九、04-2：拆解 Publisher

Publisher 最核心的程式只有幾個部分。

## 9.1 建立 Publisher

```cpp
ros::Publisher speed_pub = nh.advertise<std_msgs::Float32>("/agv_speed", 10);
```

拆開看：

```text
ros::Publisher
→ Publisher 物件

speed_pub
→ 自己程式中的變數名稱

nh.advertise
→ 告訴 ROS：這個 Node 要發布一個 Topic

std_msgs::Float32
→ Topic 使用的 Message 型別

/agv_speed
→ Topic 名稱

10
→ Publisher Queue Size
```

最核心先記：

```text
advertise
→ 宣告我要發布哪個 Topic
```

---

## 9.2 建立 Message

```cpp
std_msgs::Float32 msg;
msg.data = speeds[index];
```

這裡建立：

```text
std_msgs/Float32 Message
```

其中真正的數值放在：

```text
msg.data
```

---

## 9.3 發布 Message

```cpp
speed_pub.publish(msg);
```

意思：

```text
把 msg
↓
發布到 speed_pub 對應的 Topic
↓
/agv_speed
```

因此：

```text
advertise
→ 先建立 Publisher / 宣告 Topic

publish
→ 真正把 Message 發出去
```

---

## 9.4 ros::Rate

```cpp
ros::Rate loop_rate(1);
```

目前設定：

```text
1 Hz
```

也就是大約：

```text
每秒執行一次
```

搭配：

```cpp
loop_rate.sleep();
```

控制迴圈速度。

---

# 十、04-3：理解 Topic 與 Message

目前 Topic：

```text
/agv_speed
```

Topic 本身不是 Node，也不是一般 C++ 變數。

可以先理解成：

```text
ROS 裡一條有名字的資料通道
```

而這條通道只能傳它宣告的 Message 型別。

目前：

```text
Topic
/agv_speed

Message Type
std_msgs/Float32
```

所以資料流是：

```text
0.20
↓
包成 std_msgs/Float32
↓
發布到 /agv_speed
```

---

# 十一、使用 rostopic 直接觀察

Publisher 執行時，可以另外開 Terminal 查看。

## 11.1 查看 Topic

```bash
rostopic list
```

應該可以看到：

```text
/agv_speed
```

---

## 11.2 查看 Topic 詳細資訊

```bash
rostopic info /agv_speed
```

重點看：

```text
Type
Publishers
Subscribers
```

當 Publisher 與 Subscriber 都有執行時，概念上會看到：

```text
Type: std_msgs/Float32

Publishers:
 * /speed_publisher

Subscribers:
 * /speed_subscriber
```

這個指令非常重要。

因為它直接回答：

```text
這條 Topic 傳什麼？
誰在發布？
誰在訂閱？
```

---

## 11.3 直接看 Topic 裡的資料

```bash
rostopic echo /agv_speed
```

會直接看到類似：

```text
data: 0.2
---
data: 0.3
---
data: 0.2
---
```

這代表：

```text
不需要自己寫 Subscriber
rostopic echo 本身就能幫我們訂閱並顯示 Topic 資料
```

---

# 十二、04-4：拆解 Subscriber

Subscriber 最核心有三個東西：

```text
subscribe
Callback
ros::spin
```

---

## 12.1 建立 Subscriber

```cpp
ros::Subscriber speed_sub = nh.subscribe("/agv_speed", 10, speedCallback);
```

拆開看：

```text
ros::Subscriber
→ Subscriber 物件

speed_sub
→ 自己程式中的變數名稱

nh.subscribe
→ 告訴 ROS：我要訂閱某個 Topic

/agv_speed
→ 要訂閱的 Topic

10
→ Subscriber Queue Size

speedCallback
→ 收到資料後要執行的函式
```

核心先記：

```text
subscribe
→ 訂閱 Topic
```

---

## 12.2 Callback

```cpp
void speedCallback(const std_msgs::Float32::ConstPtr &msg)
{
    ROS_INFO("[Subscriber] Receive AGV speed: %.2f m/s", msg->data);
}
```

這個函式平常不是自己直接呼叫。

而是：

```text
/agv_speed 有新 Message
↓
ROS 收到事件
↓
呼叫 speedCallback
↓
把 Message 傳進來
↓
程式讀 msg->data
```

這就是 ROS Subscriber 很重要的 Callback 概念。

---

## 12.3 ros::spin()

```cpp
ros::spin();
```

可以先理解：

```text
讓 Subscriber Node 持續活著
↓
持續等待 ROS Event
↓
有 Message 時處理 Callback
```

如果 Subscriber 建立完就直接：

```cpp
return 0;
```

Node 很快就結束，自然也無法一直等待 `/agv_speed`。

---

# 十三、Publisher 與 Subscriber 放在一起看

現在把兩邊核心程式接起來。

Publisher：

```cpp
ros::Publisher speed_pub = nh.advertise<std_msgs::Float32>("/agv_speed", 10);
```

Subscriber：

```cpp
ros::Subscriber speed_sub = nh.subscribe("/agv_speed", 10, speedCallback);
```

兩邊共同的東西是：

```text
/agv_speed
```

因此 ROS 可以知道：

```text
/speed_publisher
        ↓
發布 /agv_speed

/speed_subscriber
        ↑
訂閱 /agv_speed
```

最後形成：

```text
/speed_publisher
       |
       | std_msgs/Float32
       v
  /agv_speed
       |
       | std_msgs/Float32
       v
/speed_subscriber
```

---

# 十四、ROS Master 在這裡做什麼

這一階段先使用簡化理解。

ROS Master 並不是把每一筆 `0.10`、`0.20`、`0.30` 速度資料都自己轉送一次。

它主要協助：

```text
Node 註冊
Topic 註冊
Publisher / Subscriber 發現彼此
建立連線資訊
```

可以先想成：

```text
speed_publisher
→ 告訴 ROS Master
→ 我會發布 /agv_speed

speed_subscriber
→ 告訴 ROS Master
→ 我要訂閱 /agv_speed

ROS Master
→ 協助雙方找到彼此

之後
→ Publisher 與 Subscriber 建立實際資料連線
```

這個理解先夠用，不追 ROS Master 更底層通訊細節。

---

# 十五、rosnode list 再看一次

Publisher 與 Subscriber 都執行後：

```bash
rosnode list
```

應該至少看到：

```text
/rosout
/speed_publisher
/speed_subscriber
```

這裡可以重新確認：

```text
Publisher 是 Node
Subscriber 也是 Node
Topic 不是 Node
```

---

# 十六、rqt_graph

等前面都確認完成後，再使用：

```bash
rqt_graph
```

預期可以用圖形看到類似：

```text
/speed_publisher
        |
        v
   /agv_speed
        |
        v
/speed_subscriber
```

這時候畫面上的圖，就可以和自己前面手動畫的資料流互相對照。

---

# 十七、這一章暫時不加入 roslaunch

雖然可以用一個 Launch File 一次啟動 Publisher 與 Subscriber，但這一章先不做。

原因：

```text
Launch 是後面的獨立學習章節
```

目前使用三個 Terminal 有一個學習上的好處：

```text
Terminal 1
roscore

Terminal 2
speed_publisher

Terminal 3
speed_subscriber
```

可以很清楚看到：

```text
ROS Master
Publisher Node
Subscriber Node
```

是三個不同角色。

---

# 十八、這一章暫時不處理的內容

先不要混入：

```text
自訂 msg
Service
Action
Parameter
Launch
TF
LaserScan
Odometry
IMU
Navigation
SLAM
```

本章只把：

```text
Publisher
Topic
Message
Subscriber
Callback
```

真正跑通並理解。

---

# 十九、回頭對照真正 AGV 系統

自己的最小 Publisher / Subscriber 跑通之後，再回頭看真正 ROS AGV 系統。

例如後續常看到：

```text
/scan
/cmd_vel
/odom
```

先不要急著研究每種 Message 的所有欄位。

先用本章模型拆：

```text
誰 Publish？
↓
Topic 是什麼？
↓
Message Type 是什麼？
↓
誰 Subscribe？
↓
收到後做什麼？
```

例如概念上：

```text
LiDAR Driver
↓
/scan
↓
Navigation / SLAM
```

或：

```text
Navigation
↓
/cmd_vel
↓
底盤控制相關 Node
```

本質上還是：

```text
Publisher
→ Topic
→ Subscriber
```

---

# 二十、本章完成條件

- [ ] `speed_publisher_node.cpp` 建立完成
- [ ] `speed_subscriber_node.cpp` 建立完成
- [ ] `CMakeLists.txt` 加入兩個 executable
- [ ] `catkin_make` 成功
- [ ] `speed_publisher_node` 可以執行
- [ ] `speed_subscriber_node` 可以執行
- [ ] Publisher 可以持續發布 `/agv_speed`
- [ ] Subscriber Callback 可以持續收到 `/agv_speed`
- [ ] 知道 `/agv_speed` 是 Topic，不是 Node
- [ ] 知道 Message 型別是 `std_msgs/Float32`
- [ ] 知道 `advertise()` 的用途
- [ ] 知道 `publish()` 的用途
- [ ] 知道 `subscribe()` 的用途
- [ ] 知道 Callback 為什麼會被執行
- [ ] 知道 `ros::spin()` 在 Subscriber 的用途
- [ ] `rosnode list` 能看到兩個 Node
- [ ] `rostopic list` 能看到 `/agv_speed`
- [ ] `rostopic info /agv_speed` 能看懂 Publishers / Subscribers
- [ ] `rostopic echo /agv_speed` 能看到實際資料
- [ ] `rqt_graph` 能看懂完整資料流

---

# 二十一、本章核心模型

最後要留下的核心不是範例中的 AGV 速度數字，而是這個模型：

```text
Publisher Node
      |
      | advertise + publish
      v
    Topic
      |
      | Message
      v
Subscriber Node
      |
      | subscribe
      v
   Callback
```

之後看到 ROS 裡任何 Topic，都可以先用這個模型拆解。

例如：

```text
/scan
/cmd_vel
/odom
```

先問：

```text
誰發布？
傳什麼？
誰訂閱？
收到後做什麼？
```

這就是 04 章真正要建立的能力。
