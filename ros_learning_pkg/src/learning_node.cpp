#include <ros/ros.h>

int main(int argc, char **argv)
{
    // 初始化 ROS，並將這個 Node 命名為 learning_node
    ros::init(argc, argv, "learning_node");

    // 建立 NodeHandle，之後 Publisher、Subscriber、Service 等都會透過它建立
    ros::NodeHandle nh;

    ROS_INFO("learning_node started");

    // 讓 Node 持續運行，等待 ROS 事件
    ros::spin();

    return 0;
}