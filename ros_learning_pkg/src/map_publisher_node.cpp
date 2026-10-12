#include <ros/ros.h>
#include <nav_msgs/OccupancyGrid.h>

int main(int argc, char **argv)
{
    // 初始化 ROS，並將這個 Node 命名為 map_publisher_node。
    ros::init(argc,argv,"map_publisher_node");
    ros::NodeHandle n;

    // 建立 Publisher：
    // Topic 是 /map，Message Type 是 nav_msgs/OccupancyGrid，Queue Size 是 10。
    ros::Publisher map_pub =
        n.advertise<nav_msgs::OccupancyGrid>("/map",10);

    // 每秒發布 1 次（1 Hz）
    ros::Rate rate(1);

    while(ros::ok())
    {
        nav_msgs::OccupancyGrid map_msg;

        // Header：這張地圖屬於 map 座標系，時間是現在
        map_msg.header.frame_id = "map";
        map_msg.header.stamp = ros::Time::now();

        // resolution：1 格 = 1 m × 1 m
        // width：X 方向 4 格（4 欄），height：Y 方向 2 格（2 列）
        map_msg.info.resolution = 1.0;
        map_msg.info.width =4;
        map_msg.info.height =2;

        // origin：地圖格子 (0,0) 的角落，放在 map 座標系的哪裡（單位：公尺）
        map_msg.info.origin.position.x = 0.0;
        map_msg.info.origin.position.y = 0.0;
        map_msg.info.origin.position.z = 0.0;

        // origin 的方向（Quaternion）：x=y=z=0、w=1 代表沒有旋轉
        map_msg.info.origin.orientation.x = 0.0;
        map_msg.info.origin.orientation.y = 0.0;
        map_msg.info.origin.orientation.z = 0.0;
        map_msg.info.origin.orientation.w = 1.0;

        // data 是一維陣列，總格數 = width × height = 8
        map_msg.data.resize(4 * 2);

        // row-major：index = y * width + x
        // 0 = 可通行、100 = 障礙物、-1 = 未知
        // 第 0 列（y = 0，RViz 裡靠近原點、在下面那一列）
        map_msg.data[0] = 100;
        map_msg.data[1] = 100;
        map_msg.data[2] = 0;
        map_msg.data[3] = -1;

        // 第 1 列（y = 1，RViz 裡在上面那一列）
        map_msg.data[4] = 0;
        map_msg.data[5] = 0;
        map_msg.data[6] = 0;
        map_msg.data[7] = 0;

        map_pub.publish(map_msg);

        // 睡到下一個 1 秒週期
        rate.sleep();
    }

    return 0;
}
