/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

CLASS_DECLARATION( idPhysics_Base, idPhysics_RigidBody )
END_CLASS

#ifdef _WATER_PHYSICS //un credited changes from original sdk
const int STOP_SPEED			= 10;
// if linearVelocity < WATER_STOP_LINEAR && angularVelocity < WATER_STOP_ANGULAR then set the RB to rest
// and we need  this->noMoveTime + NO_MOVE_TIME < gameLocal.getTime()
const idVec3 WATER_STOP_LINEAR(10.0f,10.0f,10.0f);
const idVec3 WATER_STOP_ANGULAR(500000.0f,500000.0f,500000.0f);
const int NO_MOVE_TIME			= 200;
#else
const float STOP_SPEED		= 10.0f;	
#endif



#undef RB_TIMINGS

#ifdef RB_TIMINGS
static int lastTimerReset = 0;
static int numRigidBodies = 0;
static idTimer timer_total, timer_collision;
#endif


/*
================
RigidBodyDerivatives
================
*/
void RigidBodyDerivatives( const float t, const void *clientData, const float *state, float *derivatives ) {
	const idPhysics_RigidBody *p = (idPhysics_RigidBody *) clientData;
	rigidBodyIState_t *s = (rigidBodyIState_t *) state;
	// NOTE: this struct should be build conform rigidBodyIState_t
	struct rigidBodyDerivatives_s {
		idVec3				linearVelocity;
		idMat3				angularMatrix;
		idVec3				force;
		idVec3				torque;
	} *d = (struct rigidBodyDerivatives_s *) derivatives;
	idVec3 angularVelocity;
	idMat3 inverseWorldInertiaTensor;

	inverseWorldInertiaTensor = s->orientation * p->inverseInertiaTensor * s->orientation.Transpose();
	angularVelocity = inverseWorldInertiaTensor * s->angularMomentum;
	// derivatives
	d->linearVelocity = p->inverseMass * s->linearMomentum;
	d->angularMatrix = SkewSymmetric( angularVelocity ) * s->orientation;
	
#ifdef _WATER_PHYSICS //un credited changes from original sdk
	// underwater we have a higher friction
	if( p->GetWaterLevelf() == 0.0f  ) {

		d->force = - p->linearFriction * s->linearMomentum + p->current.externalForce;
		d->torque = - p->angularFriction * s->angularMomentum + p->current.externalTorque;
	}
	else {		
		// don't let water friction go less than 25% of the water viscosity
		float percent = Max(0.25f,p->GetSubmergedPercent(s->position,s->orientation.Transpose()));

		d->force = (-p->linearFriction * p->water->GetViscosity() * percent) * s->linearMomentum + p->current.externalForce;
		d->torque = (-p->angularFriction * p->water->GetViscosity()) * s->angularMomentum + p->current.externalTorque;
	}	
#else
	d->force = - p->linearFriction * s->linearMomentum + p->current.externalForce;
	d->torque = - p->angularFriction * s->angularMomentum + p->current.externalTorque;
#endif
}

#ifdef _WATER_PHYSICS //un credited changes from original sdk
/*
================
idPhysics_RigidBody::GetSubmergedPercent

	Approximates the percentage of the body that is submerged
================
*/
float idPhysics_RigidBody::GetSubmergedPercent( const idVec3 &pos, const idMat3 &rotation ) const
{
	idVec3 depth,bottom(pos);
	idBounds bounds = this->GetBounds();
	float height,d;

	if( this->water == NULL ) 
		return 0.0f;

	// offset and rotate the bounding box
	bounds += -centerOfMass;
	bounds *= rotation;

	// gets the position of the object relative to the surface of the water
	height = abs(bounds[1] * gravityNormal * 2);

	// calculates the depth of the bottom of the object
	bottom += (height * 0.5f) * gravityNormal;
	depth = this->water->GetDepth(bottom);
	d = abs(depth * gravityNormal);

	if( d > height ) {
		// the body is totally submerged
		return 1.0f;
	}
	else if( d <= 0 ) {
		return 0.0f;
	}
	else {
		// the body is partly submerged
		return d / height;
	}
}

/*
================
idPhysics_RigidBody::GetBuoyancy //un credited changes from original sdk

	Gets buoyancy information for this RB
================
*/
bool idPhysics_RigidBody::GetBuoyancy( const idVec3 &pos, const idMat3 &rotation, idVec3 &bCenter, float &percent ) const
{
	// pos - position of the RB
	// rotation - axis for the RB
	// bCenter - after the function is called this is an approximation for the center of buoyancy
	// percent - rough percentage of the body that is under water
	//				used to calculate the volume of the submersed object (volume * percent) to give 
	//				the body somewhat realistic bobbing.
	//
	// return true if the body is in water, false otherwise

	idVec3 tbCenter(pos);
	//idBounds bounds = this->GetBounds();
	idTraceModel tm = *this->GetClipModel()->GetTraceModel();
	int i,count;

	percent = this->GetSubmergedPercent(pos,rotation);
	bCenter = pos;
	
	if( percent == 1.0f ) {
		// the body is totally submerged
		return true;
	}
	else {
		// the body is partly submerged (or not in the water)
		//
		// We do a rough approximation for center of buoyancy.
		// Normally this is done by calculating the volume of the submersed part of the body
		// so the center of buoyancy is the center of mass of the submersed volume.  This is
		// probably a slow computation so what I do is take an average of the submersed
		// vertices of the trace model.
		//
		// I was suprized when this first worked but you can use rb_showBuoyancy to see
		// what the approximation looks like.

		// set up clip model for approximation of center of buoyancy
		tm.Translate(-centerOfMass);
		tm.Rotate(rotation);
		tm.Translate(pos);

		// calculate which vertices are under water
		for( i = 0, count = 1; i < tm.numVerts; i++ ) {
			if( gameLocal.clip.Contents(tm.verts[i],NULL,this->GetAxis(),MASK_WATER,NULL) ) {
				tbCenter += tm.verts[i];
				count += 1;
			}
		}

		if( count == 1 )
			bCenter = pos;
		else
			bCenter = tbCenter / count;
		return (count != 1);
	}
}
#endif

/*
================
idPhysics_RigidBody::Integrate

  Calculate next state from the current state using an integrator.
================
*/
void idPhysics_RigidBody::Integrate( float deltaTime, rigidBodyPState_t &next ) {
	idVec3 position;

	position = current.i.position;
	current.i.position += centerOfMass * current.i.orientation;

	current.i.orientation.TransposeSelf();

	integrator->Evaluate( (float *) &current.i, (float *) &next.i, 0, deltaTime );
	next.i.orientation.OrthoNormalizeSelf();

#ifdef _WATER_PHYSICS //un credited changes from original sdk
	// apply a water gravity if the body is in water
	if( this->SetWaterLevelf() != 0.0f ) {	
		idVec3 bCenter;
		idVec3 bForce(gravityVector),rForce(-gravityVector);
		float bMass,fraction,liquidMass;
		bool inWater;

		inWater = this->GetBuoyancy(next.i.position,next.i.orientation.Transpose(),bCenter,fraction);

		// calculate water mass
		liquidMass = this->volume * this->water->GetDensity() * fraction;
		// don't let liquid mass get too high
		liquidMass = Min( liquidMass, 3 * this->mass );

		bMass = this->mass - liquidMass;

		// calculate buoyancy force
		bForce *= deltaTime * bMass;
		rForce *= deltaTime * liquidMass;

		// apply water force
		// basically here we do a ::ApplyImpulse() but we apply it to next, not current
		//gameLocal.Printf("%s waterGravityMult %f\n", GetSelf()->GetName(), waterGravityMult);
		next.i.linearMomentum += (bForce * waterGravityMult); //ivan: waterGravityMult changes or disable the gravity in water
		next.i.angularMomentum += (bCenter - next.i.position).Cross(rForce);

		// take the body out of water if it's not in water.
		if( !inWater )
			this->SetWater(NULL);
	}
	else {
		// apply normal gravity
		next.i.linearMomentum += deltaTime * gravityVector * mass;
	}
#else
	// apply gravity
	next.i.linearMomentum += deltaTime * gravityVector * mass;
#endif //un credited changes from original sdk

	current.i.orientation.TransposeSelf();
	next.i.orientation.TransposeSelf();

	current.i.position = position;
	next.i.position -= centerOfMass * next.i.orientation;

	next.atRest = current.atRest;
}

