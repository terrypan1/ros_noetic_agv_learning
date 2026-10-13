include "map_builder.lua"
include "trajectory_builder.lua"

options = {
  map_builder = MAP_BUILDER,
  trajectory_builder = TRAJECTORY_BUILDER,

  map_frame = "map",
  tracking_frame = "base_link",

  -- odom -> base_link 已經由 Gazebo 差速驅動 Plugin 發布，
  -- 所以 Cartographer 只負責發布 map -> odom
  published_frame = "odom",
  odom_frame = "odom",
  provide_odom_frame = false,
  publish_frame_projected_to_2d = false,
  use_pose_extrapolator = true,

  -- 使用 /odom，不使用 GPS 與 Landmark
  use_odometry = true,
  use_nav_sat = false,
  use_landmarks = false,

  -- 一顆 2D LiDAR，訂閱 /scan
  num_laser_scans = 1,
  num_multi_echo_laser_scans = 0,
  num_subdivisions_per_laser_scan = 1,
  num_point_clouds = 0,

  lookup_transform_timeout_sec = 0.2,
  submap_publish_period_sec = 0.3,
  pose_publish_period_sec = 5e-3,
  trajectory_publish_period_sec = 30e-3,

  rangefinder_sampling_ratio = 1.,
  odometry_sampling_ratio = 1.,
  fixed_frame_pose_sampling_ratio = 1.,
  imu_sampling_ratio = 1.,
  landmarks_sampling_ratio = 1.,
}

MAP_BUILDER.use_trajectory_builder_2d = true

-- 第一版不使用 IMU
TRAJECTORY_BUILDER_2D.use_imu_data = false

-- 配合 URDF 內 LiDAR 的量測範圍 0.12 ~ 10.0 m
TRAJECTORY_BUILDER_2D.min_range = 0.12
TRAJECTORY_BUILDER_2D.max_range = 10.0

return options