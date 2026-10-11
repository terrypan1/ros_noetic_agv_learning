# 07 TF2、Frame 與座標系

## 目前狀態：在公司專案實戰接觸過，核心基礎待重建

已觀察 `map`、`odom`、`base_link`、`laser_link_1` 等 Frame，也使用 RViz 看過 TF；尚未自己從零建立 TF broadcaster／listener。

## 已知核心觀念

TF 表達不同座標系之間隨時間變化的關係。Topic 名稱 `/map` 與 Frame 名稱 `map` 不是同一件事。

## 待完成

- [ ] 自己建立 `base_link → laser_link` 靜態變換。
- [ ] 自己建立動態變換並查詢。
- [ ] 使用 TF 工具與 RViz 驗證方向、時間及父子關係。
- [ ] 再回公司案例對照 [RViz、RobotModel 與 TF](../ROS_Noetic_jq_mcs_ros1對照實戰/03_jq_mcs_ros1_RViz_RobotModel與TF.md)。
