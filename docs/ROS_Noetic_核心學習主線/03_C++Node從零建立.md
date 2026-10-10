# 03 C++ Node 從零建立

## 目前狀態：準備開始

新的核心學習 Workspace 已建立：

```text
/root/ros_learning_ws
```

本章不再沿用舊練習環境中的：

```text
/root/catkin_ws/src/ssr_pkg
ultrasonic_node.cpp
```

那些內容保留作為過去練習操作紀錄，但不再當成「從零建立 C++ Node」的核心起點。

本章會使用新的 Workspace，從自己建立的 Package 開始，一步一步建立第一個 C++ ROS Node。

---

## 1. 本章要建立什麼

本章先使用：

```text
Package：ros_learning_pkg
Node 原始碼：basic_node.cpp
ROS Node 名稱：basic_node
```

這些名稱都是我們自己為學習取的，不是 ROS 固定要求。

最後希望得到：

```text
/root/ros_learning_ws
└── src
    └── ros_learning_pkg
        ├── CMakeLists.txt
        ├── package.xml
        └── src
            └── basic_node.cpp
```

---

## 2. 這個東西解決什麼問題

Node 是 ROS 中實際執行某個功能的程序。

但是要分清楚：

```text
一般 C++ executable
≠
ROS Node
```

例如只有：

```cpp
#include <stdio.h>

int main(int argc, char **argv)
{
    printf("Hello World\n");
    return 0;
}
```

這只是普通 C++ 程式。

即使它放在 ROS Package 裡、也被 `catkin_make` 編譯成功，仍不代表它已經初始化 ROS。

---

## 3. 第一階段：先建立普通 C++ executable

本章不直接跳到完整 ROS Node。

先理解最基本的鏈：

```text
.cpp 原始碼
↓
CMakeLists.txt
↓
catkin_make
↓
產生 executable
↓
執行程式
```

也就是先確認：

> Catkin Workspace 可以把我們 Package 裡的 C++ 原始碼編譯成真正可以執行的程式。

這一階段即使程式只有 `printf()` 也沒有問題。

---

## 4. 第二階段：加入 ROS C++ Header

接著加入：

```cpp
#include <ros/ros.h>
```

來源：

```text
[ROS 官方]
roscpp
```

這代表程式開始使用 ROS 的 C++ Client Library。

但只 `include <ros/ros.h>` 還不等於 ROS Node 已完成初始化。

---

## 5. 第三階段：加入 `ros::init()`

接著才加入：

```cpp
ros::init(argc, argv, "basic_node");
```

目前先這樣理解：

```text
ros::init(...)
↓
初始化 ROS C++ Client Library
↓
告訴 ROS 這個程式要以 basic_node 這個名稱進入 ROS 系統
```

完整的最小程式先做到：

```cpp
#include <ros/ros.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    ros::init(argc, argv, "basic_node");

    printf("basic_node started\n");

    return 0;
}
```

---

## 6. 目前先不要跳到 `NodeHandle`、Publisher、Subscriber

之前筆記直接放了：

```cpp
ros::NodeHandle nh;
ROS_INFO("ultrasonic_node started");
```

這對目前進度來說太快。

本次重建先停在：

```text
#include <ros/ros.h>
↓
main(argc, argv)
↓
ros::init(...)
↓
printf(...)
↓
return 0
```

先把「普通 C++ 程式怎麼變成開始使用 ROS 的程式」理解清楚。

後續才繼續加入：

```text
ros::NodeHandle
ROS logging
ros::spin
Publisher / Subscriber
```

不要一次全部塞進來。

---

## 7. `ros::init()` 不代表程式會一直執行

目前這個程式最後仍然有：

```cpp
return 0;
```

所以流程會是：

```text
啟動
↓
ros::init
↓
printf
↓
程式結束
```

因此這個階段不把：

```bash
rosnode list
```

能否長時間看到 `/basic_node` 當成主要驗證條件。

要讓 Node 保持執行，之後還需要學到 event loop、`ros::spin()` 或其他執行方式。

這部分後面再補。

---

## 8. `CMakeLists.txt` 在這裡扮演什麼角色

C++ 檔案放進 Package 不會自動變成 executable。

