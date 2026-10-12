#include <ros/ros.h>
#include <std_msgs/Float32.h>

// 每當 /robot_speed 收到新訊息時，ROS 就會呼叫這個 Callback。
void speedCallback(const std_msgs::Float32::ConstPtr &msg)
{
    ROS_INFO("[Subscriber] Receive robot speed: %.2f m/s", msg->data);
}

int main(int argc, char **argv)
{
    // 初始化 ROS，並將這個 Node 命名為 speed_subscriber。
    ros::init(argc, argv, "speed_subscriber");
    ros::NodeHandle nh;

    // 訂閱 /robot_speed，佇列最多保留 10 筆尚未處理的訊息。
    ros::Subscriber speed_sub =
        nh.subscribe("/robot_speed", 10, speedCallback);

    ROS_INFO("speed_subscriber started, waiting for /robot_speed ...");

    // 持續等待 ROS 訊息，收到後執行 Callback。
    ros::spin();

    return 0;
}
