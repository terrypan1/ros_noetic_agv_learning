#include <ros/ros.h>
#include <sensor_msgs/Imu.h>
#include <tf/tf.h>
#include <cmath>

// IMU Callback
void IMUCallback(const sensor_msgs::Imu &msg)
{
    // 如果 orientation_covariance[0] < 0
    // 代表這筆 IMU 沒有提供 orientation
    if (msg.orientation_covariance[0] < 0)
    {
        ROS_WARN("IMU orientation is not available.");
        return;
    }

    // 取出 Quaternion 四元數
    tf::Quaternion quaternion(
        msg.orientation.x,
        msg.orientation.y,
        msg.orientation.z,
        msg.orientation.w);

    // Quaternion 轉成 Roll / Pitch / Yaw（單位：弧度 rad）
    double roll;
    double pitch;
    double yaw;
    tf::Matrix3x3(quaternion).getRPY(roll, pitch, yaw);

    // 弧度轉角度
    roll = roll * 180.0 / M_PI;
    pitch = pitch * 180.0 / M_PI;
    yaw = yaw * 180.0 / M_PI;

    ROS_INFO_THROTTLE(
        0.5,
        "Roll: %.2f deg, Pitch: %.2f deg, Yaw: %.2f deg",
        roll,
        pitch,
        yaw
    );

    // 角速度，單位 rad/s（車子左右轉時，主要看 z）
    ROS_INFO_THROTTLE(
        0.5,
        "Angular Velocity: x = %.2f, y = %.2f, z = %.2f rad/s",
        msg.angular_velocity.x,
        msg.angular_velocity.y,
        msg.angular_velocity.z
    );

    // 線加速度，單位 m/s²（靜止時 z 約 9.8，是重力）
    ROS_INFO_THROTTLE(
        0.5,
        "Linear Acceleration: x = %.2f, y = %.2f, z = %.2f m/s^2",
        msg.linear_acceleration.x,
        msg.linear_acceleration.y,
        msg.linear_acceleration.z
    );
}

int main(int argc, char **argv)
{
    // 初始化 ROS，並將這個 Node 命名為 imu_subscriber_node。
    ros::init(argc, argv, "imu_subscriber_node");

    ros::NodeHandle n;

    // 建立 Subscriber：
    // Topic 是 /imu/data，Message Type 是 sensor_msgs/Imu，Queue Size 是 10。
    // 每收到一筆新資料，就呼叫 IMUCallback()。
    ros::Subscriber imu_subscriber_node =
        n.subscribe("/imu/data", 10, IMUCallback);

    ROS_INFO("IMU Subscriber Node started.");

    // 持續等待 /imu/data，有資料就呼叫 IMUCallback()。
    ros::spin();

    return 0;
}