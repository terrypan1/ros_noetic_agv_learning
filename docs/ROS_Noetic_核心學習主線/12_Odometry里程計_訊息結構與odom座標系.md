# 12_Odometry里程計_訊息結構與odom座標系

## 1. 本章定位

本章延續前一章 TF / Frame 的學習，專心理解 ROS 裡的 **Odometry（里程計）** 與 `odom` 座標系。

本章先不進 SLAM，也不展開 Cartographer。

核心目標是先理解：

```text
機器人怎麼根據自身運動資訊
估計自己「相對於出發點移動了多少」
```

並把這個結果透過：

```text
nav_msgs/Odometry
+
TF：odom → base_footprint / base_link
```

提供給其他 ROS Node 與 RViz 使用。

---

## 2. 什麼是 Odom（里程計）

Odom 是根據機器人自身的運動資訊，估計短時間內的相對位移與朝向。

常見資訊來源包括：

```text
輪速
Encoder 編碼器
IMU（可輔助）
```

里程計常用來估計：

```text
x

y

yaw
```

也就是：

```text
前後移動多少
左右偏移多少
車頭轉了多少
```

要先記住：

> **Odom 是相對運動估計，不等於全域真實定位。**

---

## 3. Odom 的基本資料流程

可以先把資料流理解成：

```text
輪速 / Encoder
      ↓
IMU（可輔助）
      ↓
里程計計算
      ↓
估計 x / y / yaw
      ↓
發布 nav_msgs/Odometry
      ↓
發布 TF：odom → base_footprint / base_link
```

也就是：

```text
感測器 / 底盤資料
→ 運動模型
→ Odom
```

---

## 4. 為什麼輪子可以估計移動距離

最基本概念是：

```text
輪子轉過的圈數 × 車輪周長 ≈ 移動距離
```

公式：

```text
d = N × (2πr)
```

其中：

```text
d = 移動距離
N = 車輪轉過的圈數
r = 車輪半徑
```

實車上通常不會直接數「幾圈」，而是透過 Encoder 讀取馬達或車輪的旋轉量，再換算成距離與速度。

---

## 5. Odom 不是全域定位

Odom 的特色是：

```text
短時間內
連續
平滑
更新快
```

但缺點是會慢慢累積誤差。

例如：

```text
實際位置
------------

Odom 估計位置
----------
       \____
            \____
```

時間越久，兩者可能越來越不一致。

所以：

> **Odom 很適合描述短時間、連續的運動變化，但不能單獨當成長時間全域定位。**

---

## 6. 為什麼 Odom 會累積誤差

常見原因：

### 6.1 輪胎打滑

例如：

```text
Encoder 顯示輪子轉了
但車體實際沒有完全移動那麼多
```

---

### 6.2 地面不平

地面高低差、摩擦係數不同，都可能讓理論輪速和實際位移不一致。

---

### 6.3 Encoder 誤差

Encoder 本身有解析度限制、量測誤差與安裝誤差。

---

### 6.4 時間累積

Odom 是一步一步積分出來的。

每一小步只要有一點誤差：

```text
小誤差
+
小誤差
+
小誤差
+
...
↓
長時間後變成明顯漂移
```

---

## 7. odom 座標系是什麼

`odom` 是 ROS 常見的里程計參考座標系。

它通常表示：

> 機器人從某個起始狀態開始，根據里程計持續累積出的相對位置參考。

常見 TF 關係：

```text
odom
↓
base_footprint
↓
base_link
```

或某些系統直接使用：

```text
odom
↓
base_link
```

實際要看機器人的 TF 設計。

---

## 8. base_footprint 的角色

`base_footprint` 通常代表機器人在地面上的 2D 投影基準。

常見理解：

```text
x

y

yaw
```

通常：

```text
z = 0
```

它適合描述平面移動機器人的地面位置。

---

## 9. odom → base_footprint 的意義

這條 TF：

```text
odom → base_footprint
```

是在告訴 ROS：

> `base_footprint` 目前相對於 `odom` 在哪裡、朝哪個方向。

也就是：

```text
translation
→ x / y / z

rotation
→ orientation / yaw
```

所以機器人移動時：

```text
odom frame
通常保持作為參考

base_footprint
跟著機器人位置改變
```

---

## 10. nav_msgs/Odometry 是什麼

ROS 裡常用的里程計訊息格式：

```text
nav_msgs/Odometry
```

主要內容可以分成：

```text
Header
child_frame_id
Pose
Twist
```

---

## 11. Header

```text
header.stamp
header.frame_id
```

常見情況：

```text
header.frame_id = "odom"
```

代表這筆里程計資料是以 `odom` 為參考座標系。

---

## 12. child_frame_id

常見：

```text
child_frame_id = "base_footprint"
```

或：

```text
child_frame_id = "base_link"
```

表示這筆 Odom 描述的是：

```text
機器人本體
相對於
odom
```

的位置與速度。

---

## 13. Pose

`pose` 描述：

```text
位置
+
姿態
```

常見內容：

