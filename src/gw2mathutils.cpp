/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/

#include "gw2mathutils.h"
#include "gw2api/gw2api.h"
using namespace Gw2Api;

bool getClosestWaypoint(const Vector3D& characterPosition, int map_id, PointOfInterestEntry* waypoint, double* waypointDistance) {
	ApiInnerResponseObject<MapsRootEntry, MapEntry> map;
	getMap(map_id, &map);

	Gw2Position mapPosition = Gw2Position(characterPosition, Gw2Position::Mumble, map_id, map.value.map_rect, map.value.continent_rect).toMapPosition();
	Gw2Position continentPosition = mapPosition.toContinentPosition();

	Vector2D position2D = continentPosition.position.toVector2D();
	double currentDistance;
	for (unsigned i = 0; i < map.value.floors.size(); i++) {
		int floor = map.value.floors[i];
		MapFloorRootEntry mapFloorRoot;
		getMapFloor(map.value.continent_id, floor, &mapFloorRoot);
		MapFloorEntry mapFloor = mapFloorRoot.regions[map.value.region_id].maps[map_id];

		for (unsigned j = 0; j < mapFloor.points_of_interest.size(); j++) {
			if (mapFloor.points_of_interest[j].type == "waypoint") {
				if (waypoint->name.empty()) {
					*waypoint = mapFloor.points_of_interest[j];
					currentDistance = waypoint->coord.getDistance(position2D);
				}
				else {
					double distance = mapFloor.points_of_interest[j].coord.getDistance(position2D);
					if (distance < currentDistance) {
						*waypoint = mapFloor.points_of_interest[j];
						currentDistance = distance;
					}
				}
			}
		}
	}

	if (!waypoint->name.empty()) {
		*waypointDistance = mapPosition.position.toVector2D().getDistance(
			continentPosition.toMapPosition(waypoint->coord.toVector3D(0), Gw2Position::Continent).toVector2D());
		return true;
	}
	return false;
}
