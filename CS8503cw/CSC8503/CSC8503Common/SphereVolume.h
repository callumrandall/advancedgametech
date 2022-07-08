#pragma once
#include "CollisionVolume.h"

namespace NCL {
	class SphereVolume : CollisionVolume
	{
	public:
		SphereVolume(float sphereRadius = 1.0f) {
			type	= VolumeType::Sphere;
			radius	= sphereRadius;
			OBBcol = true;
		}
		~SphereVolume() {}

		float GetRadius() const {
			return radius;
		}

		void setCol(bool b) {
			OBBcol = b;
		}

		bool getCol() const {
			return OBBcol;
		}
	protected:
		float	radius;
		bool OBBcol;
	};
}

