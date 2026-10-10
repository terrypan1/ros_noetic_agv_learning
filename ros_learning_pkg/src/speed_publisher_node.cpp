#include <ros/ros.h>
#include <std_msgs/Float32.h>

#include <vector>

int main(int argc, char **argv)
{
    // 初始化 ROS，並將這個 Node 命名為 speed_publisher。
    ros::init(argc, argv, "speed_publisher");
    ros::NodeHandle nh;

    // 在 /agv_speed Topic 發布 std_msgs/Float32，佇列最多保留 10 筆訊息。
    ros::Publisher speed_pub =
        nh.advertise<std_msgs::Float32>("/agv_speed", 10);

    // 1 Hz 表示每秒發布一次。
    ros::Rate loop_rate(1);

    const std::vector<float> speeds = {
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

        speed_pub.publish(msg);
        ROS_INFO("[Publisher] Publish AGV speed: %.2f m/s", msg.data);

        index = (index + 1) % speeds.size();
        loop_rate.sleep();
    }

    return 0;
}
