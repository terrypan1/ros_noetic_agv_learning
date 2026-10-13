#include <ros/ros.h>
#include <nav_msgs/Odometry.h>

// Callback 裡要發布 /odom_filtered，所以 Publisher 放在全域。
ros::Publisher odom_pub;

// 上一筆「有通過」的 Odometry timestamp，初始值 0。
ros::Time last_stamp;

// 每收到一筆 /odom，就檢查 timestamp 是否比上一筆新。
void odomCallback(const nav_msgs::Odometry::ConstPtr &msg)
{
    // stamp <= last_stamp：timestamp 重複或倒退，直接丟掉。
    // Cartographer 要求 Odometry 的時間必須嚴格遞增，否則會 Check failed 而死亡。
    if (msg->header.stamp <= last_stamp)
    {
        ROS_WARN(
            "[Odom Filter] drop /odom, stamp: %.9f <= last_stamp: %.9f",
            msg->header.stamp.toSec(),
            last_stamp.toSec());
        return;
    }

    // stamp > last_stamp：原封不動發布到 /odom_filtered，不修改 timestamp。
    odom_pub.publish(msg);
    last_stamp = msg->header.stamp;
}

int main(int argc, char **argv)
{
    // 初始化 ROS，並將這個 Node 命名為 odom_timestamp_filter_node。
    ros::init(argc, argv, "odom_timestamp_filter_node");
    ros::NodeHandle nh;

    // Publisher：發布 nav_msgs/Odometry 到 /odom_filtered。
    odom_pub = nh.advertise<nav_msgs::Odometry>("/odom_filtered", 10);

    // Subscriber：訂閱 Gazebo diff drive plugin 發布的 /odom。
    ros::Subscriber odom_sub = nh.subscribe("/odom", 10, odomCallback);

    ros::spin();

    return 0;
}
