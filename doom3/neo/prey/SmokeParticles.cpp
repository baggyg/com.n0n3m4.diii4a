// Copyright (C) 2004 Id Software, Inc.
//

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"

static const char *smokeParticle_SnapshotName = "_SmokeParticle_Snapshot_";

/*
================
idSmokeParticles::idSmokeParticles
================
*/
idSmokeParticles::idSmokeParticles( void ) {
	initialized = false;
	memset( &renderEntity, 0, sizeof( renderEntity ) );
	renderEntityHandle = -1;
	memset( smokes, 0, sizeof( smokes ) );
	freeSmokes = NULL;
	numActiveSmokes = 0;
	currentParticleTime = -1;
}

/*
================
idSmokeParticles::Init
================
*/
void idSmokeParticles::Init( void ) {
	if ( initialized ) {
		Shutdown();
	}

	// set up the free list
	for ( int i = 0; i < MAX_SMOKE_PARTICLES-1; i++ ) {
		smokes[i].next = &smokes[i+1];
	}
	smokes[MAX_SMOKE_PARTICLES-1].next = NULL;
	freeSmokes = &smokes[0];
	numActiveSmokes = 0;

	activeStages.Clear();

	memset( &renderEntity, 0, sizeof( renderEntity ) );

	renderEntity.bounds.Clear();
	renderEntity.axis = mat3_identity;
	renderEntity.shaderParms[ SHADERPARM_RED ]		= 1;
	renderEntity.shaderParms[ SHADERPARM_GREEN ]	= 1;
	renderEntity.shaderParms[ SHADERPARM_BLUE ]		= 1;
	renderEntity.shaderParms[3] = 1;

	renderEntity.hModel = renderModelManager->AllocModel();
	renderEntity.hModel->InitEmpty( smokeParticle_SnapshotName );

#if _HH_RENDERDEMO_HACKS //HUMANHEAD rww
	#if _HH_OLD_RENDERDEMO_PARTICLES
		renderEntity.hModel->SetGameUpdatedModel(true);
	#else
		renderEntity.notInRenderDemos = true;
	#endif
#endif //HUMANHEAD END

	// we certainly don't want particle shadows
	renderEntity.noShadow = 1;

	// huge bounds, so it will be present in every world area
#if HUMANHEAD	// HUMANHEAD pdm: Our deathwalk area is above 100000
	renderEntity.bounds.AddPoint( idVec3(-200000, -200000, -200000) );
	renderEntity.bounds.AddPoint( idVec3( 200000,  200000,  200000) );
#else
	renderEntity.bounds.AddPoint( idVec3(-100000, -100000, -100000) );
	renderEntity.bounds.AddPoint( idVec3( 100000,  100000,  100000) );
#endif

	renderEntity.callback = idSmokeParticles::ModelCallback;
	// add to renderer list
	renderEntityHandle = gameRenderWorld->AddEntityDef( &renderEntity );

	currentParticleTime = -1;

	initialized = true;
}

/*
================
idSmokeParticles::Shutdown
================
*/
void idSmokeParticles::Shutdown( void ) {
	// make sure the render entity is freed before the model is freed
	if ( renderEntityHandle != -1 ) {
		gameRenderWorld->FreeEntityDef( renderEntityHandle );
		renderEntityHandle = -1;
	}
	if ( renderEntity.hModel != NULL ) {
		renderModelManager->FreeModel( renderEntity.hModel );
		renderEntity.hModel = NULL;
	}
	initialized = false;
}

/*
================
idSmokeParticles::FreeSmokes
================
*/
void idSmokeParticles::FreeSmokes( void ) {
	for ( int activeStageNum = 0; activeStageNum < activeStages.Num(); activeStageNum++ ) {
		singleSmoke_t *smoke, *next, *last;

		activeSmokeStage_t *active = &activeStages[activeStageNum];
		const idParticleStage *stage = active->stage;

		for ( last = NULL, smoke = active->smokes; smoke; smoke = next ) {
			next = smoke->next;

			float frac = (float)( gameLocal.time - smoke->privateStartTime ) / ( stage->particleLife * 1000 );
			if ( frac >= 1.0f ) {
				// remove the particle from the stage list
				if ( last != NULL ) {
					last->next = smoke->next;
				} else {
					active->smokes = smoke->next;
				}
				// put the particle on the free list
				smoke->next = freeSmokes;
				freeSmokes = smoke;
				numActiveSmokes--;
				continue;
			}

			last = smoke;
		}

		if ( !active->smokes ) {
			// remove this from the activeStages list
			activeStages.RemoveIndex( activeStageNum );
			activeStageNum--;
		}
	}
}

