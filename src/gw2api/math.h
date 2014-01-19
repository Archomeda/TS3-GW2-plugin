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

#pragma once
#include <math.h>

#define INCH_TO_METER 0.0254F

namespace Gw2Api {

	// Structs are not complete, only the things that are used within the plugin are implemented

	struct Vector2D;
	struct Vector3D;

	struct Vector2D {
		double x;
		double y;

		Vector2D() {
			x = 0;
			y = 0;
		}

		Vector2D(double x, double y) {
			this->x = x;
			this->y = y;
		}

		Vector3D toVector3D(int y) const;

		double getDistance(const Vector2D& v) {
			Vector2D distanceV = v - *this;
			return distanceV.getSize();
		}

		double getSize() {
			return sqrt(pow(x, 2) + pow(y, 2));
		}

		friend Vector2D operator+(const Vector2D& v, double d) {
			return Vector2D(v.x + d, v.y + d);
		}
		friend Vector2D operator+(const Vector2D& v1, const Vector2D& v2) {
			return Vector2D(v1.x + v2.x, v1.y + v2.y);
		}
		friend Vector2D operator-(const Vector2D& v, double d) {
			return Vector2D(v.x - d, v.y - d);
		}
		friend Vector2D operator-(const Vector2D& v1, const Vector2D& v2) {
			return Vector2D(v1.x - v2.x, v1.y - v2.y);
		}
		friend Vector2D operator*(const Vector2D& v, double scalar) {
			return Vector2D(v.x * scalar, v.y * scalar);
		}
		friend Vector2D operator*(const Vector2D& v, const Vector2D& scalar) {
			return Vector2D(v.x * scalar.x, v.y * scalar.y);
		}
		friend Vector2D operator/(const Vector2D& v, double scalar) {
			return Vector2D(v.x / scalar, v.y / scalar);
		}
		friend Vector2D operator/(const Vector2D& v, const Vector2D& scalar) {
			return Vector2D(v.x / scalar.x, v.y / scalar.y);
		}

		Vector2D operator+=(double d) {
			this->x += d;
			this->y += d;
			return *this;
		}
		Vector2D operator+=(const Vector2D& vector) {
			this->x += vector.x;
			this->y += vector.y;
			return *this;
		}
		Vector2D operator-=(double d) {
			this->x -= d;
			this->y -= d;
			return *this;
		}
		Vector2D operator-=(const Vector2D& vector) {
			this->x -= vector.x;
			this->y -= vector.y;
			return *this;
		}
		Vector2D operator*=(double scalar) {
			this->x *= scalar;
			this->y *= scalar;
			return *this;
		}
		Vector2D operator*=(const Vector2D& scalar) {
			this->x *= scalar.x;
			this->y *= scalar.y;
			return *this;
		}
		Vector2D operator/=(double scalar) {
			this->x /= scalar;
			this->y /= scalar;
			return *this;
		}
		Vector2D operator/=(const Vector2D& scalar) {
			this->x /= scalar.x;
			this->y /= scalar.y;
			return *this;
		}

		friend bool operator==(const Vector2D& v1, const Vector2D& v2) {
			return v1.x == v2.x && v1.y == v2.y;
		}
		friend bool operator!=(const Vector2D& v1, const Vector2D& v2) {
			return !(v1 == v2);
		}
	};

	struct Vector3D {
		double x;
		double y;
		double z;

		Vector3D() {
			x = 0;
			y = 0;
			z = 0;
		}

		Vector3D(double x, double y, double z) {
			this->x = x;
			this->y = y;
			this->z = z;
		}

		Vector2D toVector2D() const;

		friend Vector3D operator*(const Vector3D& v, double scalar) {
			return Vector3D(v.x * scalar, v.y * scalar, v.z * scalar);
		}
		friend Vector3D operator/(const Vector3D& v, double scalar) {
			return Vector3D(v.x / scalar, v.y / scalar, v.z / scalar);
		}

		Vector3D operator*=(double scalar) {
			this->x *= scalar;
			this->y *= scalar;
			this->z *= scalar;
			return *this;
		}
		Vector3D operator/=(double scalar) {
			this->x /= scalar;
			this->y /= scalar;
			this->z /= scalar;
			return *this;
		}

		friend bool operator==(const Vector3D& v1, const Vector3D& v2) {
			return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
		}
		friend bool operator!=(const Vector3D& v1, const Vector3D& v2) {
			return !(v1 == v2);
		}
	};

	struct Rect {
		Vector2D upperLeft;
		Vector2D bottomRight;

		Rect() { }

		Rect(Vector2D upperLeft, Vector2D bottomRight) {
			this->upperLeft = upperLeft;
			this->bottomRight = bottomRight;
		}

		double getWidth() { return abs(bottomRight.x - upperLeft.x); }
		double getHeight() { return abs(bottomRight.y - upperLeft.y); }
		Vector2D getSize() { return Vector2D(getWidth(), getHeight()); }