/*
================
idPhysics_RigidBody::CollisionImpulse

  Calculates the collision impulse using the velocity relative to the collision object.
  The current state should be set to the moment of impact.
================
*/
bool idPhysics_RigidBody::CollisionImpulse( const trace_t &collision, idVec3 &impulse ) {
	idVec3 r, linearVelocity, angularVelocity, velocity;
	idMat3 inverseWorldInertiaTensor;
	float impulseNumerator, impulseDenominator, vel;
	impactInfo_t info;
	idEntity *ent;

	// get info from other entity involved
	ent = gameLocal.entities[collision.c.entityNum];
	ent->GetImpactInfo( self, collision.c.id, collision.c.point, &info );

	// collision point relative to the body center of mass
	r = collision.c.point - ( current.i.position + centerOfMass * current.i.orientation );
	// the velocity at the collision point
	linearVelocity = inverseMass * current.i.linearMomentum;
	inverseWorldInertiaTensor = current.i.orientation.Transpose() * inverseInertiaTensor * current.i.orientation;
	angularVelocity = inverseWorldInertiaTensor * current.i.angularMomentum;
	velocity = linearVelocity + angularVelocity.Cross(r);
	// subtract velocity of other entity
	velocity -= info.velocity;

	// velocity in normal direction
	vel = velocity * collision.c.normal;

	if ( vel > -STOP_SPEED ) {
		impulseNumerator = STOP_SPEED;
	}
	else {
		impulseNumerator = -( 1.0f + bouncyness ) * vel;
	}
	impulseDenominator = inverseMass + ( ( inverseWorldInertiaTensor * r.Cross( collision.c.normal ) ).Cross( r ) * collision.c.normal );
	if ( info.invMass ) {
		impulseDenominator += info.invMass + ( ( info.invInertiaTensor * info.position.Cross( collision.c.normal ) ).Cross( info.position ) * collision.c.normal );
	}
	impulse = (impulseNumerator / impulseDenominator) * collision.c.normal;

	// update linear and angular momentum with impulse
	current.i.linearMomentum += impulse;
	current.i.angularMomentum += r.Cross(impulse);

	// if no movement at all don't blow up
	if ( collision.fraction < 0.0001f ) {
		current.i.linearMomentum *= 0.5f;
		current.i.angularMomentum *= 0.5f;
	}

	// callback to self to let the entity know about the collision
	return self->Collide( collision, velocity );
}

#ifdef _WATER_PHYSICS //un credited changes from original sdk
/*
================
idPhysics_RigidBody::CheckForCollisions

  Check for collisions between the current and next state.
  If there is a collision the next state is set to the state at the moment of impact.
================
*/
bool idPhysics_RigidBody::CheckForCollisions( const float deltaTime, rigidBodyPState_t &next, trace_t &collision ) {
//#define TEST_COLLISION_DETECTION
	idMat3 axis;
	idRotation rotation;
	idVec3 pos;
	trace_t	waterCollision;
	bool collided = false;

#ifdef TEST_COLLISION_DETECTION
	bool startsolid;
	if ( gameLocal.clip.Contents( current.i.position, clipModel, current.i.orientation, clipMask, self ) ) {
		startsolid = true;
	}
#endif

	TransposeMultiply( current.i.orientation, next.i.orientation, axis );
	rotation = axis.ToRotation();
	rotation.SetOrigin( current.i.position );
	pos = next.i.position;

	// if there was a collision
	if ( collided || gameLocal.clip.Motion( collision, current.i.position, next.i.position, rotation, clipModel, current.i.orientation, clipMask, self ) ) {
		// set the next state to the state at the moment of impact
		next.i.position = collision.endpos;
		next.i.orientation = collision.endAxis;
		next.i.linearMomentum = current.i.linearMomentum;
		next.i.angularMomentum = current.i.angularMomentum;
		collided = true;
	}

	// Check for water collision
	// ideally we could do this check in one step but if a body moves quickly in shallow water
	// they will occasionally clip through a solid entity (ie. fall through the floor)
	if ( gameLocal.clip.Motion( waterCollision, current.i.position, pos, rotation, clipModel, current.i.orientation, MASK_WATER, self ) ) {
		idEntity *ent = gameLocal.entities[waterCollision.c.entityNum];

		// make sure the object didn't collide with something before hitting the water (we don't splash for that case)
		if( !collided || waterCollision.fraction < collision.fraction ) {

			// if the object collides with something with a physics_liquid
			if( ent->GetPhysics()->IsType( idPhysics_Liquid::Type ) ) {
				idPhysics_Liquid *liquid = static_cast<idPhysics_Liquid *>(ent->GetPhysics());
				impactInfo_t info;

				self->GetImpactInfo(ent,waterCollision.c.id,waterCollision.c.point,&info);

				// apply water splash friction
				if( this->water == NULL ) {	
					idVec3 impulse = -info.velocity * this->volume * liquid->GetDensity() * 0.25f;
					impulse = (impulse * gravityNormal) * gravityNormal;

					if( next.i.linearMomentum.LengthSqr() < impulse.LengthSqr() ) {
						// cancel falling, maintain sideways movement (lateral?)
						next.i.linearMomentum -= (next.i.linearMomentum * gravityNormal) * gravityNormal;
					}
					else {
						next.i.angularMomentum += ( waterCollision.c.point - ( next.i.position + centerOfMass * next.i.orientation ) ).Cross( impulse );
						next.i.linearMomentum += impulse * 0.5f;
					}
				}

				this->SetWater(liquid);
				this->water->Splash(this->self,this->volume,info,waterCollision);
			}
		}
	}

#ifdef TEST_COLLISION_DETECTION
	if ( gameLocal.clip.Contents( next.i.position, clipModel, next.i.orientation, clipMask, self ) ) {
		if ( !startsolid ) {
			int bah = 1;
		}
	}
#endif
	return collided;
}

#else

/*
================
idPhysics_RigidBody::CheckForCollisions

  Check for collisions between the current and next state.
  If there is a collision the next state is set to the state at the moment of impact.
================
*/
bool idPhysics_RigidBody::CheckForCollisions( const float deltaTime, rigidBodyPState_t &next, trace_t &collision ) {
//#define TEST_COLLISION_DETECTION
	idMat3 axis;
	idRotation rotation;
	bool collided = false;

#ifdef TEST_COLLISION_DETECTION
	bool startsolid;
	if ( gameLocal.clip.Contents( current.i.position, clipModel, current.i.orientation, clipMask, self ) ) {
		startsolid = true;
	}
#endif

	TransposeMultiply( current.i.orientation, next.i.orientation, axis );
	rotation = axis.ToRotation();
	rotation.SetOrigin( current.i.position );

	// if there was a collision
	if ( gameLocal.clip.Motion( collision, current.i.position, next.i.position, rotation, clipModel, current.i.orientation, clipMask, self ) ) {
		// set the next state to the state at the moment of impact
		next.i.position = collision.endpos;
		next.i.orientation = collision.endAxis;
		next.i.linearMomentum = current.i.linearMomentum;
		next.i.angularMomentum = current.i.angularMomentum;
		collided = true;
	}

#ifdef TEST_COLLISION_DETECTION
	if ( gameLocal.clip.Contents( next.i.position, clipModel, next.i.orientation, clipMask, self ) ) {
		if ( !startsolid ) {
			int bah = 1;
		}
	}
#endif
	return collided;
}

#endif //un credited changes from original sdk

/*
================
idPhysics_RigidBody::ContactFriction

  Does not solve friction for multiple simultaneous contacts but applies contact friction in isolation.
  Uses absolute velocity at the contact points instead of the velocity relative to the contact object.
================
*/
void idPhysics_RigidBody::ContactFriction( float deltaTime ) {
	int i;
	float magnitude, impulseNumerator, impulseDenominator;
	idMat3 inverseWorldInertiaTensor;
	idVec3 linearVelocity, angularVelocity;
	idVec3 massCenter, r, velocity, normal, impulse, normalVelocity;

	inverseWorldInertiaTensor = current.i.orientation.Transpose() * inverseInertiaTensor * current.i.orientation;

	massCenter = current.i.position + centerOfMass * current.i.orientation;

	for ( i = 0; i < contacts.Num(); i++ ) {

		r = contacts[i].point - massCenter;

		// calculate velocity at contact point
		linearVelocity = inverseMass * current.i.linearMomentum;
		angularVelocity = inverseWorldInertiaTensor * current.i.angularMomentum;
		velocity = linearVelocity + angularVelocity.Cross(r);

		// velocity along normal vector
		normalVelocity = ( velocity * contacts[i].normal ) * contacts[i].normal;

		// calculate friction impulse
		normal = -( velocity - normalVelocity );
		magnitude = normal.Normalize();
		impulseNumerator = contactFriction * magnitude;
		impulseDenominator = inverseMass + ( ( inverseWorldInertiaTensor * r.Cross( normal ) ).Cross( r ) * normal );
		impulse = (impulseNumerator / impulseDenominator) * normal;

		// apply friction impulse
		current.i.linearMomentum += impulse;
		current.i.angularMomentum += r.Cross(impulse);

		// if moving towards the surface at the contact point
		if ( normalVelocity * contacts[i].normal < 0.0f ) {
			// calculate impulse
			normal = -normalVelocity;
			impulseNumerator = normal.Normalize();
			impulseDenominator = inverseMass + ( ( inverseWorldInertiaTensor * r.Cross( normal ) ).Cross( r ) * normal );
			impulse = (impulseNumerator / impulseDenominator) * normal;

			// apply impulse
			current.i.linearMomentum += impulse;
			current.i.angularMomentum += r.Cross( impulse );
		}
	}
}

