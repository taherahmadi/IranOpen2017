#include <cstdlib>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Point.h>
#include <nav_msgs/Odometry.h>
#include <rail_object_detector/Detections.h>
#include <rail_object_detector/Object.h>
#include <std_msgs/String.h>
#include <tf/transform_datatypes.h>
#define PI 3.14159265
  std::vector<float> range;
  double dist = -1;
  double robotX = 0;
  double robotY = 0;
  ros::Publisher victPub;
  double yaw_angle = 0;

  struct victLocation {
    double x,y;
  };
  std::vector<victLocation> victims;
   void send_message(victLocation vl){
      ROS_INFO("VVVVIIIICCCCTTTTIIIIMMMM");
      geometry_msgs::Point point;
      point.x = vl.x;
      point.y = vl.y;
      ROS_INFO("x = %f, y = %f ",vl.x, vl.y);
      victPub.publish(point);
    }
    double computeDistance(int left,int middle){
    double min = 30.0;
    ROS_INFO("left %d, middle %d ",left,middle);
    for (int i = left; i <= middle; i++)
      if((range[i] <= min)&&(range[i] >= 1))
        min = range[i];
    ROS_INFO("distance calculated!  \n");
    return min;
  }
  victLocation getVictimLocation(){
    if (dist == -1)
      ROS_INFO("dist = -1 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
    double victX = robotX + dist * cos(yaw_angle*PI / 180);
    double victY = robotY + dist * sin(yaw_angle*PI / 180);
    victLocation vl;
    vl.x = victX;
    vl.y = victY;
    ROS_INFO("victim location is :  \n");
    return vl;
  }
  bool checkVictim(){
      victLocation vl = getVictimLocation();
      for (std::vector<victLocation>::iterator it = victims.begin() ; it != victims.end(); ++it)
          if((( vl.x  < it->x + 0.5)&&( vl.x > it->x - 0.5))&&(( vl.y < it->y + 0.5)&&( vl.y > it->y - 0.5))){
            ROS_INFO("checkVictim resulted false! \n");
            return false;
    }
    ROS_INFO("checkVictim resulted true! \n");
    return true;
  }

  void processLaserScanCallback(const sensor_msgs::LaserScan::ConstPtr& scan){
       range = scan->ranges;
  }
  void processDetectionsCallback(const rail_object_detector::Detections::ConstPtr& detect){
    ROS_INFO("detections called! \n");
     std::vector<rail_object_detector::Object> obi = detect->objects;
    if(obi.size() > 0)
    {
      if (obi[0].label == "Human"){
        ROS_INFO("object found! \n");
        int xOfVictimFromMiddle = obi[0].centroid_x - 140;
        int middleBeam = (double)(((double)xOfVictimFromMiddle / 280) * 720 * (62.4/260)) + (double)360;
        int left = obi[0].left_bot_x - 140;
        int leftBeam = (double)(((double)left / 280) * 720 * (62.4/260)) + (double)360;
        ROS_INFO("left %d, xOfVictimFromMiddle %d ",left,xOfVictimFromMiddle);


        dist = computeDistance(leftBeam,middleBeam);
        ROS_INFO("distance is : %f",dist);
        if (checkVictim()){
           ROS_INFO("new victim! \n");
          victims.push_back(getVictimLocation());
          send_message(getVictimLocation());
          /** MARKER SHOULD BE HERE */
          return;
        }
    }
    else{
      dist = -1;
      ROS_INFO("dist seted on -1 ! \n");
      }
    }
    ROS_INFO("size !> 0    \n");
  }
  void robotPoseInitialCallback(const nav_msgs::Odometry::ConstPtr& msg){
    robotX = msg->pose.pose.position.x;
    robotY = msg->pose.pose.position.y;
    tf::Pose pose;
    tf::poseMsgToTF(msg->pose.pose, pose);
    yaw_angle = tf::getYaw(pose.getRotation());
  }
  // void firstInit(){
  //   ros::NodeHandle nh;
  //   ros::Subscriber scanSub;
  //   ros::Subscriber victSub;
  //   ros::Subscriber robotPoseSub;
  //   scanSub=nh.subscribe<sensor_msgs::LaserScan>("/sos1/base_scan",10,processLaserScanCallback);
  //   victSub=scanSub=nh.subscribe<rail_object_detector::Detections>("/detector_node/detections",10,processDetectionsCallback);
  //   robotPoseSub=nh.subscribe<nav_msgs::Odometry>("/sos1/odom",10,robotPoseInitialCallback);
  //   victPub = nh.advertise<std_msgs::String>("/victim_detected", 10);
  //   ROS_INFO("firstInit done! \n");
  // }
/** needs to be optimised */

void helloWorld(const rail_object_detector::Detections::ConstPtr& msg){
  ROS_INFO("HI!");
}
int main(int argc, char **argv)
{
  ros::init(argc, argv, "position_estimator");
  ros::Time::init();
  ros::Rate pub_rate(20);
  //firstInit();
  ROS_INFO("firstInit called! \n");
  ros::NodeHandle nh;
  ros::Subscriber scanSub;
  ros::Subscriber victSub;
  ros::Subscriber robotPoseSub;
  scanSub = nh.subscribe<sensor_msgs::LaserScan>("/sos1/hokuyo",10,processLaserScanCallback);
  victSub = nh.subscribe<rail_object_detector::Detections>("/detector_node/detections",10,processDetectionsCallback);
  //victSub = nh.subscribe<rail_object_detector::Detections>("/detector_node/detections",10,helloWorld);
  robotPoseSub = nh.subscribe<nav_msgs::Odometry>("/sos1/odom",10,robotPoseInitialCallback);
  victPub = nh.advertise<geometry_msgs::Point>("/victim_detected", 10);
  ROS_INFO("firstInit done! \n");
  while (ros::ok())
  {
    ros::spinOnce();
    pub_rate.sleep();
  }
  exit(0);
  return 0;
}