```text
pose.pose.position.x
pose.pose.position.y
pose.pose.position.z

pose.pose.orientation.x
pose.pose.orientation.y
pose.pose.orientation.z
pose.pose.orientation.w
```

其中 `orientation` 仍然是 Quaternion。

所以前面學過的：

```text
Quaternion → Roll / Pitch / Yaw
```

在 Odom 這裡也會再次用到。

---

## 14. Twist

`twist` 描述機器人目前的速度。

常見：

```text
twist.twist.linear.x
```

表示前後線速度。

```text
twist.twist.angular.z
```

表示繞 Z 軸的角速度，也就是平面車體常用的旋轉速度。

所以：

```text
Pose
→ 現在在哪裡、朝哪裡

Twist
→ 現在移動多快、轉多快
```

---

## 15. Covariance

`nav_msgs/Odometry` 裡也會有 covariance。

用途仍然是描述：

```text
這個位置 / 姿態 / 速度估計
到底有多可靠
```

目前先理解用途即可，不在本章深入矩陣計算。

---

## 16. 本章實際驗證方向

本章不另外建立新的 SLAM 套件。

直接使用目前已有的 Gazebo 移動機器人範例，觀察系統既有的 `/odom`。

先檢查：

```bash
rostopic list | grep odom
```

再查看：

```bash
rostopic info /odom
```

確認 Message Type 是否為：

```text
nav_msgs/Odometry
```

---

## 17. 查看一筆 Odom 資料

```bash
rostopic echo -n 1 /odom
```

重點觀察：

```text
header.frame_id
child_frame_id
pose.pose.position
pose.pose.orientation
twist.twist.linear
twist.twist.angular
```

---

## 18. 控制機器人後再看 Odom

可以一邊控制機器人移動，一邊觀察：

```bash
rostopic echo /odom
```

應該可以看到：

```text
position.x
position.y
orientation
linear.x
angular.z
```

隨著機器人移動而變化。

---

## 19. RViz 驗證

RViz 可以把 Fixed Frame 設成：

```text
odom
```

再觀察 RobotModel / TF。

如果 Odom 與 TF 都正常，機器人移動時應該會看到：

```text
odom
作為參考座標

base_link / base_footprint
隨車體移動
```

這正好可以跟第 11 章 TF 的概念接起來。

---

## 20. 建議實際查看 TF

可使用：

```bash
rosrun tf tf_echo odom base_link
```

若系統使用 `base_footprint`，則改成：

```bash
rosrun tf tf_echo odom base_footprint
```

觀察：

```text
Translation
Rotation
```

怎麼隨著機器人移動改變。

---

## 21. Odom 與 TF 的關係

可以先記成：

```text
nav_msgs/Odometry
→ 提供里程計資料

TF
→ 提供座標系之間的關係
```

很多系統會同時提供：

```text
/odom
+
/tf
```

其中 `/odom` 給程式讀數值，TF 給整個 ROS 座標系統使用。

---

## 22. Odom 與 map 的差別

這是後面進 Cartographer 前一定要先分清楚的。

```text
odom
→ 短時間連續、平滑
→ 會漂移

map
→ 全域地圖參考座標
→ 後續透過 SLAM / Localization 修正全域位置
```

所以未來常見的 TF Tree：

```text
map
↓
odom
↓
base_footprint
↓
base_link
↓
laser_link / imu_link
```

---

## 23. 為什麼後面還需要 SLAM

因為 Odom 不能自己消除長時間誤差。

例如：

```text
車實際回到原點
```

Odom 可能認為：

```text
還差 0.3 m
角度也偏了幾度
```

這時就需要像 Cartographer 這類 SLAM / Localization 系統，透過 LiDAR 與環境特徵去做全域修正。

---

## 24. 與 Cartographer 的銜接

完成本章後，要先建立這個概念：

```text
Wheel / Encoder
↓
Odom
↓
odom frame
```

以及：

```text
LiDAR /scan
+
TF
+
Odom
↓
Cartographer
↓
建立 / 修正 map 座標系
```

本章只把 Odom 學清楚，不提前展開 Cartographer 細節。

---

## 25. 本章完成標準

完成本章後，應該可以回答：

```text
□ Odom 是什麼？
□ 為什麼 Odom 會漂移？
□ odom frame 是什麼？
□ base_footprint / base_link 跟 odom 是什麼關係？
□ nav_msgs/Odometry 裡有什麼？
□ Pose 與 Twist 分別代表什麼？
□ 為什麼 Odom 會同時搭配 TF？
□ 為什麼 Odom 不能取代 SLAM？
□ 怎麼用 rostopic 查看 /odom？
□ 怎麼用 RViz / tf_echo 驗證 odom → base_link？
```

---

# 26. 本章最重要的一句話

```text
Odom 告訴機器人：
「我相對於剛才的位置，移動了多少。」

它很適合描述短時間、連續的運動，
但會隨時間累積誤差，所以後面仍需要 SLAM / Localization 做全域修正。
```

---

# 27. 下一步

下一階段再正式把：

```text
LiDAR
TF
Odom
OccupancyGrid
```

串起來，進入 Cartographer 2D SLAM / Localization 主線。