/*
================
idPhysics_RigidBody::TestIfAtRest

  Returns true if the body is considered at rest.
  Does not catch all cases where the body is at rest but is generally good enough.
================
*/
#ifdef _WATER_PHYSICS //un credited changes from original sdk
bool idPhysics_RigidBody::TestIfAtRest( void ) {
#else
bool idPhysics_RigidBody::TestIfAtRest( void ) const {
#endif //un credited changes from original sdk
	int i;
	float gv;
	idVec3 v, av, normal, point;
	idMat3 inverseWorldInertiaTensor;
	idFixedWinding contactWinding;

	if ( current.atRest >= 0 ) {
		return true;
	}

#ifdef _WATER_PHYSICS //un credited changes from original sdk
	// do some special checks if the body is in water
	if( this->water != NULL )
	{
		if( this->current.i.linearMomentum.LengthSqr() < WATER_STOP_LINEAR.LengthSqr() &&
			this->current.i.angularMomentum.LengthSqr() < WATER_STOP_ANGULAR.LengthSqr() ) {
		
			if( this->noMoveTime == 0 ) {
				this->noMoveTime = gameLocal.GetTime();	
			}
			else if( this->noMoveTime+NO_MOVE_TIME < gameLocal.GetTime() ) {
				this->noMoveTime = 0;
				return true;
			}
		}
		else {
			this->noMoveTime = 0;
		}
	}
#endif //un credited changes from original sdk

	// need at least 3 contact points to come to rest
	if ( contacts.Num() < 3 ) {
		return false;
	}

	// get average contact plane normal
	normal.Zero();
	for ( i = 0; i < contacts.Num(); i++ ) {
		normal += contacts[i].normal;
	}
	normal /= (float) contacts.Num();
	normal.Normalize();

	// if on a too steep surface
	if ( (normal * gravityNormal) > -0.7f ) {
		return false;
	}

	// create bounds for contact points
	contactWinding.Clear();
	for ( i = 0; i < contacts.Num(); i++ ) {
		// project point onto plane through origin orthogonal to the gravity
		point = contacts[i].point - (contacts[i].point * gravityNormal) * gravityNormal;
		contactWinding.AddToConvexHull( point, gravityNormal );
	}

	// need at least 3 contact points to come to rest
	if ( contactWinding.GetNumPoints() < 3 ) {
		return false;
	}

	// center of mass in world space
	point = current.i.position + centerOfMass * current.i.orientation;
	point -= (point * gravityNormal) * gravityNormal;

	// if the point is not inside the winding
	if ( !contactWinding.PointInside( gravityNormal, point, 0 ) ) {
		return false;
	}

	// linear velocity of body
	v = inverseMass * current.i.linearMomentum;
	// linear velocity in gravity direction
	gv = v * gravityNormal;
	// linear velocity orthogonal to gravity direction
	v -= gv * gravityNormal;

	// if too much velocity orthogonal to gravity direction
	if ( v.Length() > STOP_SPEED ) {
		return false;
	}
	// if too much velocity in gravity direction
	if ( gv > 2.0f * STOP_SPEED || gv < -2.0f * STOP_SPEED ) {
		return false;
	}

	// calculate rotational velocity
	inverseWorldInertiaTensor = current.i.orientation * inverseInertiaTensor * current.i.orientation.Transpose();
	av = inverseWorldInertiaTensor * current.i.angularMomentum;

	// if too much rotational velocity
	if ( av.LengthSqr() > STOP_SPEED ) {
		return false;
	}

	return true;
}

/*
================
idPhysics_RigidBody::DropToFloorAndRest

  Drops the object straight down to the floor and verifies if the object is at rest on the floor.
================
*/
void idPhysics_RigidBody::DropToFloorAndRest( void ) {
	//gameLocal.Printf("DropToFloorAndRest\n" ); //un credited changes from original sdk
	idVec3 down;
	trace_t tr;

	if ( testSolid ) {

		testSolid = false;

		if ( gameLocal.clip.Contents( current.i.position, clipModel, current.i.orientation, clipMask, self ) ) {
			gameLocal.DWarning( "rigid body in solid for entity '%s' type '%s' at (%s)",
								self->name.c_str(), self->GetType()->classname, current.i.position.ToString(0) );
			Rest();
			dropToFloor = false;
			return;
		}
	}

	// put the body on the floor
	down = current.i.position + gravityNormal * 128.0f;
	gameLocal.clip.Translation( tr, current.i.position, down, clipModel, current.i.orientation, clipMask, self );
	current.i.position = tr.endpos;
	clipModel->Link( gameLocal.clip, self, clipModel->GetId(), tr.endpos, current.i.orientation );

	// if on the floor already
	if ( tr.fraction == 0.0f ) {
		// test if we are really at rest
		EvaluateContacts();
		if ( !TestIfAtRest() ) {
			gameLocal.DWarning( "rigid body not at rest for entity '%s' type '%s' at (%s)",
								self->name.c_str(), self->GetType()->classname, current.i.position.ToString(0) );
		}
		Rest();
		dropToFloor = false;
	} else if ( IsOutsideWorld() ) {
		gameLocal.Warning( "rigid body outside world bounds for entity '%s' type '%s' at (%s)",
							self->name.c_str(), self->GetType()->classname, current.i.position.ToString(0) );
		Rest();
		dropToFloor = false;
	}
}

/*
================
idPhysics_RigidBody::DebugDraw
================
*/
void idPhysics_RigidBody::DebugDraw( void ) {

	if ( rb_showBodies.GetBool() || ( rb_showActive.GetBool() && current.atRest < 0 ) ) {
		collisionModelManager->DrawModel( clipModel->Handle(), clipModel->GetOrigin(), clipModel->GetAxis(), vec3_origin, 0.0f );
	}

	if ( rb_showMass.GetBool() ) {
#ifdef _WATER_PHYSICS //un credited changes from original sdk
		if( this->water != NULL )  {
			idVec3 pos;
			float  percent, liquidMass;

			pos = this->current.i.position + this->centerOfMass*this->current.i.orientation;
			percent = this->GetSubmergedPercent(pos,this->current.i.orientation.Transpose());

			liquidMass = this->mass - ( this->volume * this->water->GetDensity() * percent );

			gameRenderWorld->DrawText( va( "\n%1.2f", liquidMass), current.i.position, 0.08f, colorCyan, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1 );
		}
		else
			gameRenderWorld->DrawText( va( "\n%1.2f", mass ), current.i.position, 0.08f, colorCyan, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1 );
#else
		gameRenderWorld->DrawText( va( "\n%1.2f", mass ), current.i.position, 0.08f, colorCyan, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1 );
#endif
	}

	if ( rb_showInertia.GetBool() ) {
		idMat3 &I = inertiaTensor;
		gameRenderWorld->DrawText( va( "\n\n\n( %.1f %.1f %.1f )\n( %.1f %.1f %.1f )\n( %.1f %.1f %.1f )",
									I[0].x, I[0].y, I[0].z,
									I[1].x, I[1].y, I[1].z,
									I[2].x, I[2].y, I[2].z ),
									current.i.position, 0.05f, colorCyan, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1 );
	}

	if ( rb_showVelocity.GetBool() ) {
		DrawVelocity( clipModel->GetId(), 0.1f, 4.0f );
	}

#ifdef _WATER_PHYSICS //un credited changes from original sdk
	if( rb_showBuoyancy.GetBool() && this->water != NULL ) {
		idVec3 pos;
		idVec3 bCenter;
		float  percent;

		pos = this->current.i.position + this->centerOfMass*this->current.i.orientation;
		this->GetBuoyancy(pos,this->current.i.orientation.Transpose(),bCenter,percent);

		gameRenderWorld->DebugArrow(colorGreen,pos,bCenter,1);
		gameRenderWorld->DrawText( va( "%1.2f",percent), pos, 0.08f, colorCyan, gameLocal.GetLocalPlayer()->viewAngles.ToMat3());
	}
#endif
}

/*
================
idPhysics_RigidBody::idPhysics_RigidBody
================
*/
idPhysics_RigidBody::idPhysics_RigidBody( void ) {

	// set default rigid body properties
	SetClipMask( MASK_SOLID );
	SetBouncyness( 0.6f );
	SetFriction( 0.6f, 0.6f, 0.0f );
	clipModel = NULL;

	memset( &current, 0, sizeof( current ) );

	current.atRest = -1;
	current.lastTimeStep = USERCMD_MSEC;

	current.i.position.Zero();
	current.i.orientation.Identity();

	current.i.linearMomentum.Zero();
	current.i.angularMomentum.Zero();

	saved = current;

	mass = 1.0f;
	inverseMass = 1.0f;
	centerOfMass.Zero();
	inertiaTensor.Identity();
	inverseInertiaTensor.Identity();

	// use the least expensive euler integrator
	integrator = new idODE_Euler( sizeof(rigidBodyIState_t) / sizeof(float), RigidBodyDerivatives, this );

	dropToFloor = false;
	noImpact = false;
	noContact = false;
	//ivan
	activationDelay = 0; 
	activateNextImpact = false; 
	//ivan

	hasMaster = false;
	isOrientated = false;
#ifdef _WATER_PHYSICS //un credited changes from original sdk
	water = NULL;
	noMoveTime = 0.0f;
	waterGravityMult = 1.0f;
#endif

#ifdef RB_TIMINGS
	lastTimerReset = 0;
#endif
}

/*
================
idPhysics_RigidBody::~idPhysics_RigidBody
================
*/
idPhysics_RigidBody::~idPhysics_RigidBody( void ) {
	if ( clipModel ) {
		delete clipModel;
		clipModel = NULL;
	}
	delete integrator;
}

/*
================
idPhysics_RigidBody_SavePState
================
*/
void idPhysics_RigidBody_SavePState( idSaveGame *savefile, const rigidBodyPState_t &state ) {
	savefile->WriteInt( state.atRest );
	savefile->WriteFloat( state.lastTimeStep );
	savefile->WriteVec3( state.localOrigin );
	savefile->WriteMat3( state.localAxis );
	savefile->WriteVec6( state.pushVelocity );
	savefile->WriteVec3( state.externalForce );
	savefile->WriteVec3( state.externalTorque );

	savefile->WriteVec3( state.i.position );
	savefile->WriteMat3( state.i.orientation );
	savefile->WriteVec3( state.i.linearMomentum );
	savefile->WriteVec3( state.i.angularMomentum );
}

/*
================
idPhysics_RigidBody_RestorePState
================
*/
void idPhysics_RigidBody_RestorePState( idRestoreGame *savefile, rigidBodyPState_t &state ) {
	savefile->ReadInt( state.atRest );
	savefile->ReadFloat( state.lastTimeStep );
	savefile->ReadVec3( state.localOrigin );
	savefile->ReadMat3( state.localAxis );
	savefile->ReadVec6( state.pushVelocity );
	savefile->ReadVec3( state.externalForce );
	savefile->ReadVec3( state.externalTorque );

	savefile->ReadVec3( state.i.position );
	savefile->ReadMat3( state.i.orientation );
	savefile->ReadVec3( state.i.linearMomentum );
	savefile->ReadVec3( state.i.angularMomentum );
}

/*
================
idPhysics_RigidBody::Save
================
*/
void idPhysics_RigidBody::Save( idSaveGame *savefile ) const {

	idPhysics_RigidBody_SavePState( savefile, current );
	idPhysics_RigidBody_SavePState( savefile, saved );

	savefile->WriteFloat( linearFriction );
	savefile->WriteFloat( angularFriction );
	savefile->WriteFloat( contactFriction );
	savefile->WriteFloat( bouncyness );
	savefile->WriteClipModel( clipModel );

	savefile->WriteFloat( mass );
#ifdef _WATER_PHYSICS //un credited changes from original sdk
	savefile->WriteFloat( volume );
	savefile->WriteFloat( waterGravityMult );
#endif
	savefile->WriteFloat( inverseMass );
	savefile->WriteVec3( centerOfMass );
	savefile->WriteMat3( inertiaTensor );
	savefile->WriteMat3( inverseInertiaTensor );

	savefile->WriteBool( dropToFloor );
	savefile->WriteBool( testSolid );
	savefile->WriteBool( noImpact );
	savefile->WriteBool( noContact );

	savefile->WriteBool( hasMaster );
	savefile->WriteBool( isOrientated );

	//ivan start
	savefile->WriteInt( activationDelay );
	savefile->WriteBool( activateNextImpact );
	//ivan end
}

/*
================
idPhysics_RigidBody::Restore
================
*/
void idPhysics_RigidBody::Restore( idRestoreGame *savefile ) {

	idPhysics_RigidBody_RestorePState( savefile, current );
	idPhysics_RigidBody_RestorePState( savefile, saved );

	savefile->ReadFloat( linearFriction );
	savefile->ReadFloat( angularFriction );
	savefile->ReadFloat( contactFriction );
	savefile->ReadFloat( bouncyness );
	savefile->ReadClipModel( clipModel );

	savefile->ReadFloat( mass );
#ifdef _WATER_PHYSICS //un credited changes from original sdk
	savefile->ReadFloat( volume );
	savefile->ReadFloat( waterGravityMult );
#endif
	savefile->ReadFloat( inverseMass );
	savefile->ReadVec3( centerOfMass );
	savefile->ReadMat3( inertiaTensor );
	savefile->ReadMat3( inverseInertiaTensor );

	savefile->ReadBool( dropToFloor );
	savefile->ReadBool( testSolid );
	savefile->ReadBool( noImpact );
	savefile->ReadBool( noContact );

	savefile->ReadBool( hasMaster );
	savefile->ReadBool( isOrientated );

	//ivan start
	savefile->ReadInt( activationDelay );
	savefile->ReadBool( activateNextImpact );
	//ivan end
}

/*
================
idPhysics_RigidBody::SetClipModel
================
*/
#define MAX_INERTIA_SCALE		10.0f

void idPhysics_RigidBody::SetClipModel( idClipModel *model, const float density, int id, bool freeOld ) {
	int minIndex;
	idMat3 inertiaScale;

	assert( self );
	assert( model );					// we need a clip model
	assert( model->IsTraceModel() );	// and it should be a trace model
	assert( density > 0.0f );			// density should be valid

	if ( clipModel && clipModel != model && freeOld ) {
		delete clipModel;
	}
	clipModel = model;
	clipModel->Link( gameLocal.clip, self, 0, current.i.position, current.i.orientation );

	// get mass properties from the trace model
	clipModel->GetMassProperties( density, mass, centerOfMass, inertiaTensor );

	// check whether or not the clip model has valid mass properties
	if ( mass <= 0.0f || FLOAT_IS_NAN( mass ) ) {
		gameLocal.Warning( "idPhysics_RigidBody::SetClipModel: invalid mass for entity '%s' type '%s'",
							self->name.c_str(), self->GetType()->classname );
		mass = 1.0f;
		centerOfMass.Zero();
		inertiaTensor.Identity();
	}

#ifdef _WATER_PHYSICS //un credited changes from original sdk
	this->volume = mass / density;
#endif

	// check whether or not the inertia tensor is balanced
	minIndex = Min3Index( inertiaTensor[0][0], inertiaTensor[1][1], inertiaTensor[2][2] );
	inertiaScale.Identity();
	inertiaScale[0][0] = inertiaTensor[0][0] / inertiaTensor[minIndex][minIndex];
	inertiaScale[1][1] = inertiaTensor[1][1] / inertiaTensor[minIndex][minIndex];
	inertiaScale[2][2] = inertiaTensor[2][2] / inertiaTensor[minIndex][minIndex];

	if ( inertiaScale[0][0] > MAX_INERTIA_SCALE || inertiaScale[1][1] > MAX_INERTIA_SCALE || inertiaScale[2][2] > MAX_INERTIA_SCALE ) {
		gameLocal.DWarning( "idPhysics_RigidBody::SetClipModel: unbalanced inertia tensor for entity '%s' type '%s'",
							self->name.c_str(), self->GetType()->classname );
		float min = inertiaTensor[minIndex][minIndex] * MAX_INERTIA_SCALE;
		inertiaScale[(minIndex+1)%3][(minIndex+1)%3] = min / inertiaTensor[(minIndex+1)%3][(minIndex+1)%3];
		inertiaScale[(minIndex+2)%3][(minIndex+2)%3] = min / inertiaTensor[(minIndex+2)%3][(minIndex+2)%3];
		inertiaTensor *= inertiaScale;
	}

	inverseMass = 1.0f / mass;
	inverseInertiaTensor = inertiaTensor.Inverse() * ( 1.0f / 6.0f );

	current.i.linearMomentum.Zero();
	current.i.angularMomentum.Zero();
}

/*
================
idPhysics_RigidBody::GetClipModel
================
*/
idClipModel *idPhysics_RigidBody::GetClipModel( int id ) const {
	return clipModel;
}

/*
================
idPhysics_RigidBody::GetNumClipModels
================
*/
int idPhysics_RigidBody::GetNumClipModels( void ) const {
	return 1;
}

/*
================
idPhysics_RigidBody::SetMass
================
*/
void idPhysics_RigidBody::SetMass( float mass, int id ) {
	assert( mass > 0.0f );
	inertiaTensor *= mass / this->mass;
	inverseInertiaTensor = inertiaTensor.Inverse() * (1.0f / 6.0f);
	this->mass = mass;
	inverseMass = 1.0f / mass;
}

/*
================
idPhysics_RigidBody::GetMass
================
*/
float idPhysics_RigidBody::GetMass( int id ) const {
#ifdef _WATER_PHYSICS
	if( this->water != NULL ) {
		idVec3 pos;
		float  percent,bMass;

		pos = this->current.i.position + this->centerOfMass*this->current.i.orientation;
		percent = this->GetSubmergedPercent(pos,this->current.i.orientation);		
		bMass = mass - (this->volume * this->water->GetDensity() * percent);

		return bMass;
	}
	else
		return mass;
#else
	return mass;
#endif
}

/*
================
idPhysics_RigidBody::SetFriction
================
*/
void idPhysics_RigidBody::SetFriction( const float linear, const float angular, const float contact ) {
	if (	linear < 0.0f || linear > 1.0f ||
			angular < 0.0f || angular > 1.0f ||
			contact < 0.0f || contact > 1.0f ) {
		return;
	}
	linearFriction = linear;
	angularFriction = angular;
	contactFriction = contact;
}

/*
================
idPhysics_RigidBody::SetBouncyness
================
*/
void idPhysics_RigidBody::SetBouncyness( const float b ) {
	if ( b < 0.0f || b > 1.0f ) {
		return;
	}
	bouncyness = b;
}

/*
================
idPhysics_RigidBody::Rest
================
*/
void idPhysics_RigidBody::Rest( void ) {
	current.atRest = gameLocal.time;
	current.i.linearMomentum.Zero();
	current.i.angularMomentum.Zero();
	self->BecomeInactive( TH_PHYSICS );
}

/*
================
idPhysics_RigidBody::DropToFloor
================
*/
void idPhysics_RigidBody::DropToFloor( void ) {
	dropToFloor = true;
	testSolid = true;
}

/*
================
idPhysics_RigidBody::NoContact
================
*/
void idPhysics_RigidBody::NoContact( void ) {
	noContact = true;
}

/*
================
idPhysics_RigidBody::Activate
================
*/
void idPhysics_RigidBody::Activate( void ) {
	current.atRest = -1;
	self->BecomeActive( TH_PHYSICS );
}

/*
================
idPhysics_RigidBody::PutToRest

  put to rest untill something collides with this physics object
================
*/
void idPhysics_RigidBody::PutToRest( void ) {
	Rest();
}


//ivan start
/*
================
idPhysics_RigidBody::DelayedActivationOnImpact
================
*/

void idPhysics_RigidBody::DelayedActivationOnImpact( int delay ) {
	noImpact = true; //turn off for now
	activateNextImpact = true;
	activationDelay = delay;
}

//ivan end


/*
================
idPhysics_RigidBody::EnableImpact
================
*/
void idPhysics_RigidBody::EnableImpact( void ) {
	noImpact = false;
}

/*
================
idPhysics_RigidBody::DisableImpact
================
*/
void idPhysics_RigidBody::DisableImpact( void ) {
	noImpact = true;
}

/*
================
idPhysics_RigidBody::SetContents
================
*/
void idPhysics_RigidBody::SetContents( int contents, int id ) {
	clipModel->SetContents( contents );
}

/*
================
idPhysics_RigidBody::GetContents
================
*/
int idPhysics_RigidBody::GetContents( int id ) const {
	return clipModel->GetContents();
}

/*
================
idPhysics_RigidBody::GetBounds
================
*/
const idBounds &idPhysics_RigidBody::GetBounds( int id ) const {
	return clipModel->GetBounds();
}

/*
================
idPhysics_RigidBody::GetAbsBounds
================
*/
const idBounds &idPhysics_RigidBody::GetAbsBounds( int id ) const {
	return clipModel->GetAbsBounds();
}

/*
================
idPhysics_RigidBody::Evaluate

  Evaluate the impulse based rigid body physics.
  When a collision occurs an impulse is applied at the moment of impact but
  the remaining time after the collision is ignored.
================
*/
bool idPhysics_RigidBody::Evaluate( int timeStepMSec, int endTimeMSec ) {
	rigidBodyPState_t next;
	idAngles angles;
	trace_t collision;
	idVec3 impulse;
	idEntity *ent;
	idVec3 oldOrigin, masterOrigin;
	idMat3 oldAxis, masterAxis;
	float timeStep;
	bool collided, cameToRest = false;

	timeStep = MS2SEC( timeStepMSec );
	current.lastTimeStep = timeStep;

	if ( hasMaster ) {
		oldOrigin = current.i.position;
		oldAxis = current.i.orientation;
		self->GetMasterPosition( masterOrigin, masterAxis );
		current.i.position = masterOrigin + current.localOrigin * masterAxis;
		if ( isOrientated ) {
			current.i.orientation = current.localAxis * masterAxis;
		}
		else {
			current.i.orientation = current.localAxis;
		}
		clipModel->Link( gameLocal.clip, self, clipModel->GetId(), current.i.position, current.i.orientation );
		current.i.linearMomentum = mass * ( ( current.i.position - oldOrigin ) / timeStep );
		current.i.angularMomentum = inertiaTensor * ( ( current.i.orientation * oldAxis.Transpose() ).ToAngularVelocity() / timeStep );
		current.externalForce.Zero();
		current.externalTorque.Zero();

		return ( current.i.position != oldOrigin || current.i.orientation != oldAxis );
	}

	// if the body is at rest
	if ( current.atRest >= 0 || timeStep <= 0.0f ) {
		DebugDraw();
		return false;
	}

	// if putting the body to rest
	if ( dropToFloor ) {
		DropToFloorAndRest();
		current.externalForce.Zero();
		current.externalTorque.Zero();
		return true;
	}

#ifdef RB_TIMINGS
	timer_total.Start();
#endif

	// move the rigid body velocity into the frame of a pusher
//	current.i.linearMomentum -= current.pushVelocity.SubVec3( 0 ) * mass;
//	current.i.angularMomentum -= current.pushVelocity.SubVec3( 1 ) * inertiaTensor;

	clipModel->Unlink();

	next = current;

	// calculate next position and orientation
	Integrate( timeStep, next );

#ifdef RB_TIMINGS
	timer_collision.Start();
#endif

	// check for collisions from the current to the next state
	collided = CheckForCollisions( timeStep, next, collision );

#ifdef RB_TIMINGS
	timer_collision.Stop();
#endif

	// set the new state
	current = next;

	if ( collided ) {
		// apply collision impulse
		if ( CollisionImpulse( collision, impulse ) ) {
			current.atRest = gameLocal.time;
		}
	}

	// update the position of the clip model
	clipModel->Link( gameLocal.clip, self, clipModel->GetId(), current.i.position, current.i.orientation );

	DebugDraw();

	if ( !noContact ) {

#ifdef RB_TIMINGS
		timer_collision.Start();
#endif
		// get contacts
		EvaluateContacts();

#ifdef RB_TIMINGS
		timer_collision.Stop();
#endif

		// check if the body has come to rest
		if ( TestIfAtRest() ) {
			// put to rest
			Rest();
			cameToRest = true;
		}  else {
			// apply contact friction
			ContactFriction( timeStep );
		}
	}

	if ( current.atRest < 0 ) {
		ActivateContactEntities();
	}

	if ( collided ) {
		// if the rigid body didn't come to rest or the other entity is not at rest
		ent = gameLocal.entities[collision.c.entityNum];
		if ( ent && ( !cameToRest || !ent->IsAtRest() ) ) {
			// apply impact to other entity
			ent->ApplyImpulse( self, collision.c.id, collision.c.point, -impulse );
		}
	}

	// move the rigid body velocity back into the world frame
//	current.i.linearMomentum += current.pushVelocity.SubVec3( 0 ) * mass;
//	current.i.angularMomentum += current.pushVelocity.SubVec3( 1 ) * inertiaTensor;
	current.pushVelocity.Zero();

	current.lastTimeStep = timeStep;
	current.externalForce.Zero();
	current.externalTorque.Zero();

	if ( IsOutsideWorld() ) {
		gameLocal.Warning( "rigid body moved outside world bounds for entity '%s' type '%s' at (%s)",
					self->name.c_str(), self->GetType()->classname, current.i.position.ToString(0) );
		Rest();
	}

#ifdef RB_TIMINGS
	timer_total.Stop();

	if ( rb_showTimings->integer == 1 ) {
		gameLocal.Printf( "%12s: t %u cd %u\n",
						self->name.c_str(),
						timer_total.Milliseconds(), timer_collision.Milliseconds() );
		lastTimerReset = 0;
	}
	else if ( rb_showTimings->integer == 2 ) {
		numRigidBodies++;
		if ( endTimeMSec > lastTimerReset ) {
			gameLocal.Printf( "rb %d: t %u cd %u\n",
							numRigidBodies,
							timer_total.Milliseconds(), timer_collision.Milliseconds() );
		}
	}
	if ( endTimeMSec > lastTimerReset ) {
		lastTimerReset = endTimeMSec;
		numRigidBodies = 0;
		timer_total.Clear();
		timer_collision.Clear();
	}
#endif

	return true;
}

/*
================
idPhysics_RigidBody::UpdateTime
================
*/
void idPhysics_RigidBody::UpdateTime( int endTimeMSec ) {
}

/*
================
idPhysics_RigidBody::GetTime
================
*/
int idPhysics_RigidBody::GetTime( void ) const {
	return gameLocal.time;
}

/*
================
idPhysics_RigidBody::GetImpactInfo
================
*/
void idPhysics_RigidBody::GetImpactInfo( const int id, const idVec3 &point, impactInfo_t *info ) const {
	idVec3 linearVelocity, angularVelocity;
	idMat3 inverseWorldInertiaTensor;

	linearVelocity = inverseMass * current.i.linearMomentum;
	inverseWorldInertiaTensor = current.i.orientation.Transpose() * inverseInertiaTensor * current.i.orientation;
	angularVelocity = inverseWorldInertiaTensor * current.i.angularMomentum;

	info->invMass = inverseMass;
	info->invInertiaTensor = inverseWorldInertiaTensor;
	info->position = point - ( current.i.position + centerOfMass * current.i.orientation );
	info->velocity = linearVelocity + angularVelocity.Cross( info->position );
}

/*
================
idPhysics_RigidBody::ApplyImpulse
================
*/
void idPhysics_RigidBody::ApplyImpulse( const int id, const idVec3 &point, const idVec3 &impulse ) {
	if ( noImpact ) {
		//ivan start
		if( activateNextImpact ){
			activateNextImpact = false;
			//GetSelf()->PostEventMS( &EV_ActivatePhysics, activationDelay ); //cannot do this because GetSelf() returns a const entity
			gameLocal.entities[ GetSelf()->entityNumber ]->PostEventMS( &EV_ActivatePhysics, activationDelay );
		}
		//ivan end
		return;
		
		/*
		if( nextActivationTime > 0 ){ //already started
			if( nextActivationTime < gameLocal.time ){ 
				//activated
				gameLocal.Printf("activated\n");
				//turn off both so we don't start this again
				nextActivationTime = -1; 
				activationDelay = -1;
			}else{
				//wait again
				gameLocal.Printf("wait again\n");
				return;
			}
		}else{
			if( activationDelay != -1){ //if activationDelay is set
				nextActivationTime = gameLocal.time + activationDelay;
				gameLocal.Printf("start wait\n");
			}
			return;
		}
		*/
		//ivan end
	}
	current.i.linearMomentum += impulse;
	current.i.angularMomentum += ( point - ( current.i.position + centerOfMass * current.i.orientation ) ).Cross( impulse );
	Activate();
}

/*
================
idPhysics_RigidBody::AddForce
================
*/
void idPhysics_RigidBody::AddForce( const int id, const idVec3 &point, const idVec3 &force ) {
	if ( noImpact ) {
		//ivan start
		if( activateNextImpact ){
			activateNextImpact = false;
			//GetSelf()->PostEventMS( &EV_ActivatePhysics, activationDelay ); //cannot do this because GetSelf() returns a const entity
			gameLocal.entities[ GetSelf()->entityNumber ]->PostEventMS( &EV_ActivatePhysics, activationDelay );
		}
		//ivan end
		return;
		
		/*
		if( nextActivationTime > 0 ){ //already started
			if( nextActivationTime < gameLocal.time ){ 
				//activated
				gameLocal.Printf("activated\n");
				//turn off both so we don't start this again
				nextActivationTime = -1; 
				activationDelay = -1;
			}else{
				//wait again
				gameLocal.Printf("wait again\n");
				return;
			}
		}else{
			if( activationDelay != -1){ //if activationDelay is set
				nextActivationTime = gameLocal.time + activationDelay;
				gameLocal.Printf("start wait\n");
			}
			return;
		}
		*/
		//ivan end
	}
	current.externalForce += force;
	current.externalTorque += ( point - ( current.i.position + centerOfMass * current.i.orientation ) ).Cross( force );
	Activate();
}

/*
================
idPhysics_RigidBody::IsAtRest
================
*/
bool idPhysics_RigidBody::IsAtRest( void ) const {
	return current.atRest >= 0;
}

/*
================
idPhysics_RigidBody::GetRestStartTime
================
*/
int idPhysics_RigidBody::GetRestStartTime( void ) const {
	return current.atRest;
}

/*
================
idPhysics_RigidBody::IsPushable
================
*/
bool idPhysics_RigidBody::IsPushable( void ) const {
	return ( !noImpact && !hasMaster );
}

/*
================
idPhysics_RigidBody::SaveState
================
*/
void idPhysics_RigidBody::SaveState( void ) {
	saved = current;
}

/*
================
idPhysics_RigidBody::RestoreState
================
*/
void idPhysics_RigidBody::RestoreState( void ) {
	current = saved;

	clipModel->Link( gameLocal.clip, self, clipModel->GetId(), current.i.position, current.i.orientation );

	EvaluateContacts();
}

/*
================
idPhysics::SetOrigin
================
*/
void idPhysics_RigidBody::SetOrigin( const idVec3 &newOrigin, int id ) {
	idVec3 masterOrigin;
	idMat3 masterAxis;

	current.localOrigin = newOrigin;
	if ( hasMaster ) {
		self->GetMasterPosition( masterOrigin, masterAxis );
		current.i.position = masterOrigin + newOrigin * masterAxis;
	}
	else {
		current.i.position = newOrigin;
	}

	clipModel->Link( gameLocal.clip, self, clipModel->GetId(), current.i.position, clipModel->GetAxis() );

	Activate();
}

/*
================
idPhysics::SetAxis
================
*/
void idPhysics_RigidBody::SetAxis( const idMat3 &newAxis, int id ) {
	idVec3 masterOrigin;
	idMat3 masterAxis;

	current.localAxis = newAxis;
	if ( hasMaster && isOrientated ) {
		self->GetMasterPosition( masterOrigin, masterAxis );
		current.i.orientation = newAxis * masterAxis;
	}
	else {
		current.i.orientation = newAxis;
	}

	clipModel->Link( gameLocal.clip, self, clipModel->GetId(), clipModel->GetOrigin(), current.i.orientation );

	Activate();
}

/*
================
idPhysics::Move
================
*/
void idPhysics_RigidBody::Translate( const idVec3 &translation, int id ) {

	current.localOrigin += translation;
	current.i.position += translation;

	clipModel->Link( gameLocal.clip, self, clipModel->GetId(), current.i.position, clipModel->GetAxis() );

	Activate();
}

/*
================
idPhysics::Rotate
================
*/
void idPhysics_RigidBody::Rotate( const idRotation &rotation, int id ) {
	idVec3 masterOrigin;
	idMat3 masterAxis;

	current.i.orientation *= rotation.ToMat3();
	current.i.position *= rotation;

	if ( hasMaster ) {
		self->GetMasterPosition( masterOrigin, masterAxis );
		current.localAxis *= rotation.ToMat3();
		current.localOrigin = ( current.i.position - masterOrigin ) * masterAxis.Transpose();
	}
	else {
		current.localAxis = current.i.orientation;
		current.localOrigin = current.i.position;
	}

	clipModel->Link( gameLocal.clip, self, clipModel->GetId(), current.i.position, current.i.orientation );

	Activate();
}

/*
================
idPhysics_RigidBody::GetOrigin
================
*/
const idVec3 &idPhysics_RigidBody::GetOrigin( int id ) const {
	return current.i.position;
}

/*
================
idPhysics_RigidBody::GetAxis
================
*/
const idMat3 &idPhysics_RigidBody::GetAxis( int id ) const {
	return current.i.orientation;
}

/*
================
idPhysics_RigidBody::SetLinearVelocity
================
*/
void idPhysics_RigidBody::SetLinearVelocity( const idVec3 &newLinearVelocity, int id ) {
	current.i.linearMomentum = newLinearVelocity * mass;
	Activate();
}

/*
================
idPhysics_RigidBody::SetAngularVelocity
================
*/
void idPhysics_RigidBody::SetAngularVelocity( const idVec3 &newAngularVelocity, int id ) {
	current.i.angularMomentum = newAngularVelocity * inertiaTensor;
	Activate();
}

/*
================
idPhysics_RigidBody::GetLinearVelocity
================
*/
const idVec3 &idPhysics_RigidBody::GetLinearVelocity( int id ) const {
	static idVec3 curLinearVelocity;
	curLinearVelocity = current.i.linearMomentum * inverseMass;
	return curLinearVelocity;
}

/*
================
idPhysics_RigidBody::GetAngularVelocity
================
*/
const idVec3 &idPhysics_RigidBody::GetAngularVelocity( int id ) const {
	static idVec3 curAngularVelocity;
	idMat3 inverseWorldInertiaTensor;

	inverseWorldInertiaTensor = current.i.orientation.Transpose() * inverseInertiaTensor * current.i.orientation;
	curAngularVelocity = inverseWorldInertiaTensor * current.i.angularMomentum;
	return curAngularVelocity;
}

/*
================
idPhysics_RigidBody::ClipTranslation
================
*/
void idPhysics_RigidBody::ClipTranslation( trace_t &results, const idVec3 &translation, const idClipModel *model ) const {
	if ( model ) {
		gameLocal.clip.TranslationModel( results, clipModel->GetOrigin(), clipModel->GetOrigin() + translation,
											clipModel, clipModel->GetAxis(), clipMask,
											model->Handle(), model->GetOrigin(), model->GetAxis() );
	}
	else {
		gameLocal.clip.Translation( results, clipModel->GetOrigin(), clipModel->GetOrigin() + translation,
											clipModel, clipModel->GetAxis(), clipMask, self );
	}
}

/*
================
idPhysics_RigidBody::ClipRotation
================
*/
void idPhysics_RigidBody::ClipRotation( trace_t &results, const idRotation &rotation, const idClipModel *model ) const {
	if ( model ) {
		gameLocal.clip.RotationModel( results, clipModel->GetOrigin(), rotation,
											clipModel, clipModel->GetAxis(), clipMask,
											model->Handle(), model->GetOrigin(), model->GetAxis() );
	}
	else {
		gameLocal.clip.Rotation( results, clipModel->GetOrigin(), rotation,
											clipModel, clipModel->GetAxis(), clipMask, self );
	}
}

/*
================
idPhysics_RigidBody::ClipContents
================
*/
int idPhysics_RigidBody::ClipContents( const idClipModel *model ) const {
	if ( model ) {
		return gameLocal.clip.ContentsModel( clipModel->GetOrigin(), clipModel, clipModel->GetAxis(), -1,
									model->Handle(), model->GetOrigin(), model->GetAxis() );
	}
	else {
		return gameLocal.clip.Contents( clipModel->GetOrigin(), clipModel, clipModel->GetAxis(), -1, NULL );
	}
}

/*
================
idPhysics_RigidBody::DisableClip
================
*/
void idPhysics_RigidBody::DisableClip( void ) {
	clipModel->Disable();
}

/*
================
idPhysics_RigidBody::EnableClip
================
*/
void idPhysics_RigidBody::EnableClip( void ) {
	clipModel->Enable();
}

/*
================
idPhysics_RigidBody::UnlinkClip
================
*/
void idPhysics_RigidBody::UnlinkClip( void ) {
	clipModel->Unlink();
}

/*
================
idPhysics_RigidBody::LinkClip
================
*/
void idPhysics_RigidBody::LinkClip( void ) {
	clipModel->Link( gameLocal.clip, self, clipModel->GetId(), current.i.position, current.i.orientation );
}

/*
================
idPhysics_RigidBody::EvaluateContacts
================
*/
bool idPhysics_RigidBody::EvaluateContacts( void ) {
	idVec6 dir;
	int num;

	ClearContacts();

	contacts.SetNum( 10, false );

	dir.SubVec3(0) = current.i.linearMomentum + current.lastTimeStep * gravityVector * mass;
	dir.SubVec3(1) = current.i.angularMomentum;
	dir.SubVec3(0).Normalize();
	dir.SubVec3(1).Normalize();
	num = gameLocal.clip.Contacts( &contacts[0], 10, clipModel->GetOrigin(),
					dir, CONTACT_EPSILON, clipModel, clipModel->GetAxis(), clipMask, self );
	contacts.SetNum( num, false );

	AddContactEntitiesForContacts();

	return ( contacts.Num() != 0 );
}

/*
================
idPhysics_RigidBody::SetPushed
================
*/
void idPhysics_RigidBody::SetPushed( int deltaTime ) {
	idRotation rotation;

	rotation = ( saved.i.orientation * current.i.orientation ).ToRotation();

	// velocity with which the af is pushed
	current.pushVelocity.SubVec3(0) += ( current.i.position - saved.i.position ) / ( deltaTime * idMath::M_MS2SEC );
	current.pushVelocity.SubVec3(1) += rotation.GetVec() * -DEG2RAD( rotation.GetAngle() ) / ( deltaTime * idMath::M_MS2SEC );
}

/*
================
idPhysics_RigidBody::GetPushedLinearVelocity
================
*/
const idVec3 &idPhysics_RigidBody::GetPushedLinearVelocity( const int id ) const {
	return current.pushVelocity.SubVec3(0);
}

/*
================
idPhysics_RigidBody::GetPushedAngularVelocity
================
*/
const idVec3 &idPhysics_RigidBody::GetPushedAngularVelocity( const int id ) const {
	return current.pushVelocity.SubVec3(1);
}

/*
================
idPhysics_RigidBody::SetMaster
================
*/
void idPhysics_RigidBody::SetMaster( idEntity *master, const bool orientated ) {
	idVec3 masterOrigin;
	idMat3 masterAxis;

	if ( master ) {
		if ( !hasMaster ) {
			// transform from world space to master space
			self->GetMasterPosition( masterOrigin, masterAxis );
			current.localOrigin = ( current.i.position - masterOrigin ) * masterAxis.Transpose();
			if ( orientated ) {
				current.localAxis = current.i.orientation * masterAxis.Transpose();
			}
			else {
				current.localAxis = current.i.orientation;
			}
			hasMaster = true;
			isOrientated = orientated;
			ClearContacts();
		}
	}
	else {
		if ( hasMaster ) {
			hasMaster = false;
			Activate();
		}
	}
}

const float	RB_VELOCITY_MAX				= 16000;
const int	RB_VELOCITY_TOTAL_BITS		= 16;
const int	RB_VELOCITY_EXPONENT_BITS	= idMath::BitsForInteger( idMath::BitsForFloat( RB_VELOCITY_MAX ) ) + 1;
const int	RB_VELOCITY_MANTISSA_BITS	= RB_VELOCITY_TOTAL_BITS - 1 - RB_VELOCITY_EXPONENT_BITS;
const float	RB_MOMENTUM_MAX				= 1e20f;
const int	RB_MOMENTUM_TOTAL_BITS		= 16;
const int	RB_MOMENTUM_EXPONENT_BITS	= idMath::BitsForInteger( idMath::BitsForFloat( RB_MOMENTUM_MAX ) ) + 1;
const int	RB_MOMENTUM_MANTISSA_BITS	= RB_MOMENTUM_TOTAL_BITS - 1 - RB_MOMENTUM_EXPONENT_BITS;
const float	RB_FORCE_MAX				= 1e20f;
const int	RB_FORCE_TOTAL_BITS			= 16;
const int	RB_FORCE_EXPONENT_BITS		= idMath::BitsForInteger( idMath::BitsForFloat( RB_FORCE_MAX ) ) + 1;
const int	RB_FORCE_MANTISSA_BITS		= RB_FORCE_TOTAL_BITS - 1 - RB_FORCE_EXPONENT_BITS;

/*
================
idPhysics_RigidBody::WriteToSnapshot
================
*/
void idPhysics_RigidBody::WriteToSnapshot( idBitMsgDelta &msg ) const {
	idCQuat quat, localQuat;

	quat = current.i.orientation.ToCQuat();
	localQuat = current.localAxis.ToCQuat();

	msg.WriteInt( current.atRest );
	msg.WriteFloat( current.i.position[0] );
	msg.WriteFloat( current.i.position[1] );
	msg.WriteFloat( current.i.position[2] );
	msg.WriteFloat( quat.x );
	msg.WriteFloat( quat.y );
	msg.WriteFloat( quat.z );
	msg.WriteFloat( current.i.linearMomentum[0], RB_MOMENTUM_EXPONENT_BITS, RB_MOMENTUM_MANTISSA_BITS );
	msg.WriteFloat( current.i.linearMomentum[1], RB_MOMENTUM_EXPONENT_BITS, RB_MOMENTUM_MANTISSA_BITS );
	msg.WriteFloat( current.i.linearMomentum[2], RB_MOMENTUM_EXPONENT_BITS, RB_MOMENTUM_MANTISSA_BITS );
	msg.WriteFloat( current.i.angularMomentum[0], RB_MOMENTUM_EXPONENT_BITS, RB_MOMENTUM_MANTISSA_BITS );
	msg.WriteFloat( current.i.angularMomentum[1], RB_MOMENTUM_EXPONENT_BITS, RB_MOMENTUM_MANTISSA_BITS );
	msg.WriteFloat( current.i.angularMomentum[2], RB_MOMENTUM_EXPONENT_BITS, RB_MOMENTUM_MANTISSA_BITS );
	msg.WriteDeltaFloat( current.i.position[0], current.localOrigin[0] );
	msg.WriteDeltaFloat( current.i.position[1], current.localOrigin[1] );
	msg.WriteDeltaFloat( current.i.position[2], current.localOrigin[2] );
	msg.WriteDeltaFloat( quat.x, localQuat.x );
	msg.WriteDeltaFloat( quat.y, localQuat.y );
	msg.WriteDeltaFloat( quat.z, localQuat.z );
	msg.WriteDeltaFloat( 0.0f, current.pushVelocity[0], RB_VELOCITY_EXPONENT_BITS, RB_VELOCITY_MANTISSA_BITS );
	msg.WriteDeltaFloat( 0.0f, current.pushVelocity[1], RB_VELOCITY_EXPONENT_BITS, RB_VELOCITY_MANTISSA_BITS );
	msg.WriteDeltaFloat( 0.0f, current.pushVelocity[2], RB_VELOCITY_EXPONENT_BITS, RB_VELOCITY_MANTISSA_BITS );
	msg.WriteDeltaFloat( 0.0f, current.externalForce[0], RB_FORCE_EXPONENT_BITS, RB_FORCE_MANTISSA_BITS );
	msg.WriteDeltaFloat( 0.0f, current.externalForce[1], RB_FORCE_EXPONENT_BITS, RB_FORCE_MANTISSA_BITS );
	msg.WriteDeltaFloat( 0.0f, current.externalForce[2], RB_FORCE_EXPONENT_BITS, RB_FORCE_MANTISSA_BITS );
	msg.WriteDeltaFloat( 0.0f, current.externalTorque[0], RB_FORCE_EXPONENT_BITS, RB_FORCE_MANTISSA_BITS );
	msg.WriteDeltaFloat( 0.0f, current.externalTorque[1], RB_FORCE_EXPONENT_BITS, RB_FORCE_MANTISSA_BITS );
	msg.WriteDeltaFloat( 0.0f, current.externalTorque[2], RB_FORCE_EXPONENT_BITS, RB_FORCE_MANTISSA_BITS );
}

/*
================
idPhysics_RigidBody::ReadFromSnapshot
================
*/
void idPhysics_RigidBody::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	idCQuat quat, localQuat;

	current.atRest = msg.ReadInt();
	current.i.position[0] = msg.ReadFloat();
	current.i.position[1] = msg.ReadFloat();
	current.i.position[2] = msg.ReadFloat();
	quat.x = msg.ReadFloat();
	quat.y = msg.ReadFloat();
	quat.z = msg.ReadFloat();
	current.i.linearMomentum[0] = msg.ReadFloat( RB_MOMENTUM_EXPONENT_BITS, RB_MOMENTUM_MANTISSA_BITS );
	current.i.linearMomentum[1] = msg.ReadFloat( RB_MOMENTUM_EXPONENT_BITS, RB_MOMENTUM_MANTISSA_BITS );
	current.i.linearMomentum[2] = msg.ReadFloat( RB_MOMENTUM_EXPONENT_BITS, RB_MOMENTUM_MANTISSA_BITS );
	current.i.angularMomentum[0] = msg.ReadFloat( RB_MOMENTUM_EXPONENT_BITS, RB_MOMENTUM_MANTISSA_BITS );
	current.i.angularMomentum[1] = msg.ReadFloat( RB_MOMENTUM_EXPONENT_BITS, RB_MOMENTUM_MANTISSA_BITS );
	current.i.angularMomentum[2] = msg.ReadFloat( RB_MOMENTUM_EXPONENT_BITS, RB_MOMENTUM_MANTISSA_BITS );
	current.localOrigin[0] = msg.ReadDeltaFloat( current.i.position[0] );
	current.localOrigin[1] = msg.ReadDeltaFloat( current.i.position[1] );
	current.localOrigin[2] = msg.ReadDeltaFloat( current.i.position[2] );
	localQuat.x = msg.ReadDeltaFloat( quat.x );
	localQuat.y = msg.ReadDeltaFloat( quat.y );
	localQuat.z = msg.ReadDeltaFloat( quat.z );
	current.pushVelocity[0] = msg.ReadDeltaFloat( 0.0f, RB_VELOCITY_EXPONENT_BITS, RB_VELOCITY_MANTISSA_BITS );
	current.pushVelocity[1] = msg.ReadDeltaFloat( 0.0f, RB_VELOCITY_EXPONENT_BITS, RB_VELOCITY_MANTISSA_BITS );
	current.pushVelocity[2] = msg.ReadDeltaFloat( 0.0f, RB_VELOCITY_EXPONENT_BITS, RB_VELOCITY_MANTISSA_BITS );
	current.externalForce[0] = msg.ReadDeltaFloat( 0.0f, RB_FORCE_EXPONENT_BITS, RB_FORCE_MANTISSA_BITS );
	current.externalForce[1] = msg.ReadDeltaFloat( 0.0f, RB_FORCE_EXPONENT_BITS, RB_FORCE_MANTISSA_BITS );
	current.externalForce[2] = msg.ReadDeltaFloat( 0.0f, RB_FORCE_EXPONENT_BITS, RB_FORCE_MANTISSA_BITS );
	current.externalTorque[0] = msg.ReadDeltaFloat( 0.0f, RB_FORCE_EXPONENT_BITS, RB_FORCE_MANTISSA_BITS );
	current.externalTorque[1] = msg.ReadDeltaFloat( 0.0f, RB_FORCE_EXPONENT_BITS, RB_FORCE_MANTISSA_BITS );
	current.externalTorque[2] = msg.ReadDeltaFloat( 0.0f, RB_FORCE_EXPONENT_BITS, RB_FORCE_MANTISSA_BITS );

	current.i.orientation = quat.ToMat3();
	current.localAxis = localQuat.ToMat3();

	if ( clipModel ) {
		clipModel->Link( gameLocal.clip, self, clipModel->GetId(), current.i.position, current.i.orientation );
	}
}
