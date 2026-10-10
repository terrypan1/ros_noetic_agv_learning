#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <geometry_msgs/Twist.h>

#include <algorithm>
#include <clocale>
#include <cmath>

// 安全距離：前方障礙物比這個近，就停止前進並轉向。
const float kSafeDistance = 1.0f;

// 淨空距離：轉向時要轉到前方超過這個距離才恢復前進。
// 比安全距離大（遲滯），避免剛好轉到 1.0 m 就停，在「轉向／前進」之間來回切換。
const float kClearDistance = 1.5f;

// 車道半寬：車身半寬 0.25 m + 餘裕 0.10 m。
// 只要障礙物落在小車前方這條寬 0.7 m 的通道內，就會撞到。
const float kHalfWidth = 0.35f;

// 淨空通道半寬：轉向時，要連這條更寬的通道（寬 1.2 m）都淨空才恢復前進。
// 讓車子轉得更徹底，經過障礙物旁邊時留更多空間。
const float kClearHalfWidth = 0.60f;

// 車身尾端在 laser_link 後方多遠：LiDAR 在車體中心前 0.3 m，車體半長 0.4 m → 0.7 m。
// 車身旁邊的點也要檢查，否則斜著擦過障礙物時，角落跑到 LiDAR 旁邊就看不到了。
const float kBodyRear = 0.70f;

// 目前是否正在轉向（要記住狀態，才能做遲滯）。
bool g_turning = false;

// 前進速度（m/s）與轉向角速度（rad/s）。
const float kForwardSpeed = 0.10f;
const float kTurnSpeed = 0.30f;

// Callback 裡要發布 /cmd_vel，所以 Publisher 放在全域。
ros::Publisher vel_pub;

// 取得小車前方車道（左右各 half_width）內，最近障礙物的前方距離。
// 用車道而不是角度，因為角度範圍越近越窄，會漏掉車身兩側會撞到的東西。
float getFrontDistance(const sensor_msgs::LaserScan::ConstPtr& msg, float half_width)
{
    float min_dist = INFINITY;

    for (std::size_t i = 0; i < msg->ranges.size(); ++i)
    {
        float dist = msg->ranges[i];

        // 過濾無效值：NaN、inf（沒打到東西）、小於 range_min。
        if (!std::isfinite(dist) || dist < msg->range_min)
        {
            continue;
        }

        // 每個 index 對應的角度 = angle_min + index × angle_increment
        float angle = msg->angle_min + i * msg->angle_increment;

        // 極座標 → 直角座標（laser_link：x 朝前、y 朝左）
        float x = dist * std::cos(angle);
        float y = dist * std::sin(angle);

        // 只看車道寬度內、從車尾到前方的點（車身旁邊的點也算）。
        if (x < -kBodyRear || std::fabs(y) > half_width)
        {
            continue;
        }

        // 用「往前還有多遠會撞到」（x）比較，而不是斜線距離 dist。
        // 車身旁邊的點（x <= 0）代表已經貼著車身，距離當作 0。
        float ahead = std::max(x, 0.0f);

        if (ahead < min_dist)
        {
            min_dist = ahead;
        }
    }

    return min_dist;
}

void lidarCallback(const sensor_msgs::LaserScan::ConstPtr& msg)
{
    // 一般車道：判斷要不要開始轉向。
    float front_distance = getFrontDistance(msg, kHalfWidth);

    // 較寬的通道：判斷轉向是否可以結束。
    float clear_distance = getFrontDistance(msg, kClearHalfWidth);

    geometry_msgs::Twist vel_cmd;

    // 遲滯：前進中 < 1.0 m 才開始轉；轉向中要較寬的通道 > 1.5 m 才停止轉向。
    if (!g_turning && front_distance < kSafeDistance)
    {
        g_turning = true;
    }
    else if (g_turning && clear_distance > kClearDistance)
    {
        g_turning = false;
    }

    if (g_turning)
    {
        // 前方太近：不再往前，原地左轉。
        vel_cmd.linear.x = 0.0;
        vel_cmd.angular.z = kTurnSpeed;

        ROS_INFO_THROTTLE(
            1.0,
            "[避障] 前方 %.2f m → 轉向（轉到 > %.1f m）",
            clear_distance,
            kClearDistance);
    }
    else
    {
        // 前方安全：直線前進。
        vel_cmd.linear.x = kForwardSpeed;
        vel_cmd.angular.z = 0.0;

        ROS_INFO_THROTTLE(
            1.0,
            "[避障] 前方 %.2f m，安全 → 前進",
            front_distance);
    }

    // 將速度命令發布到 /cmd_vel，交給 Gazebo diff drive plugin 控制左右輪。
    vel_pub.publish(vel_cmd);
}

int main(int argc, char **argv)
{
    // 使用系統的 locale（UTF-8），讓 ROS_INFO 可以正確顯示中文，不會變成 ???。
    setlocale(LC_ALL, "");

    // 初始化 ROS，並將這個 Node 命名為 lidar_avoidance。
    ros::init(argc, argv, "lidar_avoidance");
    ros::NodeHandle nh;

    // Publisher：發布 geometry_msgs/Twist 到 /cmd_vel。
    vel_pub = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 10);

    // Subscriber：訂閱 /scan，每收到一筆 LaserScan 就判斷一次。
    ros::Subscriber lidar_sub =
        nh.subscribe("/scan", 10, lidarCallback);

    ROS_INFO("lidar_avoidance started, waiting for /scan ...");

    // 這個 Node 的速度命令都在 Callback 裡發布，
    // 所以 /scan 的頻率（10 Hz）就是 /cmd_vel 的發布頻率。
    ros::spin();

    return 0;
}
