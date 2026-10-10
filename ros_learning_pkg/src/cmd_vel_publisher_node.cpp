#include <ros/ros.h>
#include <geometry_msgs/Twist.h>

int main(int argc, char **argv)
{
    // 初始化 ROS，並將這個 Node 命名為 cmd_vel_publisher。
    ros::init(argc, argv, "cmd_vel_publisher");
    ros::NodeHandle nh;

    // 建立 Publisher：
    // Topic 是 /cmd_vel，Message Type 是 geometry_msgs/Twist，Queue Size 是 10。
    ros::Publisher cmd_vel_pub =
        nh.advertise<geometry_msgs::Twist>("/cmd_vel", 10);

    // 以 10 Hz（每秒 10 次）的頻率持續發布速度命令。
    ros::Rate loop_rate(10);

    while (ros::ok())
    {
        geometry_msgs::Twist cmd;

        // linear.x：前進／後退速度，單位是 m/s。
        // 0.10 代表每秒向前移動 0.10 公尺。
        cmd.linear.x = 0.10;

        // angular.z：左轉／右轉角速度，單位是 rad/s。
        // 目前設為 0.00，所以只會直行、不會旋轉。
        cmd.angular.z = 0.00;

        // 將 Twist Message 發布到 /cmd_vel。
        cmd_vel_pub.publish(cmd);

        // 每秒顯示一次，避免 10 Hz 的訊息塞滿 Terminal。
        ROS_INFO_THROTTLE(
            1.0,
            "[cmd_vel Publisher] linear.x: %.2f m/s, angular.z: %.2f rad/s",
            cmd.linear.x,
            cmd.angular.z);

        // 目前這個 Node 沒有 Subscriber Callback，因此發布並不依賴 spinOnce()。
        loop_rate.sleep();
    }

    return 0;
}