需要在 Package 的：

```text
CMakeLists.txt
```

告訴 Catkin / CMake：

```text
哪一個 .cpp 要被編譯
↓
產生哪一個 executable
↓
需要連結哪些 ROS library
```

本章至少會接觸：

```cmake
add_executable(basic_node src/basic_node.cpp)

target_link_libraries(basic_node
  ${catkin_LIBRARIES}
)
```

目前不用背 CMake 語法。

只要知道：

> C++ 原始碼本身不會自動被 Catkin 編譯成 Node，必須在 `CMakeLists.txt` 宣告。

---

## 9. `package.xml` 與 `roscpp` 的關係

建立 Package 時會宣告：

```text
roscpp
```

因此 Package 才知道它依賴 ROS C++ Client Library。

目前先建立這個關係：

```text
basic_node.cpp
↓ 使用
<ros/ros.h>
↓ 來自
roscpp
↓ Package 相依資訊
package.xml / CMakeLists.txt
```

後續學習 dependency 時再深入展開。

---

## 10. 本章實際操作順序

本章照下面順序做，不直接跳步：

```text
1. 在 ros_learning_ws 建立自己的 Package
↓
2. 看 Package 自動產生哪些檔案
↓
3. 建立 basic_node.cpp
↓
4. 先寫普通 C++ main + printf
↓
5. CMakeLists.txt 加入 executable
↓
6. catkin_make
↓
7. 確認普通 C++ executable 可執行
↓
8. 加入 <ros/ros.h>
↓
9. 加入 ros::init(...)
↓
10. 再重新編譯與執行
```

今天至少先完成到：

```cpp
ros::init(argc, argv, "basic_node");
```

---

## 11. Windows、WSL 與 Git 對應

正式學習筆記仍保存於 Windows。

WSL 執行環境：

```text
/root/ros_learning_ws/src
```

原則：

```text
Windows
→ Git 正本 / 長期保存

WSL
→ ROS 編譯 / 執行 / 測試
```

`build/` 與 `devel/` 不需要同步回 Windows Git。

---

## 12. 舊 `ssr_pkg` 與 `ultrasonic_node` 怎麼處理

舊的：

```text
/root/catkin_ws/src/ssr_pkg
/root/catkin_ws/src/ssr_pkg/src/ultrasonic_node.cpp
```

不刪除。

它們代表之前練習操作留下的學習痕跡。

但現在核心主線改成：

```text
/root/ros_learning_ws
↓
ros_learning_pkg
↓
basic_node.cpp
```

所以之後不要再用舊 `ssr_pkg` 的完成狀態，判定新核心主線已經學會。

---

## 13. 元件來源

```text
[ROS 官方 binary / library]
roscpp
<ros/ros.h>
ros::init

[自己建立]
ros_learning_pkg
basic_node.cpp
basic_node

[舊學習實作]
ssr_pkg
ultrasonic_node.cpp
```

---

## 14. 本章完成判定

本章完成時應能說明：

```text
普通 C++ executable 是什麼
↓
ROS Package 如何把 .cpp 編譯成 executable
↓
<ros/ros.h> 從哪裡來
↓
ros::init() 在做什麼
↓
Node 名稱是自己定義，不是 ROS 固定給的
```

實作上至少完成：

- [ ] 使用新的 `/root/ros_learning_ws`。
- [ ] 建立自己的 `ros_learning_pkg`。
- [ ] 建立 `basic_node.cpp`。
- [ ] 普通 C++ `printf()` 版本編譯成功。
- [ ] 加入 `<ros/ros.h>`。
- [ ] 加入 `ros::init(argc, argv, "basic_node")`。
- [ ] `catkin_make` 成功。
- [ ] executable 可以正常啟動並輸出文字。

目前不要求：

```text
Publisher
Subscriber
Topic 實作
ros::NodeHandle
ros::spin
完整 rosnode 長時間觀察
```

---

## 15. 下一步

完成本章的 `ros::init()` 階段後，再繼續補 C++ Node 的存活、ROS logging 與基本觀察。

確認 C++ Node 基礎完成後，才進入：

[04 Publisher、Subscriber 與 Topic](04_Publisher_Subscriber與Topic.md)
