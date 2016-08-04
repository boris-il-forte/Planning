/*
 * rtt_planning,
 *
 *
 * Copyright (C) 2016 Davide Tateo
 * Versione 1.0
 *
 * This file is part of rtt_planning.
 *
 * rtt_planning is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rtt_planning is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with rtt_planning.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDE_RTT_PLANNING_RTT_RTT_H_
#define INCLUDE_RTT_PLANNING_RTT_RTT_H_

#include "rtt_planning/rtt/RTTNode.h"
#include "rtt_planning/distance/Distance.h"

class RTT
{
public:
	RTT(Distance& distance, Eigen::VectorXd& x0);

	RTTNode* searchNearestNode(Eigen::VectorXd& x);
	void addNode(RTTNode* parent, Eigen::VectorXd& xNew);


private:
	RTTNode* root;
	std::vector<RTTNode*> nodes;
	Distance& distance;
};


#endif /* INCLUDE_RTT_PLANNING_RTT_RTT_H_ */
