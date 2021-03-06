/*
 * rrt_planning,
 *
 *
 * Copyright (C) 2016 Alessandro Riva, Davide Tateo
 * Versione 1.0
 *
 * This file is part of rrt_planning.
 *
 * rrt_planning is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rrt_planning is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with rrt_planning.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pluginlib/class_list_macros.h>
#include <visualization_msgs/Marker.h>

#include "rrt_planning/ThetaStarRRTPlanner.h"

#include "rrt_planning/extenders/MotionPrimitivesExtender.h"
#include "rrt_planning/map/ROSMap.h"
#include "rrt_planning/kinematics_models/DifferentialDrive.h"
#include "rrt_planning/utils/RandomGenerator.h"
#include "rrt_planning/rrt/RRT.h"

using namespace Eigen;

//register this planner as a BaseGlobalPlanner plugin
PLUGINLIB_EXPORT_CLASS(rrt_planning::ThetaStarRRTPlanner, nav_core::BaseGlobalPlanner)

using namespace std;

//Default Constructor
namespace rrt_planning
{

ThetaStarRRTPlanner::ThetaStarRRTPlanner()
{
    K = 0;
    deltaX = 0;
    laneWidth = 0;
    greedy = 0;
    deltaTheta = 0;

    map = nullptr;
    distance = nullptr;

    thetaStarPlanner = new ThetaStarPlanner();
}

ThetaStarRRTPlanner::ThetaStarRRTPlanner(std::string name, costmap_2d::Costmap2DROS* costmap_ros)
{
    initialize(name, costmap_ros);
    thetaStarPlanner = new ThetaStarPlanner(name, costmap_ros);
}


void ThetaStarRRTPlanner::initialize(std::string name, costmap_2d::Costmap2DROS* costmap_ros)
{
    thetaStarPlanner->initialize(name, costmap_ros);

    map = new ROSMap(costmap_ros);
    distance = new L2ThetaDistance();

    //Get parameters from ros parameter server
    ros::NodeHandle private_nh("~/" + name);

    private_nh.param("iterations", K, 30000);
    private_nh.param("deltaX", deltaX, 0.5);
    private_nh.param("laneWidth", laneWidth, 2.0);
    private_nh.param("greedy", greedy, 0.1);
    private_nh.param("deltaTheta", deltaTheta, M_PI/4);

    extenderFactory.initialize(private_nh, *map, *distance);
    visualizer.initialize(private_nh);
}

bool ThetaStarRRTPlanner::makePlan(const geometry_msgs::PoseStamped& start,
                                   const geometry_msgs::PoseStamped& goal,
                                   std::vector<geometry_msgs::PoseStamped>& plan)
{
    visualizer.clean();

    // Retrive Theta* plan
    vector<geometry_msgs::PoseStamped> thetaStarPlan;

    if(!thetaStarPlanner->makePlan(start, goal, thetaStarPlan))
    {
        ROS_INFO("Impossible to compute the Theta* plan");
        return false;
    }

    // Compute RRT-Theta* plan
    Distance& distance = *this->distance;

    VectorXd&& x0 = convertPose(start);
    VectorXd&& xGoal = convertPose(goal);

    RRT rrt(distance, x0);

    ROS_INFO("Theta*-RRT started");

    for(unsigned int i = 0; i < K; i++)
    {

        VectorXd xRand;

        if(RandomGenerator::sampleEvent(greedy))
            xRand = xGoal;
        else
            xRand = extenderFactory.getKinematicModel().sampleOnLane(thetaStarPlan, laneWidth, deltaTheta);

        visualizer.addPoint(xRand);

        auto* node = rrt.searchNearestNode(xRand);

        VectorXd xNew;

        if(newState(xRand, node->x, xNew))
        {
            rrt.addNode(node, xNew);

            visualizer.addSegment(node->x, xNew);

            if(distance(xNew, xGoal) < deltaX)
            {
                auto&& path = rrt.getPathToLastNode();
                publishPlan(path, plan, start.header.stamp);

                visualizer.displayPlan(plan);
                visualizer.flush();

                ROS_INFO("Plan found");

                return true;
            }
        }

    }

    visualizer.flush();

    ROS_WARN_STREAM("Failed to found a plan in " << K << " RRT iterations");
    return false;

}


bool ThetaStarRRTPlanner::newState(const VectorXd& xRand,
                                   const VectorXd& xNear,
                                   VectorXd& xNew)
{
    return extenderFactory.getExtender().compute(xNear, xRand, xNew);
}

VectorXd ThetaStarRRTPlanner::convertPose(const geometry_msgs::PoseStamped& msg)
{
    auto& q_ros = msg.pose.orientation;
    auto& t_ros = msg.pose.position;

    Quaterniond q(q_ros.w, q_ros.x, q_ros.y, q_ros.z);

    Vector3d theta = q.matrix().eulerAngles(0, 1, 2);

    VectorXd x(3);
    x << t_ros.x, t_ros.y, theta(2);

    return x;
}

void ThetaStarRRTPlanner::publishPlan(std::vector<VectorXd>& path,
                                      std::vector<geometry_msgs::PoseStamped>& plan, const ros::Time& stamp)
{
    for(auto x : path)
    {
        geometry_msgs::PoseStamped msg;

        msg.header.stamp = stamp;
        msg.header.frame_id = "map";

        msg.pose.position.x = x(0);
        msg.pose.position.y = x(1);
        msg.pose.position.z = 0;

        Matrix3d m;
        m = AngleAxisd(x(2), Vector3d::UnitZ())
            * AngleAxisd(0, Vector3d::UnitY())
            * AngleAxisd(0, Vector3d::UnitX());

        Quaterniond q(m);

        msg.pose.orientation.x = q.x();
        msg.pose.orientation.y = q.y();
        msg.pose.orientation.z = q.z();
        msg.pose.orientation.w = q.w();

        plan.push_back(msg);
    }
}


ThetaStarRRTPlanner::~ThetaStarRRTPlanner()
{
    if(distance)
        delete distance;

    if(map)
        delete map;

    if(thetaStarPlanner)
        delete thetaStarPlanner;

}


};

