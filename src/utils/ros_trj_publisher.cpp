#include "trajectory_utils/utils/ros_trj_publisher.h"
#include <boost/shared_ptr.hpp>
#include <ros/time.h>
#include <geometry_msgs/PoseStamped.h>

using namespace trajectory_utils;

trajectory_publisher::trajectory_publisher(const std::string& topic_name)
{
    _trj_publisher = _n.advertise<nav_msgs::Path>(topic_name, 1000);

    _decimation = 0.1;
    _decimation2 = 10;

    _has_traj = false;

}

void trajectory_publisher::deleteAllMarkersAndTrj()
{
    if(_has_traj){
        _visual_tools->deleteAllMarkers();


        _trj_msg.poses.clear();
        _trj_msg.header.stamp = ros::Time::now();

        _trj_publisher.publish(_trj_msg);}
}

void trajectory_publisher::setDecimation(const double decimation)
{
    _decimation = decimation;
}

void trajectory_publisher::setDecimation2(const int decimation2)
{
    _decimation2 = decimation2;
}

void trajectory_publisher::publish(bool delete_visual_tools)
{
    if(delete_visual_tools)
    {
        _visual_tools->deleteAllMarkers();

        nav_msgs::Path tmp;
        tmp.header.frame_id = _frame;
        tmp.header.stamp = ros::Time::now();
        _trj_publisher.publish(tmp);
    }

    _visual_tools->enableBatchPublishing(true);

    _trj_msg.header.frame_id = _frame;
    _trj_msg.header.stamp = ros::Time::now();
    _trj_publisher.publish(_trj_msg);

    _visual_tools->publishAxisLabeled(_trj_msg.poses[0].pose, "start",
            rviz_visual_tools::LARGE);
    for(unsigned int i = 0; i < _trj_msg.poses.size(); i += _decimation2)
        _visual_tools->publishAxis(_trj_msg.poses[i].pose, 0.1, 0.01);
    _visual_tools->publishAxisLabeled(_trj_msg.poses[_trj_msg.poses.size()-1].pose, "end",
            rviz_visual_tools::LARGE);

#if ROS_VERSION_MINIMUM(1,12,7) //Kinetic
    _visual_tools->triggerAndDisable();
#else
    _visual_tools->triggerBatchPublishAndDisable();
#endif
}

void trajectory_publisher::setTrj(const boost::shared_ptr<KDL::Trajectory_Composite> trj,
                                  const std::string& base_frame,
                                  const std::string& distal_frame)
{
    _frame = base_frame;

    _visual_tools.reset(new rviz_visual_tools::RvizVisualTools(_frame,"/trj_visual_markers_"+_frame+"_to_"+distal_frame));

    _trj_msg.poses.clear();

    geometry_msgs::PoseStamped T;
    KDL::Frame F;
    for(double t = 0; t <= trj->Duration(); t += _decimation)
    {
        F = trj->Pos(t);
        double x, y, z, w;
        F.M.GetQuaternion(x,y,z,w);

        T.pose.position.x = F.p.x();
        T.pose.position.y = F.p.y();
        T.pose.position.z = F.p.z();
        T.pose.orientation.x = x;
        T.pose.orientation.y = y;
        T.pose.orientation.z = z;
        T.pose.orientation.w = w;

        //T.header.frame_id = frame;
        //T.header.stamp = ros::Time::now();

        _trj_msg.poses.push_back(T);

    }
    //_trj_msg.header.frame_id = frame;
    //_trj_msg.header.stamp = ros::Time::now();

    _has_traj = true;
}