		friend Rect operator*(const Rect& r, double scalar) {
			return Rect(r.upperLeft * scalar, r.bottomRight * scalar);
		}
		friend Rect operator*(const Rect& r, Vector2D scalar) {
			return Rect(r.upperLeft * scalar, r.bottomRight * scalar);
		}
		friend Rect operator/(const Rect& r, double scalar) {
			return Rect(r.upperLeft / scalar, r.bottomRight / scalar);
		}
		friend Rect operator/(const Rect& r, Vector2D scalar) {
			return Rect(r.upperLeft / scalar, r.bottomRight / scalar);
		}

		Rect operator*=(double scalar) {
			this->upperLeft *= scalar;
			this->bottomRight *= scalar;
			return *this;
		}
		Rect operator*=(Vector2D scalar) {
			this->upperLeft *= scalar;
			this->bottomRight *= scalar;
			return *this;
		}
		Rect operator/=(double scalar) {
			this->upperLeft /= scalar;
			this->bottomRight /= scalar;
			return *this;
		}
		Rect operator/=(Vector2D scalar) {
			this->upperLeft /= scalar;
			this->bottomRight /= scalar;
			return *this;
		}
	};

	struct Gw2Position {
		enum Unit {
			Mumble, // Uses meters and inverted z-axis
			Map, // Uses inches
			Continent // Uses scaled "inches"
		};

		Vector3D position;
		int mapID;
		Unit unit;
		Rect mapRectangle;
		Rect continentRectangle;

		Gw2Position();

		Gw2Position(const Vector3D& position, const Unit& unit, int mapID, const Rect& mapRectangle, const Rect& continentRectangle) {
			this->position = position;
			this->unit = unit;
			this->mapID = mapID;
			this->mapRectangle = mapRectangle;
			this->continentRectangle = continentRectangle;
		}

		Gw2Position(double x, double y, double z, const Unit& unit, int mapID, const Rect& mapRectangle, const Rect& continentRectangle) {
			this->position = Vector3D(x, y, z);
			this->unit = unit;
			this->mapID = mapID;
			this->mapRectangle = mapRectangle;
			this->continentRectangle = continentRectangle;
		}

		Gw2Position toMumblePosition() {
			Gw2Position newPos = Gw2Position(*this);
			newPos.unit = Mumble;
			newPos.position = toMumblePosition(position, unit);
			return newPos;
		}

		Gw2Position toMapPosition() {
			Gw2Position newPos = Gw2Position(*this);
			newPos.unit = Map;
			newPos.position = toMapPosition(position, unit);
			return newPos;
		}

		Gw2Position toContinentPosition() {
			Gw2Position newPos = Gw2Position(*this);
			newPos.unit = Continent;
			newPos.position = toContinentPosition(position, unit);
			return newPos;
		}

		Vector3D toMumblePosition(Vector3D position, Unit oldUnit) {
			Vector3D newPos = position;
			switch (oldUnit) {
				case Map:
					newPos *= INCH_TO_METER;
					newPos.z = -newPos.z;
					break;

				case Continent:
					newPos = toMumblePosition(toMapPosition(newPos, Continent), Map);
					break;
			}
			return newPos;
		}

		Vector3D toMapPosition(Vector3D position, Unit oldUnit) {
			Vector3D newPos = position;
			switch (oldUnit) {
				case Mumble:					
					newPos /= INCH_TO_METER;
					newPos.z = -newPos.z;
					break;

				case Continent:
					Vector2D continentPos = Vector2D(position.x, position.z);
					Vector2D relativeContinentPos = (continentPos - continentRectangle.upperLeft) / continentRectangle.getSize();
					Vector2D mapPos = mapRectangle.upperLeft + relativeContinentPos * mapRectangle.getSize();
					newPos = Vector3D(mapPos.x, position.y, mapPos.y);
					break;
			}
			return newPos;
		}

		Vector3D toContinentPosition(Vector3D position, Unit oldUnit) {
			Vector3D newPos = position;
			switch (oldUnit) {
				case Mumble:
					newPos = toContinentPosition(toMapPosition(newPos, Mumble), Map);
					break;

				case Map:
					Vector2D mapPos = Vector2D(position.x, position.z);
					Vector2D relativeMapPos = (mapPos - mapRectangle.upperLeft) / mapRectangle.getSize();
					Vector2D continentPos = continentRectangle.upperLeft + relativeMapPos * continentRectangle.getSize();
					newPos = Vector3D(continentPos.x, position.y, continentPos.y);
					break;
			}
			return newPos;
		}
	};

	inline Vector3D Vector2D::toVector3D(int y) const {
		return Vector3D(this->x, y, this->y);
	}

	inline Vector2D Vector3D::toVector2D() const {
		return Vector2D(this->x, this->z);
	}

}