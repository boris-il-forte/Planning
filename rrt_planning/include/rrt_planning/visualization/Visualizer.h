/*
 * rrt_planning,
 *
 *
 * Copyright (C) 2016 Davide Tateo
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

#ifndef INCLUDE_RRT_PLANNING_VISUALIZATION_VISUALIZER_H_
#define INCLUDE_RRT_PLANNING_VISUALIZATION_VISUALIZER_H_

#include <ros/ros.h>
#include <Eigen/Dense>
#include <geometry_msgs/PoseStamped.h>


namespace rrt_planning
{

class Visualizer
{
    typedef std::pair<Eigen::VectorXd, Eigen::VectorXd> Segment;

public:
    Visualizer();

    void addPoint(const Eigen::VectorXd& point);
    void addSegment(const Eigen::VectorXd& start, const Eigen::VectorXd& end);
    void displayPlan(const std::vector<geometry_msgs::PoseStamped>& plan);

    void displayPoints();
    void displaySegments();

    void flush();

    void clean();

private:
    ros::Publisher pub;

    std::vector<Eigen::VectorXd> points;
    std::vector<Segment> segments;

private:
    static const unsigned int minPoints = 100;
    static const unsigned int minSegments = 100;

};

}

#endif /* INCLUDE_RRT_PLANNING_VISUALIZATION_VISUALIZER_H_ */