/*
================
idSmokeParticles::EmitSmoke

Called by game code to drop another particle into the list
================
*/
bool idSmokeParticles::EmitSmoke( const idDeclParticle *smoke, const int systemStartTime, const float diversity, const idVec3 &origin, const idMat3 &axis ) {
	bool	continues = false;

	if ( !smoke ) {
		return false;
	}

	if ( !gameLocal.isNewFrame ) {
		return false;
	}

	// dedicated doesn't smoke. No UpdateRenderEntity, so they would not be freed
	if ( gameLocal.localClientNum < 0 ) {
		return false;
	}

	assert( gameLocal.time == 0 || systemStartTime <= gameLocal.time );
	if ( systemStartTime > gameLocal.time ) {
		return false;
	}

//HUMANHEAD rww
#if _HH_RENDERDEMO_HACKS
 #if !_HH_OLD_RENDERDEMO_PARTICLES
	//gameRenderWorld->DemoSmokeEvent(smoke, gameLocal.time-systemStartTime, diversity, origin, axis); // jmarshall
 #endif
#endif
//HUMANHEAD END

	idRandom steppingRandom( 0xffff * diversity );

	// for each stage in the smoke that is still emitting particles, emit a new singleSmoke_t
	for ( int stageNum = 0; stageNum < smoke->stages.Num(); stageNum++ ) {
		const idParticleStage *stage = smoke->stages[stageNum];

		if ( !stage->cycleMsec ) {
			continue;
		}

		if ( !stage->material ) {
			continue;
		}

		if ( stage->particleLife <= 0 ) {
			continue;
		}

		// see how many particles we should emit this tic
#ifdef HUMANHEAD
		//HUMANHEAD: aob - added cycles, deadTime, and timeOffset to calculations
		int		systemTime = gameLocal.GetTime() - systemStartTime - SEC2MS(stage->timeOffset);
		int		currentCycle = (systemTime / stage->cycleMsec) + 1;
		
		if( stage->cycles && currentCycle > stage->cycles ) {
			//All done with this stage.
			continue;
		}
		
		int		inCycleTime = systemTime - (currentCycle - 1) * stage->cycleMsec;
		float	currentStageFrac = MS2SEC(inCycleTime) / stage->particleLife;
		float	previousStageFrac = currentStageFrac - MS2SEC(USERCMD_MSEC) / stage->particleLife;
		if( currentStageFrac < 0.0f ) {
			//This stage hasn't started yet
			continues = true;
			continue;
		} else if( currentStageFrac > 1.0f ) {
			//This stage is in its deadTime
			continues = true;
			continue;
		}

		//We assume that inCycleTime >= 0
		inCycleTime = hhMath::hhMax<int>( USERCMD_MSEC, inCycleTime );
		int		finalParticleTime = hhMath::hhMax<int>( USERCMD_MSEC, SEC2MS(stage->particleLife) * stage->spawnBunching );

		float	currentBunchingFrac = MS2SEC(inCycleTime) / MS2SEC(finalParticleTime);
		int		nowCount = floor( currentBunchingFrac * stage->totalParticles );
		nowCount = hhMath::ClampInt( 0, stage->totalParticles, nowCount );

		float	previousBunchingFrac = currentBunchingFrac - MS2SEC(USERCMD_MSEC) / MS2SEC(finalParticleTime);
		int		prevCount = floor( previousBunchingFrac * stage->totalParticles );
		prevCount = hhMath::ClampInt( -1, stage->totalParticles, prevCount );

		continues = true;
		if( currentBunchingFrac > 1.0f && previousBunchingFrac > 1.0f ) {
			//Don't spawn any more particles until next cycle
			continue;
		}
		//HUMANHEAD END
#else
		// FIXME: 			smoke.privateStartTime += stage->timeOffset;
		int		finalParticleTime = stage->cycleMsec * stage->spawnBunching;
		int		deltaMsec = gameLocal.time - systemStartTime;

		int		nowCount, prevCount;
		if ( finalParticleTime == 0 ) {
			// if spawnBunching is 0, they will all come out at once
			if ( gameLocal.time == systemStartTime ) {
				prevCount = -1;
				nowCount = stage->totalParticles-1;
			} else {
				prevCount = stage->totalParticles;
			}
		} else {
			nowCount = floor( ( (float)deltaMsec / finalParticleTime ) * stage->totalParticles );
			if ( nowCount >= stage->totalParticles ) {
				nowCount = stage->totalParticles-1;
			}
			prevCount = floor( ((float)( deltaMsec - USERCMD_MSEC ) / finalParticleTime) * stage->totalParticles );
			if ( prevCount < -1 ) {
				prevCount = -1;
			}
		}

		if ( prevCount >= stage->totalParticles ) {
			// no more particles from this stage
			continue;
		}

		if ( nowCount < stage->totalParticles-1 ) {
			// the system will need to emit particles next frame as well
			continues = true;
		}
#endif

		// find an activeSmokeStage that matches this
		activeSmokeStage_t	*active;
		int i;
		for ( i = 0 ; i < activeStages.Num() ; i++ ) {
			active = &activeStages[i];
			if ( active->stage == stage ) {
				break;
			}
		}
		if ( i == activeStages.Num() ) {
			// add a new one
			activeSmokeStage_t	newActive;

			newActive.smokes = NULL;
			newActive.stage = stage;
			i = activeStages.Append( newActive );
			active = &activeStages[i];
		}

		// add all the required particles
		for ( prevCount++ ; prevCount <= nowCount ; prevCount++ ) {
			if ( !freeSmokes ) {
				gameLocal.Printf( "idSmokeParticles::EmitSmoke: no free smokes with %d active stages\n", activeStages.Num() );
				return true;
			}
			singleSmoke_t	*newSmoke = freeSmokes;
			freeSmokes = freeSmokes->next;
			numActiveSmokes++;

			newSmoke->index = prevCount;
			newSmoke->axis = axis;
			newSmoke->origin = origin;
			newSmoke->random = steppingRandom;
			//HUMANHEAD: aob - changed time calculation to just gameLocal.GetTime()
			newSmoke->privateStartTime = gameLocal.GetTime();
			//HUMANHEAD END
			newSmoke->next = active->smokes;
			active->smokes = newSmoke;

			steppingRandom.RandomInt();	// advance the random
		}
	}

	return continues;
}

