#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>

#include <clocale>

// 每次 /scan 有新資料時，ROS 會呼叫這個 Callback。
// msg 裡面就是最新一筆 sensor_msgs/LaserScan。
void lidarCallback(const sensor_msgs::LaserScan::ConstPtr& msg)
{
    // 第一階段：先確認 Subscriber 真的收到 /scan，並看懂 LaserScan 欄位。
    // LiDAR 是 10 Hz，用 THROTTLE 每 2 秒印一次，避免塞滿 Terminal。
    ROS_INFO_THROTTLE(2.0, "---------------- LaserScan ----------------");
    ROS_INFO_THROTTLE(2.0, "frame_id        = %s", msg->header.frame_id.c_str());
    ROS_INFO_THROTTLE(2.0, "ranges size     = %zu", msg->ranges.size());
    ROS_INFO_THROTTLE(2.0, "angle_min       = %f rad", msg->angle_min);
    ROS_INFO_THROTTLE(2.0, "angle_max       = %f rad", msg->angle_max);
    ROS_INFO_THROTTLE(2.0, "angle_increment = %f rad", msg->angle_increment);
    ROS_INFO_THROTTLE(2.0, "range_min       = %f m", msg->range_min);
    ROS_INFO_THROTTLE(2.0, "range_max       = %f m", msg->range_max);

    // 第二階段：取得正前方距離（常見做法：取陣列中間）。
    // 本專案 LiDAR 掃描 -180° ~ +179°，所以中間 index 對應 0°，也就是正前方。
    int nNum = msg->ranges.size();
    int nMid = nNum / 2;
    float fMidDist = msg->ranges[nMid];

    // 用 angle_min + index × angle_increment 驗證這個 index 真的是 0°。
    float fMidAngle = msg->angle_min + nMid * msg->angle_increment;

    // 前方沒有東西時，Gazebo 會回傳 inf（超出 range_max）。
    ROS_INFO_THROTTLE(
        2.0,
        "前方測距 ranges[%d]（角度 %.2f rad）= %f 米",
        nMid,
        fMidAngle,
        fMidDist);
}

int main(int argc, char **argv)
{
    // 使用系統的 locale（UTF-8），讓 ROS_INFO 可以正確顯示中文，不會變成 ???。
    setlocale(LC_ALL, "");

    // 初始化 ROS，並將這個 Node 命名為 lidar_scan_subscriber。
    ros::init(argc, argv, "lidar_scan_subscriber");
    ros::NodeHandle nh;

    // 建立 Subscriber：
    // Topic 是 /scan，Message Type 是 sensor_msgs/LaserScan，Queue Size 是 10。
    ros::Subscriber lidar_sub =
        nh.subscribe("/scan", 10, lidarCallback);

    ROS_INFO("lidar_scan_subscriber started, waiting for /scan ...");

    // 持續等待 /scan，有資料就呼叫 lidarCallback()。
    ros::spin();

    return 0;
}