/*
================
idSmokeParticles::UpdateRenderEntity
================
*/
bool idSmokeParticles::UpdateRenderEntity( renderEntity_s *renderEntity, const renderView_t *renderView ) {
	// FIXME: re-use model surfaces
	renderEntity->hModel->InitEmpty( smokeParticle_SnapshotName );

	// this may be triggered by a model trace or other non-view related source,
	// to which we should look like an empty model
	if ( !renderView ) {
		return false;
	}

	// don't regenerate it if it is current
	//HUMANHEAD rww - viewID 0 is used only for subviews such as portals, so don't check the time against them
	if (g_forceSingleSmokeView.GetBool() || renderView->viewID != 0) {
		if ( renderView->time == currentParticleTime && !renderView->forceUpdate ) {
			return false;
		}
		currentParticleTime = renderView->time;
	}

	//HUMANHEAD rww
	//for renderdemo hackery purposes this is moved down here. we need to not clear surfaces until we are about
	//to actually render, so that they can be saved off.
	//note: this causes terribleness with orphaned surfaces, need a proper workaround
	//renderEntity->hModel->InitEmpty( smokeParticle_SnapshotName );
	//HUMANHEAD END

	particleGen_t g;

	g.renderEnt = renderEntity;
	g.renderView = renderView;

	for ( int activeStageNum = 0; activeStageNum < activeStages.Num(); activeStageNum++ ) {
		singleSmoke_t *smoke, *next, *last;

		activeSmokeStage_t *active = &activeStages[activeStageNum];
		const idParticleStage *stage = active->stage;

		if ( !stage->material ) {
			continue;
		}

		// allocate a srfTriangles that can hold all the particles
		int count = 0;
		for ( smoke = active->smokes; smoke; smoke = smoke->next ) {
			count++;
		}
		int	quads = count * stage->NumQuadsPerParticle();
		srfTriangles_t *tri = renderEntity->hModel->AllocSurfaceTriangles( quads * 4, quads * 6 );
		tri->numIndexes = quads * 6;
		tri->numVerts = quads * 4;

		// just always draw the particles
#if HUMANHEAD	// HUMANHEAD pdm: Our deathwalk area is above 100000
		tri->bounds[0][0] =
		tri->bounds[0][1] =
		tri->bounds[0][2] = -199999;
		tri->bounds[1][0] =
		tri->bounds[1][1] =
		tri->bounds[1][2] = 199999;
#else
		tri->bounds[0][0] =
		tri->bounds[0][1] =
		tri->bounds[0][2] = -99999;
		tri->bounds[1][0] =
		tri->bounds[1][1] =
		tri->bounds[1][2] = 99999;
#endif

		tri->numVerts = 0;
		for ( last = NULL, smoke = active->smokes; smoke; smoke = next ) {
			next = smoke->next;

			g.frac = (float)( gameLocal.time - smoke->privateStartTime ) / ( stage->particleLife * 1000 );
			if ( g.frac >= 1.0f ) {
				// remove the particle from the stage list
				if ( last != NULL ) {
					last->next = smoke->next;
				} else {
					active->smokes = smoke->next;
				}
				// put the particle on the free list
				smoke->next = freeSmokes;
				freeSmokes = smoke;
				numActiveSmokes--;
				continue;
			}

			g.index = smoke->index;
			g.random = smoke->random;

			g.origin = smoke->origin;
			g.axis = smoke->axis;

			g.originalRandom = g.random;
			g.age = g.frac * stage->particleLife;

			tri->numVerts += stage->CreateParticle( &g, tri->verts + tri->numVerts );

			last = smoke;
		}
		if ( tri->numVerts > quads * 4 ) {
			gameLocal.Error( "idSmokeParticles::UpdateRenderEntity: miscounted verts" );
		}

		if ( tri->numVerts == 0 ) {

			// they were all removed
			renderEntity->hModel->FreeSurfaceTriangles( tri );

			if ( !active->smokes ) {
				// remove this from the activeStages list
				activeStages.RemoveIndex( activeStageNum );
				activeStageNum--;
			}
		} else {
			// build the index list
			int	indexes = 0;
			for ( int i = 0 ; i < tri->numVerts ; i += 4 ) {
				tri->indexes[indexes+0] = i;
				tri->indexes[indexes+1] = i+2;
				tri->indexes[indexes+2] = i+3;
				tri->indexes[indexes+3] = i;
				tri->indexes[indexes+4] = i+3;
				tri->indexes[indexes+5] = i+1;
				indexes += 6;
			}
			tri->numIndexes = indexes;

			modelSurface_t	surf;
			surf.geometry = tri;
			surf.shader = stage->material;
			surf.id = 0;

			//HUMANHEAD rww - for debugging particle locations
			//==================================================================
			//Debug_ClearDebugLines();
			//Debug_AddDebugLinesForTri(tri);
			//==================================================================

			renderEntity->hModel->AddSurface( surf );
		}
	}
	return true;
}

/*
================
idSmokeParticles::ModelCallback
================
*/
bool idSmokeParticles::ModelCallback( renderEntity_s *renderEntity, const renderView_t *renderView ) {
	// update the particles
	if ( gameLocal.smokeParticles ) {
		return gameLocal.smokeParticles->UpdateRenderEntity( renderEntity, renderView );
	}

	return true;
}
