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

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"

const int ASYNC_PLAYER_INV_AMMO_BITS = idMath::BitsForInteger( 999 );	// 9 bits to cover the range [0, 999]
const int ASYNC_PLAYER_INV_CLIP_BITS = -7;								// -7 bits to cover the range [-1, 60]

/*
===============================================================================

Player control of the Doom Marine.
This object handles all player movement and world interaction.

===============================================================================
*/

//ivan start
const int SECRET_SCORE = 0;
const int OBJCOMPL_SCORE = 0;
const int EXTRALIFE_SCORE_TRESHOLD = 10000;
//ivan end

// distance between ladder rungs (actually is half that distance, but this sounds better)
const int LADDER_RUNG_DISTANCE = 32;

// amount of health per dose from the health station
const int HEALTH_PER_DOSE = 10;

// time before a weapon dropped to the floor disappears
const int WEAPON_DROP_TIME = 20 * 1000;

// time before a next or prev weapon switch happens
const int WEAPON_SWITCH_DELAY = 150;

// how many units to raise spectator above default view height so it's in the head of someone
const int SPECTATE_RAISE = 25;

const int HEALTHPULSE_TIME = 333;

// minimum speed to bob and play run/walk animations at
const float MIN_BOB_SPEED = 5.0f;

const idEventDef EV_Player_GetButtons( "getButtons", NULL, 'd' );
const idEventDef EV_Player_GetMove( "getMove", NULL, 'v' );
const idEventDef EV_Player_GetViewAngles( "getViewAngles", NULL, 'v' );
const idEventDef EV_Player_StopFxFov( "stopFxFov" );
const idEventDef EV_Player_EnableWeapon( "enableWeapon" );
const idEventDef EV_Player_DisableWeapon( "disableWeapon" );
const idEventDef EV_Player_GetCurrentWeapon( "getCurrentWeapon", NULL, 's' );
const idEventDef EV_Player_GetPreviousWeapon( "getPreviousWeapon", NULL, 's' );
const idEventDef EV_Player_SelectWeapon( "selectWeapon", "s" );
const idEventDef EV_Player_GetWeaponEntity( "getWeaponEntity", NULL, 'e' );
const idEventDef EV_Player_OpenPDA( "openPDA" );
const idEventDef EV_Player_InPDA( "inPDA", NULL, 'd' );
const idEventDef EV_Player_ExitTeleporter( "exitTeleporter" );
const idEventDef EV_Player_StopAudioLog( "stopAudioLog" );
const idEventDef EV_Player_HideTip( "hideTip" );
const idEventDef EV_Player_LevelTrigger( "levelTrigger" );
const idEventDef EV_SpectatorTouch( "spectatorTouch", "et" );
const idEventDef EV_Player_GetIdealWeapon( "getIdealWeapon", NULL, 's' );

const idEventDef EV_Player_WeaponAvailable( "weaponAvailable", "s", 'd');//
const idEventDef EV_Player_GetImpulseKey( "getImpulseKey", NULL, 'd' ); // Added By Clone JC Denton

//ivan start
const idEventDef EV_Player_HudEvent( "hudEvent", "s" ); 
const idEventDef EV_Player_SetHudParm( "setHudParm", "ss" );
const idEventDef EV_Player_GetHudFloat( "getHudFloat", "s", 'f');
const idEventDef EV_Player_ShowStats( "showStats" );
const idEventDef EV_Player_DropWeapon( "<dropWeapon>", "d" );
const idEventDef EV_Player_DoubleJumpEnabled( "doubleJumpEnabled", "d" );
const idEventDef EV_Player_WallJumpEnabled( "wallJumpEnabled", "d" );
const idEventDef EV_Player_SetFullBodyAnimOn( "setFullBodyAnimOn", "ddd" ); //rev 2019 rivensin is ddd. HQ was d
const idEventDef EV_Player_SetFullBodyAnimOff( "setFullBodyAnimOff" );
const idEventDef EV_Player_SetGravityInAnimMove( "setGravityInAnimMove", "f" );

const idEventDef EV_Player_HideInfo( "hideInfo" );
const idEventDef EV_Player_GetWaterLevel( "getWaterLevel", NULL, 'd' );

//smart AI start
const idEventDef EV_Player_ForceUpdateNpcStatus( "forceUpdateNpcStatus" );
const idEventDef EV_Player_StartKick( "startKick", "sf" );
const idEventDef EV_Player_StopKick( "stopKick" );

const idEventDef EV_Player_SetCommonEnemy( "setCommonEnemy", "E" );
const idEventDef EV_Player_GetCommonEnemy( "getCommonEnemy", NULL, 'e' );
//smart AI end

//ivan end

CLASS_DECLARATION( idActor, idPlayer )
EVENT( EV_Player_GetButtons,			idPlayer::Event_GetButtons )
EVENT( EV_Player_GetMove,				idPlayer::Event_GetMove )
EVENT( EV_Player_GetViewAngles,			idPlayer::Event_GetViewAngles )
EVENT( EV_Player_StopFxFov,				idPlayer::Event_StopFxFov )
EVENT( EV_Player_EnableWeapon,			idPlayer::Event_EnableWeapon )
EVENT( EV_Player_DisableWeapon,			idPlayer::Event_DisableWeapon )
EVENT( EV_Player_GetCurrentWeapon,		idPlayer::Event_GetCurrentWeapon )
EVENT( EV_Player_GetPreviousWeapon,		idPlayer::Event_GetPreviousWeapon )
EVENT( EV_Player_SelectWeapon,			idPlayer::Event_SelectWeapon )
EVENT( EV_Player_GetWeaponEntity,		idPlayer::Event_GetWeaponEntity )
EVENT( EV_Player_OpenPDA,				idPlayer::Event_OpenPDA )
EVENT( EV_Player_InPDA,					idPlayer::Event_InPDA )
EVENT( EV_Player_ExitTeleporter,		idPlayer::Event_ExitTeleporter )
EVENT( EV_Player_StopAudioLog,			idPlayer::Event_StopAudioLog )
EVENT( EV_Player_HideTip,				idPlayer::Event_HideTip )
EVENT( EV_Player_LevelTrigger,			idPlayer::Event_LevelTrigger )
EVENT( EV_Gibbed,						idPlayer::Event_Gibbed )
EVENT( EV_Player_WeaponAvailable,		idPlayer::Event_WeaponAvailable )//
EVENT( EV_Player_GetImpulseKey,			idPlayer::Event_GetImpulseKey )	// Added By Clone JCD
EVENT( EV_Player_GetIdealWeapon,		idPlayer::Event_GetIdealWeapon )

//ivan start
EVENT( EV_Player_SetFullBodyAnimOn,     idPlayer::Event_SetFullBodyAnimOn) 
EVENT( EV_Player_SetFullBodyAnimOff,    idPlayer::Event_SetFullBodyAnimOff)
EVENT( EV_Player_SetGravityInAnimMove,  idPlayer::Event_SetGravityInAnimMove)
EVENT( EV_Player_StartKick,				idPlayer::Event_StartKick )
EVENT( EV_Player_StopKick,			    idPlayer::Event_StopKick )
EVENT( EV_Weapon_StartAutoMelee,		idPlayer::Event_StartAutoMelee )		//proxy for weapon
EVENT( EV_Weapon_StopAutoMelee,			idPlayer::Event_StopAutoMelee )			//proxy for weapon
EVENT( EV_Player_HudEvent,				idPlayer::Event_HudEvent) 
EVENT( EV_Player_SetHudParm,			idPlayer::Event_SetHudParm) 
EVENT( EV_Player_GetHudFloat,			idPlayer::Event_GetHudFloat) 
EVENT( EV_Player_ShowStats,				idPlayer::Event_ShowStats) 
EVENT( EV_Player_DropWeapon,			idPlayer::Event_DropWeapon) 
EVENT( EV_Player_DoubleJumpEnabled,		idPlayer::Event_DoubleJumpEnabled) 
EVENT( EV_Player_WallJumpEnabled,		idPlayer::Event_WallJumpEnabled) 
EVENT( EV_SetSkin,						idPlayer::Event_SetSkin ) //override idEntity::Event_SetSkin
EVENT( EV_Player_HideInfo,				idPlayer::Event_HideInfo )
EVENT( EV_Player_GetWaterLevel,			idPlayer::Event_GetWaterLevel)


//smart AI start
EVENT( EV_Player_ForceUpdateNpcStatus,  idPlayer::Event_ForceUpdateNpcStatus)
EVENT( EV_Player_SetCommonEnemy,		idPlayer::Event_SetCommonEnemy)
EVENT( EV_Player_GetCommonEnemy,		idPlayer::Event_GetCommonEnemy) 
//smart AI end

//ivan end
END_CLASS

const int MAX_RESPAWN_TIME = 10000;
const int RAGDOLL_DEATH_TIME = 500; //ivan -was 3000;
const int MAX_PDAS = 64;
const int MAX_PDA_ITEMS = 128;
const int STEPUP_TIME = 200;
const int MAX_INVENTORY_ITEMS = 20;

idVec3 idPlayer::colorBarTable[ 5 ] = {
	idVec3( 0.25f, 0.25f, 0.25f ),
	idVec3( 1.00f, 0.00f, 0.00f ),
	idVec3( 0.00f, 0.80f, 0.10f ),
	idVec3( 0.20f, 0.50f, 0.80f ),
	idVec3( 1.00f, 0.80f, 0.10f )
};

/*
==============
idInventory::Clear
==============
*/
void idInventory::Clear( void ) {
	maxHealth		= 0;
	weapons			= 0;
	powerups		= 0;
	armor			= 0;
	maxarmor		= 0;
	deplete_armor	= 0;
	deplete_rate	= 0.0f;
	deplete_ammount	= 0;
	nextArmorDepleteTime = 0;

	memset( ammo, 0, sizeof( ammo ) );

	ClearPowerUps();

	// set to -1 so that the gun knows to have a full clip the first time we get it and at the start of the level
	memset( clip, -1, sizeof( clip ) );

	items.DeleteContents( true );
	memset(pdasViewed, 0, 4 * sizeof( pdasViewed[0] ) );
	pdas.Clear();
	videos.Clear();
	emails.Clear();
	selVideo = 0;
	selEMail = 0;
	selPDA = 0;
	selAudio = 0;
	pdaOpened = false;
	turkeyScore = false;

	levelTriggers.Clear();

	nextItemPickup = 0;
	nextItemNum = 1;
	onePickupTime = 0;
	pickupItemNames.Clear();
	objectiveNames.Clear();

	ammoPredictTime = 0;

	lastGiveTime = 0;

	ammoPulse	= false;
	weaponPulse	= false;
	armorPulse	= false;

	//ivan start
	nextWeaponPickup = 0; 
	memset( weaponSlot, -1, sizeof( weaponSlot ) ); 
	//ivan end
}

/*
==============
idInventory::GivePowerUp
==============
*/
void idInventory::GivePowerUp( idPlayer *player, int powerup, int msec ) {
	if ( !msec ) {
		// get the duration from the .def files
		const idDeclEntityDef *def = NULL;
		switch ( powerup ) {
			case BERSERK:
				def = gameLocal.FindEntityDef( "powerup_berserk", false );
				break;
			case INVISIBILITY:
				def = gameLocal.FindEntityDef( "powerup_invisibility", false );
				break;
			case MEGAHEALTH:
				def = gameLocal.FindEntityDef( "powerup_megahealth", false );
				break;
			case ADRENALINE:
				def = gameLocal.FindEntityDef( "powerup_adrenaline", false );
				break;
		}
		assert( def );
		msec = def->dict.GetInt( "time" ) * 1000;
	}
	powerups |= 1 << powerup;
	powerupEndTime[ powerup ] = gameLocal.time + msec;
}

/*
==============
idInventory::ClearPowerUps
==============
*/
void idInventory::ClearPowerUps( void ) {
	int i;
	for ( i = 0; i < MAX_POWERUPS; i++ ) {
		powerupEndTime[ i ] = 0;
	}
	powerups = 0;
}

/*
==============
idInventory::GetPersistantData
==============
*/
void idInventory::GetPersistantData( idDict &dict ) {
	int		i;
	int		num;
	idDict	*item;
	idStr	key;
	const idKeyValue *kv;
	const char *name;

	// armor
	dict.SetInt( "armor", armor ); 

	// don't bother with powerups, maxhealth, maxarmor, or the clip

	// ammo
	for( i = 0; i < AMMO_NUMTYPES; i++ ) {
		name = idWeapon::GetAmmoNameForNum( ( ammo_t )i );
		if ( name ) {
			dict.SetInt( name, ammo[ i ] );
		}
	}
	//Save the clip data //un noted change from original sdk
	for( i = 0; i < MAX_WEAPONS; i++ ) {              //new
		dict.SetInt( va("clip%i", i), clip[ i ] );
	}
	// items
	num = 0;
	for( i = 0; i < items.Num(); i++ ) {
		item = items[ i ];

		// copy all keys with "inv_"
		kv = item->MatchPrefix( "inv_" );
		if ( kv ) {
			while( kv ) {
				sprintf( key, "item_%i %s", num, kv->GetKey().c_str() );
				dict.Set( key, kv->GetValue() );
				kv = item->MatchPrefix( "inv_", kv );
			}
			num++;
		}
	}
	dict.SetInt( "items", num );

	// pdas viewed
	for ( i = 0; i < 4; i++ ) {
		dict.SetInt( va("pdasViewed_%i", i), pdasViewed[i] );
	}

	dict.SetInt( "selPDA", selPDA );
	dict.SetInt( "selVideo", selVideo );
	dict.SetInt( "selEmail", selEMail );
	dict.SetInt( "selAudio", selAudio );
	dict.SetInt( "pdaOpened", pdaOpened );
	dict.SetInt( "turkeyScore", turkeyScore );

	// pdas
	for ( i = 0; i < pdas.Num(); i++ ) {
		sprintf( key, "pda_%i", i );
		dict.Set( key, pdas[ i ] );
	}
	dict.SetInt( "pdas", pdas.Num() );

	// video cds
	for ( i = 0; i < videos.Num(); i++ ) {
		sprintf( key, "video_%i", i );
		dict.Set( key, videos[ i ].c_str() );
	}
	dict.SetInt( "videos", videos.Num() );

	// emails
	for ( i = 0; i < emails.Num(); i++ ) {
		sprintf( key, "email_%i", i );
		dict.Set( key, emails[ i ].c_str() );
	}
	dict.SetInt( "emails", emails.Num() );

	// weapons
	dict.SetInt( "weapon_bits", weapons );

	dict.SetInt( "levelTriggers", levelTriggers.Num() );
	for ( i = 0; i < levelTriggers.Num(); i++ ) {
		sprintf( key, "levelTrigger_Level_%i", i );
		dict.Set( key, levelTriggers[i].levelName );
		sprintf( key, "levelTrigger_Trigger_%i", i );
		dict.Set( key, levelTriggers[i].triggerName );
	}

	//ivan start
	// save slots assignments
	for( i = 0; i < NUM_SLOTS; i++ ) {
		sprintf( key, "weapInSlot_%i", i );
		dict.SetInt( key, weaponSlot[ i ] );
	}
	dict.SetBool("giveInitialWeapons", false );
	//ivan end
}

/*
==============
idInventory::RestoreInventory
==============
*/
void idInventory::RestoreInventory( idPlayer *owner, const idDict &dict ) {
	int			i;
	int			num;
	idDict		*item;
	idStr		key;
	idStr		itemname;
	const idKeyValue *kv;
	const char	*name;

	Clear();

	// health/armor
	maxHealth		= dict.GetInt( "maxhealth", "100" );
	armor			= dict.GetInt( "armor", "50" );
	maxarmor		= dict.GetInt( "maxarmor", "100" );
	deplete_armor	= dict.GetInt( "deplete_armor", "0" );
	deplete_rate	= dict.GetFloat( "deplete_rate", "2.0" );
	deplete_ammount	= dict.GetInt( "deplete_ammount", "1" );

	// the clip and powerups aren't restored

	/*
	//ivan start - moved here from below so ammo are reset correctly
	// weapons are stored as a number for persistant data, but as strings in the entityDef
	weapons	= dict.GetInt( "weapon_bits", "0" );

#ifdef ID_DEMO_BUILD
	Give( owner, dict, "weapon", dict.GetString( "weapon" ), NULL, false );
#else
	if( dict.GetBool("giveInitialWeapons", "1" ) ){ //ivan
		if ( g_skill.GetInteger() >= 3 ) {
			Give( owner, dict, "weapon", dict.GetString( "weapon_nightmare" ), NULL, false );
		} else {
			Give( owner, dict, "weapon", dict.GetString( "weapon" ), NULL, false );
		}
	}
#endif
	//ivan end
*/

	// ammo
	for( i = 0; i < AMMO_NUMTYPES; i++ ) {
		name = idWeapon::GetAmmoNameForNum( ( ammo_t )i );
		if ( name ) {
			ammo[ i ] = dict.GetInt( name );
		}
	}
	//Restore the clip data //un noted change from original sdk
	for( i = 0; i < MAX_WEAPONS; i++ ) {//new
		clip[i] = dict.GetInt(va("clip%i", i), "-1");
	}
	// items
	num = dict.GetInt( "items" );
	items.SetNum( num );
	for( i = 0; i < num; i++ ) {
		item = new idDict();
		items[ i ] = item;
		sprintf( itemname, "item_%i ", i );
		kv = dict.MatchPrefix( itemname );
		while( kv ) {
			key = kv->GetKey();
			key.Strip( itemname );
			item->Set( key, kv->GetValue() );
			kv = dict.MatchPrefix( itemname, kv );
		}
	}

	// pdas viewed
	for ( i = 0; i < 4; i++ ) {
		pdasViewed[i] = dict.GetInt(va("pdasViewed_%i", i));
	}

	selPDA = dict.GetInt( "selPDA" );
	selEMail = dict.GetInt( "selEmail" );
	selVideo = dict.GetInt( "selVideo" );
	selAudio = dict.GetInt( "selAudio" );
	pdaOpened = dict.GetBool( "pdaOpened" );
	turkeyScore = dict.GetBool( "turkeyScore" );

	// pdas
	num = dict.GetInt( "pdas" );
	pdas.SetNum( num );
	for ( i = 0; i < num; i++ ) {
		sprintf( itemname, "pda_%i", i );
		pdas[i] = dict.GetString( itemname, "default" );
	}

	// videos
	num = dict.GetInt( "videos" );
	videos.SetNum( num );
	for ( i = 0; i < num; i++ ) {
		sprintf( itemname, "video_%i", i );
		videos[i] = dict.GetString( itemname, "default" );
	}

	// emails
	num = dict.GetInt( "emails" );
	emails.SetNum( num );
	for ( i = 0; i < num; i++ ) {
		sprintf( itemname, "email_%i", i );
		emails[i] = dict.GetString( itemname, "default" );
	}

	// weapons are stored as a number for persistant data, but as strings in the entityDef
	weapons	= dict.GetInt( "weapon_bits", "0" );

	if( dict.GetBool("giveInitialWeapons", "1" ) ){ //ivan
		if ( g_skill.GetInteger() >= 3 ) {
			Give( owner, dict, "weapon", dict.GetString( "weapon_nightmare" ), NULL, false );
		} else {
			Give( owner, dict, "weapon", dict.GetString( "weapon" ), NULL, false );
		}
	} //ivan

	num = dict.GetInt( "levelTriggers" );
	for ( i = 0; i < num; i++ ) {
		sprintf( itemname, "levelTrigger_Level_%i", i );
		idLevelTriggerInfo lti;
		lti.levelName = dict.GetString( itemname );
		sprintf( itemname, "levelTrigger_Trigger_%i", i );
		lti.triggerName = dict.GetString( itemname );
		levelTriggers.Append( lti );
	}

	//ivan start
	// restore slots assignments
	for( i = 0; i < NUM_SLOTS; i++ ) {
		sprintf( key, "weapInSlot_%i", i );
		weaponSlot[i] = dict.GetInt( key, "-1" ); //default empty
	}
	//ivan end

}

/*
==============
idInventory::Save
==============
*/
void idInventory::Save( idSaveGame *savefile ) const {
	int i;

	savefile->WriteInt( maxHealth );
	savefile->WriteInt( weapons );
	savefile->WriteInt( powerups );
	savefile->WriteInt( armor );
	savefile->WriteInt( maxarmor );
	savefile->WriteInt( ammoPredictTime );
	savefile->WriteInt( deplete_armor );
	savefile->WriteFloat( deplete_rate );
	savefile->WriteInt( deplete_ammount );
	savefile->WriteInt( nextArmorDepleteTime );

	for( i = 0; i < AMMO_NUMTYPES; i++ ) {
		savefile->WriteInt( ammo[ i ] );
	}
	for( i = 0; i < MAX_WEAPONS; i++ ) {
		savefile->WriteInt( clip[ i ] );
	}
	for( i = 0; i < MAX_POWERUPS; i++ ) {
		savefile->WriteInt( powerupEndTime[ i ] );
	}

	savefile->WriteInt( items.Num() );
	for( i = 0; i < items.Num(); i++ ) {
		savefile->WriteDict( items[ i ] );
	}

	savefile->WriteInt( pdasViewed[0] );
	savefile->WriteInt( pdasViewed[1] );
	savefile->WriteInt( pdasViewed[2] );
	savefile->WriteInt( pdasViewed[3] );

	savefile->WriteInt( selPDA );
	savefile->WriteInt( selVideo );
	savefile->WriteInt( selEMail );
	savefile->WriteInt( selAudio );
	savefile->WriteBool( pdaOpened );
	savefile->WriteBool( turkeyScore );

	savefile->WriteInt( pdas.Num() );
	for( i = 0; i < pdas.Num(); i++ ) {
		savefile->WriteString( pdas[ i ] );
	}

	savefile->WriteInt( pdaSecurity.Num() );
	for( i=0; i < pdaSecurity.Num(); i++ ) {
		savefile->WriteString( pdaSecurity[ i ] );
	}

	savefile->WriteInt( videos.Num() );
	for( i = 0; i < videos.Num(); i++ ) {
		savefile->WriteString( videos[ i ] );
	}

	savefile->WriteInt( emails.Num() );
	for ( i = 0; i < emails.Num(); i++ ) {
		savefile->WriteString( emails[ i ] );
	}

	//ivan start
	for( i = 0; i < NUM_SLOTS; i++ ) {
		savefile->WriteInt( weaponSlot[ i ] );
	}
	
	savefile->WriteInt( nextWeaponPickup ); 
	//ivan end

	savefile->WriteInt( nextItemPickup );
	savefile->WriteInt( nextItemNum );
	savefile->WriteInt( onePickupTime );

	savefile->WriteInt( pickupItemNames.Num() );
	for( i = 0; i < pickupItemNames.Num(); i++ ) {
		savefile->WriteString( pickupItemNames[i].icon );
		savefile->WriteString( pickupItemNames[i].name );
	}

	savefile->WriteInt( objectiveNames.Num() );
	for( i = 0; i < objectiveNames.Num(); i++ ) {
		savefile->WriteString( objectiveNames[i].screenshot );
		savefile->WriteString( objectiveNames[i].text );
		savefile->WriteString( objectiveNames[i].title );
	}

	savefile->WriteInt( levelTriggers.Num() );
	for ( i = 0; i < levelTriggers.Num(); i++ ) {
		savefile->WriteString( levelTriggers[i].levelName );
		savefile->WriteString( levelTriggers[i].triggerName );
	}

	savefile->WriteBool( ammoPulse );
	savefile->WriteBool( weaponPulse );
	savefile->WriteBool( armorPulse );

	savefile->WriteInt( lastGiveTime );
}

/*
==============
idInventory::Restore
==============
*/
void idInventory::Restore( idRestoreGame *savefile ) {
	int i, num;

	savefile->ReadInt( maxHealth );
	savefile->ReadInt( weapons );
	savefile->ReadInt( powerups );
	savefile->ReadInt( armor );
	savefile->ReadInt( maxarmor );
	savefile->ReadInt( ammoPredictTime );
	savefile->ReadInt( deplete_armor );
	savefile->ReadFloat( deplete_rate );
	savefile->ReadInt( deplete_ammount );
	savefile->ReadInt( nextArmorDepleteTime );

	for( i = 0; i < AMMO_NUMTYPES; i++ ) {
		savefile->ReadInt( ammo[ i ] );
	}
	for( i = 0; i < MAX_WEAPONS; i++ ) {
		savefile->ReadInt( clip[ i ] );
	}
	for( i = 0; i < MAX_POWERUPS; i++ ) {
		savefile->ReadInt( powerupEndTime[ i ] );
	}

	savefile->ReadInt( num );
	for( i = 0; i < num; i++ ) {
		idDict *itemdict = new idDict;

		savefile->ReadDict( itemdict );
		items.Append( itemdict );
	}

	// pdas
	savefile->ReadInt( pdasViewed[0] );
	savefile->ReadInt( pdasViewed[1] );
	savefile->ReadInt( pdasViewed[2] );
	savefile->ReadInt( pdasViewed[3] );

	savefile->ReadInt( selPDA );
	savefile->ReadInt( selVideo );
	savefile->ReadInt( selEMail );
	savefile->ReadInt( selAudio );
	savefile->ReadBool( pdaOpened );
	savefile->ReadBool( turkeyScore );

	savefile->ReadInt( num );
	for( i = 0; i < num; i++ ) {
		idStr strPda;
		savefile->ReadString( strPda );
		pdas.Append( strPda );
	}

	// pda security clearances
	savefile->ReadInt( num );
	for ( i = 0; i < num; i++ ) {
		idStr invName;
		savefile->ReadString( invName );
		pdaSecurity.Append( invName );
	}

	// videos
	savefile->ReadInt( num );
	for( i = 0; i < num; i++ ) {
		idStr strVideo;
		savefile->ReadString( strVideo );
		videos.Append( strVideo );
	}

	// email
	savefile->ReadInt( num );
	for( i = 0; i < num; i++ ) {
		idStr strEmail;
		savefile->ReadString( strEmail );
		emails.Append( strEmail );
	}

	//ivan start
	for( i = 0; i < NUM_SLOTS; i++ ) {
		savefile->ReadInt( weaponSlot[ i ] );
	}
	
	savefile->ReadInt( nextWeaponPickup ); 
	//ivan end

	savefile->ReadInt( nextItemPickup );
	savefile->ReadInt( nextItemNum );
	savefile->ReadInt( onePickupTime );
	savefile->ReadInt( num );
	for( i = 0; i < num; i++ ) {
		idItemInfo info;

		savefile->ReadString( info.icon );
		savefile->ReadString( info.name );

		pickupItemNames.Append( info );
	}

	savefile->ReadInt( num );
	for( i = 0; i < num; i++ ) {
		idObjectiveInfo obj;

		savefile->ReadString( obj.screenshot );
		savefile->ReadString( obj.text );
		savefile->ReadString( obj.title );

		objectiveNames.Append( obj );
	}

	savefile->ReadInt( num );
	for ( i = 0; i < num; i++ ) {
		idLevelTriggerInfo lti;
		savefile->ReadString( lti.levelName );
		savefile->ReadString( lti.triggerName );
		levelTriggers.Append( lti );
	}

	savefile->ReadBool( ammoPulse );
	savefile->ReadBool( weaponPulse );
	savefile->ReadBool( armorPulse );

	savefile->ReadInt( lastGiveTime );
}

/*
==============
idInventory::AmmoIndexForAmmoClass
==============
*/
ammo_t idInventory::AmmoIndexForAmmoClass( const char *ammo_classname ) const {
	return idWeapon::GetAmmoNumForName( ammo_classname );
}

/*
==============
idInventory::AmmoIndexForAmmoClass
==============
*/
int idInventory::MaxAmmoForAmmoClass( idPlayer *owner, const char *ammo_classname ) const {
	return owner->spawnArgs.GetInt( va( "max_%s", ammo_classname ), "0" );
}

/*
==============
idInventory::AmmoPickupNameForIndex
==============
*/
const char *idInventory::AmmoPickupNameForIndex( ammo_t ammonum ) const {
	return idWeapon::GetAmmoPickupNameForNum( ammonum );
}

/*
==============
idInventory::WeaponIndexForAmmoClass
mapping could be prepared in the constructor
==============
*/
int idInventory::WeaponIndexForAmmoClass( const idDict & spawnArgs, const char *ammo_classname ) const {
	int i;
	const char *weapon_classname;
	for( i = 0; i < MAX_WEAPONS; i++ ) {
		weapon_classname = spawnArgs.GetString( va( "def_weapon%d", i ) );
		if ( !weapon_classname ) {
			continue;
		}
		const idDeclEntityDef *decl = gameLocal.FindEntityDef( weapon_classname, false );
		if ( !decl ) {
			continue;
		}
		if ( !idStr::Icmp( ammo_classname, decl->dict.GetString( "ammoType" ) ) ) {
			return i;
		}
	}
	return -1;
}

/*
==============
idInventory::AmmoIndexForWeaponClass
==============
*/
ammo_t idInventory::AmmoIndexForWeaponClass( const char *weapon_classname, int *ammoRequired ) {
	const idDeclEntityDef *decl = gameLocal.FindEntityDef( weapon_classname, false );
	if ( !decl ) {
		gameLocal.Error( "Unknown weapon in decl '%s'", weapon_classname );
	}
	if ( ammoRequired ) {
		*ammoRequired = decl->dict.GetInt( "ammoRequired" );
	}
	ammo_t ammo_i = AmmoIndexForAmmoClass( decl->dict.GetString( "ammoType" ) );
	return ammo_i;
}

/*
==============
idInventory::AddPickupName
==============
*/
//void idInventory::AddPickupName( const char *name, const char *icon ) { //un noted change from original sdk
void idInventory::AddPickupName( const char *name, const char *icon, idPlayer* owner ) { //New, Dont know what it does
	int num;

	num = pickupItemNames.Num();
	if ( ( num == 0 ) || ( pickupItemNames[ num - 1 ].name.Icmp( name ) != 0 ) ) {
		idItemInfo &info = pickupItemNames.Alloc();

		if ( idStr::Cmpn( name, STRTABLE_ID, STRTABLE_ID_LENGTH ) == 0 ) {
			info.name = common->GetLanguageDict()->GetString( name );
		} else {
			info.name = name;
		}
		info.icon = icon;

		if ( gameLocal.isServer ) { //un noted change from original sdk
			idBitMsg	msg;
			byte		msgBuf[MAX_EVENT_PARAM_SIZE];

			msg.Init( msgBuf, sizeof( msgBuf ) );
			msg.WriteString( name, MAX_EVENT_PARAM_SIZE );
			owner->ServerSendEvent( idPlayer::EVENT_PICKUPNAME, &msg, false, -1 );
		}
	}

} //un noted change from original sdk

/*
==============
idInventory::Give
==============
*/
bool idInventory::Give( idPlayer *owner, const idDict &spawnArgs, const char *statname, const char *value, int *idealWeapon, bool updateHud ) {
	int						i;
	const char				*pos;
	const char				*end;
	int						len;
	idStr					weaponString;
	int						max;
	const idDeclEntityDef	*weaponDecl;
	bool					tookWeapon;
	int						amount;
	idItemInfo				info;
	const char				*name;

	if ( !idStr::Icmpn( statname, "ammo_", 5 ) ) {
		i = AmmoIndexForAmmoClass( statname );
		max = MaxAmmoForAmmoClass( owner, statname );
		if ( ammo[ i ] >= max ) {
			return false;
		}
		amount = atoi( value );
		if ( amount ) {
			ammo[ i ] += amount;
			if ( ( max > 0 ) && ( ammo[ i ] > max ) ) {
				ammo[ i ] = max;
			}
			ammoPulse = true;

			name = AmmoPickupNameForIndex( i );
			if ( idStr::Length( name ) ) {
				AddPickupName( name, "", owner ); //new _D3XP //un noted change from original sdk
			}
		}
	} else if ( !idStr::Icmp( statname, "armor" ) ) {
		if ( armor >= maxarmor ) {
			return false;	// can't hold any more, so leave the item
		}
		amount = atoi( value );
		if ( amount ) {
			armor += amount;
			if ( armor > maxarmor ) {
				armor = maxarmor;
			}
			nextArmorDepleteTime = 0;
			armorPulse = true;
		}
	} else if ( idStr::FindText( statname, "inclip_" ) == 0 ) {
		i = WeaponIndexForAmmoClass( spawnArgs, statname + 7 );
		if ( i != -1 ) {
			// set, don't add. not going over the clip size limit. 
      //un noted change from original sdk
		}

	} else if ( !idStr::Icmp( statname, "berserk" ) ) {
		GivePowerUp( owner, BERSERK, SEC2MS( atof( value ) ) );
	} else if ( !idStr::Icmp( statname, "mega" ) ) {
		GivePowerUp( owner, MEGAHEALTH, SEC2MS( atof( value ) ) );
	} else if ( !idStr::Icmp( statname, "weapon" ) ) {
		tookWeapon = false;
		for( pos = value; pos != NULL; pos = end ) {
			end = strchr( pos, ',' );
			if ( end ) {
				len = end - pos;
				end++;
			} else {
				len = strlen( pos );
			}

			idStr weaponName( pos, 0, len );

			// find the number of the matching weapon name
			for( i = 0; i < MAX_WEAPONS; i++ ) {
				if ( weaponName == spawnArgs.GetString( va( "def_weapon%d", i ) ) ) {
					break;
				}
			}

			if ( i >= MAX_WEAPONS ) {
				gameLocal.Warning( "Unknown weapon '%s'", weaponName.c_str() );
				continue; //un noted change from original sdk
			}

			// cache the media for this weapon
			weaponDecl = gameLocal.FindEntityDef( weaponName, false );

			// don't pickup "no ammo" weapon types twice
			// not for D3 SP .. there is only one case in the game where you can get a no ammo
			// weapon when you might already have it, in that case it is more conistent to pick it up
			if ( gameLocal.isMultiplayer && weaponDecl && ( weapons & ( 1 << i ) ) && !weaponDecl->dict.GetInt( "ammoRequired" ) ) {
				continue;
			}

			if ( !gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) || ( weaponName == "weapon_fists" ) || ( weaponName == "weapon_soulcube" ) ) {
				if ( ( weapons & ( 1 << i ) ) == 0 || gameLocal.isMultiplayer ) {
					/*
					//commented out by ivan: picked up weapon is now selected in idPlayer::AddWeaponToSlots
					if ( owner->GetUserInfo()->GetBool( "ui_autoSwitch" ) && idealWeapon ) {
						assert( !gameLocal.isClient );
						*idealWeapon = i;
					}
					*/
					if ( owner->hud && updateHud && lastGiveTime + 1000 < gameLocal.time ) {
						owner->hud->SetStateInt( "newWeapon", i );
						owner->hud->HandleNamedEvent( "newWeapon" );
						lastGiveTime = gameLocal.time;
					}
					weaponPulse = true;
					weapons |= ( 1 << i );
					tookWeapon = true;
				}
			}
		}
		return tookWeapon;
	} else if ( !idStr::Icmp( statname, "item" ) || !idStr::Icmp( statname, "icon" ) || !idStr::Icmp( statname, "name" ) ) {
		// ignore these as they're handled elsewhere
		return false;
	} else {
		// unknown item
		gameLocal.Warning( "Unknown stat '%s' added to player's inventory", statname );
		return false;
	}

	return true;
}

//ivan start
/*
===============
idInventory::GetSlotByWeap
===============
*/
int idInventory::GetSlotByWeap( int i ){
	int	w;
	for( w = 0; w < NUM_SLOTS; w++ ) { //if already in
		if ( i == weaponSlot[w] ) {
			return w;
		}
	}
	return -1;
}

/*
===============
idInventory::FindFreeSlot
===============
*/
int idInventory::FindFreeSlot( void ){
	int	w;
	for( w = 0; w < NUM_SLOTS; w++ ) { //if a free slot
		if ( -1 == weaponSlot[w] ) {
			return w;
		}
	}
	return -1;
}

/*
===============
idInventory::AssignWeapToSlot
===============
*/
void idInventory::AssignWeapToSlot( int w, int slot ){
	weaponSlot[slot] = w;
	weaponPulse = false; //will trigger UpdateHudWeapon();
}



/*
===============
idInventory::NumWeapForAmmoType
===============
*/
int idInventory::NumWeapForAmmoType( const idDict &spawnArgs, ammo_t ammoType ) {
	ammo_t tempAmmoType;

	idStr	weap;
	int		w;
	int numfound = 0;

	// check if we have any weapons
	if ( !weapons ) { //should never happen...
		return 0;
	}

	for( w = 0; w < MAX_WEAPONS; w++ ) {
		if ( weapons & ( 1 << w ) ) {
			weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
			if ( weap != "" ) {
				tempAmmoType = AmmoIndexForWeaponClass( weap.c_str() , NULL );
				if( tempAmmoType == ammoType ) numfound++;
			} 
		}
	}


	/*
	const char *weap;
	int w; 
	int s;
	int numfound = 0;

	s = NUM_SLOTS;
	while( s > 0 ) {
		s--; 

		w = weaponSlot[s];
		if( w == -1 ){ //slot is empty
			continue;
		}

		weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
		tempAmmoType = AmmoIndexForWeaponClass( weap, NULL );

		if( tempAmmoType == ammoType ) numfound++;
	}
	*/
	
	//gameLocal.Printf("number of weapons with same ammo type: %d\n", numfound );
	return numfound;
}

//ivan end


/*
===============
idInventoy::Drop
===============
*/
void idInventory::Drop( const idDict &spawnArgs, const char *weapon_classname, int weapon_index, bool* ammoRemoved ) { //ivan - ammoRemoved added
	// remove the weapon bit
	// also remove the ammo associated with the weapon as we pushed it in the item
	assert( weapon_index != -1 || weapon_classname );
	if ( weapon_index == -1 ) {
		for( weapon_index = 0; weapon_index < MAX_WEAPONS; weapon_index++ ) {
			if ( !idStr::Icmp( weapon_classname, spawnArgs.GetString( va( "def_weapon%d", weapon_index ) ) ) ) {
				break;
			}
		}
		if ( weapon_index >= MAX_WEAPONS ) {
			gameLocal.Error( "Unknown weapon '%s'", weapon_classname );
		}
	} else if ( !weapon_classname ) {
		weapon_classname = spawnArgs.GetString( va( "def_weapon%d", weapon_index ) );
	}

	//ivan - was: weapons &= ( 0xffffffff ^ ( 1 << weapon_index ) ); //remove weapon

	//remove ammo?
	ammo_t ammo_i = AmmoIndexForWeaponClass( weapon_classname, NULL );
	if ( ammo_i ) {
		//ivan start
		
		/*
		//was: 
		clip[ weapon_index ] = -1;
		ammo[ ammo_i ] = 0;
		*/

		if(1){ //fix: never remove ammo //was: if( NumWeapForAmmoType( spawnArgs, ammo_i ) > 1 ){
			clip[ weapon_index ] = -1;
			//ammo[ ammo_i ] = 0; //don't remove ammo!
			if( ammoRemoved ) *ammoRemoved = false;
		}else{
			clip[ weapon_index ] = -1;
			ammo[ ammo_i ] = 0;
			if( ammoRemoved ) *ammoRemoved = true;
		}
		//ivan end
	}

	//ivan start - remove weapons
	weapons &= ( 0xffffffff ^ ( 1 << weapon_index ) ); //remove weapon


	//ivan start - clear the slot
	int slot = GetSlotByWeap( weapon_index );
	if( slot >= 0 ){ //this weapon was is a slot
		weaponSlot[slot] = -1; //empty slot
	}
	//ivan end
}

/*
===============
idInventory::HasAmmo
===============
*/
int idInventory::HasAmmo( ammo_t type, int amount ) {
	if ( ( type == 0 ) || !amount ) {
		// always allow weapons that don't use ammo to fire
		return -1;
	}

	// check if we have infinite ammo
	if ( ammo[ type ] < 0 ) {
		return -1;
	}

	// return how many shots we can fire
	return ammo[ type ] / amount;
}

/*
===============
idInventory::HasAmmo
===============
*/
int idInventory::HasAmmo( const char *weapon_classname, bool includeClip, idPlayer* owner ) {		//_D3XP //un noted change from original sdk
	int ammoRequired;
	ammo_t ammo_i = AmmoIndexForWeaponClass( weapon_classname, &ammoRequired );

	int ammoCount = HasAmmo( ammo_i, ammoRequired );
	if(includeClip && owner) {
		ammoCount += clip[owner->SlotForWeapon(weapon_classname)];
	}
	return ammoCount; //un noted change from original sdk
}

/*
===============
idInventory::UseAmmo
===============
*/
bool idInventory::UseAmmo( ammo_t type, int amount ) {
	if ( !HasAmmo( type, amount ) ) {
		return false;
	}

	// take an ammo away if not infinite
	if ( ammo[ type ] >= 0 ) {
		ammo[ type ] -= amount;
		ammoPredictTime = gameLocal.time; // mp client: we predict this. mark time so we're not confused by snapshots
	}

	return true;
}

/*
===============
idInventory::UpdateArmor
===============
*/
void idInventory::UpdateArmor( void ) {
	if ( deplete_armor != 0.0f && deplete_armor < armor ) {
		if ( !nextArmorDepleteTime ) {
			nextArmorDepleteTime = gameLocal.time + deplete_rate * 1000;
		} else if ( gameLocal.time > nextArmorDepleteTime ) {
			armor -= deplete_ammount;
			if ( armor < deplete_armor ) {
				armor = deplete_armor;
			}
			nextArmorDepleteTime = gameLocal.time + deplete_rate * 1000;
		}
	}
}

/*
==============
idPlayer::idPlayer
==============
*/
idPlayer::idPlayer() {
	memset( &usercmd, 0, sizeof( usercmd ) );

	noclip					= false;
	godmode					= false;
	
	waitForDamage			= 1;	//rev 2018
	noDamage				= 0;	//rev 2020
	//ivan start
	animMoveNoGravity		= false; 
	animMoveType			= ANIMMOVE_NONE;
	comboOn					= false;
	allowTurn				= true;
	blendModelYaw			= false; 
	//ivan end

//rev 2018 start
	touchofdeathx			= 1;
	touchofdeathy			= 1;
//rev 2018 end
	
	spawnAnglesSet			= false;
	spawnAngles				= ang_zero;
	viewAngles				= ang_zero;
	cmdAngles				= ang_zero;

	oldButtons				= 0;
	buttonMask				= 0;
	oldFlags				= 0;

	lastHitTime				= 0;
	lastSndHitTime			= 0;
	lastSavingThrowTime		= 0;

	weapon					= NULL;

	hud						= NULL;
	objectiveSystem			= NULL;
	objectiveSystemOpen		= false;

	heartRate				= BASE_HEARTRATE;
	heartInfo.Init( 0, 0, 0, 0 );
	lastHeartAdjust			= 0;
	lastHeartBeat			= 0;
	lastDmgTime				= 0;
	lastChargeTime			= 0;	//rev 2020 charge checks the time difference since we last charge attacked or added to the charge attack gauge
	chargeAmount			= 0;	//rev 2020 charge maximum amount of times we can charge
	chargeDir				= 0;	//rev 2020 direction to charge, 1 up, 2 upward, 3 forward, 4 downward, 5 down
	deathClearContentsTime	= 0;
	lastArmorPulse			= -10000;
	stamina					= 0.0f;
	healthPool				= 0.0f;
	nextHealthPulse			= 0;
	healthPulse				= false;
	nextHealthTake			= 0;
	healthTake				= false;

	scoreBoardOpen			= false;
	forceScoreBoard			= false;
	forceRespawn			= false;
	spectating				= false;
	spectator				= 0;
	colorBar				= vec3_zero;
	colorBarIndex			= 0;
	forcedReady				= false;
	wantSpectate			= false;

	lastHitToggle			= false;

	minRespawnTime			= 0;
	maxRespawnTime			= 0;

	firstPersonViewOrigin	= vec3_zero;
	firstPersonViewAxis		= mat3_identity;

	hipJoint				= INVALID_JOINT;
	chestJoint				= INVALID_JOINT;
	headJoint				= INVALID_JOINT;

	
	/*
	bobFoot					= 0;
	bobFrac					= 0.0f;
	bobfracsin				= 0.0f;
	bobCycle				= 0; //un noted change from original sdk
	*/
	xyspeed					= 0.0f;
	/*
	stepUpTime				= 0;
	stepUpDelta				= 0.0f; //un noted change from original sdk
	*/
	idealLegsYaw			= 0.0f;
	legsYaw					= 0.0f;
	legsForward				= true;
	oldViewYaw				= 0.0f;
	//viewBobAngles			= ang_zero; //un noted change from original sdk
	//viewBob					= vec3_zero;
	landChange				= 0;
	landTime				= 0;

	currentWeapon			= -1;
	currentSlot				= -1; //ivan - default is none
	idealWeapon				= -1;
	previousWeapon			= -1;
	quickWeapon				= -1; //new //un noted change from original sdk
	weaponSwitchTime		=  0;
	weaponEnabled			= true;
	weapon_soulcube			= -1;
	weapon_pda				= -1;
	weapon_fists			= -1;
	showWeaponViewModel		= true;

	skin					= NULL;
	powerUpSkin				= NULL;
	baseSkinName			= "";

	numProjectilesFired		= 0;
	numProjectileHits		= 0;

	airless					= false;
	airTics					= 0;
	lastAirDamage			= 0;

	gibDeath				= false;
	gibsLaunched			= false;
	gibsDir					= vec3_zero;

	zoomFov.Init( 0, 0, 0, 0 );
	centerView.Init( 0, 0, 0, 0 );

	fxFov					= false;

	influenceFov			= 0;
	influenceActive			= 0;
	influenceRadius			= 0.0f;
	influenceEntity			= NULL;
	influenceMaterial		= NULL;
	influenceSkin			= NULL;

	privateCameraView		= NULL;

	memset( loggedViewAngles, 0, sizeof( loggedViewAngles ) );
	memset( loggedAccel, 0, sizeof( loggedAccel ) );
	currentLoggedAccel	= 0;

	focusTime				= 0;
	focusGUIent				= NULL;
	focusUI					= NULL;
	focusCharacter			= NULL;
	talkCursor				= 0;
	focusVehicle			= NULL;
	cursor					= NULL;

	oldMouseX				= 0;
	oldMouseY				= 0;

	pdaAudio				= "";
	pdaVideo				= "";
	pdaVideoWave			= "";

	lastDamageDef			= 0;
	lastDamageDir			= vec3_zero;
	lastDamageLocation		= 0;
	smoothedFrame			= 0;
	smoothedOriginUpdated	= false;
	smoothedOrigin			= vec3_zero;
	smoothedAngles			= ang_zero;

	fl.networkSync			= true;

	latchedTeam				= -1;
	doingDeathSkin			= false;
	weaponGone				= false;
	useInitialSpawns		= false;
	tourneyRank				= 0;
	lastSpectateTeleport	= 0;
	tourneyLine				= 0;
	hiddenWeapon			= false;
	tipUp					= false;
	objectiveUp				= false;
	teleportEntity			= NULL;
	teleportKiller			= -1;
	respawning				= false;
	ready					= false;
	leader					= false;
	lastSpectateChange		= 0;
	lastTeleFX				= -9999;
	weaponCatchup			= false;
	lastSnapshotSequence	= 0;

	MPAim					= -1;
	lastMPAim				= -1;
	lastMPAimTime			= 0;
	MPAimFadeTime			= 0;
	MPAimHighlight			= false;

	spawnedTime				= 0;
	lastManOver				= false;
	lastManPlayAgain		= false;
	lastManPresent			= false;

	isTelefragged			= false;

	isLagged				= false;
	isChatting				= false;

	selfSmooth				= false;

	//ivan start - kick vars
	kickDefName				= "";
	kickDef					= NULL;
	lastKickedEnt			= NULL;
	nextKickFx				= 0;
	nextKickSnd				= 0;
	kickEnabled				= false;
	kickDmgMultiplier		= 0.0f;
	kickDistance			= 0.0f;
	fromJointKick			= INVALID_JOINT;
	toJointKick				= INVALID_JOINT;
	kickBox.Zero();
	//ivan end

#ifdef _DENTONMOD_PLAYER_CPP
	memset( &weaponZoom, 0, sizeof( weaponZoom ) ); // New
	memset(	projectileType, 0, sizeof(projectileType) );
#endif //_DENTONMOD_PLAYER_CPP

	//ivan start
	health_lost				= 0;
	oldCameraPos			= vec3_zero;
	fastXpos				= 0.0f;
	isXlocked				= true;
	blendModelYaw			= false;
	forcedMovWasLocked		= false;
	forcedMovIncreasingX	= false;
	forcedMovState			= FORCEDMOVE_STATE_DISABLED;
	forcedMovCanBeAborted	= false;
	forcedMovTotalForce		= false;
	forcedMovDelta			= vec3_zero;
	forcedMovOldOrg			= vec3_zero;
	forcedMovTarget			= NULL;
	skipCameraZblend		= true; //force instant update first frame
	enableCameraYblend		= false;
	enableCameraXblend		= false;
	cameraSettings.lockYaxis	= false;
	cameraSettings.lockZaxis	= false;
	cameraSettings.lockedYpos	= 0.0f;
	cameraSettings.lockedZpos	= 0.0f;
	cameraSettings.distance		= CAMERA_DEFAULT_DISTANCE;
	cameraSettings.height		= CAMERA_DEFAULT_HEIGHT;
	lastCheckPoint.entityNumber		= -1; //an non existing entityId
	lastCheckPoint.spawnPos			= vec3_zero;
	lastCheckPoint.cameraSettings	= cameraSettings;
	inhibitInputTime		= 0;
	inhibitAimCrouchTime	= 0;
	updXlock				= true;
	animBasedMovement		= false;
#ifdef SHOW_MOVING_CROSSHAIR
	reqDefaultCrossPos		= true;
	cposx					= 0;
	cposy					= 0;
	coffx					= 0.0f;
#endif
	numLives				= 0; //will be initialized later
	score					= 0;
#ifdef AUTOUPD_RESPAWN_POS
	tempRespawnPos			= vec3_zero;
	nextRespPosTime			= 0;
#endif
	skipMouseUpd			= false;
	interactFlag			= 0;
	interactShownWeaponNum	= 0;
	//interactShownWeaponName	= "";
	//ivan  end
}

/*
==============
idPlayer::LinkScriptVariables

set up conditions for animation
==============
*/
void idPlayer::LinkScriptVariables( void ) {
	AI_FORWARD.LinkTo(			scriptObject, "AI_FORWARD" );
	AI_BACKWARD.LinkTo(			scriptObject, "AI_BACKWARD" );
	AI_STRAFE_LEFT.LinkTo(		scriptObject, "AI_STRAFE_LEFT" );
	AI_STRAFE_RIGHT.LinkTo(		scriptObject, "AI_STRAFE_RIGHT" );
	AI_ATTACK_HELD.LinkTo(		scriptObject, "AI_ATTACK_HELD" );
	AI_WEAPON_FIRED.LinkTo(		scriptObject, "AI_WEAPON_FIRED" );
	AI_JUMP.LinkTo(				scriptObject, "AI_JUMP" );
	AI_DEAD.LinkTo(				scriptObject, "AI_DEAD" );
	AI_CROUCH.LinkTo(			scriptObject, "AI_CROUCH" );
	AI_ONGROUND.LinkTo(			scriptObject, "AI_ONGROUND" );
	AI_ONLADDER.LinkTo(			scriptObject, "AI_ONLADDER" );
	AI_HARDLANDING.LinkTo(		scriptObject, "AI_HARDLANDING" );
	AI_SOFTLANDING.LinkTo(		scriptObject, "AI_SOFTLANDING" );
	AI_RUN.LinkTo(				scriptObject, "AI_RUN" );
	AI_PAIN.LinkTo(				scriptObject, "AI_PAIN" );
	AI_RELOAD.LinkTo(			scriptObject, "AI_RELOAD" );
	AI_TELEPORT.LinkTo(			scriptObject, "AI_TELEPORT" );
	AI_TURN_LEFT.LinkTo(		scriptObject, "AI_TURN_LEFT" );
	AI_TURN_RIGHT.LinkTo(		scriptObject, "AI_TURN_RIGHT" );
}

/*
==============
idPlayer::SetupWeaponEntity
==============
*/
void idPlayer::SetupWeaponEntity( void ) {
	int w;
	const char *weap;

	if ( weapon.GetEntity() ) {
		// get rid of old weapon
		weapon.GetEntity()->Clear();
		currentWeapon = -1;
	} else if ( !gameLocal.isClient ) {
		weapon = static_cast<idWeapon *>( gameLocal.SpawnEntityType( idWeapon::Type, NULL ) );
		weapon.GetEntity()->SetOwner( this );
		currentWeapon = -1;
	}

	for( w = 0; w < MAX_WEAPONS; w++ ) {
		weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
		if ( weap && *weap ) {
			idWeapon::CacheWeapon( weap );
		}
	}
}

//ivan start
void idPlayer::SetupWeaponSlots( void ){
	int i;
	//add the weapons we already have to slots
	for ( i = 0; i < MAX_WEAPONS; i++ ) {
		if ( inventory.weapons & ( 1 << i ) ) {
			AddWeaponToSlots( i, false );
		}
	}

	currentSlot	= (i > 0) ? 0 : -1;
	
	//fix: if we start with only one weapon in slots hud is not up to date --> force upd here
	if(hud){
		if( currentSlot != -1 ){
			hud->HandleNamedEvent( va( "selectSlot%d", currentSlot ) );
		}else{
			hud->HandleNamedEvent( "selectExtraSlot" );
		}
	}
	//fix end
}
//ivan end

/*
==============
idPlayer::Init
==============
*/
void idPlayer::Init( bool quickRespawn ) { //ivan - quickRespawn added
	const char			*value;
	const idKeyValue	*kv;

	noclip					= false;
	godmode					= false;

	oldButtons				= 0;
	oldFlags				= 0;

#ifdef _DENTONMOD_PLAYER_CPP
	memset( &weaponZoom, 0, sizeof( weaponZoom ) ); // New
#endif //_DENTONMOD_PLAYER_CPP

	if( !quickRespawn ){ //ivan - don't reset this if it's a quick respawn: it's the valid weapon to select!
		idealWeapon			= -1; 
	}
	currentWeapon			= -1;
	previousWeapon			= -1;
	quickWeapon				= -1; //new
	weaponSwitchTime		= 0;
	weaponEnabled			= true;
	weapon_soulcube			= SlotForWeapon( "weapon_soulcube" );
	weapon_pda				= SlotForWeapon( "weapon_pda" );
	weapon_fists			= SlotForWeapon( "weapon_fists" );
	showWeaponViewModel		= GetUserInfo()->GetBool( "ui_showGun" );


	lastDmgTime				= 0;
	lastChargeTime			= 0;	//rev 2020 charge
	lastChargeTime			= 0;	//rev 2020 charge
	lastArmorPulse			= -10000;
	lastHeartAdjust			= 0;
	lastHeartBeat			= 0;
	heartInfo.Init( 0, 0, 0, 0 );

	chargeAmount			= spawnArgs.GetInt( "charge_amount", "2" );	//rev 2020 charge
	chargeDir				= 0;	//rev 2020 charge
	/* //un noted change from original sdk
	bobCycle				= 0;
	bobFrac					= 0.0f;
	*/
	landChange				= 0;
	landTime				= 0;
	zoomFov.Init( 0, 0, 0, 0 );
	centerView.Init( 0, 0, 0, 0 );

	oldCameraPos			= vec3_zero; //ivan
	
	fxFov					= false;

	influenceFov			= 0;
	influenceActive			= 0;
	influenceRadius			= 0.0f;
	influenceEntity			= NULL;
	influenceMaterial		= NULL;
	influenceSkin			= NULL;

	currentLoggedAccel		= 0;

	focusTime				= 0;
	focusGUIent				= NULL;
	focusUI					= NULL;
	focusCharacter			= NULL;
	talkCursor				= 0;
	focusVehicle			= NULL;

	// remove any damage effects
	playerView.ClearEffects();

	// damage values
	fl.takedamage			= true;
	ClearPain();

	// restore persistent data
	//ivan start
	//was: RestorePersistantInfo();

	if( quickRespawn ){ //don't restore persistant info if it's a quick respawn
		health = spawnArgs.GetInt( "health", "100" );
	}else{
		RestorePersistantInfo();
	}
	//ivan end

	//bobCycle		= 0; //un noted change from original sdk
	stamina			= 0.0f;
	healthPool		= 0.0f;
	nextHealthPulse = 0;
	healthPulse		= false;
	nextHealthTake	= 0;
	healthTake		= false;

	SetupWeaponEntity();

	if( !quickRespawn ){ //ivan - don't setup weapon slots again if it's a quick respawn
		currentWeapon = -1;
		previousWeapon = -1;
		quickWeapon	= -1; //new

		//ivan start - setup slots - this has to be done after RestorePersistantInfo() so that weapons could be already assigned to slots
		SetupWeaponSlots();
		//ivan end
	}

	heartRate = BASE_HEARTRATE;
	AdjustHeartRate( BASE_HEARTRATE, 0.0f, 0.0f, true );

	idealLegsYaw = 0.0f;
	legsYaw = 0.0f;
	legsForward	= true;
	oldViewYaw = 0.0f;

	// set the pm_ cvars
	if ( !gameLocal.isMultiplayer || gameLocal.isServer ) {
		kv = spawnArgs.MatchPrefix( "pm_", NULL );
		while( kv ) {
			cvarSystem->SetCVarString( kv->GetKey(), kv->GetValue() );
			kv = spawnArgs.MatchPrefix( "pm_", kv );
		}
	}

	// disable stamina on hell levels
	if ( gameLocal.world && gameLocal.world->spawnArgs.GetBool( "no_stamina" ) ) {
		pm_stamina.SetFloat( 0.0f );
	}

	// stamina always initialized to maximum
	stamina = pm_stamina.GetFloat();

	// air always initialized to maximum too
	airTics = pm_airTics.GetFloat();
	airless = false;

	gibDeath = false;
	gibsLaunched = false;
	gibsDir.Zero();

	// set the gravity
	physicsObj.SetGravity( gameLocal.GetGravity() );

	// start out standing
	SetEyeHeight( pm_normalviewheight.GetFloat() );

	/* //un noted change from original sdk
	stepUpTime = 0;
	stepUpDelta = 0.0f;
	viewBobAngles.Zero();
	viewBob.Zero();
	*/

	value = spawnArgs.GetString( "model" );
	if ( value && ( *value != 0 ) ) {
		SetModel( value );
	}

	if ( cursor ) {
		cursor->SetStateInt( "talkcursor", 0 );
		cursor->SetStateString( "combatcursor", "1" );
		cursor->SetStateString( "itemcursor", "0" );
		cursor->SetStateString( "guicursor", "0" );
	}

	if ( ( gameLocal.isMultiplayer || g_testDeath.GetBool() ) && skin ) {
		SetSkin( skin );
		renderEntity.shaderParms[6] = 0.0f;
	} else if ( spawnArgs.GetString( "spawn_skin", NULL, &value ) ) {
		skin = declManager->FindSkin( value );
		SetSkin( skin );
		renderEntity.shaderParms[6] = 0.0f;
	}

	value = spawnArgs.GetString( "bone_hips", "" );
	hipJoint = animator.GetJointHandle( value );
	if ( hipJoint == INVALID_JOINT ) {
		gameLocal.Error( "Joint '%s' not found for 'bone_hips' on '%s'", value, name.c_str() );
	}

	value = spawnArgs.GetString( "bone_chest", "" );
	chestJoint = animator.GetJointHandle( value );
	if ( chestJoint == INVALID_JOINT ) {
		gameLocal.Error( "Joint '%s' not found for 'bone_chest' on '%s'", value, name.c_str() );
	}

	value = spawnArgs.GetString( "bone_head", "" );
	headJoint = animator.GetJointHandle( value );
	if ( headJoint == INVALID_JOINT ) {
		gameLocal.Error( "Joint '%s' not found for 'bone_head' on '%s'", value, name.c_str() );
	}

	// initialize the script variables
	AI_FORWARD		= false;
	AI_BACKWARD		= false;
	AI_STRAFE_LEFT	= false;
	AI_STRAFE_RIGHT	= false;
	AI_ATTACK_HELD	= false;
	AI_WEAPON_FIRED	= false;
	AI_JUMP			= false;
	AI_DEAD			= false;
	AI_CROUCH		= false;
	AI_ONGROUND		= true;
	AI_ONLADDER		= false;
	AI_HARDLANDING	= false;
	AI_SOFTLANDING	= false;
	AI_RUN			= false;
	AI_PAIN			= false;
	AI_RELOAD		= false;
	AI_TELEPORT		= false;
	AI_TURN_LEFT	= false;
	AI_TURN_RIGHT	= false;

	// reset the script object
	ConstructScriptObject();

	// execute the script so the script object's constructor takes effect immediately
	scriptThread->Execute();

	forceScoreBoard		= false;
	forcedReady			= false;

	privateCameraView	= NULL;

	lastSpectateChange	= 0;
	lastTeleFX			= -9999;

	hiddenWeapon		= false;
	tipUp				= false;
	objectiveUp			= false;
	teleportEntity		= NULL;
	teleportKiller		= -1;
	leader				= false;

	SetPrivateCameraView( NULL );

	lastSnapshotSequence	= 0;

	MPAim				= -1;
	lastMPAim			= -1;
	lastMPAimTime		= 0;
	MPAimFadeTime		= 0;
	MPAimHighlight		= false;

	if ( hud ) {
		hud->HandleNamedEvent( "aim_clear" );
	}

	cvarSystem->SetCVarBool( "ui_chat", false );

	//ivan start

	//fix: upd distance and height Cvars on start or respawn
	UpdateCameraCvarsFromSettings();

	if( !quickRespawn ){ //don't reset this if it's a quick respawn
		health_lost	= 0;
	}
	save_walk_dir			= false;
	keep_walk_dir			= false;
	old_viewAngles_yaw		= 0.0f;
	fw_toggled				= false;
	fw_inverted				= false;
	viewPos					= 0.0f;
	fastXpos				= 0.0f;
	isXlocked				= true;
	blendModelYaw			= false;
	forcedMovWasLocked		= false;
	forcedMovIncreasingX	= false;
	forcedMovCanBeAborted	= false;
	forcedMovTotalForce		= false;
	forcedMovState			= FORCEDMOVE_STATE_DISABLED;
	forcedMovDelta			= vec3_zero;
	forcedMovOldOrg			= vec3_zero;
	forcedMovTarget			= NULL;
	skipCameraZblend		= true; //force instant update first frame
	enableCameraYblend		= false;
	enableCameraXblend		= false;
	inhibitInputTime		= 0;
	inhibitAimCrouchTime	= 0;

	//cameraSettings.lockYaxis	= false;
	//cameraSettings.lockedYpos		= 0.0f;
	//cameraSettings.distance		= CAMERA_DEFAULT_DISTANCE; //don't reset this.

#ifdef SHOW_MOVING_CROSSHAIR
	reqDefaultCrossPos		= true;
	cposx					= 0; 
	cposy					= 0; 
	coffx					= 0.0f;
#endif
	//ivan end
}

/*
==============
idPlayer::Spawn

Prepare any resources used by the player.
==============
*/
void idPlayer::Spawn( void ) { //ivan note: this is only done the first time player is spawned in the map
	idStr		temp;
	idBounds	bounds;

	if ( entityNumber >= MAX_CLIENTS ) {
		gameLocal.Error( "entityNum > MAX_CLIENTS for player.  Player may only be spawned with a client." );
	}

	// allow thinking during cinematics
	cinematic = true;

	if ( gameLocal.isMultiplayer ) {
		// always start in spectating state waiting to be spawned in
		// do this before SetClipModel to get the right bounding box
		spectating = true;
	}

	// set our collision model
	physicsObj.SetSelf( this );
	SetClipModel();
	physicsObj.SetMass( spawnArgs.GetFloat( "mass", "100" ) );
	physicsObj.SetContents( CONTENTS_BODY );
	physicsObj.SetClipMask( MASK_PLAYERSOLID );
	SetPhysics( &physicsObj );
	InitAASLocation();

	skin = renderEntity.customSkin;

	// only the local player needs guis
	if ( !gameLocal.isMultiplayer || entityNumber == gameLocal.localClientNum ) {

		// load HUD
		if ( gameLocal.isMultiplayer ) {
			hud = uiManager->FindGui( "guis/mphud.gui", true, false, true );
		} else if ( spawnArgs.GetString( "hud", "", temp ) ) {
			hud = uiManager->FindGui( temp, true, false, true );
		}

		if ( hud ) {
			hud->Activate( true, gameLocal.time );
		}

		// load cursor
		if ( spawnArgs.GetString( "cursor", "", temp ) ) {
			cursor = uiManager->FindGui( temp, true, gameLocal.isMultiplayer, gameLocal.isMultiplayer );
		}
		if ( cursor ) {
			cursor->Activate( true, gameLocal.time );
		}

		objectiveSystem = uiManager->FindGui( "guis/pda.gui", true, false, true );
		objectiveSystemOpen = false;
	}

	SetLastHitTime( 0 );

	// load the armor sound feedback
	declManager->FindSound( "player_sounds_hitArmor" );

	// set up conditions for animation
	LinkScriptVariables();

	animator.RemoveOriginOffset( true );

	// initialize user info related settings
	// on server, we wait for the userinfo broadcast, as this controls when the player is initially spawned in game
	if ( gameLocal.isClient || entityNumber == gameLocal.localClientNum ) {
		UserInfoChanged( false );
	}

	// create combat collision hull for exact collision detection
	SetCombatModel();

	// init the damage effects
	playerView.SetPlayerEntity( this );

	// supress model in non-player views, but allow it in mirrors and remote views
	renderEntity.suppressSurfaceInViewID = entityNumber+1;

	// don't project shadow on self or weapon
	renderEntity.noSelfShadow = true;

	idAFAttachment *headEnt = head.GetEntity();
	if ( headEnt ) {
		headEnt->GetRenderEntity()->suppressSurfaceInViewID = entityNumber+1;
		headEnt->GetRenderEntity()->noSelfShadow = true;
	}

	if ( gameLocal.isMultiplayer ) {
		Init( false );
		Hide();	// properly hidden if starting as a spectator
		if ( !gameLocal.isClient ) {
			// set yourself ready to spawn. idMultiplayerGame will decide when/if appropriate and call SpawnFromSpawnSpot
			SetupWeaponEntity();
			SpawnFromSpawnSpot();
			forceRespawn = true;
			assert( spectating );
		}
	} else {
		SetupWeaponEntity();
		SpawnFromSpawnSpot();
	}

	// trigger playtesting item gives, if we didn't get here from a previous level
	// the devmap key will be set on the first devmap, but cleared on any level
	// transitions
	if ( !gameLocal.isMultiplayer && gameLocal.serverInfo.FindKey( "devmap" ) ) {
		// fire a trigger with the name "devmap"
		idEntity *ent = gameLocal.FindEntity( "devmap" );
		if ( ent ) {
			ent->ActivateTargets( this );
		}
	}
	if ( hud ) {
		// We can spawn with a full soul cube, so we need to make sure the hud knows this
		if ( weapon_soulcube > 0 && ( inventory.weapons & ( 1 << weapon_soulcube ) ) ) {
			int max_souls = inventory.MaxAmmoForAmmoClass( this, "ammo_souls" );
			if ( inventory.ammo[ idWeapon::GetAmmoNumForName( "ammo_souls" ) ] >= max_souls ) {
				hud->HandleNamedEvent( "soulCubeReady" );
			}
		}
		hud->HandleNamedEvent( "itemPickup" );
	}

	if ( GetPDA() ) {
		// Add any emails from the inventory
		for ( int i = 0; i < inventory.emails.Num(); i++ ) {
			GetPDA()->AddEmail( inventory.emails[i] );
		}
		GetPDA()->SetSecurity( common->GetLanguageDict()->GetString( "#str_00066" ) );
	}

	if ( gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) ) {
		hiddenWeapon = true;
		if ( weapon.GetEntity() ) {
			weapon.GetEntity()->LowerWeapon();
		}
		idealWeapon = 0;
	} else {
		hiddenWeapon = false;
	}

	if ( hud ) {
		UpdateHudWeapon();
		hud->StateChanged( gameLocal.time );
	}

	tipUp = false;
	objectiveUp = false;

	if ( inventory.levelTriggers.Num() ) {
		PostEventMS( &EV_Player_LevelTrigger, 0 );
	}

	inventory.pdaOpened = false;
	inventory.selPDA = 0;

	if ( !gameLocal.isMultiplayer ) {
		if ( g_skill.GetInteger() < 2 ) {
			if ( health < 25 ) {
				health = 25;
			}
			if ( g_useDynamicProtection.GetBool() ) {
				g_damageScale.SetFloat( 1.0f );
			}
		} else {
			g_damageScale.SetFloat( 1.0f );
			g_armorProtection.SetFloat( ( g_skill.GetInteger() < 2 ) ? 0.4f : 0.2f );
			if ( g_skill.GetInteger() == 3 ) {
				healthTake = true;
				nextHealthTake = gameLocal.time + g_healthTakeTime.GetInteger() * 1000;
			}
		}
	}
	//Setup the weapon toggle lists //NEW //un noted change from original sdk
	const idKeyValue *kv;
	kv = spawnArgs.MatchPrefix( "weapontoggle", NULL );
	while( kv ) {
		WeaponToggle_t newToggle;
		strcpy(newToggle.name, kv->GetKey().c_str());

		idStr toggleData = kv->GetValue();

		idLexer src;
		idToken token;
		src.LoadMemory(toggleData, toggleData.Length(), "toggleData");
		while(1) {
			if(!src.ReadToken(&token)) {
				break;
			}
			int index = atoi(token.c_str());
			newToggle.toggleList.Append(index);

			//Skip the ,
			src.ReadToken(&token);
		}
		weaponToggles.Set(newToggle.name, newToggle);

		kv = spawnArgs.MatchPrefix( "weapontoggle", kv );
	}

#ifdef _DENTONMOD_PLAYER_CPP
	memset(	projectileType, 0, sizeof(projectileType) );
#endif

	//ivan start
	
	//set num lives depending difficulty level the first time player spawns
	if( numLives <= 0 ){ //if this is not the first level we play, numLives we'll be > 0 at this point, so this is done only for the first map.
		if ( g_skill.GetInteger() == 0 ) {
			spawnArgs.GetInt( "numLives_easy", "0", numLives );
		} else if ( g_skill.GetInteger() == 1 ) {
			spawnArgs.GetInt( "numLives_medium", "0", numLives );
		} else {
			spawnArgs.GetInt( "numLives_hard", "0", numLives );
		}
	}

	updXlock = true; //make sure this is true for player so AI know he can change X lock pos.
	isOnScreen = true; //never updated. let AIs know that player is on screen.
	//ivan end
}

/*
==============
idPlayer::~idPlayer()

Release any resources used by the player.
==============
*/
idPlayer::~idPlayer() {
	delete weapon.GetEntity();
	weapon = NULL;
}

/*
===========
idPlayer::Save
===========
*/
void idPlayer::Save( idSaveGame *savefile ) const {
	int i;

	savefile->WriteUsercmd( usercmd );
	playerView.Save( savefile );

	savefile->WriteBool( noclip );
	savefile->WriteBool( godmode );
	//ivan start
	savefile->WriteBool( animMoveNoGravity ); 
	savefile->WriteInt( animMoveType ); 
	savefile->WriteBool( comboOn ); 
	savefile->WriteBool( allowTurn ); 
	savefile->WriteBool( blendModelYaw );
 
	//not saved: lastSpread
	//ivan end

	// don't save spawnAnglesSet, since we'll have to reset them after loading the savegame
	savefile->WriteAngles( spawnAngles );
	savefile->WriteAngles( viewAngles );
	savefile->WriteAngles( cmdAngles );

	savefile->WriteInt( buttonMask );
	savefile->WriteInt( oldButtons );
	savefile->WriteInt( oldFlags );

	savefile->WriteInt( lastHitTime );
	savefile->WriteInt( lastSndHitTime );
	savefile->WriteInt( lastSavingThrowTime );

	// idBoolFields don't need to be saved, just re-linked in Restore

	inventory.Save( savefile );
	weapon.Save( savefile );

/*	
//Rev 2020 Note... the restore part was not added in... thus the game would crash.  Thanks DG!
//Rev 2020 Note:  Currently not using multiple huds.  Maybe in the future.
	//savefile->WriteUserInterface( hud, false ); // DG: don't save HUD, just create it like in Spawn()
	savefile->WriteString( "" ); // DG: write empty string which is handled as "HUD is NULL" by Restore() for backwards-compat
*/
	savefile->WriteUserInterface( hud, false );
	savefile->WriteUserInterface( objectiveSystem, false );
	savefile->WriteBool( objectiveSystemOpen );

	savefile->WriteInt( weapon_soulcube );
	savefile->WriteInt( weapon_pda );
	savefile->WriteInt( weapon_fists );

	savefile->WriteInt( heartRate );

	savefile->WriteFloat( heartInfo.GetStartTime() );
	savefile->WriteFloat( heartInfo.GetDuration() );
	savefile->WriteFloat( heartInfo.GetStartValue() );
	savefile->WriteFloat( heartInfo.GetEndValue() );

	savefile->WriteInt( lastHeartAdjust );
	savefile->WriteInt( lastHeartBeat );
	savefile->WriteInt( lastDmgTime );
	savefile->WriteInt( deathClearContentsTime );
	savefile->WriteBool( doingDeathSkin );
	savefile->WriteInt( lastArmorPulse );
	savefile->WriteFloat( stamina );
	savefile->WriteFloat( healthPool );
	savefile->WriteInt( nextHealthPulse );
	savefile->WriteBool( healthPulse );
	savefile->WriteInt( nextHealthTake );
	savefile->WriteBool( healthTake );

	savefile->WriteBool( hiddenWeapon );
	soulCubeProjectile.Save( savefile );

	savefile->WriteInt( spectator );
	savefile->WriteVec3( colorBar );
	savefile->WriteInt( colorBarIndex );
	savefile->WriteBool( scoreBoardOpen );
	savefile->WriteBool( forceScoreBoard );
	savefile->WriteBool( forceRespawn );
	savefile->WriteBool( spectating );
	savefile->WriteInt( lastSpectateTeleport );
	savefile->WriteBool( lastHitToggle );
	savefile->WriteBool( forcedReady );
	savefile->WriteBool( wantSpectate );
	savefile->WriteBool( weaponGone );
	savefile->WriteBool( useInitialSpawns );
	savefile->WriteInt( latchedTeam );
	savefile->WriteInt( tourneyRank );
	savefile->WriteInt( tourneyLine );

	teleportEntity.Save( savefile );
	savefile->WriteInt( teleportKiller );

	savefile->WriteInt( minRespawnTime );
	savefile->WriteInt( maxRespawnTime );

	savefile->WriteVec3( firstPersonViewOrigin );
	savefile->WriteMat3( firstPersonViewAxis );

	// don't bother saving dragEntity since it's a dev tool

	savefile->WriteJoint( hipJoint );
	savefile->WriteJoint( chestJoint );
	savefile->WriteJoint( headJoint );

	savefile->WriteStaticObject( physicsObj );

	savefile->WriteInt( aasLocation.Num() );
	for( i = 0; i < aasLocation.Num(); i++ ) {
		savefile->WriteInt( aasLocation[ i ].areaNum );
		savefile->WriteVec3( aasLocation[ i ].pos );
	}

	/* //un noted change from original sdk
	savefile->WriteInt( bobFoot );
	savefile->WriteFloat( bobFrac );
	savefile->WriteFloat( bobfracsin );
	savefile->WriteInt( bobCycle );
	*/
	savefile->WriteFloat( xyspeed );
	/* //un noted change from original sdk
	savefile->WriteInt( stepUpTime );
	savefile->WriteFloat( stepUpDelta );
	*/
	savefile->WriteFloat( idealLegsYaw );
	savefile->WriteFloat( legsYaw );
	savefile->WriteBool( legsForward );
	savefile->WriteFloat( oldViewYaw );
	//savefile->WriteAngles( viewBobAngles ); //un noted change from original sdk
	//savefile->WriteVec3( viewBob );
	savefile->WriteInt( landChange );
	savefile->WriteInt( landTime );

	savefile->WriteInt( currentWeapon );
	savefile->WriteInt( idealWeapon );
	savefile->WriteInt( previousWeapon );
	savefile->WriteInt( quickWeapon );      //new //un noted change from original sdk
	savefile->WriteInt( weaponSwitchTime );
	savefile->WriteBool( weaponEnabled );
	savefile->WriteBool( showWeaponViewModel );

	savefile->WriteSkin( skin );
	savefile->WriteSkin( powerUpSkin );
	savefile->WriteString( baseSkinName );

	savefile->WriteInt( numProjectilesFired );
	savefile->WriteInt( numProjectileHits );

	savefile->WriteBool( airless );
	savefile->WriteInt( airTics );
	savefile->WriteInt( lastAirDamage );

	savefile->WriteBool( gibDeath );
	savefile->WriteBool( gibsLaunched );
	savefile->WriteVec3( gibsDir );

	//Remeber the order of saving this info... cause last time I did a stupid mistake....
#ifdef _DENTONMOD_PLAYER_CPP
	weaponZoom_s flags = weaponZoom;			// Save the weapon Zoom Info
	LittleBitField( &flags, sizeof( flags ) ); 
	savefile->Write( &flags, sizeof( flags ) );

	for( i = 0; i < MAX_WEAPONS; i++ ) {
		savefile->WriteByte( projectileType[ i ] );
	}
#endif //_DENTONMOD_PLAYER_CPP

	savefile->WriteFloat( zoomFov.GetStartTime() );
	savefile->WriteFloat( zoomFov.GetDuration() );
	savefile->WriteFloat( zoomFov.GetStartValue() );
	savefile->WriteFloat( zoomFov.GetEndValue() );

	savefile->WriteFloat( centerView.GetStartTime() );
	savefile->WriteFloat( centerView.GetDuration() );
	savefile->WriteFloat( centerView.GetStartValue() );
	savefile->WriteFloat( centerView.GetEndValue() );

	savefile->WriteBool( fxFov );

	savefile->WriteFloat( influenceFov );
	savefile->WriteInt( influenceActive );
	savefile->WriteFloat( influenceRadius );
	savefile->WriteObject( influenceEntity );
	savefile->WriteMaterial( influenceMaterial );
	savefile->WriteSkin( influenceSkin );

	savefile->WriteObject( privateCameraView );

	for( i = 0; i < NUM_LOGGED_VIEW_ANGLES; i++ ) {
		savefile->WriteAngles( loggedViewAngles[ i ] );
	}
	for( i = 0; i < NUM_LOGGED_ACCELS; i++ ) {
		savefile->WriteInt( loggedAccel[ i ].time );
		savefile->WriteVec3( loggedAccel[ i ].dir );
	}
	savefile->WriteInt( currentLoggedAccel );

	savefile->WriteObject( focusGUIent );
	// can't save focusUI
	savefile->WriteObject( focusCharacter );
	savefile->WriteInt( talkCursor );
	savefile->WriteInt( focusTime );
	savefile->WriteObject( focusVehicle );
	savefile->WriteUserInterface( cursor, false );

	savefile->WriteInt( oldMouseX );
	savefile->WriteInt( oldMouseY );

	savefile->WriteString( pdaAudio );
	savefile->WriteString( pdaVideo );
	savefile->WriteString( pdaVideoWave );

	savefile->WriteBool( tipUp );
	savefile->WriteBool( objectiveUp );

	savefile->WriteInt( lastDamageDef );
	savefile->WriteVec3( lastDamageDir );
	savefile->WriteInt( lastDamageLocation );
	savefile->WriteInt( smoothedFrame );
	savefile->WriteBool( smoothedOriginUpdated );
	savefile->WriteVec3( smoothedOrigin );
	savefile->WriteAngles( smoothedAngles );

	savefile->WriteBool( ready );
	savefile->WriteBool( respawning );
	savefile->WriteBool( leader );
	savefile->WriteInt( lastSpectateChange );
	savefile->WriteInt( lastTeleFX );

	savefile->WriteFloat( pm_stamina.GetFloat() );

	if ( hud ) {
		hud->SetStateString( "message", common->GetLanguageDict()->GetString( "#str_02916" ) );
		hud->HandleNamedEvent( "Message" );
	}
	savefile->WriteInt(weaponToggles.Num());         //new, all lines from here
	for(i = 0; i < weaponToggles.Num(); i++) {
		WeaponToggle_t* weaponToggle = weaponToggles.GetIndex(i);
		savefile->WriteString(weaponToggle->name);
		savefile->WriteInt(weaponToggle->toggleList.Num());
		for(int j = 0; j < weaponToggle->toggleList.Num(); j++) {
			savefile->WriteInt(weaponToggle->toggleList[j]);
		}
	}
	
	//ivan start
	savefile->WriteInt( health_lost );
	savefile->WriteInt( currentSlot );
	savefile->WriteFloat( viewPos );
	savefile->WriteVec3( oldCameraPos );
	/*
	//moved to idActor
	savefile->WriteFloat( lockedXpos );
	savefile->WriteBool( isXlocked );
	*/
	savefile->WriteBool( forcedMovWasLocked ); 
	savefile->WriteBool( blendModelYaw ); 
	savefile->WriteBool( forcedMovIncreasingX ); 
	savefile->WriteInt( forcedMovState ); 
	savefile->WriteBool( forcedMovCanBeAborted ); 
	savefile->WriteBool( forcedMovTotalForce );
	savefile->WriteVec3( forcedMovDelta );
	savefile->WriteVec3( forcedMovOldOrg ); 
	savefile->WriteObject( forcedMovTarget );
	savefile->WriteBool( skipCameraZblend );  
	savefile->WriteBool( enableCameraYblend ); 
	savefile->WriteBool( enableCameraXblend ); 
	savefile->WriteInt( inhibitInputTime ); 
	savefile->WriteInt( inhibitAimCrouchTime ); 
	savefile->WriteBool( cameraSettings.lockYaxis ); 
	savefile->WriteBool( cameraSettings.lockZaxis ); 
	savefile->WriteFloat( cameraSettings.lockedYpos );
	savefile->WriteFloat( cameraSettings.lockedZpos );
	savefile->WriteFloat( cameraSettings.distance ); 
	savefile->WriteFloat( cameraSettings.height ); 
	savefile->WriteInt( lastCheckPoint.entityNumber ); 
	savefile->WriteVec3( lastCheckPoint.spawnPos );
	savefile->WriteBool( lastCheckPoint.cameraSettings.lockYaxis );
	savefile->WriteBool( lastCheckPoint.cameraSettings.lockZaxis ); 
	savefile->WriteFloat( lastCheckPoint.cameraSettings.lockedYpos );
	savefile->WriteFloat( lastCheckPoint.cameraSettings.lockedZpos );
	savefile->WriteFloat( lastCheckPoint.cameraSettings.distance ); 
	savefile->WriteFloat( lastCheckPoint.cameraSettings.height ); 
	savefile->WriteBool( animBasedMovement ); 
	savefile->WriteInt( numLives ); 
	savefile->WriteInt( score ); 
	savefile->WriteInt( interactFlag );
	savefile->WriteInt( interactShownWeaponNum );
//interactShownWeaponName not saved
	savefile->WriteBool( skipMouseUpd ); 

//ivan start - kick
	savefile->WriteString( kickDefName );
//kickDef is not saved! -> it'll be reastored thanks to kickDefName 
	savefile->WriteObject( lastKickedEnt ); 
	savefile->WriteInt( nextKickFx );
	savefile->WriteInt( nextKickSnd );
	savefile->WriteBool( kickEnabled );
	savefile->WriteFloat( kickDmgMultiplier );
	savefile->WriteFloat( kickDistance );
	savefile->WriteBounds( kickBox ); 
	savefile->WriteJoint( fromJointKick );
	savefile->WriteJoint( toJointKick );

#ifdef AUTOUPD_RESPAWN_POS
	savefile->WriteInt( nextRespPosTime ); 
	//not saved: idVec3	tempRespawnPos
#endif
	
	/* 
	//the following are not saved:
	bool					save_walk_dir;
	bool					keep_walk_dir;
	float					old_viewAngles_yaw;
	bool					fw_toggled;
	bool					fw_inverted;
	//bool					skipMouseUpd;
	*/

#ifdef SHOW_MOVING_CROSSHAIR
	savefile->WriteBool( reqDefaultCrossPos );
	savefile->WriteInt( cposx ); 
	savefile->WriteInt( cposy ); 
	//the following are not saved:
	//coffx
#endif

	friendsCommonEnemy.Save( savefile ); //smart AI
	//ivan end
}

/*
===========
idPlayer::Restore
===========
*/
void idPlayer::Restore( idRestoreGame *savefile ) {
	int	  i;
	int	  num;
	float set;

	savefile->ReadUsercmd( usercmd );
	playerView.Restore( savefile );

	savefile->ReadBool( noclip );
	savefile->ReadBool( godmode );

	//ivan start
	savefile->ReadBool( animMoveNoGravity );
	savefile->ReadInt( animMoveType );
	savefile->ReadBool( comboOn ); 
	savefile->ReadBool( allowTurn );
	savefile->ReadBool( blendModelYaw );
	//ivan end
	
	savefile->ReadAngles( spawnAngles );
	savefile->ReadAngles( viewAngles );
	savefile->ReadAngles( cmdAngles );

	memset( usercmd.angles, 0, sizeof( usercmd.angles ) );
	SetViewAngles( viewAngles );
	spawnAnglesSet = true;

	savefile->ReadInt( buttonMask );
	savefile->ReadInt( oldButtons );
	savefile->ReadInt( oldFlags );

	usercmd.flags = 0;
	oldFlags = 0;

	savefile->ReadInt( lastHitTime );
	savefile->ReadInt( lastSndHitTime );
	savefile->ReadInt( lastSavingThrowTime );

	// Re-link idBoolFields to the scriptObject, values will be restored in scriptObject's restore
	LinkScriptVariables();

	inventory.Restore( savefile );
	weapon.Restore( savefile );

	for ( i = 0; i < inventory.emails.Num(); i++ ) {
		GetPDA()->AddEmail( inventory.emails[i] );
	}

	savefile->ReadUserInterface( hud );
	savefile->ReadUserInterface( objectiveSystem );
	savefile->ReadBool( objectiveSystemOpen );

	savefile->ReadInt( weapon_soulcube );
	savefile->ReadInt( weapon_pda );
	savefile->ReadInt( weapon_fists );

	savefile->ReadInt( heartRate );

	savefile->ReadFloat( set );
	heartInfo.SetStartTime( set );
	savefile->ReadFloat( set );
	heartInfo.SetDuration( set );
	savefile->ReadFloat( set );
	heartInfo.SetStartValue( set );
	savefile->ReadFloat( set );
	heartInfo.SetEndValue( set );

	savefile->ReadInt( lastHeartAdjust );
	savefile->ReadInt( lastHeartBeat );
	savefile->ReadInt( lastDmgTime );
	savefile->ReadInt( deathClearContentsTime );
	savefile->ReadBool( doingDeathSkin );
	savefile->ReadInt( lastArmorPulse );
	savefile->ReadFloat( stamina );
	savefile->ReadFloat( healthPool );
	savefile->ReadInt( nextHealthPulse );
	savefile->ReadBool( healthPulse );
	savefile->ReadInt( nextHealthTake );
	savefile->ReadBool( healthTake );

	savefile->ReadBool( hiddenWeapon );
	soulCubeProjectile.Restore( savefile );

	savefile->ReadInt( spectator );
	savefile->ReadVec3( colorBar );
	savefile->ReadInt( colorBarIndex );
	savefile->ReadBool( scoreBoardOpen );
	savefile->ReadBool( forceScoreBoard );
	savefile->ReadBool( forceRespawn );
	savefile->ReadBool( spectating );
	savefile->ReadInt( lastSpectateTeleport );
	savefile->ReadBool( lastHitToggle );
	savefile->ReadBool( forcedReady );
	savefile->ReadBool( wantSpectate );
	savefile->ReadBool( weaponGone );
	savefile->ReadBool( useInitialSpawns );
	savefile->ReadInt( latchedTeam );
	savefile->ReadInt( tourneyRank );
	savefile->ReadInt( tourneyLine );

	teleportEntity.Restore( savefile );
	savefile->ReadInt( teleportKiller );

	savefile->ReadInt( minRespawnTime );
	savefile->ReadInt( maxRespawnTime );

	savefile->ReadVec3( firstPersonViewOrigin );
	savefile->ReadMat3( firstPersonViewAxis );

	// don't bother saving dragEntity since it's a dev tool
	dragEntity.Clear();

	savefile->ReadJoint( hipJoint );
	savefile->ReadJoint( chestJoint );
	savefile->ReadJoint( headJoint );

	savefile->ReadStaticObject( physicsObj );
	RestorePhysics( &physicsObj );

	savefile->ReadInt( num );
	aasLocation.SetGranularity( 1 );
	aasLocation.SetNum( num );
	for( i = 0; i < num; i++ ) {
		savefile->ReadInt( aasLocation[ i ].areaNum );
		savefile->ReadVec3( aasLocation[ i ].pos );
	}

	/* //un noted change from original sdk
	savefile->ReadInt( bobFoot );
	savefile->ReadFloat( bobFrac );
	savefile->ReadFloat( bobfracsin );
	savefile->ReadInt( bobCycle );
	*/
	savefile->ReadFloat( xyspeed );
	/* //un noted change from original sdk
	savefile->ReadInt( stepUpTime );
	savefile->ReadFloat( stepUpDelta );
	*/
	savefile->ReadFloat( idealLegsYaw );
	savefile->ReadFloat( legsYaw );
	savefile->ReadBool( legsForward );
	savefile->ReadFloat( oldViewYaw );
	//savefile->ReadAngles( viewBobAngles ); //un noted change from original sdk
	//savefile->ReadVec3( viewBob );
	savefile->ReadInt( landChange );
	savefile->ReadInt( landTime );

	savefile->ReadInt( currentWeapon );
	savefile->ReadInt( idealWeapon );
	savefile->ReadInt( previousWeapon );
	savefile->ReadInt( quickWeapon );   //new //un noted change from original sdk
	savefile->ReadInt( weaponSwitchTime );
	savefile->ReadBool( weaponEnabled );
	savefile->ReadBool( showWeaponViewModel );

	savefile->ReadSkin( skin );
	savefile->ReadSkin( powerUpSkin );
	savefile->ReadString( baseSkinName );

	savefile->ReadInt( numProjectilesFired );
	savefile->ReadInt( numProjectileHits );

	savefile->ReadBool( airless );
	savefile->ReadInt( airTics );
	savefile->ReadInt( lastAirDamage );

	savefile->ReadBool( gibDeath );
	savefile->ReadBool( gibsLaunched );
	savefile->ReadVec3( gibsDir );

#ifdef _DENTONMOD_PLAYER_CPP
	// Remember the order of saving this info...
	savefile->Read( &weaponZoom, sizeof( weaponZoom ) );
	LittleBitField( &weaponZoom, sizeof( weaponZoom ) );

	for( i = 0; i < MAX_WEAPONS; i++ ) {
		savefile->ReadByte( projectileType[ i ] );
	}
#endif //_DENTONMOD_PLAYER_CPP

	savefile->ReadFloat( set );
	zoomFov.SetStartTime( set );
	savefile->ReadFloat( set );
	zoomFov.SetDuration( set );
	savefile->ReadFloat( set );
	zoomFov.SetStartValue( set );
	savefile->ReadFloat( set );
	zoomFov.SetEndValue( set );

	savefile->ReadFloat( set );
	centerView.SetStartTime( set );
	savefile->ReadFloat( set );
	centerView.SetDuration( set );
	savefile->ReadFloat( set );
	centerView.SetStartValue( set );
	savefile->ReadFloat( set );
	centerView.SetEndValue( set );

	savefile->ReadBool( fxFov );

	savefile->ReadFloat( influenceFov );
	savefile->ReadInt( influenceActive );
	savefile->ReadFloat( influenceRadius );
	savefile->ReadObject( reinterpret_cast<idClass *&>( influenceEntity ) );
	savefile->ReadMaterial( influenceMaterial );
	savefile->ReadSkin( influenceSkin );

	savefile->ReadObject( reinterpret_cast<idClass *&>( privateCameraView ) );

	for( i = 0; i < NUM_LOGGED_VIEW_ANGLES; i++ ) {
		savefile->ReadAngles( loggedViewAngles[ i ] );
	}
	for( i = 0; i < NUM_LOGGED_ACCELS; i++ ) {
		savefile->ReadInt( loggedAccel[ i ].time );
		savefile->ReadVec3( loggedAccel[ i ].dir );
	}
	savefile->ReadInt( currentLoggedAccel );

	savefile->ReadObject( reinterpret_cast<idClass *&>( focusGUIent ) );
	// can't save focusUI
	focusUI = NULL;
	savefile->ReadObject( reinterpret_cast<idClass *&>( focusCharacter ) );
	savefile->ReadInt( talkCursor );
	savefile->ReadInt( focusTime );
	savefile->ReadObject( reinterpret_cast<idClass *&>( focusVehicle ) );
	savefile->ReadUserInterface( cursor );

	savefile->ReadInt( oldMouseX );
	savefile->ReadInt( oldMouseY );

	savefile->ReadString( pdaAudio );
	savefile->ReadString( pdaVideo );
	savefile->ReadString( pdaVideoWave );

	savefile->ReadBool( tipUp );
	savefile->ReadBool( objectiveUp );

	savefile->ReadInt( lastDamageDef );
	savefile->ReadVec3( lastDamageDir );
	savefile->ReadInt( lastDamageLocation );
	savefile->ReadInt( smoothedFrame );
	savefile->ReadBool( smoothedOriginUpdated );
	savefile->ReadVec3( smoothedOrigin );
	savefile->ReadAngles( smoothedAngles );

	savefile->ReadBool( ready );
	savefile->ReadBool( respawning );
	savefile->ReadBool( leader );
	savefile->ReadInt( lastSpectateChange );
	savefile->ReadInt( lastTeleFX );

	// set the pm_ cvars
	const idKeyValue	*kv;
	kv = spawnArgs.MatchPrefix( "pm_", NULL );
	while( kv ) {
		cvarSystem->SetCVarString( kv->GetKey(), kv->GetValue() );
		kv = spawnArgs.MatchPrefix( "pm_", kv );
	}

	savefile->ReadFloat( set );
	pm_stamina.SetFloat( set );

	// create combat collision hull for exact collision detection
	SetCombatModel();
	int weaponToggleCount;  //new all lines from here //un noted change from original sdk
	savefile->ReadInt(weaponToggleCount);
	for(i = 0; i < weaponToggleCount; i++) {
		WeaponToggle_t newToggle;
		memset(&newToggle, 0, sizeof(newToggle));

		idStr name;
		savefile->ReadString(name);
		strcpy(newToggle.name, name.c_str());

		int indexCount;
		savefile->ReadInt(indexCount);
		for(int j = 0; j < indexCount; j++) {
			int temp;
			savefile->ReadInt(temp);
			newToggle.toggleList.Append(temp);
		}
		weaponToggles.Set(newToggle.name, newToggle);
	}

	//ivan start
	savefile->ReadInt( health_lost );
	savefile->ReadInt( currentSlot );
	savefile->ReadFloat( viewPos );
	savefile->ReadVec3( oldCameraPos );	
	/*
	//moved to idActor
	savefile->ReadFloat( lockedXpos );	
	savefile->ReadBool( isXlocked );
	*/
	savefile->ReadBool( forcedMovWasLocked ); 
	savefile->ReadBool( blendModelYaw ); 
	savefile->ReadBool( forcedMovIncreasingX ); 
	savefile->ReadInt( forcedMovState );
	savefile->ReadBool( forcedMovCanBeAborted ); 
	savefile->ReadBool( forcedMovTotalForce ); 
	savefile->ReadVec3( forcedMovDelta ); 
	savefile->ReadVec3( forcedMovOldOrg ); 
	savefile->ReadObject( reinterpret_cast<idClass *&>( forcedMovTarget ) ); 
	savefile->ReadBool( skipCameraZblend ); 
	savefile->ReadBool( enableCameraYblend );
	savefile->ReadBool( enableCameraXblend );
	savefile->ReadInt( inhibitInputTime );
	savefile->ReadInt( inhibitAimCrouchTime );
	savefile->ReadBool( cameraSettings.lockYaxis );
	savefile->ReadBool( cameraSettings.lockZaxis ); 
	savefile->ReadFloat( cameraSettings.lockedYpos );
	savefile->ReadFloat( cameraSettings.lockedZpos );
	savefile->ReadFloat( cameraSettings.distance );
	savefile->ReadFloat( cameraSettings.height );
	savefile->ReadInt( lastCheckPoint.entityNumber );
	savefile->ReadVec3( lastCheckPoint.spawnPos );
	savefile->ReadBool( lastCheckPoint.cameraSettings.lockYaxis );
	savefile->ReadBool( lastCheckPoint.cameraSettings.lockZaxis );
	savefile->ReadFloat( lastCheckPoint.cameraSettings.lockedYpos );
	savefile->ReadFloat( lastCheckPoint.cameraSettings.lockedZpos );
	savefile->ReadFloat( lastCheckPoint.cameraSettings.distance ); 
	savefile->ReadFloat( lastCheckPoint.cameraSettings.height ); 
	savefile->ReadBool( animBasedMovement ); 
	savefile->ReadInt( numLives );
	savefile->ReadInt( score );
	savefile->ReadInt( interactFlag );
	savefile->ReadInt( interactShownWeaponNum );
//interactShownWeaponName not saved
	//interactShownWeaponName = "";
	savefile->ReadBool( skipMouseUpd );

//Rev 2020 moved below to match save game file order. prevents crash.  Thanks DG
	//ivan start - kick
	savefile->ReadString( kickDefName );
	kickDef = gameLocal.FindEntityDef( kickDefName, false );
	savefile->ReadObject( reinterpret_cast<idClass *&>( lastKickedEnt ) ); //ivan
	savefile->ReadInt( nextKickFx );
	savefile->ReadInt( nextKickSnd );
	savefile->ReadBool( kickEnabled );
	savefile->ReadFloat( kickDmgMultiplier );
	savefile->ReadFloat( kickDistance );
	savefile->ReadBounds( kickBox ); 
	savefile->ReadJoint( fromJointKick );
	savefile->ReadJoint( toJointKick );	

#ifdef AUTOUPD_RESPAWN_POS
	savefile->ReadInt( nextRespPosTime ); 
	tempRespawnPos		= lastCheckPoint.spawnPos; //was not saved
#endif
	
	//the following are not saved:
	save_walk_dir		= false;
	keep_walk_dir		= false;
	old_viewAngles_yaw	= 0.0f;
	fw_toggled			= false;
	fw_inverted			= false;
	//skipMouseUpd		= false;

#ifdef SHOW_MOVING_CROSSHAIR
	savefile->ReadBool( reqDefaultCrossPos );
	savefile->ReadInt( cposx ); 
	savefile->ReadInt( cposy ); 
	coffx = 0.0f;
#endif

	friendsCommonEnemy.Restore( savefile ); //smart AI

	//fix: upd camera Cvars on reload
	UpdateCameraCvarsFromSettings();
	//ivan end
	
////rev 2021 Dhewm 1.5.1 updates:
	// DG: workaround for lingering messages that are shown forever after loading a savegame
	//     (one way to get them is saving again, while the message from first save is still
	//      shown, and then load)
	if ( hud ) {
		hud->SetStateString( "message", "" );
	}
}

/*
===============
idPlayer::PrepareForRestart
================
*/
void idPlayer::PrepareForRestart( void ) {
	ClearPowerUps();
	Spectate( true );
	forceRespawn = true;

	// we will be restarting program, clear the client entities from program-related things first
	ShutdownThreads();

	// the sound world is going to be cleared, don't keep references to emitters
	FreeSoundEmitter( false );
}

/*
===============
idPlayer::Restart
================
*/
void idPlayer::Restart( void ) {
	idActor::Restart();

	// client needs to setup the animation script object again
	if ( gameLocal.isClient ) {
		Init( false );
	} else {
		// choose a random spot and prepare the point of view in case player is left spectating
		assert( spectating );
		SpawnFromSpawnSpot();
	}

	useInitialSpawns = true;
	UpdateSkinSetup( true );
}

/*
===============
idPlayer::ServerSpectate
================
*/
void idPlayer::ServerSpectate( bool spectate ) {
	assert( !gameLocal.isClient );

	if ( spectating != spectate ) {
		Spectate( spectate );
		if ( spectate ) {
			SetSpectateOrigin();
		} else {
			if ( gameLocal.gameType == GAME_DM ) {
				// make sure the scores are reset so you can't exploit by spectating and entering the game back
				// other game types don't matter, as you either can't join back, or it's team scores
				gameLocal.mpGame.ClearFrags( entityNumber );
			}
		}
	}
	if ( !spectate ) {
		SpawnFromSpawnSpot();
	}
}

/*
===========
idPlayer::SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
void idPlayer::SelectInitialSpawnPoint( idVec3 &origin, idAngles &angles ) {
	idEntity *spot;
	idStr skin;

	spot = gameLocal.SelectInitialSpawnPoint( this );

	// set the player skin from the spawn location
	if ( spot->spawnArgs.GetString( "skin", NULL, skin ) ) {
		spawnArgs.Set( "spawn_skin", skin );
	}

	UpdateCameraSettingsFromEntity( spot ); //ivan

	// activate the spawn locations targets
	spot->PostEventMS( &EV_ActivateTargets, 0, this );

	origin = spot->GetPhysics()->GetOrigin();
	origin[2] += 4.0f + CM_BOX_EPSILON;		// move up to make sure the player is at least an epsilon above the floor
	angles = spot->GetPhysics()->GetAxis().ToAngles();
}

/*
===========
idPlayer::SpawnFromSpawnSpot

Chooses a spawn location and spawns the player
============
*/
void idPlayer::SpawnFromSpawnSpot( void ) {
	idVec3		spawn_origin;
	idAngles	spawn_angles;

	SelectInitialSpawnPoint( spawn_origin, spawn_angles );
	SpawnToPoint( spawn_origin, spawn_angles );
}

//ivan start
/*
==============
idPlayer::Hq2QuickRespawn
==============
*/
void idPlayer::Hq2QuickRespawn( void ) {
	cameraSettings = lastCheckPoint.cameraSettings;
	SpawnToPoint( lastCheckPoint.spawnPos, ang_zero, true ); 
}
//ivan end

/*
===========
idPlayer::SpawnToPoint

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState

when called here with spectating set to true, just place yourself and init
============
*/
void idPlayer::SpawnToPoint( const idVec3 &spawn_origin, const idAngles &spawn_angles, bool quickRespawn ) { //ivan - quickRespawn added
	idVec3 spec_origin;

	assert( !gameLocal.isClient );

	respawning = true;

	Init( quickRespawn );

	fl.noknockback = false;

	// stop any ragdolls being used
	StopRagdoll();

	// set back the player physics
	SetPhysics( &physicsObj );

	physicsObj.SetClipModelAxis();
	physicsObj.EnableClip();

	if ( !spectating ) {
		SetCombatContents( true );
	}

	physicsObj.SetLinearVelocity( vec3_origin );

	// setup our initial view
	if ( !spectating ) {
		SetOrigin( spawn_origin );
		//ivan start - make sure they are ok at map start
		lastCheckPoint.spawnPos = spawn_origin;
#ifdef AUTOUPD_RESPAWN_POS
		tempRespawnPos = spawn_origin;
#endif
		lastCheckPoint.cameraSettings = cameraSettings;
		//ivan end
	} else {
		spec_origin = spawn_origin;
		spec_origin[ 2 ] += pm_normalheight.GetFloat();
		spec_origin[ 2 ] += SPECTATE_RAISE;
		SetOrigin( spec_origin );
	}

	// if this is the first spawn of the map, we don't have a usercmd yet,
	// so the delta angles won't be correct.  This will be fixed on the first think.
	viewAngles = ang_zero;
	SetDeltaViewAngles( ang_zero );
	SetViewAngles( spawn_angles );
	spawnAngles = spawn_angles;
	spawnAnglesSet = false;

	//ivan start
	//gameLocal.Printf("spawnAngles.yaw: %f \n", spawnAngles.yaw);
	if(spawnAngles.yaw > 0 && spawnAngles.yaw < 180){ //from 0 to 180 -> set 90 (left)
		viewPos = VIEWPOS_LEFT_MAX;
	}else{ //from -180 to 0 -> set -90 (right)
		viewPos = -VIEWPOS_LEFT_MAX;
	}
	//ivan end

	legsForward = true;
	legsYaw = 0.0f;
	idealLegsYaw = 0.0f;
	oldViewYaw = viewAngles.yaw;

	if ( spectating ) {
		Hide();
	} else {
		Show();
	}

	if ( gameLocal.isMultiplayer ) {
		if ( !spectating ) {
			// we may be called twice in a row in some situations. avoid a double fx and 'fly to the roof'
			if ( lastTeleFX < gameLocal.time - 1000 ) {
				idEntityFx::StartFx( spawnArgs.GetString( "fx_spawn" ), &spawn_origin, NULL, this, true );
				lastTeleFX = gameLocal.time;
			}
		}
		AI_TELEPORT = true;
	} else {
		//ivan start
		if( quickRespawn ){
			idEntityFx::StartFx( spawnArgs.GetString( "fx_spawn" ), &spawn_origin, NULL, this, false );
		}
		//ivan end
		AI_TELEPORT = false;
	}

	// kill anything at the new position
	if ( !spectating ) {
		physicsObj.SetClipMask( MASK_PLAYERSOLID ); // the clip mask is usually maintained in Move(), but KillBox requires it
		gameLocal.KillBox( this );
	}

	// don't allow full run speed for a bit
	physicsObj.SetKnockBack( 100 );

	// set our respawn time and buttons so that if we're killed we don't respawn immediately
	minRespawnTime = gameLocal.time;
	maxRespawnTime = gameLocal.time;
	if ( !spectating ) {
		forceRespawn = false;
	}

	privateCameraView = NULL;

	//ivan start

	//start locked at the same X pos.
	fastXpos = spawn_origin.x; 

	//update AI distances. Note: cameraSettings.distance has been set by the initial Spawn point or by Hq2QuickRespawn()
	gameLocal.UpdateSeeDistances( cameraSettings.distance ); 
	
	//ivan end

	BecomeActive( TH_THINK );

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	
	Think();

	respawning			= false;
	lastManOver			= false;
	lastManPlayAgain	= false;
	isTelefragged		= false;

#ifdef SHOW_MOVING_CROSSHAIR //un noted change from original sdk
	//reqDefaultCrossPos = true;
#endif
}

/*
===============
idPlayer::SavePersistantInfo

Saves any inventory and player stats when changing levels.
===============
*/
void idPlayer::SavePersistantInfo( void ) {
	idDict &playerInfo = gameLocal.persistentPlayerInfo[entityNumber];

	playerInfo.Clear();
	inventory.GetPersistantData( playerInfo );
	playerInfo.SetInt( "health", health );
	playerInfo.SetInt( "current_weapon", currentWeapon );

	//ivan start
	playerInfo.SetBool( "doubleJump", physicsObj.IsDoubleJumpEnabled() );
	playerInfo.SetBool( "wallJump", physicsObj.IsWallJumpEnabled() ); 
	playerInfo.SetInt( "numLives", numLives );
	playerInfo.SetInt( "score", score );
	//ivan end
}

/*
===============
idPlayer::RestorePersistantInfo

Restores any inventory and player stats when changing levels.
===============
*/
void idPlayer::RestorePersistantInfo( void ) {
	if ( gameLocal.isMultiplayer ) {
		gameLocal.persistentPlayerInfo[entityNumber].Clear();
	}

	spawnArgs.Copy( gameLocal.persistentPlayerInfo[entityNumber] );

	inventory.RestoreInventory( this, spawnArgs );

	health = spawnArgs.GetInt( "health", "100" );
	if ( !gameLocal.isClient ) {
		idealWeapon = spawnArgs.GetInt( "current_weapon", "1" );
	}

	//ivan start
	physicsObj.SetDoubleJumpEnabled( spawnArgs.GetBool( "doubleJump", "0" ) );
	physicsObj.SetWallJumpEnabled( spawnArgs.GetBool( "wallJump", "0" ) );
	numLives = spawnArgs.GetInt( "numLives", "0" );
	score = spawnArgs.GetInt( "score", "0" );
	//ivan end
}

/*
================
idPlayer::GetUserInfo
================
*/
idDict *idPlayer::GetUserInfo( void ) {
	return &gameLocal.userInfo[ entityNumber ];
}

/*
==============
idPlayer::UpdateSkinSetup
==============
*/
void idPlayer::UpdateSkinSetup( bool restart ) {
	if ( restart ) {
		team = ( idStr::Icmp( GetUserInfo()->GetString( "ui_team" ), "Blue" ) == 0 );
	}
	if ( gameLocal.gameType == GAME_TDM ) {
		if ( team ) {
			baseSkinName = "skins/characters/player/marine_mp_blue";
		} else {
			baseSkinName = "skins/characters/player/marine_mp_red";
		}
		if ( !gameLocal.isClient && team != latchedTeam ) {
			gameLocal.mpGame.SwitchToTeam( entityNumber, latchedTeam, team );
		}
		latchedTeam = team;
	} else {
		baseSkinName = GetUserInfo()->GetString( "ui_skin" );
	}
	if ( !baseSkinName.Length() ) {
		baseSkinName = "skins/characters/player/marine_mp";
	}
	skin = declManager->FindSkin( baseSkinName, false );
	assert( skin );
	// match the skin to a color band for scoreboard
	if ( baseSkinName.Find( "red" ) != -1 ) {
		colorBarIndex = 1;
	} else if ( baseSkinName.Find( "green" ) != -1 ) {
		colorBarIndex = 2;
	} else if ( baseSkinName.Find( "blue" ) != -1 ) {
		colorBarIndex = 3;
	} else if ( baseSkinName.Find( "yellow" ) != -1 ) {
		colorBarIndex = 4;
	} else {
		colorBarIndex = 0;
	}
	colorBar = colorBarTable[ colorBarIndex ];
	/*
	//ivan - disable berserk skin
	if ( PowerUpActive( BERSERK ) ) {
		powerUpSkin = declManager->FindSkin( baseSkinName + "_berserk" );
	}
	*/
}

/*
==============
idPlayer::BalanceTDM
==============
*/
bool idPlayer::BalanceTDM( void ) {
	int			i, balanceTeam, teamCount[2];
	idEntity	*ent;

	teamCount[ 0 ] = teamCount[ 1 ] = 0;
	for( i = 0; i < gameLocal.numClients; i++ ) {
		ent = gameLocal.entities[ i ];
		if ( ent && ent->IsType( idPlayer::Type ) ) {
			teamCount[ static_cast< idPlayer * >( ent )->team ]++;
		}
	}
	balanceTeam = -1;
	if ( teamCount[ 0 ] < teamCount[ 1 ] ) {
		balanceTeam = 0;
	} else if ( teamCount[ 0 ] > teamCount[ 1 ] ) {
		balanceTeam = 1;
	}
	if ( balanceTeam != -1 && team != balanceTeam ) {
		common->DPrintf( "team balance: forcing player %d to %s team\n", entityNumber, balanceTeam ? "blue" : "red" );
		team = balanceTeam;
		GetUserInfo()->Set( "ui_team", team ? "Blue" : "Red" );
		return true;
	}
	return false;
}

/*
==============
idPlayer::UserInfoChanged
==============
*/
bool idPlayer::UserInfoChanged( bool canModify ) {
	idDict	*userInfo;
	bool	modifiedInfo;
	bool	spec;
	bool	newready;

	userInfo = GetUserInfo();
	showWeaponViewModel = userInfo->GetBool( "ui_showGun" );

	if ( !gameLocal.isMultiplayer ) {
		return false;
	}

	modifiedInfo = false;

	spec = ( idStr::Icmp( userInfo->GetString( "ui_spectate" ), "Spectate" ) == 0 );
	if ( gameLocal.serverInfo.GetBool( "si_spectators" ) ) {
		// never let spectators go back to game while sudden death is on
		if ( canModify && gameLocal.mpGame.GetGameState() == idMultiplayerGame::SUDDENDEATH && !spec && wantSpectate == true ) {
			userInfo->Set( "ui_spectate", "Spectate" );
			modifiedInfo |= true;
		} else {
			if ( spec != wantSpectate && !spec ) {
				// returning from spectate, set forceRespawn so we don't get stuck in spectate forever
				forceRespawn = true;
			}
			wantSpectate = spec;
		}
	} else {
		if ( canModify && spec ) {
			userInfo->Set( "ui_spectate", "Play" );
			modifiedInfo |= true;
		} else if ( spectating ) {
			// allow player to leaving spectator mode if they were in it when si_spectators got turned off
			forceRespawn = true;
		}
		wantSpectate = false;
	}

	newready = ( idStr::Icmp( userInfo->GetString( "ui_ready" ), "Ready" ) == 0 );
	if ( ready != newready && gameLocal.mpGame.GetGameState() == idMultiplayerGame::WARMUP && !wantSpectate ) {
		gameLocal.mpGame.AddChatLine( common->GetLanguageDict()->GetString( "#str_07180" ), userInfo->GetString( "ui_name" ), newready ? common->GetLanguageDict()->GetString( "#str_04300" ) : common->GetLanguageDict()->GetString( "#str_04301" ) );
	}
	ready = newready;
	team = ( idStr::Icmp( userInfo->GetString( "ui_team" ), "Blue" ) == 0 );
	// server maintains TDM balance
	if ( canModify && gameLocal.gameType == GAME_TDM && !gameLocal.mpGame.IsInGame( entityNumber ) && g_balanceTDM.GetBool() ) {
		modifiedInfo |= BalanceTDM( );
	}
	UpdateSkinSetup( false );

	isChatting = userInfo->GetBool( "ui_chat", "0" );
	if ( canModify && isChatting && AI_DEAD ) {
		// if dead, always force chat icon off.
		isChatting = false;
		userInfo->SetBool( "ui_chat", false );
		modifiedInfo |= true;
	}

	return modifiedInfo;
}

/*
===============
idPlayer::UpdateHudAmmo
===============
*/
void idPlayer::UpdateHudAmmo( idUserInterface *_hud ) {
	int inclip;
	int ammoamount;
	int	maxAmmo; //ivan

	assert( weapon.GetEntity() );
	assert( _hud );

	inclip		= weapon.GetEntity()->AmmoInClip();
	ammoamount	= weapon.GetEntity()->AmmoAvailable();
	maxAmmo		= weapon.GetEntity()->GetMaxAmmo(); //ivan

	
//rev 2020 display all ammo types in hud start
	int ammo_exp;
	int ammo_souls;
	int ammo_cells;
	int ammo_bullets;
	int ammo_shells;
	ammo_exp = inventory.HasAmmo( 1, 1 );	//first number must match ammo.def value
	ammo_souls = inventory.HasAmmo( 2, 1 );
	ammo_cells = inventory.HasAmmo( 3, 1 );
	ammo_bullets = inventory.HasAmmo( 4, 1 );
	ammo_shells	= inventory.HasAmmo( 5, 1 );
	
	_hud->SetStateString( "player_ammo_bullets", va( "%i", ammo_bullets ) );
	_hud->SetStateString( "player_ammo_shells", va( "%i", ammo_shells ) ); 
	_hud->SetStateString( "player_ammo_cells", va( "%i", ammo_cells ) ); 
	_hud->SetStateString( "player_ammo_souls", va( "%i", ammo_souls ) ); 
	_hud->SetStateString( "player_ammo_exp", va( "%i", ammo_exp ) ); 		
//rev 2020 display all ammo types in hud end
	
	if ( ammoamount < 0 || !weapon.GetEntity()->IsReady() ) {
		// show infinite ammo
		_hud->SetStateString( "player_ammo", "" );
		_hud->SetStateString( "player_totalammo", "--" ); //ivan - "--" added
		_hud->SetStateString( "player_ammo_pct", "100" ); //ivan
	} else {
		// show remaining ammo				
		_hud->SetStateString( "player_totalammo", va( "%i", ammoamount ) ); //new
		//_hud->SetStateString( "player_totalammo", va( "%i", ammoamount - inclip ) ); 
		_hud->SetStateString( "player_ammo", weapon.GetEntity()->ClipSize() ? va( "%i", inclip ) : "--" );		// how much in the current clip
		_hud->SetStateString( "player_clips", weapon.GetEntity()->ClipSize() ? va( "%i", ammoamount / weapon.GetEntity()->ClipSize() ) : "--" );
		_hud->SetStateString( "player_allammo", va( "%i/%i", inclip, ammoamount ) );	//new
		//_hud->SetStateString( "player_allammo", va( "%i/%i", inclip, ammoamount - inclip ) );

		//ivan start
		if ( maxAmmo <= 0 ) {
			_hud->SetStateString( "player_ammo_pct", "0" ); //ivan
		}else{
			_hud->SetStateString( "player_ammo_pct", va( "%i", 100*ammoamount/maxAmmo ) );
		}
		//ivan end
	}
	_hud->SetStateBool( "player_ammo_empty", ( ammoamount == 0 ) );
	_hud->SetStateBool( "player_clip_empty", ( weapon.GetEntity()->ClipSize() ? inclip == 0 : false ) );
	_hud->SetStateBool( "player_clip_low", ( weapon.GetEntity()->ClipSize() ? inclip <= weapon.GetEntity()->LowAmmo() : false ) );
	//Let the HUD know the total amount of ammo regardless of the ammo required value //un noted change from original sdk
	_hud->SetStateString( "player_ammo_count", va("%i", weapon.GetEntity()->AmmoCount())); // new //un noted change from original sdk

	_hud->HandleNamedEvent( "updateAmmo" );
}

/*
===============
idPlayer::UpdateHudStats
===============
*/
void idPlayer::UpdateHudStats( idUserInterface *_hud ) {
	int staminapercentage;
	float max_stamina;

	assert( _hud );
	
	chargeAmount = spawnArgs.GetInt( "charge_amount" );	//rev 2020 charge  needed to check again to get the CURRENT amount
	max_stamina = pm_stamina.GetFloat();	
	if ( !max_stamina ) {
		// stamina disabled, so show full stamina bar
		staminapercentage = 100.0f;
	} else {
		staminapercentage = idMath::FtoiFast( 100.0f * stamina / max_stamina );
	}

	_hud->SetStateInt( "player_health", health );
	_hud->SetStateInt( "player_stamina", staminapercentage );
	_hud->SetStateInt( "player_charges", chargeAmount );	//rev 2020 charge
	_hud->SetStateInt( "player_armor", inventory.armor );
	_hud->SetStateInt( "player_hr", heartRate );
	_hud->SetStateInt( "player_nostamina", ( max_stamina == 0 ) ? 1 : 0 );
	 //ivan start
	_hud->SetStateInt( "player_lives", numLives );
	_hud->SetStateInt( "player_score", score );
	//ivan end

	_hud->HandleNamedEvent( "updateArmorHealthAir" );

	if ( healthPulse ) {
		_hud->HandleNamedEvent( "healthPulse" );
		StartSound( "snd_healthpulse", SND_CHANNEL_ITEM, 0, false, NULL );
		healthPulse = false;
	}

	if ( healthTake ) {
		_hud->HandleNamedEvent( "healthPulse" );
		StartSound( "snd_healthtake", SND_CHANNEL_ITEM, 0, false, NULL );
		healthTake = false;
	}

	if ( inventory.ammoPulse ) {
		_hud->HandleNamedEvent( "ammoPulse" );
		inventory.ammoPulse = false;
	}
	if ( inventory.weaponPulse ) {
		// We need to update the weapon hud manually, but not
		// the armor/ammo/health because they are updated every
		// frame no matter what
		UpdateHudWeapon();
		_hud->HandleNamedEvent( "weaponPulse" );
		inventory.weaponPulse = false;
	}
	if ( inventory.armorPulse ) {
		_hud->HandleNamedEvent( "armorPulse" );
		inventory.armorPulse = false;
	}

	UpdateHudAmmo( _hud );
}

/*
===============
idPlayer::UpdateHudWeapon
===============
*/
void idPlayer::UpdateHudWeapon( bool flashWeapon ) {
	idUserInterface *hud = idPlayer::hud;

	// if updating the hud of a followed client
	if ( gameLocal.localClientNum >= 0 && gameLocal.entities[ gameLocal.localClientNum ] && gameLocal.entities[ gameLocal.localClientNum ]->IsType( idPlayer::Type ) ) {
		idPlayer *p = static_cast< idPlayer * >( gameLocal.entities[ gameLocal.localClientNum ] );
		if ( p->spectating && p->spectator == entityNumber ) {
			assert( p->hud );
			hud = p->hud;
		}
	}

	if ( !hud ) {
		return;
	}

	for ( int i = 0; i < MAX_WEAPONS; i++ ) {
		const char *weapnum = va( "def_weapon%d", i );
		const char *hudWeap = va( "weapon%d", i );
		int weapstate = 0;
		if ( inventory.weapons & ( 1 << i ) ) {
			const char *weap = spawnArgs.GetString( weapnum );
			if ( weap && *weap ) {
				weapstate++;
			}
			if ( idealWeapon == i ) {
				weapstate++;
			}
		}
		hud->SetStateInt( hudWeap, weapstate );
	}
	if ( flashWeapon ) {
		hud->HandleNamedEvent( "weaponChange" );

		//ivan - upd short ammo name
		hud->SetStateString( "player_slotammotxt", spawnArgs.GetString( va( "weapon%d_slotammotxt", idealWeapon ), "Ammo:" ) );
	}

	//ivan start - see also idWeapon::UpdateGUI

	bool slotChanged = SetCurrentSlot( inventory.GetSlotByWeap( idealWeapon ) );

	for ( int i = 0; i < NUM_SLOTS; i++ ) {
		const char *slotnum = va( "weapon_in_slot%d", i );
		hud->SetStateInt( slotnum, inventory.weaponSlot[ i ] );
	}

	hud->SetStateInt( "currentSlot", currentSlot );
	hud->SetStateInt( "currentWeapon", idealWeapon ); //show ideal as current

	if( slotChanged ){
		if( currentSlot != -1 ){
			hud->HandleNamedEvent( va( "selectSlot%d", currentSlot ) );
		}else{
			hud->HandleNamedEvent( "selectExtraSlot" );
		}
	}

	hud->HandleNamedEvent( "updSlotIcons" );
	//ivan end
}

/*
===============
idPlayer::DrawHUD
===============
*/
void idPlayer::DrawHUD( idUserInterface *_hud ) {

	if ( !weapon.GetEntity() || influenceActive != INFLUENCE_NONE || privateCameraView || gameLocal.GetCamera() || !_hud || !g_showHud.GetBool() ) {
		return;
	}

	UpdateHudStats( _hud );

	//_hud->SetStateString( "weapicon", weapon.GetEntity()->Icon() ); //ivan - this is not used

	// FIXME: this is temp to allow the sound meter to show up in the hud
	// it should be commented out before shipping but the code can remain
	// for mod developers to enable for the same functionality
	_hud->SetStateInt( "s_debug", cvarSystem->GetCVarInteger( "s_showLevelMeter" ) );

	weapon.GetEntity()->UpdateGUI();

	_hud->Redraw( gameLocal.realClientTime );

	// weapon targeting crosshair
	if ( g_mouselook.GetBool() ){ //was: if ( !GuiActive() ) { //un noted change from original sdk
		if ( cursor && weapon.GetEntity()->ShowCrosshair() ) {
			cursor->Redraw( gameLocal.realClientTime );
		}
	}
}

/*
===============
idPlayer::EnterCinematic
===============
*/
void idPlayer::EnterCinematic( void ) {
	Hide();
	StopAudioLog();
	StopSound( SND_CHANNEL_PDA, false );
	if ( hud ) {
		hud->HandleNamedEvent( "radioChatterDown" );
	}

	physicsObj.SetLinearVelocity( vec3_origin );

	SetState( "EnterCinematic" );
	UpdateScript();

	if ( weaponEnabled && weapon.GetEntity() ) {
		weapon.GetEntity()->EnterCinematic();
	}

	AI_FORWARD		= false;
	AI_BACKWARD		= false;
	AI_STRAFE_LEFT	= false;
	AI_STRAFE_RIGHT	= false;
	AI_RUN			= false;
	AI_ATTACK_HELD	= false;
	AI_WEAPON_FIRED	= false;
	AI_JUMP			= false;
	AI_CROUCH		= false;
	AI_ONGROUND		= true;
	AI_ONLADDER		= false;
	AI_DEAD			= ( health <= 0 );
	AI_RUN			= false;
	AI_PAIN			= false;
	AI_HARDLANDING	= false;
	AI_SOFTLANDING	= false;
	AI_RELOAD		= false;
	AI_TELEPORT		= false;
	AI_TURN_LEFT	= false;
	AI_TURN_RIGHT	= false;
}

/*
===============
idPlayer::ExitCinematic
===============
*/
void idPlayer::ExitCinematic( void ) {
	Show();

	if ( weaponEnabled && weapon.GetEntity() ) {
		weapon.GetEntity()->ExitCinematic();
	}

	SetState( "ExitCinematic" );
	UpdateScript();
}

/*
=====================
idPlayer::UpdateConditions
=====================
*/
void idPlayer::UpdateConditions( void ) {
	idVec3	velocity;
	float	forwardspeed;
	float	sidespeed;

	// minus the push velocity to avoid playing the walking animation and sounds when riding a mover
	velocity = physicsObj.GetLinearVelocity() - physicsObj.GetPushedLinearVelocity();

	if ( influenceActive ) {
		AI_FORWARD		= false;
		AI_BACKWARD		= false;
		AI_STRAFE_LEFT	= false;
		AI_STRAFE_RIGHT	= false;
	} else if ( gameLocal.time - lastDmgTime < 500 ) {
		forwardspeed = velocity * viewAxis[ 0 ];
		sidespeed = velocity * viewAxis[ 1 ];
		AI_FORWARD		= AI_ONGROUND && ( forwardspeed > 20.01f );
		AI_BACKWARD		= AI_ONGROUND && ( forwardspeed < -20.01f );
		AI_STRAFE_LEFT	= AI_ONGROUND && ( sidespeed > 20.01f );
		AI_STRAFE_RIGHT	= AI_ONGROUND && ( sidespeed < -20.01f );
	} else if ( xyspeed > MIN_BOB_SPEED ) {
		AI_FORWARD		= AI_ONGROUND && ( usercmd.forwardmove > 0 );
		AI_BACKWARD		= AI_ONGROUND && ( usercmd.forwardmove < 0 );
		AI_STRAFE_LEFT	= AI_ONGROUND && ( usercmd.rightmove < 0 );
		AI_STRAFE_RIGHT	= AI_ONGROUND && ( usercmd.rightmove > 0 );
	} else {
		AI_FORWARD		= false;
		AI_BACKWARD		= false;
		AI_STRAFE_LEFT	= false;
		AI_STRAFE_RIGHT	= false;
	}

	/* ivan - commented out
	AI_RUN			= ( usercmd.buttons & BUTTON_RUN ) && ( ( !pm_stamina.GetFloat() ) || ( stamina > pm_staminathreshold.GetFloat() ) );
	*/
	AI_DEAD			= ( health <= 0 );
}

/*
==================
WeaponFireFeedback

Called when a weapon fires, generates head twitches, etc
==================
*/
void idPlayer::WeaponFireFeedback( const idDict *weaponDef ) {
	// force a blink
	blink_time = 0;

	// play the fire animation
	AI_WEAPON_FIRED = true;

	// update view feedback
	playerView.WeaponFireFeedback( weaponDef );
}

/*
===============
idPlayer::StopFiring
===============
*/
void idPlayer::StopFiring( void ) {
	AI_ATTACK_HELD	= false;
	AI_WEAPON_FIRED = false;
	AI_RELOAD		= false;
	if ( weapon.GetEntity() ) {
		weapon.GetEntity()->EndAttack();
	}
}

/*
===============
idPlayer::FireWeapon
===============
*/
void idPlayer::FireWeapon( void ) {
	idMat3 axis;
	idVec3 muzzle;

	if ( privateCameraView ) {
		return;
	}

	if ( g_editEntityMode.GetInteger() ) {
		GetViewPos( muzzle, axis );
		if ( gameLocal.editEntities->SelectEntity( muzzle, axis[0], this ) ) {
			return;
		}
	}

	if ( !hiddenWeapon && weapon.GetEntity()->IsReady() ) {

#ifdef _DENTONMOD
		if ( ( weapon_soulcube >= 0 ) && ( currentWeapon == weapon_soulcube ) && currentWeapon == idealWeapon ) {
			if ( hud ) {
				hud->HandleNamedEvent( "soulCubeNotReady" );
			}
			quickWeapon = weapon_soulcube; 
			SelectWeapon( previousWeapon, false );
		}
		if ( weapon.GetEntity()->AmmoInClip() || weapon.GetEntity()->AmmoAvailable() ) {
			AI_ATTACK_HELD = true;
			weapon.GetEntity()->BeginAttack();
#else 
		if ( weapon.GetEntity()->AmmoInClip() || weapon.GetEntity()->AmmoAvailable() ) {
			AI_ATTACK_HELD = true;
			weapon.GetEntity()->BeginAttack();

			if ( ( weapon_soulcube >= 0 ) && ( currentWeapon == weapon_soulcube ) ) {
				if ( hud ) {
					hud->HandleNamedEvent( "soulCubeNotReady" );
				}
				SelectWeapon( previousWeapon, false );
			}
#endif //un noted change from original sdk
		} else {
			NextBestWeapon();
		}

	}

	if ( hud ) {
		/* ivan
		if ( tipUp ) {
			HideTip();
		}
		*/
		// may want to track with with a bool as well
		// keep from looking up named events so often
		if ( objectiveUp ) {
			HideObjective();
		}
	}
}

/*
===============
idPlayer::CacheWeapons
===============
*/
void idPlayer::CacheWeapons( void ) {
	idStr	weap;
	int		w;

	// check if we have any weapons
	if ( !inventory.weapons ) {
		return;
	}

	for( w = 0; w < MAX_WEAPONS; w++ ) {
		if ( inventory.weapons & ( 1 << w ) ) {
			weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
			if ( weap != "" ) {
				idWeapon::CacheWeapon( weap );
			} else {
				inventory.weapons &= ~( 1 << w );
			}
		}
	}
}

/*
===============
idPlayer::Give
===============
*/
bool idPlayer::Give( const char *statname, const char *value ) {
	int amount;

	if ( AI_DEAD ) {
		return false;
	}

	if ( !idStr::Icmp( statname, "health" ) ) {
		if ( health >= inventory.maxHealth ) {
			return false;
		}
		amount = atoi( value );
		if ( amount ) {
			health += amount;
			if ( health > inventory.maxHealth ) {
				health = inventory.maxHealth;
			}
			if ( hud ) {
				hud->HandleNamedEvent( "healthPulse" );
			}
		}

	} else if ( !idStr::Icmp( statname, "stamina" ) ) {
		if ( stamina >= 100 ) {
			return false;
		}
		stamina += atof( value );
		if ( stamina > 100 ) {
			stamina = 100;
		}

	} else if ( !idStr::Icmp( statname, "heartRate" ) ) {
		heartRate += atoi( value );
		if ( heartRate > MAX_HEARTRATE ) {
			heartRate = MAX_HEARTRATE;
		}

	} else if ( !idStr::Icmp( statname, "air" ) ) {
		if ( airTics >= pm_airTics.GetInteger() ) {
			return false;
		}
		airTics += atoi( value ) / 100.0 * pm_airTics.GetInteger();
		if ( airTics > pm_airTics.GetInteger() ) {
			airTics = pm_airTics.GetInteger();
		}
	} 
	
	//ivan start
	else if ( !idStr::Icmp( statname, "extraLives" )){
		amount = atoi( value );
		if ( amount > 0 ) {
			AddLifes( amount );
		}else{
			return false;
		}
	}
	
	//was: else {
	else if ( idStr::Icmp( statname, "takeme" )){ //if equals "takeme" this condition if false and true will be returned
		//ivan end
		return inventory.Give( this, spawnArgs, statname, value, &idealWeapon, true );
	}
	return true;
}

/*
===============
idPlayer::GiveHealthPool

adds health to the player health pool
===============
*/
void idPlayer::GiveHealthPool( float amt ) {

	if ( AI_DEAD ) {
		return;
	}

	if ( health > 0 ) {
		healthPool += amt;
		if ( healthPool > inventory.maxHealth - health ) {
			healthPool = inventory.maxHealth - health;
		}
		nextHealthPulse = gameLocal.time;
	}
}

//ivan start 
/*
bool idPlayer::CanPickupWeapons( void ){
	return ( gameLocal.time > inventory.nextWeaponPickup );
}
*/

void idPlayer::AddWeaponToSlots( idStr weaponName, bool select ){
	int	i;

	// find the number of the matching weapon name
	for( i = 0; i < MAX_WEAPONS; i++ ) {
		if ( weaponName == spawnArgs.GetString( va( "def_weapon%d", i ) ) ) {
			break;
		}
	}
	if( i == MAX_WEAPONS ){ 
		gameLocal.Warning("Weapon not found!");
		return;
	}

	AddWeaponToSlots( i, select );
}

void idPlayer::AddWeaponToSlots( int weaponNum, bool select ){
	int slot = -1;
	const char* weap;

	//gameLocal.Printf("Try to add weapon %d\n", weaponNum);

	if( weaponNum >= MAX_WEAPONS || weaponNum < 0 ){ 
		gameLocal.Warning("Weapon out of range!");
		return;
	}

	//weapon num valid?
	weap = spawnArgs.GetString( va( "def_weapon%d", weaponNum ) );
	if ( !weap[ 0 ] ) {
		gameLocal.Warning("def_weapon%d empty!", weaponNum );
		return;
	}

	//understand what to do
	if( !spawnArgs.GetBool( va( "weapon%d_cycle", weaponNum ) ) ){
		//gameLocal.Printf("weapon %d is not in cycle\n", weaponNum);
		return;
	}		
	//already in a slot?
	slot = inventory.GetSlotByWeap( weaponNum );
	if( slot != -1 ){ 
		//gameLocal.Printf("weapon %d is already assigned to slot: %d\n", weaponNum, slot);
		//do nothing - as asual
		//TODO: select it?
		if( select ){
			SelectWeapon( inventory.weaponSlot[slot], false );
		}
		return;
	}
	
	//free slot?
	slot = inventory.FindFreeSlot();
	if( slot == -1 ){ //if no free slots, use the current one
		//gameLocal.Printf("there are no free slots, use the current one\n");

		if(currentSlot == -1){ //the current weapon shouldn't be dropped
			//gameLocal.Printf("the current weapon shouldn't be dropped, drop the one in slot 0\n");
			//make sure we drop a weapon in cycle
			DropNotSelectedWeapon( inventory.weaponSlot[0] );
		}else{
			//gameLocal.Printf("drop current weapon\n");
			if( !DropWeapon( false, false ) ){ //parm2: if selectNext is false, DropWeapon will not select another weapon
				//weapon was not dropped, so we'll try to drop it later.
				PostEventMS( &EV_Player_DropWeapon, 100, inventory.weaponSlot[currentSlot] );
			}
		}
		slot = (currentSlot != -1) ? currentSlot : 0;
	}

	inventory.AssignWeapToSlot( weaponNum, slot );
	//gameLocal.Printf("weapon %d is has been assigned to slot: %d\n", weaponNum, slot);
	if(select){
		SelectWeapon( weaponNum, false );
	}
}

//returns true if slot changed
bool idPlayer::SetCurrentSlot( int newslot ){
	if( newslot != currentSlot ){
		currentSlot = newslot;
		//gameLocal.Printf("SetCurrentSlot %d\n", currentSlot );
		//GUI EVENT is raised in ::UpdateHudWeapon
		return true;
	}else{
		return false;
	}
}

//get the weapon number associated to this item. -1 if not a weapon or invalid weapon.
int idPlayer::GetWeaponNumByItem( idItem *item ){
	const idKeyValue	*arg;
	arg = item->spawnArgs.MatchPrefix( "inv_weapon", NULL );
	if ( arg ) { //it's a weapon
		int	weaponNum;
		idStr weaponName;
		weaponName = arg->GetValue(); //weapon name

		// find the number of the matching weapon name
		for( weaponNum = 0; weaponNum < MAX_WEAPONS; weaponNum++ ) {
			if ( weaponName == spawnArgs.GetString( va( "def_weapon%d", weaponNum ) ) ) {
				return  weaponNum;
			}
		}
	} //end if

	return -1;
}

bool idPlayer::WeaponItemCanGiveAmmo( int weaponNum ){
	if( weaponNum == -1 ){
		return false;
	}
	return ( inventory.GetSlotByWeap( weaponNum ) != -1 ); //ok only if it's in a slot
}

/*
//bypass the interaction if we already have the weapon: give ammo 
bool idPlayer::WeaponItemCanGiveAmmo( idItem *item ){
	const idKeyValue	*arg;
	arg = item->spawnArgs.MatchPrefix( "inv_weapon", NULL );
	if ( arg ) { //it's a weapon
		int	weaponNum;
		idStr weaponName;
		weaponName = arg->GetValue(); //weapon name

		// find the number of the matching weapon name
		for( weaponNum = 0; weaponNum < MAX_WEAPONS; weaponNum++ ) {
			if ( weaponName == spawnArgs.GetString( va( "def_weapon%d", weaponNum ) ) ) {
				return ( inventory.GetSlotByWeap( weaponNum ) != -1 ); //ok only if it's in a slot
			}
		}
	} //end if

	return false;
}
*/


//ivan end

/*
===============
idPlayer::GiveItem

Returns false if the item shouldn't be picked up
===============
*/
bool idPlayer::GiveItem( idItem *item ) { 
	int					i;
	const idKeyValue	*arg;
	idDict				attr;
	bool				gave;
	int					numPickup;

	if ( gameLocal.isMultiplayer && spectating ) {
		return false;
	}

	//ivan start
	arg = item->spawnArgs.MatchPrefix( "inv_weapon", NULL );
	if ( arg ) { //it's a weapon
		if( gameLocal.time < inventory.nextWeaponPickup ){ return false; }
	}
	//ivan end

	item->GetAttributes( attr );

	gave = false;
	numPickup = inventory.pickupItemNames.Num();
	for( i = 0; i < attr.GetNumKeyVals(); i++ ) {
		arg = attr.GetKeyVal( i );
		if ( Give( arg->GetKey(), arg->GetValue() ) ) {
			gave = true;
		}
	}

	arg = item->spawnArgs.MatchPrefix( "inv_weapon", NULL );
	
	//ivan start - weapon taken
	if ( gave && arg ) { 
		AddWeaponToSlots( arg->GetValue(), inventory.weaponPulse ); //was true. //select only if new weapon taken
		inventory.nextWeaponPickup = gameLocal.time + 500;
	}
	//ivan end

	if ( arg && hud ) {
		//ivan start - weapon taken
		AddWeaponToSlots( arg->GetValue(), inventory.weaponPulse ); //was true. //select only if new weapon taken
		inventory.nextWeaponPickup = gameLocal.time + 500;
		//ivan end

		// We need to update the weapon hud manually, but not
		// the armor/ammo/health because they are updated every
		// frame no matter what
		UpdateHudWeapon( false );
		hud->HandleNamedEvent( "weaponPulse" );
	}

	// display the pickup feedback on the hud
	if ( gave ) {
		
		//ivan start - score on items
		AddScore( item->spawnArgs.GetInt( "score", "0" ) ); 
		//ivan end

		if( numPickup == inventory.pickupItemNames.Num() ){
			//		inventory.AddPickupName( item->spawnArgs.GetString( "inv_name" ), item->spawnArgs.GetString( "inv_icon" ) );
			inventory.AddPickupName( item->spawnArgs.GetString( "inv_name" ), item->spawnArgs.GetString( "inv_icon" ), this ); //New _D3XP
		}
	}

	return gave;
}

/*
===============
idPlayer::PowerUpModifier
===============
*/
float idPlayer::PowerUpModifier( int type ) {
	float mod = 1.0f;

	if ( PowerUpActive( BERSERK ) ) {
		switch( type ) {
			case SPEED: {
				mod *= 1.7f;
				break;
						}
			case PROJECTILE_DAMAGE: {
				mod *= 2.0f;
				break;
									}
			case MELEE_DAMAGE: {
				mod *= 30.0f;
				break;
							   }
			case MELEE_DISTANCE: {
				mod *= 2.0f;
				break;
								 }
		}
	}

	if ( gameLocal.isMultiplayer && !gameLocal.isClient ) {
		if ( PowerUpActive( MEGAHEALTH ) ) {
			if ( healthPool <= 0 ) {
				GiveHealthPool( 100 );
			}
		} else {
			healthPool = 0;
		}
	}

	return mod;
}

/*
===============
idPlayer::PowerUpActive
===============
*/
bool idPlayer::PowerUpActive( int powerup ) const {
	return ( inventory.powerups & ( 1 << powerup ) ) != 0;
}

/*
===============
idPlayer::GivePowerUp
===============
*/
bool idPlayer::GivePowerUp( int powerup, int time ) {
	const char *sound;
	const char *skin;

	if ( powerup >= 0 && powerup < MAX_POWERUPS ) {

		if ( gameLocal.isServer ) {
			idBitMsg	msg;
			byte		msgBuf[MAX_EVENT_PARAM_SIZE];

			msg.Init( msgBuf, sizeof( msgBuf ) );
			msg.WriteShort( powerup );
			msg.WriteBits( 1, 1 );
			ServerSendEvent( EVENT_POWERUP, &msg, false, -1 );
		}

		if ( powerup != MEGAHEALTH ) {
			inventory.GivePowerUp( this, powerup, time );
		}

		const idDeclEntityDef *def = NULL;

		switch( powerup ) {
			case BERSERK: {
				if ( spawnArgs.GetString( "snd_berserk_third", "", &sound ) ) {
					StartSoundShader( declManager->FindSound( sound ), SND_CHANNEL_DEMONIC, 0, false, NULL );
				}
				/*
				if ( baseSkinName.Length() ) {
					powerUpSkin = declManager->FindSkin( baseSkinName + "_berserk" );
				}
				
				//ivan: disable weapon change
				if ( !gameLocal.isClient ) {
					idealWeapon = 0;
				}
				*/
				break;
						  }
			case INVISIBILITY: {
				spawnArgs.GetString( "skin_invisibility", "", &skin );
				powerUpSkin = declManager->FindSkin( skin );
				// remove any decals from the model
				if ( modelDefHandle != -1 ) {
					gameRenderWorld->RemoveDecals( modelDefHandle );
				}
				if ( weapon.GetEntity() ) {
					weapon.GetEntity()->UpdateSkin();
				}
				if ( spawnArgs.GetString( "snd_invisibility", "", &sound ) ) {
					StartSoundShader( declManager->FindSound( sound ), SND_CHANNEL_ANY, 0, false, NULL );
				}
				break;
							   }
			case ADRENALINE: {
//rev 2020 give the player full charge attack amount when picking up adrenaline instead of stamina
				//stamina = 100.0f;	
				def = gameLocal.FindEntityDef( "powerup_adrenaline", false );
				if ( def ) {
					spawnArgs.Set( "charge_amount", "2" );
				}
//rev 2020 end
				break;
							 }
			case MEGAHEALTH: {
				if ( spawnArgs.GetString( "snd_megahealth", "", &sound ) ) {
					StartSoundShader( declManager->FindSound( sound ), SND_CHANNEL_ANY, 0, false, NULL );
				}
				def = gameLocal.FindEntityDef( "powerup_megahealth", false );
				if ( def ) {
					health = def->dict.GetInt( "inv_health" );
				}
				break;
							 }
		}

		if ( hud ) {
			hud->HandleNamedEvent( "itemPickup" );
		}

		return true;
	} else {
		gameLocal.Warning( "Player given power up %i\n which is out of range", powerup );
	}
	return false;
}

/*
==============
idPlayer::ClearPowerup
==============
*/
void idPlayer::ClearPowerup( int i ) {

	if ( gameLocal.isServer ) {
		idBitMsg	msg;
		byte		msgBuf[MAX_EVENT_PARAM_SIZE];

		msg.Init( msgBuf, sizeof( msgBuf ) );
		msg.WriteShort( i );
		msg.WriteBits( 0, 1 );
		ServerSendEvent( EVENT_POWERUP, &msg, false, -1 );
	}

	powerUpSkin = NULL;
	inventory.powerups &= ~( 1 << i );
	inventory.powerupEndTime[ i ] = 0;
	switch( i ) {
		case BERSERK: {
			StopSound( SND_CHANNEL_DEMONIC, false );
			break;
					  }
		case INVISIBILITY: {
			if ( weapon.GetEntity() ) {
				weapon.GetEntity()->UpdateSkin();
			}
			break;
						   }
	}
}

/*
==============
idPlayer::UpdatePowerUps
==============
*/
void idPlayer::UpdatePowerUps( void ) {
	int i;

	if ( !gameLocal.isClient ) {
		for ( i = 0; i < MAX_POWERUPS; i++ ) {
			if ( PowerUpActive( i ) && inventory.powerupEndTime[i] <= gameLocal.time ) {
				ClearPowerup( i );
			}
		}
	}

	if ( health > 0 ) {
		if ( powerUpSkin ) {
			renderEntity.customSkin = powerUpSkin;
		} else {
			renderEntity.customSkin = skin;
		}
	}

	if ( healthPool && gameLocal.time > nextHealthPulse && !AI_DEAD && health > 0 ) {
		assert( !gameLocal.isClient );	// healthPool never be set on client
		int amt = ( healthPool > 5 ) ? 5 : healthPool;
		health += amt;
		if ( health > inventory.maxHealth ) {
			health = inventory.maxHealth;
			healthPool = 0;
		} else {
			healthPool -= amt;
		}
		nextHealthPulse = gameLocal.time + HEALTHPULSE_TIME;
		healthPulse = true;
	}

	if ( !gameLocal.inCinematic && influenceActive == 0 && g_skill.GetInteger() == 3 && gameLocal.time > nextHealthTake && !AI_DEAD && health > g_healthTakeLimit.GetInteger() ) {
		assert( !gameLocal.isClient );	// healthPool never be set on client
		health -= g_healthTakeAmt.GetInteger();
		if ( health < g_healthTakeLimit.GetInteger() ) {
			health = g_healthTakeLimit.GetInteger();
		}
		nextHealthTake = gameLocal.time + g_healthTakeTime.GetInteger() * 1000;
		healthTake = true;
	}
}

/*
===============
idPlayer::ClearPowerUps
===============
*/
void idPlayer::ClearPowerUps( void ) {
	int i;
	for ( i = 0; i < MAX_POWERUPS; i++ ) {
		if ( PowerUpActive( i ) ) {
			ClearPowerup( i );
		}
	}
	inventory.ClearPowerUps();
}

/*
===============
idPlayer::GiveInventoryItem
===============
*/
bool idPlayer::GiveInventoryItem( idDict *item ) {
	if ( gameLocal.isMultiplayer && spectating ) {
		return false;
	}
	inventory.items.Append( new idDict( *item ) );
	idItemInfo info;
	const char* itemName = item->GetString( "inv_name" );
	if ( idStr::Cmpn( itemName, STRTABLE_ID, STRTABLE_ID_LENGTH ) == 0 ) {
		info.name = common->GetLanguageDict()->GetString( itemName );
	} else {
		info.name = itemName;
	}
	info.icon = item->GetString( "inv_icon" );
	inventory.pickupItemNames.Append( info );
	if ( hud ) {
		hud->SetStateString( "itemicon", info.icon );
		hud->HandleNamedEvent( "invPickup" );
	}
	return true;
}

/*
==============
idPlayer::UpdateObjectiveInfo
==============
*/
void idPlayer::UpdateObjectiveInfo( void ) {
	if ( objectiveSystem == NULL ) {
		return;
	}
	objectiveSystem->SetStateString( "objective1", "" );
	objectiveSystem->SetStateString( "objective2", "" );
	objectiveSystem->SetStateString( "objective3", "" );
	for ( int i = 0; i < inventory.objectiveNames.Num(); i++ ) {
		objectiveSystem->SetStateString( va( "objective%i", i+1 ), "1" );
		objectiveSystem->SetStateString( va( "objectivetitle%i", i+1 ), inventory.objectiveNames[i].title.c_str() );
		objectiveSystem->SetStateString( va( "objectivetext%i", i+1 ), inventory.objectiveNames[i].text.c_str() );
		objectiveSystem->SetStateString( va( "objectiveshot%i", i+1 ), inventory.objectiveNames[i].screenshot.c_str() );
	}
	objectiveSystem->StateChanged( gameLocal.time );
}

/*
===============
idPlayer::GiveObjective
===============
*/
void idPlayer::GiveObjective( const char *title, const char *text, const char *screenshot ) {
	idObjectiveInfo info;
	info.title = title;
	info.text = text;
	info.screenshot = screenshot;
	inventory.objectiveNames.Append( info );
	ShowObjective( "newObjective" );
	if ( hud ) {
		hud->HandleNamedEvent( "newObjective" );
	}
}

/*
===============
idPlayer::CompleteObjective
===============
*/
void idPlayer::CompleteObjective( const char *title ) {
	int c = inventory.objectiveNames.Num();
	for ( int i = 0;  i < c; i++ ) {
		if ( idStr::Icmp(inventory.objectiveNames[i].title, title) == 0 ) {
			inventory.objectiveNames.RemoveIndex( i );
			break;
		}
	}
	ShowObjective( "newObjectiveComplete" );

	if ( hud ) {
		hud->HandleNamedEvent( "newObjectiveComplete" );
	}

	//AddScore( OBJCOMPL_SCORE ); //un noted change from original sdk
}

/*
===============
idPlayer::GiveVideo
===============
*/
void idPlayer::GiveVideo( const char *videoName, idDict *item ) {

	if ( videoName == NULL || *videoName == 0 ) {
		return;
	}

	inventory.videos.AddUnique( videoName );

	if ( item ) {
		idItemInfo info;
		info.name = item->GetString( "inv_name" );
		info.icon = item->GetString( "inv_icon" );
		inventory.pickupItemNames.Append( info );
	}
	if ( hud ) {
		hud->HandleNamedEvent( "videoPickup" );
	}
}

/*
===============
idPlayer::GiveSecurity
===============
*/
void idPlayer::GiveSecurity( const char *security ) {
	GetPDA()->SetSecurity( security );
	if ( hud ) {
		hud->SetStateString( "pda_security", "1" );
		hud->HandleNamedEvent( "securityPickup" );
	}
}

/*
===============
idPlayer::GiveEmail
===============
*/
void idPlayer::GiveEmail( const char *emailName ) {

	if ( emailName == NULL || *emailName == 0 ) {
		return;
	}

	inventory.emails.AddUnique( emailName );
	GetPDA()->AddEmail( emailName );

	if ( hud ) {
		hud->HandleNamedEvent( "emailPickup" );
	}
}

/*
===============
idPlayer::GivePDA
===============
*/
void idPlayer::GivePDA( const char *pdaName, idDict *item )
{
	if ( gameLocal.isMultiplayer && spectating ) {
		return;
	}

	if ( item ) {
		inventory.pdaSecurity.AddUnique( item->GetString( "inv_name" ) );
	}

	if ( pdaName == NULL || *pdaName == 0 ) {
		pdaName = "personal";
	}

	const idDeclPDA *pda = static_cast< const idDeclPDA* >( declManager->FindType( DECL_PDA, pdaName ) );

	inventory.pdas.AddUnique( pdaName );

	// Copy any videos over
	for ( int i = 0; i < pda->GetNumVideos(); i++ ) {
		const idDeclVideo *video = pda->GetVideoByIndex( i );
		if ( video ) {
			inventory.videos.AddUnique( video->GetName() );
		}
	}

	// This is kind of a hack, but it works nicely
	// We don't want to display the 'you got a new pda' message during a map load
	if ( gameLocal.GetFrameNum() > 10 ) {
		if ( pda && hud ) {
			idStr pdaName = pda->GetPdaName();
			pdaName.RemoveColors();
			hud->SetStateString( "pda", "1" );
			hud->SetStateString( "pda_text", pdaName );
			const char *sec = pda->GetSecurity();
			hud->SetStateString( "pda_security", ( sec && *sec ) ? "1" : "0" );
			hud->HandleNamedEvent( "pdaPickup" );
		}

		if ( inventory.pdas.Num() == 1 ) {
			GetPDA()->RemoveAddedEmailsAndVideos();
			if ( !objectiveSystemOpen ) {
				TogglePDA();
			}
			objectiveSystem->HandleNamedEvent( "showPDATip" );
			//ShowTip( spawnArgs.GetString( "text_infoTitle" ), spawnArgs.GetString( "text_firstPDA" ), true );
		}

		if ( inventory.pdas.Num() > 1 && pda->GetNumVideos() > 0 && hud ) {
			hud->HandleNamedEvent( "videoPickup" );
		}
	}
}

/*
===============
idPlayer::FindInventoryItem
===============
*/
idDict *idPlayer::FindInventoryItem( const char *name ) {
	for ( int i = 0; i < inventory.items.Num(); i++ ) {
		const char *iname = inventory.items[i]->GetString( "inv_name" );
		if ( iname && *iname ) {
			if ( idStr::Icmp( name, iname ) == 0 ) {
				return inventory.items[i];
			}
		}
	}
	return NULL;
}

/*
===============
idPlayer::RemoveInventoryItem
===============
*/
void idPlayer::RemoveInventoryItem( const char *name ) {
	idDict *item = FindInventoryItem(name);
	if ( item ) {
		RemoveInventoryItem( item );
	}
}

/*
===============
idPlayer::RemoveInventoryItem
===============
*/
void idPlayer::RemoveInventoryItem( idDict *item ) {
	inventory.items.Remove( item );
	delete item;
}

/*
===============
idPlayer::GiveItem
===============
*/
void idPlayer::GiveItem( const char *itemname ) {
	idDict args;

	args.Set( "classname", itemname );
	args.Set( "owner", name.c_str() );
	gameLocal.SpawnEntityDef( args );
	if ( hud ) {
		hud->HandleNamedEvent( "itemPickup" );
	}
}

/*
==================
idPlayer::SlotForWeapon
==================
*/
int idPlayer::SlotForWeapon( const char *weaponName ) { //ivan - note: this returns the number of the weapon, not one of our new slots.
	int i;

	for( i = 0; i < MAX_WEAPONS; i++ ) {
		const char *weap = spawnArgs.GetString( va( "def_weapon%d", i ) );
		if ( !idStr::Cmp( weap, weaponName ) ) {
			return i;
		}
	}

	// not found
	return -1;
}

/*
===============
idPlayer::Reload
===============
*/
void idPlayer::Reload( void ) {
	if ( gameLocal.isClient ) {
		return;
	}

	if ( spectating || gameLocal.inCinematic || influenceActive ) {
		return;
	}

	if ( weapon.GetEntity() && weapon.GetEntity()->IsLinked() ) {
		weapon.GetEntity()->Reload();
	}
}

/*
===============
idPlayer::WeaponSpecialFunction 

Weapon special function- Added by Clone JC Denton
===============
*/
void idPlayer::WeaponSpecialFunction( bool keyTapped ) {

	if ( gameLocal.isClient ) {
		return;
	}

	if ( spectating || gameLocal.inCinematic || influenceActive ) {
		return;
	}

	if ( !hiddenWeapon && weapon.GetEntity() && weapon.GetEntity()->IsLinked() ) {
		weapon.GetEntity()->BeginSpecialFunction( keyTapped );
	}
}

/*
===============
idPlayer::NextBestWeapon
===============
*/
void idPlayer::NextBestWeapon( void ) {
	const char *weap;

	//ivan start
	int w; //was: int w = MAX_WEAPONS;
	int s;	
	bool foundInSlots;
	//ivan end

#ifdef _DENTONMOD
	if ( gameLocal.isClient || !weaponEnabled || currentWeapon != idealWeapon ) {
#else
	if ( gameLocal.isClient || !weaponEnabled ) {
#endif
		return;
	}

	//ivan start - use slots to select next best weapon

	//look in slots first, then check other weapons.
	foundInSlots = false;
	s = NUM_SLOTS;
	while( s > 0 ) {
		s--; 

		w = inventory.weaponSlot[s];
		if( w == -1 ){ //slot is empty
			continue;
		}

		//check as usual:
		weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
		if ( !weap[ 0 ] || ( ( inventory.weapons & ( 1 << w ) ) == 0 ) || ( !inventory.HasAmmo( weap, true, this ) ) ) { //new
			continue;
		}
		/*
		if ( !spawnArgs.GetBool( va( "weapon%d_best", w ) ) ) {
			continue;
		}
		*/

		foundInSlots = true;
		break;
	}

	if( !foundInSlots ){ //not found 
		w = MAX_WEAPONS; 
		while ( w > 0 ) { //check every weapon (TODO: avoid checking the ones in slots too, as it has already been done)
			w--;

			//check as usual:
			weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
			if ( !weap[ 0 ] || ( ( inventory.weapons & ( 1 << w ) ) == 0 ) || ( !inventory.HasAmmo( weap, true, this ) ) ) { //new
				continue;
			}
			/*
			if ( !spawnArgs.GetBool( va( "weapon%d_best", w ) ) ) {
				continue;
			}
			*/
			break;
		}
	}

	/* was:
	while ( w > 0 ) {
		w--;
		weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
		if ( !weap[ 0 ] || ( ( inventory.weapons & ( 1 << w ) ) == 0 ) || ( !inventory.HasAmmo( weap, true, this ) ) ) { //new
			//if ( !weap[ 0 ] || ( ( inventory.weapons & ( 1 << w ) ) == 0 ) || ( !inventory.HasAmmo( weap ) ) ) {
			continue;
		}
		if ( !spawnArgs.GetBool( va( "weapon%d_best", w ) ) ) {
			continue;
		}
		break;
	}
	*/
	//ivan end

#ifdef _DENTONMOD
	if( w != idealWeapon ) {
		quickWeapon = idealWeapon;
	}
#endif
	idealWeapon = w;
	weaponSwitchTime = gameLocal.time + WEAPON_SWITCH_DELAY;
	UpdateHudWeapon();
	//gameLocal.Printf("idPlayer::NextBestWeapon - idealWeapon: %d\n",idealWeapon); //un noted change from original sdk
}

/*
===============
idPlayer::NextWeapon
===============
*/
void idPlayer::NextWeapon( void ) {
	const char *weap;
	int w;
	int s, i; //ivan

	if ( !weaponEnabled || spectating || hiddenWeapon || gameLocal.inCinematic || gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) || health < 0 ) {
		return;
	}

	if ( gameLocal.isClient ) {
		return;
	}

	// check if we have any weapons
	if ( !inventory.weapons ) {
		return;
	}

	//ivan start - use slots to select next weapon
	s = currentSlot;
	i = 0; 
	while( 1 ) {
		s++;

		i++; //keep track of the number of iterations. 
		if( i > NUM_SLOTS ){ //if we do more iterations than number of slots, than no weapon is selectable.
			return;
		}

		if ( s >= NUM_SLOTS ) { //loop slots
			s = 0;
		}

		w = inventory.weaponSlot[s];
		if( w == -1 ){ //slot is empty
			continue;
		}

		weap = spawnArgs.GetString( va( "def_weapon%d", w ) );

		/*
		//the following should be already assured by slots
		if ( !spawnArgs.GetBool( va( "weapon%d_cycle", w ) ) ) {
			continue;
		}
		*/

		if ( !weap[ 0 ] ) {
			continue;
		}

		if ( ( inventory.weapons & ( 1 << w ) ) == 0 ) { //keep this check just to make sure...
			gameLocal.Warning("Found a non-available weapon in slot %d", s); 
			continue;
		}
		
		//allow selecting empty weapons
		if ( spawnArgs.GetBool( va( "weapon%d_allowempty", w ) ) ) {
			break;
		}

		if ( inventory.HasAmmo( weap, true, this ) ) { //new
			//if ( inventory.HasAmmo( weap ) ) { 
			break;
		}
	}

	/* was:
	w = idealWeapon;
	while( 1 ) {
		w++;
		if ( w >= MAX_WEAPONS ) {
			w = 0;
		}
		weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
		if ( !spawnArgs.GetBool( va( "weapon%d_cycle", w ) ) ) {
			continue;
		}
		if ( !weap[ 0 ] ) {
			continue;
		}
		if ( ( inventory.weapons & ( 1 << w ) ) == 0 ) {
			continue;
		}

		//ivan start - allow selecting empty weapons
		if ( spawnArgs.GetBool( va( "weapon%d_allowempty", w ) ) ) {
			break;
		}
		//ivan end

		if ( inventory.HasAmmo( weap, true, this ) ) {//new 
			//		if ( inventory.HasAmmo( weap ) ) {
			break;
		}
	}
	*/
	//ivan end

	if ( ( w != currentWeapon ) && ( w != idealWeapon ) ) {
		idealWeapon = w;
		weaponSwitchTime = gameLocal.time + WEAPON_SWITCH_DELAY;
		UpdateHudWeapon();
	}
}

/*
===============
idPlayer::PrevWeapon
===============
*/
void idPlayer::PrevWeapon( void ) {
	const char *weap;
	int w;
	int s, i; //ivan

	if ( !weaponEnabled || spectating || hiddenWeapon || gameLocal.inCinematic || gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) || health < 0 ) {
		return;
	}

	if ( gameLocal.isClient ) {
		return;
	}

	// check if we have any weapons
	if ( !inventory.weapons ) {
		return;
	}

	//ivan start - use slots to select prev weapon
	s = currentSlot;
	i = 0; 
	while( 1 ) {
		s--;

		i++; //keep track of the number of iterations. 
		if( i > NUM_SLOTS ){ //if we do more iterations than number of slots, than no weapon is selectable.
			return;
		}

		if ( s < 0 ) { //loop slots
			s = NUM_SLOTS - 1;
		}

		w = inventory.weaponSlot[s];
		if( w == -1 ){ //slot is empty
			continue;
		}
	
		weap = spawnArgs.GetString( va( "def_weapon%d", w ) );

		/*
		//the following should be already assured by slots
		if ( !spawnArgs.GetBool( va( "weapon%d_cycle", w ) ) ) {
			continue;
		}
		*/

		if ( !weap[ 0 ] ) {
			continue;
		}

		if ( ( inventory.weapons & ( 1 << w ) ) == 0 ) { //keep this check just to make sure...
			gameLocal.Warning("Found a non-available weapon in slot %d", s);
			continue;
		}
		
		//allow selecting empty weapons
		if ( spawnArgs.GetBool( va( "weapon%d_allowempty", w ) ) ) {
			break;
		}

		if ( inventory.HasAmmo( weap, true, this ) ) { //new
			//if ( inventory.HasAmmo( weap ) ) { 
			break;
		}
	}

	/* was:
	w = idealWeapon;
	while( 1 ) {
		w--;
		if ( w < 0 ) {
			w = MAX_WEAPONS - 1;
		}
		weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
		if ( !spawnArgs.GetBool( va( "weapon%d_cycle", w ) ) ) {
			continue;
		}
		if ( !weap[ 0 ] ) {
			continue;
		}
		if ( ( inventory.weapons & ( 1 << w ) ) == 0 ) {
			continue;
		}

		if ( inventory.HasAmmo( weap, true, this ) ) { //new
			//if ( inventory.HasAmmo( weap ) ) { 
			break;
		}
	}
	*/

	//ivan end

	if ( ( w != currentWeapon ) && ( w != idealWeapon ) ) {
		idealWeapon = w;
		weaponSwitchTime = gameLocal.time + WEAPON_SWITCH_DELAY;
		UpdateHudWeapon();
	}
}

/*
===============
idPlayer::SelectWeapon
===============
*/
void idPlayer::SelectWeapon( int num, bool force, const bool toggleWeapons ) { //un noted change from original sdk
	const char *weap;

	if ( !weaponEnabled || spectating || gameLocal.inCinematic || health < 0 ) {
		return;
	}

	if ( ( num < 0 ) || ( num >= MAX_WEAPONS ) ) {
		return;
	}

	if ( gameLocal.isClient ) {
		return;
	}

	if ( ( num != weapon_pda ) && gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) ) {
		num = weapon_fists;
		hiddenWeapon ^= 1;
		if ( hiddenWeapon && weapon.GetEntity() ) {
			weapon.GetEntity()->LowerWeapon();
		} else {
			weapon.GetEntity()->RaiseWeapon();
		}
	}

	weap = spawnArgs.GetString( va( "def_weapon%d", num ) );
	if ( !weap[ 0 ] ) {
		gameLocal.Printf( "Invalid weapon\n" );
		return;
	}


	WeaponToggle_t* weaponToggle; //un noted change from original sdk

	//Is the weapon a toggle weapon & player is trying to select it.
	if(weaponToggles.Get(va("weapontoggle%d", num), &weaponToggle) && toggleWeapons ) {

		int weaponToggleIndex = 0;

		//Find the current Weapon in the list
		int currentIndex = -1;
		for(int i = 0; i < weaponToggle->toggleList.Num(); i++) {
			if(weaponToggle->toggleList[i] == idealWeapon) {
				currentIndex = i;
				break;
			}
		}
		if(currentIndex == -1) {
			//Didn't find the current weapon so select the first item
			weaponToggleIndex = 0;
		} else {
			//Roll to the next available item in the list
			weaponToggleIndex = currentIndex;
			weaponToggleIndex++;
			if(weaponToggleIndex >= weaponToggle->toggleList.Num()) {
				weaponToggleIndex = 0;
			}
		}

		for(int i = 0; i < weaponToggle->toggleList.Num(); i++) {

			//Is it available
			if(inventory.weapons & ( 1 << weaponToggle->toggleList[weaponToggleIndex])) {
				break;
			}

			weaponToggleIndex++;
			if(weaponToggleIndex >= weaponToggle->toggleList.Num()) {
				weaponToggleIndex = 0;
			}
		}

		num = weaponToggle->toggleList[weaponToggleIndex];
	}

	if ( force || ( inventory.weapons & ( 1 << num ) ) ) {

		if ( !inventory.HasAmmo( weap, true, this ) && !spawnArgs.GetBool( va( "weapon%d_allowempty", num ) ) ) { //un noted change from original sdk
			return;
		}
		if ( ( previousWeapon >= 0 ) && ( idealWeapon == num ) && ( spawnArgs.GetBool( va( "weapon%d_toggle", num ) ) ) ) {
			weap = spawnArgs.GetString( va( "def_weapon%d", previousWeapon ) );

			if ( !inventory.HasAmmo( weap, true, this ) && !spawnArgs.GetBool( va( "weapon%d_allowempty", previousWeapon ) ) ) { //un noted change from original sdk
				return;
			}
			idealWeapon = previousWeapon;
		} else if ( ( weapon_pda >= 0 ) && ( num == weapon_pda ) && ( inventory.pdas.Num() == 0 ) ) {
			ShowTip( spawnArgs.GetString( "text_infoTitle" ), spawnArgs.GetString( "text_noPDA" ), true );
			return;
		} else {
			idealWeapon = num;
		}
		UpdateHudWeapon();
	}
}

//ivan start

void idPlayer::SetLock2D( bool on ){ 
	if ( forcedMovState == FORCEDMOVE_STATE_DISABLED ){ 
		isXlocked = on; 
	}else{ //a forced movement is active --> upd the Waslocked var so it'll be correctly restored at the end
		forcedMovWasLocked = on;
	}	
}


void idPlayer::SaveCheckPointPos( idEntity* checkEnt, bool useEntOrigin ){
	if( checkEnt != NULL && lastCheckPoint.entityNumber != checkEnt->entityNumber ){ 
		lastCheckPoint.entityNumber = checkEnt->entityNumber;
		lastCheckPoint.spawnPos = useEntOrigin ? checkEnt->GetPhysics()->GetOrigin() : GetPhysics()->GetOrigin(); //save current pos
		lastCheckPoint.cameraSettings = cameraSettings;
		if( hud ){ 
			ShowInfo( "Checkpoint!" );
			//hud->HandleNamedEvent( "lastCheckPoint" ); 
		}
		StartSound( "snd_checkPoint", SND_CHANNEL_ITEM, 0, false, NULL );
	}
}

void idPlayer::AddLifes( int num ){
	numLives += num;
	if( hud ){ 
		ShowInfo( "Extra Life!" );
		//hud->HandleNamedEvent( "extraLife" ); 
	}
	StartSound( "snd_extraLife", SND_CHANNEL_ITEM, 0, false, NULL );
}


void idPlayer::AddSecretFound( void ){
	gameLocal.secrets_found_counter++;
	if ( hud ) {
		ShowInfo( "Secret Found!" );
		//hud->HandleNamedEvent( "secretFound" );
	}
	//AddScore( SECRET_SCORE ); 
}

void idPlayer::AddScore( int num ){
	int oldscore, deltaTresholds;

	if( num <= 0 ) return;

	oldscore = score;
	score += num;

	deltaTresholds = score/EXTRALIFE_SCORE_TRESHOLD - oldscore/EXTRALIFE_SCORE_TRESHOLD;
	if( deltaTresholds > 0){
		AddLifes( deltaTresholds );
	}

	//if( hud ){ hud->HandleNamedEvent( "scoreAdded" ); }
}

void idPlayer::DropNotSelectedWeapon( int weapNum ) {
	int ammoavailable;
	int ammoRequired;

	if(weapNum < 0 || weapNum > MAX_WEAPONS){
		gameLocal.Warning("Weapon %d out of range", weapNum);
		return;
	}

	//check if it's the one selected
	if ( weapNum == currentWeapon ) {
		gameLocal.Warning("Cannot drop the weapon: weapNum == currentWeapon"); //ivan	
		return;
	}

	//check if we have it
	if (( inventory.weapons & ( 1 << weapNum ) ) == 0 ) {
		gameLocal.Warning("Cannot drop the weapon: player doesn't have the weapon"); //ivan	
		return;
	}
	
	//get info
	const char *weapName = spawnArgs.GetString( va( "def_weapon%d", weapNum ) );
	ammo_t ammo_i = inventory.AmmoIndexForWeaponClass( weapName, &ammoRequired );
	ammoavailable = inventory.HasAmmo( ammo_i, ammoRequired );

	//spawn
	idEntity *item = NULL;
	const idDeclEntityDef *decl = gameLocal.FindEntityDef( weapName );
	const char *classname = decl->dict.GetString( "def_dropItem" );
	if ( !classname[0] ) {
		gameLocal.Warning("Cannot drop the weapon: def_dropItem is not a valid classname");
		return;
	}
	item = idMoveableItem::DropItem( classname, firstPersonViewOrigin , firstPersonViewAxis, vec3_origin, 500, WEAPON_DROP_TIME );
	if ( !item ) {
		gameLocal.Warning("Cannot drop the weapon: failed spawning the entity");
		return;
	}

	// set the appropriate ammo in the dropped object
	const idKeyValue * keyval = item->spawnArgs.MatchPrefix( "inv_ammo_" );
	if ( keyval ) {
		item->spawnArgs.SetInt( keyval->GetKey(), ammoavailable );
	}

	// remove from our local inventory completely
	inventory.Drop( spawnArgs, item->spawnArgs.GetString( "inv_weapon" ), -1 );
	UpdateHudWeapon();
}


//ivan end



/*
=================
idPlayer::DropWeapon
=================
*/
bool idPlayer::DropWeapon( bool died, bool selectNext ) { //ivan - bool selectNext added
	idVec3 forward, up;
	int inclip, ammoavailable;
	bool dropAmmoInItem = true; //ivan - drop all the ammo by default

	assert( !gameLocal.isClient );

	if ( spectating || weaponGone || weapon.GetEntity() == NULL ) {
		return false; //un noted change from original sdk
	}

	if ( ( !died && !weapon.GetEntity()->IsReady() ) || weapon.GetEntity()->IsReloading() ) {
		return false; //un noted change from original sdk
	}

	// ammoavailable is how many shots we can fire
	// inclip is which amount is in clip right now
	ammoavailable = weapon.GetEntity()->AmmoAvailable();
	inclip = weapon.GetEntity()->AmmoInClip();

	/*
	//ivan - commented out because we have grenadelauncher instead.
	// don't drop a grenade if we have none left
	if ( !idStr::Icmp( idWeapon::GetAmmoNameForNum( weapon.GetEntity()->GetAmmoType() ), "ammo_grenades" ) && ( ammoavailable - inclip <= 0 ) ) {
		return false;
	}
	*/

	//ammoavailable += inclip;  //new //ivan - not necessary: AmmoAvailable() has been fixed 
	//ivan note: the prev line broke the following if. --> now is fixed 

	// expect an ammo setup that makes sense before doing any dropping
	// ammoavailable is -1 for infinite ammo, and weapons like chainsaw
	// a bad ammo config usually indicates a bad weapon state, so we should not drop
	// used to be an assertion check, but it still happens in edge cases
	if ( ( ammoavailable >= 0 ) && ( ammoavailable < inclip ) ) { //ivan - was: ( ammoavailable != -1 )
		common->DPrintf( "idPlayer::DropWeapon: bad ammo setup\n" );
		return false; //un noted change from original sdk
	}

	idEntity *item = NULL;
	if ( died ) {
		// ain't gonna throw you no weapon if I'm dead
		item = weapon.GetEntity()->DropItem( vec3_origin, 0, WEAPON_DROP_TIME, died );
	} else {
		viewAngles.ToVectors( &forward, NULL, &up );
		item = weapon.GetEntity()->DropItem( 250.0f * forward + 150.0f * up, 500, WEAPON_DROP_TIME, died );
	}
	if ( !item ) {
		return false; //un noted change from original sdk
	}
	// set the appropriate ammo in the dropped object
	const idKeyValue * keyval = item->spawnArgs.MatchPrefix( "inv_ammo_" );
	if ( keyval ) {	
		//item->spawnArgs.SetInt( keyval->GetKey(), ammoavailable ); //ivan - commented out
		idStr inclipKey = keyval->GetKey();
		inclipKey.Insert( "inclip_", 4 );
		inclipKey.Insert( va("%.2d", currentWeapon), 11); //new //un noted change from original sdk
		item->spawnArgs.SetInt( inclipKey, inclip );
	}
	
	//ivan start (old code removed)
	// remove from our local inventory completely
	inventory.Drop( spawnArgs, item->spawnArgs.GetString( "inv_weapon" ), -1 , &dropAmmoInItem ); //dropAmmoInItem added
		
	if ( keyval ) item->spawnArgs.SetInt( keyval->GetKey(), dropAmmoInItem ? ammoavailable : 0 );

	item->spawnArgs.SetInt( "score", 0 ); //make sure weapons cannot give score more than once
		
	weapon.GetEntity()->ResetAmmoClip();
		
	if( selectNext ){
		NextBestWeapon(); //this is better because does also check weapons out of slots if all slots are empty!
	}

	if ( !died ) { weapon.GetEntity()->WeaponStolen(); }
	weaponGone = true;
	//ivan end

	return true;
}

/*
=================
idPlayer::StealWeapon
steal the target player's current weapon
=================
*/
void idPlayer::StealWeapon( idPlayer *player ) {
	assert( !gameLocal.isClient );

	// make sure there's something to steal
	idWeapon *player_weapon = static_cast< idWeapon * >( player->weapon.GetEntity() );
	if ( !player_weapon || !player_weapon->CanDrop() || weaponGone ) {
		return;
	}
	// steal - we need to effectively force the other player to abandon his weapon
	int newweap = player->currentWeapon;
	if ( newweap == -1 ) {
		return;
	}
	// might be just dropped - check inventory
	if ( ! ( player->inventory.weapons & ( 1 << newweap ) ) ) {
		return;
	}
	const char *weapon_classname = spawnArgs.GetString( va( "def_weapon%d", newweap ) );
	assert( weapon_classname );
	int ammoavailable = player->weapon.GetEntity()->AmmoAvailable();
	int inclip = player->weapon.GetEntity()->AmmoInClip();

	//ammoavailable += inclip; //new  //ivan - not necessary: AmmoAvailable() has been fixed 

	if ( ( ammoavailable != -1 ) && ( ammoavailable - inclip < 0 ) ) {
		// see DropWeapon
		common->DPrintf( "idPlayer::StealWeapon: bad ammo setup\n" );
		// we still steal the weapon, so let's use the default ammo levels
		inclip = -1;
		const idDeclEntityDef *decl = gameLocal.FindEntityDef( weapon_classname );
		assert( decl );
		const idKeyValue *keypair = decl->dict.MatchPrefix( "inv_ammo_" );
		assert( keypair );
		ammoavailable = atoi( keypair->GetValue() );
	}

	player->weapon.GetEntity()->WeaponStolen();
	player->inventory.Drop( player->spawnArgs, NULL, newweap );
	player->SelectWeapon( weapon_fists, false );
	// in case the robbed player is firing rounds with a continuous fire weapon like the chaingun/plasma etc.
	// this will ensure the firing actually stops
	player->weaponGone = true;

	// give weapon, setup the ammo count
	Give( "weapon", weapon_classname );
	ammo_t ammo_i = player->inventory.AmmoIndexForWeaponClass( weapon_classname, NULL );
	idealWeapon = newweap;
	inventory.ammo[ ammo_i ] += ammoavailable;
	inventory.clip[ newweap ] = inclip;
}

/*
===============
idPlayer::ActiveGui
===============
*/
idUserInterface *idPlayer::ActiveGui( void ) {
	if ( objectiveSystemOpen ) {
		return objectiveSystem;
	}

	return focusUI;
}

/*
===============
idPlayer::Weapon_Combat
===============
*/
void idPlayer::Weapon_Combat( void ) {
	if ( influenceActive || !weaponEnabled || gameLocal.inCinematic || privateCameraView ) {
		return;
	}

	weapon.GetEntity()->RaiseWeapon();
	if ( weapon.GetEntity()->IsReloading() ) {
		if ( !AI_RELOAD ) {
			AI_RELOAD = true;
			SetState( "ReloadWeapon" );
			UpdateScript();
		}
	} else {
		AI_RELOAD = false;
	}

	if ( idealWeapon == weapon_soulcube && soulCubeProjectile.GetEntity() != NULL ) {
		idealWeapon = currentWeapon;
	}

	if ( idealWeapon != currentWeapon ) {
		if ( weaponCatchup ) {
			assert( gameLocal.isClient );

			currentWeapon = idealWeapon;
			weaponGone = false;
			animPrefix = spawnArgs.GetString( va( "def_weapon%d", currentWeapon ) );
			weapon.GetEntity()->GetWeaponDef( animPrefix, inventory.clip[ currentWeapon ] );
			animPrefix.Strip( "weapon_" );

			weapon.GetEntity()->NetCatchup();
			const function_t *newstate = GetScriptFunction( "NetCatchup" );
			if ( newstate ) {
				SetState( newstate );
				UpdateScript();
			}
			weaponCatchup = false;
		} else {
			if ( weapon.GetEntity()->IsReady() ) {
				weapon.GetEntity()->PutAway();
			}


			if ( weapon.GetEntity()->IsHolstered() ) {
				assert( idealWeapon >= 0 );
				assert( idealWeapon < MAX_WEAPONS );

				if ( currentWeapon != weapon_pda && !spawnArgs.GetBool( va( "weapon%d_toggle", currentWeapon ) ) ) {
					previousWeapon = currentWeapon;
				}
				currentWeapon = idealWeapon;
				weaponGone = false;
				animPrefix = spawnArgs.GetString( va( "def_weapon%d", currentWeapon ) );
				weapon.GetEntity()->GetWeaponDef( animPrefix, inventory.clip[ currentWeapon ] );
				animPrefix.Strip( "weapon_" );

				weapon.GetEntity()->Raise();
			}
		}
	} else {
		weaponGone = false;	// if you drop and re-get weap, you may miss the = false above
		if ( weapon.GetEntity()->IsHolstered() ) {
			if ( !weapon.GetEntity()->AmmoAvailable() ) {
#ifdef _DENTONMOD	// If weapon with no ammo is soulcube, switch to previous weapon	
				if ( ( weapon_soulcube >= 0 ) && ( currentWeapon == weapon_soulcube ) && currentWeapon == idealWeapon ) {
					if ( hud ) {
						hud->HandleNamedEvent( "soulCubeNotReady" );
					}
					quickWeapon = weapon_soulcube; 
					SelectWeapon( previousWeapon, false );
				} else {
					// weapons can switch automatically if they have no more ammo
					NextBestWeapon();
				}
#else
				// weapons can switch automatically if they have no more ammo
				NextBestWeapon();
#endif
			} else {
				weapon.GetEntity()->Raise();
				state = GetScriptFunction( "RaiseWeapon" );
				if ( state ) {
					SetState( state );
				}
			}
		}
	}

	// check for attack
	AI_WEAPON_FIRED = false;
	if ( !influenceActive ) {
		if ( ( usercmd.buttons & BUTTON_ATTACK ) && !weaponGone ) {
			FireWeapon();
		} else if ( oldButtons & BUTTON_ATTACK ) {
			AI_ATTACK_HELD = false;
			weapon.GetEntity()->EndAttack();
		}

		// check for Weapon special function, new //un noted change from original sdk
		if ( ( usercmd.buttons & BUTTON_5 ) && !weaponGone ) {  // BUTTON_5 is being used for weapon special function 
			WeaponSpecialFunction( !(oldButtons & BUTTON_5) );	// The condition holds True when key is being tapped rather than held
		} else if ( oldButtons & BUTTON_5 ) {
			weapon.GetEntity()->EndSpecialFunction();
		}
	}

	// update our ammo clip in our inventory
	if ( ( currentWeapon >= 0 ) && ( currentWeapon < MAX_WEAPONS ) ) {
		inventory.clip[ currentWeapon ] = weapon.GetEntity()->AmmoInClip();
		if ( hud && ( currentWeapon == idealWeapon ) ) {
			UpdateHudAmmo( hud );
		}
	}
}

/*
===============
idPlayer::Weapon_NPC
===============
*/
void idPlayer::Weapon_NPC( void ) {
	if ( idealWeapon != currentWeapon ) {
		Weapon_Combat();
	}
	StopFiring();
	weapon.GetEntity()->LowerWeapon();

	if ( ( usercmd.buttons & BUTTON_ATTACK ) && !( oldButtons & BUTTON_ATTACK ) ) {
		buttonMask |= BUTTON_ATTACK;
		focusCharacter->TalkTo( this );
	}
}

/*
===============
idPlayer::LowerWeapon
===============
*/
void idPlayer::LowerWeapon( void ) {
	if ( weapon.GetEntity() && !weapon.GetEntity()->IsHidden() ) {
		weapon.GetEntity()->LowerWeapon();
	}
}

/*
===============
idPlayer::RaiseWeapon
===============
*/
void idPlayer::RaiseWeapon( void ) {
	if ( weapon.GetEntity() && weapon.GetEntity()->IsHidden() ) {
		weapon.GetEntity()->RaiseWeapon();
	}
}

/*
===============
idPlayer::WeaponLoweringCallback
===============
*/
void idPlayer::WeaponLoweringCallback( void ) {
	SetState( "LowerWeapon" );
	UpdateScript();
}

/*
===============
idPlayer::WeaponRisingCallback
===============
*/
void idPlayer::WeaponRisingCallback( void ) {
	SetState( "RaiseWeapon" );
	UpdateScript();
}

/*
===============
idPlayer::Weapon_GUI
===============
*/
void idPlayer::Weapon_GUI( void ) {

	if ( !objectiveSystemOpen ) {
		if ( idealWeapon != currentWeapon ) {
			Weapon_Combat();
		}
		StopFiring();
		weapon.GetEntity()->LowerWeapon();
	}

	// disable click prediction for the GUIs. handy to check the state sync does the right thing
	if ( gameLocal.isClient && !net_clientPredictGUI.GetBool() ) {
		return;
	}

	if ( ( oldButtons ^ usercmd.buttons ) & BUTTON_ATTACK ) {
		sysEvent_t ev;
		const char *command = NULL;
		bool updateVisuals = false;

		idUserInterface *ui = ActiveGui();
		if ( ui ) {
			ev = sys->GenerateMouseButtonEvent( 1, ( usercmd.buttons & BUTTON_ATTACK ) != 0 );
			command = ui->HandleEvent( &ev, gameLocal.time, &updateVisuals );
			if ( updateVisuals && focusGUIent && ui == focusUI ) {
				focusGUIent->UpdateVisuals();
			}
		}
		if ( gameLocal.isClient ) {
			// we predict enough, but don't want to execute commands
			return;
		}
		if ( focusGUIent ) {
			HandleGuiCommands( focusGUIent, command );
		} else {
			HandleGuiCommands( this, command );
		}
	}
}

/*
===============
idPlayer::UpdateWeapon
===============
*/
void idPlayer::UpdateWeapon( void ) {
	if ( health <= 0 ) {
		return;
	}

	assert( !spectating );

	if ( gameLocal.isClient ) {
		// clients need to wait till the weapon and it's world model entity
		// are present and synchronized ( weapon.worldModel idEntityPtr to idAnimatedEntity )
		if ( !weapon.GetEntity()->IsWorldModelReady() ) {
			return;
		}
	}

	// always make sure the weapon is correctly setup before accessing it
	if ( !weapon.GetEntity()->IsLinked() ) {
		if ( idealWeapon != -1 ) {
			animPrefix = spawnArgs.GetString( va( "def_weapon%d", idealWeapon ) );
			weapon.GetEntity()->GetWeaponDef( animPrefix, inventory.clip[ idealWeapon ] );
			assert( weapon.GetEntity()->IsLinked() );
		} else {
			return;
		}
	}

	/* ivan
	if ( hiddenWeapon && tipUp && usercmd.buttons & BUTTON_ATTACK ) {
		HideTip();
	}
	*/

	if ( g_dragEntity.GetBool() ) {
		StopFiring();
		weapon.GetEntity()->LowerWeapon();
		dragEntity.Update( this );
	} else if ( ActiveGui() ) {
		// gui handling overrides weapon use
		Weapon_GUI();
	} else	if ( focusCharacter && ( focusCharacter->health > 0 ) ) {
		Weapon_NPC();
	} else {
		Weapon_Combat();
	}

	if ( hiddenWeapon ) {
		weapon.GetEntity()->LowerWeapon();
	}

	// update weapon state, particles, dlights, etc
	weapon.GetEntity()->PresentWeapon( showWeaponViewModel );
}

/*
===============
idPlayer::SpectateFreeFly
===============
*/
void idPlayer::SpectateFreeFly( bool force ) {
	idPlayer	*player;
	idVec3		newOrig;
	idVec3		spawn_origin;
	idAngles	spawn_angles;

	player = gameLocal.GetClientByNum( spectator );
	if ( force || gameLocal.time > lastSpectateChange ) {
		spectator = entityNumber;
		if ( player && player != this && !player->spectating && !player->IsInTeleport() ) {
			newOrig = player->GetPhysics()->GetOrigin();
			if ( player->physicsObj.IsCrouching() ) {
				newOrig[ 2 ] += pm_crouchviewheight.GetFloat();
			} else {
				newOrig[ 2 ] += pm_normalviewheight.GetFloat();
			}
			newOrig[ 2 ] += SPECTATE_RAISE;
			idBounds b = idBounds( vec3_origin ).Expand( pm_spectatebbox.GetFloat() * 0.5f );
			idVec3 start = player->GetPhysics()->GetOrigin();
			start[2] += pm_spectatebbox.GetFloat() * 0.5f;
			trace_t t;
			// assuming spectate bbox is inside stand or crouch box
			gameLocal.clip.TraceBounds( t, start, newOrig, b, MASK_PLAYERSOLID, player );
			newOrig.Lerp( start, newOrig, t.fraction );
			SetOrigin( newOrig );
			idAngles angle = player->viewAngles;
			angle[ 2 ] = 0;
			SetViewAngles( angle );
		} else {
			SelectInitialSpawnPoint( spawn_origin, spawn_angles );
			spawn_origin[ 2 ] += pm_normalviewheight.GetFloat();
			spawn_origin[ 2 ] += SPECTATE_RAISE;
			SetOrigin( spawn_origin );
			SetViewAngles( spawn_angles );
		}
		lastSpectateChange = gameLocal.time + 500;
	}
}

/*
===============
idPlayer::SpectateCycle
===============
*/
void idPlayer::SpectateCycle( void ) {
	idPlayer *player;

	if ( gameLocal.time > lastSpectateChange ) {
		int latchedSpectator = spectator;
		spectator = gameLocal.GetNextClientNum( spectator );
		player = gameLocal.GetClientByNum( spectator );
		assert( player ); // never call here when the current spectator is wrong
		// ignore other spectators
		while ( latchedSpectator != spectator && player->spectating ) {
			spectator = gameLocal.GetNextClientNum( spectator );
			player = gameLocal.GetClientByNum( spectator );
		}
		lastSpectateChange = gameLocal.time + 500;
	}
}

/*
===============
idPlayer::UpdateSpectating
===============
*/
void idPlayer::UpdateSpectating( void ) {
	assert( spectating );
	assert( !gameLocal.isClient );
	assert( IsHidden() );
	idPlayer *player;
	if ( !gameLocal.isMultiplayer ) {
		return;
	}
	player = gameLocal.GetClientByNum( spectator );
	if ( !player || ( player->spectating && player != this ) ) {
		SpectateFreeFly( true );
	} else if ( usercmd.upmove > 0 ) {
		SpectateFreeFly( false );
	} else if ( usercmd.buttons & BUTTON_ATTACK ) {
		SpectateCycle();
	}
}

/*
===============
idPlayer::HandleSingleGuiCommand
===============
*/
bool idPlayer::HandleSingleGuiCommand( idEntity *entityGui, idLexer *src ) {
	idToken token;

	if ( !src->ReadToken( &token ) ) {
		return false;
	}

	if ( token == ";" ) {
		return false;
	}

	if ( token.Icmp( "addhealth" ) == 0 ) {
		if ( entityGui && health < 100 ) {
			int _health = entityGui->spawnArgs.GetInt( "gui_parm1" );
			int amt = ( _health >= HEALTH_PER_DOSE ) ? HEALTH_PER_DOSE : _health;
			_health -= amt;
			entityGui->spawnArgs.SetInt( "gui_parm1", _health );
			if ( entityGui->GetRenderEntity() && entityGui->GetRenderEntity()->gui[ 0 ] ) {
				entityGui->GetRenderEntity()->gui[ 0 ]->SetStateInt( "gui_parm1", _health );
			}
			health += amt;
			if ( health > 100 ) {
				health = 100;
			}
		}
		return true;
	}

	if ( token.Icmp( "ready" ) == 0 ) {
		PerformImpulse( IMPULSE_17 );
		return true;
	}

	if ( token.Icmp( "updatepda" ) == 0 ) {
		UpdatePDAInfo( true );
		return true;
	}

	if ( token.Icmp( "updatepda2" ) == 0 ) {
		UpdatePDAInfo( false );
		return true;
	}

	if ( token.Icmp( "stoppdavideo" ) == 0 ) {
		if ( objectiveSystem && objectiveSystemOpen && pdaVideoWave.Length() > 0 ) {
			StopSound( SND_CHANNEL_PDA, false );
		}
		return true;
	}

	if ( token.Icmp( "close" ) == 0 ) {
		if ( objectiveSystem && objectiveSystemOpen ) {
			TogglePDA();
		}
	}

	if ( token.Icmp( "playpdavideo" ) == 0 ) {
		if ( objectiveSystem && objectiveSystemOpen && pdaVideo.Length() > 0 ) {
			const idMaterial *mat = declManager->FindMaterial( pdaVideo );
			if ( mat ) {
				int c = mat->GetNumStages();
				for ( int i = 0; i < c; i++ ) {
					const shaderStage_t *stage = mat->GetStage(i);
					if ( stage && stage->texture.cinematic ) {
						stage->texture.cinematic->ResetTime( gameLocal.time );
					}
				}
				if ( pdaVideoWave.Length() ) {
					const idSoundShader *shader = declManager->FindSound( pdaVideoWave );
					StartSoundShader( shader, SND_CHANNEL_PDA, 0, false, NULL );
				}
			}
		}
	}

	if ( token.Icmp( "playpdaaudio" ) == 0 ) {
		if ( objectiveSystem && objectiveSystemOpen && pdaAudio.Length() > 0 ) {
			const idSoundShader *shader = declManager->FindSound( pdaAudio );
			int ms;
			StartSoundShader( shader, SND_CHANNEL_PDA, 0, false, &ms );
			StartAudioLog();
			CancelEvents( &EV_Player_StopAudioLog );
			PostEventMS( &EV_Player_StopAudioLog, ms + 150 );
		}
		return true;
	}

	if ( token.Icmp( "stoppdaaudio" ) == 0 ) {
		if ( objectiveSystem && objectiveSystemOpen && pdaAudio.Length() > 0 ) {
			// idSoundShader *shader = declManager->FindSound( pdaAudio );
			StopAudioLog();
			StopSound( SND_CHANNEL_PDA, false );
		}
		return true;
	}

	src->UnreadToken( &token );
	return false;
}

/*
==============
idPlayer::Collide
==============
*/
bool idPlayer::Collide( const trace_t &collision, const idVec3 &velocity ) {
	idEntity *other;
	
	if ( gameLocal.isClient ) {
		return false;
	}

	other = gameLocal.entities[ collision.c.entityNum ];

//REV 2020 UPDATED.  JUST CHECK WHAT WERE TOUCHING.  The groundent check is not needed.
//rev 2018 start damage the player when touching enemies
	//idEntity *groundEnt = physicsObj.GetGroundEntity();

//Rev 2020 update 2: commented out.
//the AI now checks if over lapping the player and calls the hurtbox to happen.  Thus we don't need this anymore.
/*	
	if ( other->IsType( idAI::Type ) ) {	
		//if ( other && other->IsType( idAI::Type )  ) {
		touchofdeathx = other->spawnArgs.GetInt( "touchofdeath" );
		if ( touchofdeathx > 0 ) {
		Damage( NULL, NULL, vec3_origin, "damage_touchofdeath", 1.0f, 0 );					
		}
	}
*/	
//REV 2020 END
	if ( other ) {
		
		other->Signal( SIG_TOUCH );
		if ( !spectating ) {
			if ( other->RespondsTo( EV_Touch ) ) {
				other->ProcessEvent( &EV_Touch, this, &collision );					
			}
		} else {
			if ( other->RespondsTo( EV_SpectatorTouch ) ) {
				other->ProcessEvent( &EV_SpectatorTouch, this, &collision );
			}
		}
	}
	return false;
}


/*
================
idPlayer::UpdateLocation

Searches nearby locations
================
*/
void idPlayer::UpdateLocation( void ) {
	if ( hud ) {
		idLocationEntity *locationEntity = gameLocal.LocationForPoint( GetEyePosition() );
		if ( locationEntity ) {
			hud->SetStateString( "location", locationEntity->GetLocation() );
		} else {
			hud->SetStateString( "location", common->GetLanguageDict()->GetString( "#str_02911" ) );
		}
	}
}

/*
================
idPlayer::ClearFocus

Clears the focus cursor
================
*/
void idPlayer::ClearFocus( void ) {
	focusCharacter	= NULL;
	focusGUIent		= NULL;
	focusUI			= NULL;
	focusVehicle	= NULL;
	talkCursor		= 0;
}

/*
================
idPlayer::UpdateFocus

Searches nearby entities for interactive guis, possibly making one of them
the focus and sending it a mouse move event
================
*/
void idPlayer::UpdateFocus( void ) {
	idClipModel *clipModelList[ MAX_GENTITIES ];
	idClipModel *clip;
	int			listedClipModels;
	idEntity	*oldFocus;
	idEntity	*ent;
	idUserInterface *oldUI;
	idAI		*oldChar;
	int			oldTalkCursor;
	int			i, j;
	idVec3		start, end;
	bool		allowFocus;
	const char *command;
	trace_t		trace;
	guiPoint_t	pt;
	const idKeyValue *kv;
	sysEvent_t	ev;
	idUserInterface *ui;

	if ( gameLocal.inCinematic ) {
		return;
	}

	// only update the focus character when attack button isn't pressed so players
	// can still chainsaw NPC's
	if ( gameLocal.isMultiplayer || ( !focusCharacter && ( usercmd.buttons & BUTTON_ATTACK ) ) ) {
		allowFocus = false;
	} else {
		allowFocus = true;
	}

	oldFocus		= focusGUIent;
	oldUI			= focusUI;
	oldChar			= focusCharacter;
	oldTalkCursor	= talkCursor;

	if ( focusTime <= gameLocal.time ) {
		ClearFocus();
	}

	// don't let spectators interact with GUIs
	if ( spectating ) {
		return;
	}

	start = GetEyePosition();
	end = start + viewAngles.ToForward() * 80.0f;

	// player identification -> names to the hud
	if ( gameLocal.isMultiplayer && entityNumber == gameLocal.localClientNum ) {
		idVec3 end = start + viewAngles.ToForward() * 768.0f;
		gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_BOUNDINGBOX, this );
		int iclient = -1;
		if ( ( trace.fraction < 1.0f ) && ( trace.c.entityNum < MAX_CLIENTS ) ) {
			iclient = trace.c.entityNum;
		}
		if ( MPAim != iclient ) {
			lastMPAim = MPAim;
			MPAim = iclient;
			lastMPAimTime = gameLocal.realClientTime;
		}
	}

	idBounds bounds( start );
	bounds.AddPoint( end );

	listedClipModels = gameLocal.clip.ClipModelsTouchingBounds( bounds, -1, clipModelList, MAX_GENTITIES );

	// no pretense at sorting here, just assume that there will only be one active
	// gui within range along the trace
	for ( i = 0; i < listedClipModels; i++ ) {
		clip = clipModelList[ i ];
		ent = clip->GetEntity();

		if ( ent->IsHidden() ) {
			continue;
		}

		if ( allowFocus ) {
			if ( ent->IsType( idAFAttachment::Type ) ) {
				idEntity *body = static_cast<idAFAttachment *>( ent )->GetBody();
				if ( body && body->IsType( idAI::Type ) && ( static_cast<idAI *>( body )->GetTalkState() >= TALK_OK ) ) {
					gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_RENDERMODEL, this );
					if ( ( trace.fraction < 1.0f ) && ( trace.c.entityNum == ent->entityNumber ) ) {
						ClearFocus();
						focusCharacter = static_cast<idAI *>( body );
						talkCursor = 1;
						focusTime = gameLocal.time + FOCUS_TIME;
						break;
					}
				}
				continue;
			}

			if ( ent->IsType( idAI::Type ) ) {
				if ( static_cast<idAI *>( ent )->GetTalkState() >= TALK_OK ) {
					gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_RENDERMODEL, this );
					if ( ( trace.fraction < 1.0f ) && ( trace.c.entityNum == ent->entityNumber ) ) {
						ClearFocus();
						focusCharacter = static_cast<idAI *>( ent );
						talkCursor = 1;
						focusTime = gameLocal.time + FOCUS_TIME;
						break;
					}
				}
				continue;
			}

			if ( ent->IsType( idAFEntity_Vehicle::Type ) ) {
				gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_RENDERMODEL, this );
				if ( ( trace.fraction < 1.0f ) && ( trace.c.entityNum == ent->entityNumber ) ) {
					ClearFocus();
					focusVehicle = static_cast<idAFEntity_Vehicle *>( ent );
					focusTime = gameLocal.time + FOCUS_TIME;
					break;
				}
				continue;
			}
		}

		if ( !ent->GetRenderEntity() || !ent->GetRenderEntity()->gui[ 0 ] || !ent->GetRenderEntity()->gui[ 0 ]->IsInteractive() ) {
			continue;
		}

		if ( ent->spawnArgs.GetBool( "inv_item" ) ) {
			// don't allow guis on pickup items focus
			continue;
		}

		pt = gameRenderWorld->GuiTrace( ent->GetModelDefHandle(), start, end );
		if ( pt.x != -1 ) {
			// we have a hit
			renderEntity_t *focusGUIrenderEntity = ent->GetRenderEntity();
			if ( !focusGUIrenderEntity ) {
				continue;
			}

			if ( pt.guiId == 1 ) {
				ui = focusGUIrenderEntity->gui[ 0 ];
			} else if ( pt.guiId == 2 ) {
				ui = focusGUIrenderEntity->gui[ 1 ];
			} else {
				ui = focusGUIrenderEntity->gui[ 2 ];
			}

			if ( ui == NULL ) {
				continue;
			}

			ClearFocus();
			focusGUIent = ent;
			focusUI = ui;

			if ( oldFocus != ent ) {
				//new activation
				// going to see if we have anything in inventory a gui might be interested in
				// need to enumerate inventory items
				focusUI->SetStateInt( "inv_count", inventory.items.Num() );
				for ( j = 0; j < inventory.items.Num(); j++ ) {
					idDict *item = inventory.items[ j ];
					const char *iname = item->GetString( "inv_name" );
					const char *iicon = item->GetString( "inv_icon" );
					const char *itext = item->GetString( "inv_text" );

					focusUI->SetStateString( va( "inv_name_%i", j), iname );
					focusUI->SetStateString( va( "inv_icon_%i", j), iicon );
					focusUI->SetStateString( va( "inv_text_%i", j), itext );
					kv = item->MatchPrefix("inv_id", NULL);
					if ( kv ) {
						focusUI->SetStateString( va( "inv_id_%i", j ), kv->GetValue() );
					}
					focusUI->SetStateInt( iname, 1 );
				}


				for( j = 0; j < inventory.pdaSecurity.Num(); j++ ) {
					const char *p = inventory.pdaSecurity[ j ];
					if ( p && *p ) {
						focusUI->SetStateInt( p, 1 );
					}
				}

				int staminapercentage = ( int )( 100.0f * stamina / pm_stamina.GetFloat() );
				focusUI->SetStateString( "player_health", va("%i", health ) );
				focusUI->SetStateString( "player_stamina", va( "%i%%", staminapercentage ) );
				focusUI->SetStateString( "player_armor", va( "%i%%", inventory.armor ) );

				kv = focusGUIent->spawnArgs.MatchPrefix( "gui_parm", NULL );
				while ( kv ) {
					focusUI->SetStateString( kv->GetKey(), kv->GetValue() );
					kv = focusGUIent->spawnArgs.MatchPrefix( "gui_parm", kv );
				}
			}

			// clamp the mouse to the corner
			ev = sys->GenerateMouseMoveEvent( -2000, -2000 );
			command = focusUI->HandleEvent( &ev, gameLocal.time );
			HandleGuiCommands( focusGUIent, command );

			// move to an absolute position
			ev = sys->GenerateMouseMoveEvent( pt.x * SCREEN_WIDTH, pt.y * SCREEN_HEIGHT );
			command = focusUI->HandleEvent( &ev, gameLocal.time );
			HandleGuiCommands( focusGUIent, command );
			focusTime = gameLocal.time + FOCUS_GUI_TIME;
			break;
		}
	}

	if ( focusGUIent && focusUI ) {
		if ( !oldFocus || oldFocus != focusGUIent ) {
			command = focusUI->Activate( true, gameLocal.time );
			HandleGuiCommands( focusGUIent, command );
			StartSound( "snd_guienter", SND_CHANNEL_ANY, 0, false, NULL );
			// HideTip();
			// HideObjective();
		}
	} else if ( oldFocus && oldUI ) {
		command = oldUI->Activate( false, gameLocal.time );
		HandleGuiCommands( oldFocus, command );
		StartSound( "snd_guiexit", SND_CHANNEL_ANY, 0, false, NULL );
	}

	if ( cursor && ( oldTalkCursor != talkCursor ) ) {
		cursor->SetStateInt( "talkcursor", talkCursor );
	}

	if ( oldChar != focusCharacter && hud ) {
		if ( focusCharacter ) {
			//smart AI start 
			if(focusCharacter->spawnArgs.GetBool( "showStatus", "0" )){  //ff1.1
				hud->SetStateString( "npc", "Status:" );
				hud->SetStateString( "npc_action", focusCharacter->spawnArgs.GetString( "shownState", "Inactive" ) );
			}else{
				hud->SetStateString( "npc", focusCharacter->spawnArgs.GetString( "npc_name", "Joe" ) ); 
				hud->SetStateString( "npc_action", common->GetLanguageDict()->GetString( "#str_02036" ) );
			}
			//smart AI end

			/*
			//ivan - commented out start 
			hud->SetStateString( "npc", focusCharacter->spawnArgs.GetString( "npc_name", "Joe" ) );
			//ivan - commented out end 
			*/

			hud->HandleNamedEvent( "showNPC" );
			// HideTip();
			// HideObjective();
		} else {
			hud->SetStateString( "npc", "" );
			hud->HandleNamedEvent( "hideNPC" );
		}
	}
}

/*
=================
idPlayer::CrashLand

Check for hard landings that generate sound events
=================
*/
void idPlayer::CrashLand( const idVec3 &oldOrigin, const idVec3 &oldVelocity ) {
	idVec3		origin, velocity;
	idVec3		gravityVector, gravityNormal;
	float		delta;
	float		hardDelta, fatalDelta;
	float		dist;
	float		vel, acc;
	float		t;
	float		a, b, c, den;
	waterLevel_t waterLevel;
	bool		noDamage;

	AI_SOFTLANDING = false;
	AI_HARDLANDING = false;

	// if the player is not on the ground
	if ( !physicsObj.HasGroundContacts() ) {
		return;
	}

	gravityNormal = physicsObj.GetGravityNormal();

	// if the player wasn't going down
	if ( ( oldVelocity * -gravityNormal ) >= 0.0f ) {
		return;
	}

	waterLevel = physicsObj.GetWaterLevel();

	// never take falling damage if completely underwater
	if ( waterLevel == WATERLEVEL_HEAD ) {
		return;
	}

	// no falling damage if touching a nodamage surface
	noDamage = false;
	for ( int i = 0; i < physicsObj.GetNumContacts(); i++ ) {
		const contactInfo_t &contact = physicsObj.GetContact( i );
		if ( contact.material->GetSurfaceFlags() & SURF_NODAMAGE ) {
			noDamage = true;
			//StartSound( "snd_land_hard", SND_CHANNEL_ANY, 0, false, NULL ); //rev 2018 removed.  not needed.
			break;
		}
	}

	origin = GetPhysics()->GetOrigin();
	gravityVector = physicsObj.GetGravity();

	// calculate the exact velocity on landing
	dist = ( origin - oldOrigin ) * -gravityNormal;
	vel = oldVelocity * -gravityNormal;
	acc = -gravityVector.Length();

	a = acc / 2.0f;
	b = vel;
	c = -dist;

	den = b * b - 4.0f * a * c;
	if ( den < 0 ) {
		return;
	}
	t = ( -b - idMath::Sqrt( den ) ) / ( 2.0f * a );

	delta = vel + t * acc;
	delta = delta * delta * 0.0001;

	// reduce falling damage if there is standing water
	if ( waterLevel == WATERLEVEL_WAIST ) {
		delta *= 0.25f;
	}
	if ( waterLevel == WATERLEVEL_FEET ) {
		delta *= 0.5f;
	}

	if ( delta < 1.0f ) {
		return;
	}

	// allow falling a bit further for multiplayer
	if ( gameLocal.isMultiplayer ) {
		fatalDelta	= 75.0f;
		hardDelta	= 50.0f;
	} else {
		fatalDelta	= 65.0f;
		hardDelta	= 45.0f;
	}

	if ( delta > fatalDelta ) {
		AI_HARDLANDING = true;
		landChange = -32;
		landTime = gameLocal.time;
		if ( !noDamage ) {
			pain_debounce_time = gameLocal.time + pain_delay + 1;  // ignore pain since we'll play our landing anim
			//ivan commented out - was: Damage( NULL, NULL, idVec3( 0, 0, -1 ), "damage_fatalfall", 1.0f, 0 );
		}
	} else if ( delta > hardDelta ) {
		AI_HARDLANDING = true;
		landChange	= -24;
		landTime	= gameLocal.time;
		if ( !noDamage ) {
			pain_debounce_time = gameLocal.time + pain_delay + 1;  // ignore pain since we'll play our landing anim
			//ivan commented out - was: Damage( NULL, NULL, idVec3( 0, 0, -1 ), "damage_hardfall", 1.0f, 0 );
		}
	} else if ( delta > 30 ) {
		AI_HARDLANDING = true;
		landChange	= -16;
		landTime	= gameLocal.time;
		if ( !noDamage ) {
			pain_debounce_time = gameLocal.time + pain_delay + 1;  // ignore pain since we'll play our landing anim
			//ivan commented out - was: Damage( NULL, NULL, idVec3( 0, 0, -1 ), "damage_softfall", 1.0f, 0 );
		}
	} else if ( delta > 7 ) {
		AI_SOFTLANDING = true;
		landChange	= -8;
		landTime	= gameLocal.time;
	} else if ( delta > 3 ) {
		// just walk on
	}
}

/*
===============
idPlayer::BobCycle
===============
*/
//ivan start
void idPlayer::BobCycle( const idVec3 &pushVelocity ) {
	idVec3	vel, gravityDir, velocity;

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	velocity = physicsObj.GetLinearVelocity() - pushVelocity;

	gravityDir = physicsObj.GetGravityNormal();
	vel = velocity - ( velocity * gravityDir ) * gravityDir;
	xyspeed = vel.LengthFast();
}

/*
================
idPlayer::UpdateDeltaViewAngles
================
*/
void idPlayer::UpdateDeltaViewAngles( const idAngles &angles ) {
	// set the delta angle
	idAngles delta;
	for( int i = 0; i < 3; i++ ) {
		delta[ i ] = angles[ i ] - SHORT2ANGLE( usercmd.angles[ i ] );
	}
	SetDeltaViewAngles( delta );
}

/*
================
idPlayer::SetViewAngles
================
*/
void idPlayer::SetViewAngles( const idAngles &angles ) {
	UpdateDeltaViewAngles( angles );
	viewAngles = angles;
}

//ivan start 
#ifdef SHOW_MOVING_CROSSHAIR

//max pos the center of the crosshair can reach. Values should be <= 320 and 240.
//remove 16 pixels so the crosshair is always fully inside the screen
#define CROSS_MAX_H_POS 304
#define CROSS_MAX_Y_POS 224

//default pos: 0,0 is screen center. Y axis is upside down.
#define CROSS_DEFAULT_X 200
#define CROSS_DEFAULT_Y 50

void idPlayer::UpdateCrosshairPos( void ) {
	if( !cursor ){ return; }

	if( skipMouseUpd ) { skipMouseUpd = false; return; }
 
	if( reqDefaultCrossPos ) { 
		if( ( usercmd.mx != oldMouseX ) || ( usercmd.my != oldMouseY ) ){ //turn off only after the first change
			reqDefaultCrossPos = false;
		}
		
		cposx = (viewPos <= 0) ? CROSS_DEFAULT_X : -CROSS_DEFAULT_X; //<= 0 faces right. > 0 faces left 
		cposy = CROSS_DEFAULT_Y;

		cursor->SetStateInt( "cposx", cposx );
		cursor->SetStateInt( "cposy", cposy );

		//gameLocal.Printf("reqDefaultCrossPos\n");
		return;
	}

	if ( usercmd.mx != oldMouseX ) {
		cposx += (usercmd.mx - oldMouseX); 
		//oldMouseX = usercmd.mx;
		if( cposx > CROSS_MAX_H_POS ){ cposx = CROSS_MAX_H_POS; } else if( cposx < -CROSS_MAX_H_POS ){ cposx = -CROSS_MAX_H_POS; }
		cursor->SetStateInt( "cposx", cposx );
		//gameLocal.Printf("deltax: %d\n", (usercmd.mx - oldMouseX) );
	}

	if ( usercmd.my != oldMouseY ) {
		cposy += (usercmd.my - oldMouseY);
		//oldMouseY = usercmd.my;
		if( cposy > CROSS_MAX_Y_POS ){ cposy = CROSS_MAX_Y_POS; } else if( cposy < -CROSS_MAX_Y_POS ){ cposy = -CROSS_MAX_Y_POS; }
		cursor->SetStateInt( "cposy", cposy );
		//gameLocal.Printf("deltay: %d\n", (usercmd.my - oldMouseY) );
	}	
	
	//gameLocal.Printf("cposx: %d, cposy: %d\n", cposx, cposy );
	//gameLocal.Printf("mx: %hd, my: %hd\n", usercmd.mx , usercmd.my);	
}

void idPlayer::UpdateAimFromCrosshair( void ) {
	float offsetY, temp;

	//-- calculate the pitch --
	if( !renderView ){
		viewAngles.pitch = 0.0f; //flat
		return;
	}
	
	//-- X offset (float, idPlayer::coffx ) -- (horizontal)
	if( cameraSettings.lockYaxis ){ //X offset is != 0 only when Y camera is fixed at a certain pos
		coffx = (0.93f * (renderView->vieworg.y - firstPersonViewOrigin.y)); 
	}else{
		coffx = 0.0f;
	}

	// -- Y offset (float, local var) -- (vertical)
	offsetY = (renderView->vieworg.z - firstPersonViewOrigin.z);
	if ( weapon.GetEntity() ) {
		offsetY += weapon.GetEntity()->GetMouseAimOffsetY(); //per-weapon offset
	}

	//fix for fov != 90 - just a bad approximation, but it's acceptable
	if( renderView->fov_x != 90.0f ){
		offsetY *= 95.0f/renderView->fov_x; 
		coffx *= 90.0f/renderView->fov_x;
	}

	//fix for distance != 350
	/*
	if( cameraSettings.distance != 350.0f ){
		temp = 350.0f/cameraSettings.distance; 
		offsetY *= temp; 
		coffx *= temp;
	}*/

	temp = (firstPersonViewOrigin.x - renderView->vieworg.x); //renderView's X should be < than player's one because camera is on his left
	if( temp != 350.0f ){
		temp = 350.0f/temp; 
		offsetY *= temp; 
		coffx *= temp;
	}
	

	//-- from pos to angle --

	//X delta 
	temp = idMath::Fabs( ((float)cposx) - coffx );
	if( temp == 0.0f ){ temp = 0.01f; } //avoid dbz error if X delta = 0
	
	// Y/X deltas ratio 
	temp = ( ((float)cposy) - offsetY ) / temp;
	
	//from ratio to angle
	viewAngles.pitch = RAD2DEG( idMath::ATan16( temp ) ); // we don't need to clamp the pitch for an arctg

	//todo: comment this out - it's just for debug
	if( cursor ){
		cursor->SetStateInt( "g_debugWeapon", g_debugWeapon.GetInteger() );
		if ( g_debugWeapon.GetBool() ) { 
			cursor->SetStateInt( "coffsety", (int) offsetY );	//debug only
			cursor->SetStateInt( "coffsetx", (int) coffx );	//debug only
			//gameLocal.Printf("coffsety: %d, coffsetx: %d\n", (int) offsetY, coffx );
			//gameLocal.Printf("viewAngles.pitch: %f, da %f\n", viewAngles.pitch, temp);
		}
	}
}
#endif
//ivan end


/*
================
idPlayer::UpdateViewAngles
================
*/
void idPlayer::UpdateViewAngles( void ) {
	//int i; rev 2019 commented out in rivensin
	idAngles delta;
	idAngles modelang; //ivan
	float deltaModelYaw; //ivan

	if ( !noclip && ( gameLocal.inCinematic || privateCameraView || gameLocal.GetCamera() || influenceActive == INFLUENCE_LEVEL2 || objectiveSystemOpen ) ) {
		// no view changes at all, but we still want to update the deltas or else when
		// we get out of this mode, our view will snap to a kind of random angle
		UpdateDeltaViewAngles( viewAngles );
		return;
	}

	// if dead
	if ( health <= 0 ) {
		if ( pm_thirdPersonDeath.GetBool() ) {
			viewAngles.roll = 0.0f;
			viewAngles.pitch = 30.0f;
		} else {
			viewAngles.roll = 40.0f;
			viewAngles.pitch = -15.0f;
		}
		return;
	}
	
	//ivan start

#ifdef SHOW_MOVING_CROSSHAIR
	if( g_mouselook.GetBool() ){  //always allow the crosshair to move if g_mouselook 1
		UpdateCrosshairPos();
		oldMouseX = usercmd.mx;
		oldMouseY = usercmd.my;
	}
#endif

	//---------------------
	// CASE 1/2: forcedMov look
	//---------------------

	if ( forcedMovState != FORCEDMOVE_STATE_DISABLED ){ //was: !isXlocked
		forcedMovDelta.Normalize();
		viewAngles = forcedMovDelta.ToAngles();
	} 
	//CASE 1/2 end
	
	//---------------------
	// CASE 2/2: X locked look 
	//---------------------
	
	else{ 
		
		//---------------------
		// CASE 2A: mouse-based aim
		//---------------------
		
		if (g_mouselook.GetBool()){  
			//ivan end 

#ifdef SHOW_MOVING_CROSSHAIR
			// aim: mouse-based mode start 

			/*
			UpdateCrosshairPos();

			//always upd old mouse pos
			oldMouseX = usercmd.mx;
			oldMouseY = usercmd.my;
			*/

			UpdateAimFromCrosshair(); 

			//aim: mouse-based mode end

			//upd direction (left or right)
			if( !keep_walk_dir || save_walk_dir ){ // do this only if we are updating the dir
				if( cposx >= coffx ){ //right
					viewPos = -VIEWPOS_LEFT_MAX; //viewAngles.yaw = -90;
				} else { //left
					viewPos = VIEWPOS_LEFT_MAX;//viewAngles.yaw = 90;
				}
			}
			//upd direction end

#else
			// aim: mouse-based mode start 

			// circularly clamp the angles with deltas
			for (int i = 0; i < 3; i++ ) { //un noted change from original sdk
				cmdAngles[i] = SHORT2ANGLE( usercmd.angles[i] );
				if ( influenceActive == INFLUENCE_LEVEL3 ) {
					viewAngles[i] += idMath::ClampFloat( -1.0f, 1.0f, idMath::AngleDelta( idMath::AngleNormalize180( SHORT2ANGLE( usercmd.angles[i]) + deltaViewAngles[i] ) , viewAngles[i] ) );
				} else {
					viewAngles[i] = idMath::AngleNormalize180( SHORT2ANGLE( usercmd.angles[i]) + deltaViewAngles[i] );
				}
			}

			if ( !centerView.IsDone( gameLocal.time ) ) {
				viewAngles.pitch = centerView.GetCurrentValue(gameLocal.time);
			}

			// clamp the pitch
			if ( noclip ) {
				if ( viewAngles.pitch > 89.0f ) {
					// don't let the player look down more than 89 degrees while noclipping
					viewAngles.pitch = 89.0f;
				} else if ( viewAngles.pitch < -89.0f ) {
					// don't let the player look up more than 89 degrees while noclipping
					viewAngles.pitch = -89.0f;
				}
			} else {
				if ( viewAngles.pitch > pm_maxviewpitch.GetFloat() ) {
					// don't let the player look down enough to see the shadow of his (non-existant) feet
					viewAngles.pitch = pm_maxviewpitch.GetFloat();
				} else if ( viewAngles.pitch < pm_minviewpitch.GetFloat() ) {
					// don't let the player look up more than 89 degrees
					viewAngles.pitch = pm_minviewpitch.GetFloat();
				}
			}


			//aim: mouse-based mode end //un noted change from original sdk

			//upd direction (left or right)
			if( !keep_walk_dir || save_walk_dir ){ // do this only if we are updating the dir

				if( usercmd.forwardmove > 0 ){
					viewPos = -VIEWPOS_LEFT_MAX;
				}else if( usercmd.forwardmove < 0 ){
					viewPos = VIEWPOS_LEFT_MAX;
				}else{
					//gameLocal.Printf("viewAngles.pitch: %f\n", viewAngles.pitch);
					if( ( viewAngles.pitch < -PITCH_MOUSE_ALLOWED ) || ( viewAngles.pitch > PITCH_MOUSE_ALLOWED ) ){
						i = usercmd.mx - oldMouseX; //X mouse delta
						if( i > 0){
							if(viewPos >= -VIEWPOS_LEFT_MAX + VIEWPOS_MOUSE_STEP ){
								viewPos -= VIEWPOS_MOUSE_STEP;
							}
							//gameLocal.Printf("to right - viewPos: %f\n",viewPos);
						} else if( i < 0){
							if(viewPos <= VIEWPOS_LEFT_MAX - VIEWPOS_MOUSE_STEP ){
								viewPos += VIEWPOS_MOUSE_STEP;
							}
							//gameLocal.Printf("to left - viewPos: %f\n",viewPos);
						}
					}
				}
			}//upd direction end
#endif

		//---------------------
		// CASE 2B: contra style aim
		//---------------------

		}else{
			//aim: contra style mode start
			if ( ( ( oldButtons ^ usercmd.buttons ) & BUTTON_6 ) || ( ( oldButtons ^ usercmd.buttons ) & BUTTON_7 ) || fw_toggled ){ //something has changed: start transition
				if ( usercmd.buttons & BUTTON_6 ){ //up					
					if( usercmd.forwardmove ){
						//centerView.Init( gameLocal.time, LOOK_BLEND_TIME, viewAngles.pitch, -35.0f );
						viewAngles.pitch =  -35.0f;	//rev 2020 instant aim changing					
					}else{
						//centerView.Init( gameLocal.time, LOOK_BLEND_TIME, viewAngles.pitch, -89.0f );
						viewAngles.pitch =  -89.0f; //rev 2020 instant aim changing
					}
				} else if ( usercmd.buttons & BUTTON_7 ){ //down		
					if( usercmd.forwardmove ){
						//centerView.Init( gameLocal.time, LOOK_BLEND_TIME, viewAngles.pitch, 35.0f );
						viewAngles.pitch =  35.0f;	//rev 2020 instant aim changing							
					}else{
						//centerView.Init( gameLocal.time, LOOK_BLEND_TIME, viewAngles.pitch, 89.0f  );
						viewAngles.pitch =  89.0f;	//rev 2020 instant aim changing	
					}
				} else if( viewAngles.pitch != 0.0f ){ //none of them pressed
					//centerView.Init( gameLocal.time, LOOK_BLEND_TIME, viewAngles.pitch, 0 );
					viewAngles.pitch =  0;	//rev 2020 instant aim changing	
				}
			}else if ( !centerView.IsDone( gameLocal.time ) ) { //upd transition, if any
				viewAngles.pitch = centerView.GetCurrentValue(gameLocal.time);
			}
			//aim: contra style mode end

			//upd direction (left or right)
			if( !keep_walk_dir || save_walk_dir ){ // do this only if we are updating the dir
				if( usercmd.forwardmove > 0 ){
					viewPos = -VIEWPOS_LEFT_MAX;
				}else if( usercmd.forwardmove < 0 ){
					viewPos = VIEWPOS_LEFT_MAX;
				}
			}//upd direction end
		}//end contra 

		//---------------------
		// Common stuff for 2A and 2B
		//---------------------
/*
		//upd direction (left or right)
		if( !keep_walk_dir || save_walk_dir ){ // do this only if we are updating the dir

			if( usercmd.forwardmove > 0 ){
				viewPos = -VIEWPOS_LEFT_MAX;
			}else if( usercmd.forwardmove < 0 ){
				viewPos = VIEWPOS_LEFT_MAX;
			}else if (g_mouselook.GetBool()){
				//gameLocal.Printf("viewAngles.pitch: %f\n", viewAngles.pitch);
				if( ( viewAngles.pitch < -PITCH_MOUSE_ALLOWED ) || ( viewAngles.pitch > PITCH_MOUSE_ALLOWED ) ){
					i = usercmd.mx - oldMouseX; //X mouse delta
					if( i > 0){
						if(viewPos >= -VIEWPOS_LEFT_MAX + VIEWPOS_MOUSE_STEP ){
							viewPos -= VIEWPOS_MOUSE_STEP;
						}
						//gameLocal.Printf("to right - viewPos: %f\n",viewPos);
					} else if( i < 0){
						if(viewPos <= VIEWPOS_LEFT_MAX - VIEWPOS_MOUSE_STEP ){
							viewPos += VIEWPOS_MOUSE_STEP;
						}
						//gameLocal.Printf("to left - viewPos: %f\n",viewPos);
					}
				}
			}
		}//upd direction end
*/
		//save the current yaw: it will be used until RUN button is released
		if(save_walk_dir){ 
			save_walk_dir = false;
			if(viewPos <= 0){ //right
				old_viewAngles_yaw = -90;
			} else { //left
				old_viewAngles_yaw = 90;
			}
		}

		//use the saved one or the current one
		fw_inverted = false;
		if( keep_walk_dir ){ //RUN button is pressed
			viewAngles.yaw = old_viewAngles_yaw;

			//swap cmd if we are facing left
			if(usercmd.forwardmove){
				if(old_viewAngles_yaw > 0){
					usercmd.forwardmove = -usercmd.forwardmove;
					fw_inverted = true; //ivan - we need to know if we did this 
				}
			}
		}else{ //RUN button not pressed
			if(viewPos <= 0){ //right
				viewAngles.yaw = -90;
			} else { //left
				viewAngles.yaw = 90;
			}

			//swap cmd if we are facing left
			if(usercmd.forwardmove){
				if(viewPos > 0){ //left
					usercmd.forwardmove = -usercmd.forwardmove;
					fw_inverted = true; //ivan - we need to know if we did this 
				}
			}
		}

		//always upd old mouse pos
		oldMouseX = usercmd.mx;
		oldMouseY = usercmd.my;

		
	} //CASE 2/2 end 	
		
	//common code from here

	UpdateDeltaViewAngles( viewAngles );

	// orient the model towards the direction we're looking
	if( !animBasedMovement ){ //un noted change from original sdk
		//blend start - blend model yaw if needed
		if(blendModelYaw){ //blend 
			modelang = renderEntity.axis.ToAngles(); 
			//gameLocal.Printf( "modelang yaw: %f - viewAngles.yaw: %f\n", modelang.yaw, viewAngles.yaw );
			deltaModelYaw = viewAngles.yaw - modelang.yaw; 
			deltaModelYaw = idMath::AngleNormalize180(deltaModelYaw);
			//gameLocal.Printf( "deltaModelYaw blend %f (normalized)\n", deltaModelYaw );
			if(deltaModelYaw > 1.0f || deltaModelYaw < -1.0f){ //blend again
				SetAngles( idAngles( 0, idMath::AngleNormalize180( modelang.yaw + deltaModelYaw*0.4f ), 0 ) ); //was 0.3 //rev 2019 note rivensin is 0.5f fyi
			}else{ //stop blending 
				SetAngles( idAngles( 0, viewAngles.yaw, 0 ) );
				blendModelYaw = false;
			}
		}else{ //old 
			SetAngles( idAngles( 0, viewAngles.yaw, 0 ) );
		}
	}//...else don't update model angles! 
	//blend end
	
	//ivan end

	// save in the log for analyzing weapon angle offsets
	loggedViewAngles[ gameLocal.framenum & (NUM_LOGGED_VIEW_ANGLES-1) ] = viewAngles;
}

/*
==============
idPlayer::AdjustHeartRate

Player heartrate works as follows

DEF_HEARTRATE is resting heartrate

Taking damage when health is above 75 adjusts heart rate by 1 beat per second
Taking damage when health is below 75 adjusts heart rate by 5 beats per second
Maximum heartrate from damage is MAX_HEARTRATE

Firing a weapon adds 1 beat per second up to a maximum of COMBAT_HEARTRATE

Being at less than 25% stamina adds 5 beats per second up to ZEROSTAMINA_HEARTRATE

All heartrates are target rates.. the heart rate will start falling as soon as there have been no adjustments for 5 seconds
Once it starts falling it always tries to get to DEF_HEARTRATE

The exception to the above rule is upon death at which point the rate is set to DYING_HEARTRATE and starts falling
immediately to zero

Heart rate volumes go from zero ( -40 db for DEF_HEARTRATE to 5 db for MAX_HEARTRATE ) the volume is
scaled linearly based on the actual rate

Exception to the above rule is once the player is dead, the dying heart rate starts at either the current volume if
it is audible or -10db and scales to 8db on the last few beats
==============
*/
void idPlayer::AdjustHeartRate( int target, float timeInSecs, float delay, bool force ) {

	if ( heartInfo.GetEndValue() == target ) {
		return;
	}

	if ( AI_DEAD && !force ) {
		return;
	}

	lastHeartAdjust = gameLocal.time;

	heartInfo.Init( gameLocal.time + delay * 1000, timeInSecs * 1000, heartRate, target );
}

/*
==============
idPlayer::GetBaseHeartRate
==============
*/
int idPlayer::GetBaseHeartRate( void ) {
	int base = idMath::FtoiFast( ( BASE_HEARTRATE + LOWHEALTH_HEARTRATE_ADJ ) - ( (float)health / 100.0f ) * LOWHEALTH_HEARTRATE_ADJ );
	int rate = idMath::FtoiFast( base + ( ZEROSTAMINA_HEARTRATE - base ) * ( 1.0f - stamina / pm_stamina.GetFloat() ) );
	int diff = ( lastDmgTime ) ? gameLocal.time - lastDmgTime : 99999;
	rate += ( diff < 5000 ) ? ( diff < 2500 ) ? ( diff < 1000 ) ? 15 : 10 : 5 : 0;
	return rate;
}

/*
==============
idPlayer::SetCurrentHeartRate
==============
*/
void idPlayer::SetCurrentHeartRate( void ) {

	int base = idMath::FtoiFast( ( BASE_HEARTRATE + LOWHEALTH_HEARTRATE_ADJ ) - ( (float) health / 100.0f ) * LOWHEALTH_HEARTRATE_ADJ );

	if ( PowerUpActive( ADRENALINE )) {
		heartRate = 135;
	} else {
		heartRate = idMath::FtoiFast( heartInfo.GetCurrentValue( gameLocal.time ) );
		int currentRate = GetBaseHeartRate();
		if ( health >= 0 && gameLocal.time > lastHeartAdjust + 2500 ) {
			AdjustHeartRate( currentRate, 2.5f, 0.0f, false );
		}
	}

	int bps = idMath::FtoiFast( 60.0f / heartRate * 1000.0f );
	if ( gameLocal.time - lastHeartBeat > bps ) {
		int dmgVol = DMG_VOLUME;
		int deathVol = DEATH_VOLUME;
		int zeroVol = ZERO_VOLUME;
		float pct = 0.0;
		if ( heartRate > BASE_HEARTRATE && health > 0 ) {
			pct = (float)(heartRate - base) / (MAX_HEARTRATE - base);
			pct *= ((float)dmgVol - (float)zeroVol);
		} else if ( health <= 0 ) {
			pct = (float)(heartRate - DYING_HEARTRATE) / (BASE_HEARTRATE - DYING_HEARTRATE);
			if ( pct > 1.0f ) {
				pct = 1.0f;
			} else if (pct < 0.0f) {
				pct = 0.0f;
			}
			pct *= ((float)deathVol - (float)zeroVol);
		}

		pct += (float)zeroVol;

		if ( pct != zeroVol ) {
			//StartSound( "snd_heartbeat", SND_CHANNEL_HEART, SSF_PRIVATE_SOUND, false, NULL );		//rev 2021 get rid of this
			// modify just this channel to a custom volume
			soundShaderParms_t	parms;
			memset( &parms, 0, sizeof( parms ) );
			parms.volume = pct;
			refSound.referenceSound->ModifySound( SND_CHANNEL_HEART, &parms );
		}

		lastHeartBeat = gameLocal.time;
	}
}

//ivan start

//UpdateAir is not used anymore

/*
==============
idPlayer::UpdateAir
==============
*/

/*
void idPlayer::UpdateAir( void ) {
	if ( health <= 0 ) {
		return;
	}


#ifdef _WATER_PHYSICS
	idPhysics_Player *phys = dynamic_cast<idPhysics_Player *>(this->GetPhysics());
#endif
	

	// see if the player is connected to the info_vacuum
	bool	newAirless = false;

	if ( gameLocal.vacuumAreaNum != -1 ) {
		int	num = GetNumPVSAreas();
		if ( num > 0 ) {
			int		areaNum;

			// if the player box spans multiple areas, get the area from the origin point instead,
			// otherwise a rotating player box may poke into an outside area
			if ( num == 1 ) {
				const int	*pvsAreas = GetPVSAreas();
				areaNum = pvsAreas[0];
			} else {
				areaNum = gameRenderWorld->PointInArea( this->GetPhysics()->GetOrigin() );
			}
			newAirless = gameRenderWorld->AreasAreConnected( gameLocal.vacuumAreaNum, areaNum, PS_BLOCK_AIR );
		}
	}

#ifdef _WATER_PHYSICS
	// check if the player is in water
	if( phys != NULL && phys->GetWaterLevel() >= WATERLEVEL_HEAD )
		newAirless = true;
#endif

	if ( newAirless ) {
		if ( !airless ) {
			StartSound( "snd_decompress", SND_CHANNEL_ANY, SSF_GLOBAL, false, NULL );
			StartSound( "snd_noAir", SND_CHANNEL_BODY2, 0, false, NULL );
			if ( hud ) {
				hud->HandleNamedEvent( "noAir" );
			}
		}
		airTics--;
		if ( airTics < 0 ) {
			airTics = 0;
			// check for damage
			const idDict *damageDef = gameLocal.FindEntityDefDict( "damage_noair", false );
			int dmgTiming = 1000 * ((damageDef) ? damageDef->GetFloat( "delay", "3.0" ) : 3.0f );
			if ( gameLocal.time > lastAirDamage + dmgTiming ) {
				Damage( NULL, NULL, vec3_origin, "damage_noair", 1.0f, 0 );
				lastAirDamage = gameLocal.time;
			}
		}

	} else {
		if ( airless ) {
			StartSound( "snd_recompress", SND_CHANNEL_ANY, SSF_GLOBAL, false, NULL );
			StopSound( SND_CHANNEL_BODY2, false );
			if ( hud ) {
				hud->HandleNamedEvent( "Air" );
			}
		}
		airTics+=2;	// regain twice as fast as lose
		if ( airTics > pm_airTics.GetInteger() ) {
			airTics = pm_airTics.GetInteger();
		}
	}

	airless = newAirless;

	if ( hud ) {
		hud->SetStateInt( "player_air", 100 * airTics / pm_airTics.GetInteger() );
	}
}

*/

#ifdef _WATER_PHYSICS

/*
==============
idPlayer::UpdateWaterAir
==============
*/

void idPlayer::UpdateWaterAir( void ) {	
	if ( health <= 0 ) {
		return;
	}

	bool isUnderWater;

	// check if the player is in water
	isUnderWater = ( physicsObj.GetWaterLevel() >= WATERLEVEL_HEAD ? true : false );

	if ( isUnderWater ) {
		if ( !airless ) {
			//StartSound( "snd_water_enter", SND_CHANNEL_ANY, SSF_GLOBAL, false, NULL ); //note: the splash sound is already played by the func_liquid
			StartSound( "snd_water_swim", SND_CHANNEL_BODY2, 0, false, NULL );
			if ( hud ) {
				hud->HandleNamedEvent( "noAir" ); //turn on
			}
		}
		airTics--;
		if ( airTics < 0 ) {
			airTics = 0;
			// check for damage
			const idDict *damageDef = gameLocal.FindEntityDefDict( "damage_noair", false );
			int dmgTiming = 1000 * ((damageDef) ? damageDef->GetFloat( "delay", "3.0" ) : 3.0f );
			if ( gameLocal.time > lastAirDamage + dmgTiming ) {
				Damage( NULL, NULL, vec3_origin, "damage_noair", 1.0f, 0 );
				lastAirDamage = gameLocal.time;
			}
		}

	} else {
		if ( airless ) {
			if ( airTics < pm_airTics.GetInteger() * 0.9f ) { //do this only if are below 90% air
				StartSound( "snd_water_exit", SND_CHANNEL_ANY, SSF_GLOBAL, false, NULL );
			}
			StopSound( SND_CHANNEL_BODY2, false );
		}
	
		// regain faster
		if ( airTics < pm_airTics.GetInteger() ) {
			airTics += 10;
			if ( airTics >= pm_airTics.GetInteger() ) { //max reached
				airTics = pm_airTics.GetInteger(); //make sure it doesn't exceed the max
				if ( hud ) {
					hud->HandleNamedEvent( "Air" ); //turn off
				}
			}
		}
	}

	airless = isUnderWater;

	if ( hud ) {
		hud->SetStateInt( "player_air", 100 * airTics / pm_airTics.GetInteger() );
	}
}

#endif

//ivan end

/*
==============
idPlayer::AddGuiPDAData
==============
*/
int idPlayer::AddGuiPDAData( const declType_t dataType, const char *listName, const idDeclPDA *src, idUserInterface *gui ) {
	int c, i;
	idStr work;
	if ( dataType == DECL_EMAIL ) {
		c = src->GetNumEmails();
		for ( i = 0; i < c; i++ ) {
			const idDeclEmail *email = src->GetEmailByIndex( i );
			if ( email == NULL ) {
				work = va( "-\tEmail %d not found\t-", i );
			} else {
				work = email->GetFrom();
				work += "\t";
				work += email->GetSubject();
				work += "\t";
				work += email->GetDate();
			}
			gui->SetStateString( va( "%s_item_%i", listName, i ), work );
		}
		return c;
	} else if ( dataType == DECL_AUDIO ) {
		c = src->GetNumAudios();
		for ( i = 0; i < c; i++ ) {
			const idDeclAudio *audio = src->GetAudioByIndex( i );
			if ( audio == NULL ) {
				work = va( "Audio Log %d not found", i );
			} else {
				work = audio->GetAudioName();
			}
			gui->SetStateString( va( "%s_item_%i", listName, i ), work );
		}
		return c;
	} else if ( dataType == DECL_VIDEO ) {
		c = inventory.videos.Num();
		for ( i = 0; i < c; i++ ) {
			const idDeclVideo *video = GetVideo( i );
			if ( video == NULL ) {
				work = va( "Video CD %s not found", inventory.videos[i].c_str() );
			} else {
				work = video->GetVideoName();
			}
			gui->SetStateString( va( "%s_item_%i", listName, i ), work );
		}
		return c;
	}
	return 0;
}

/*
==============
idPlayer::GetPDA
==============
*/
const idDeclPDA *idPlayer::GetPDA( void ) const {
	if ( inventory.pdas.Num() ) {
		return static_cast< const idDeclPDA* >( declManager->FindType( DECL_PDA, inventory.pdas[ 0 ] ) );
	} else {
		return NULL;
	}
}


/*
==============
idPlayer::GetVideo
==============
*/
const idDeclVideo *idPlayer::GetVideo( int index ) {
	if ( index >= 0 && index < inventory.videos.Num() ) {
		return static_cast< const idDeclVideo* >( declManager->FindType( DECL_VIDEO, inventory.videos[index], false ) );
	}
	return NULL;
}


/*
==============
idPlayer::UpdatePDAInfo
==============
*/
void idPlayer::UpdatePDAInfo( bool updatePDASel ) {
	int j, sel;

	if ( objectiveSystem == NULL ) {
		return;
	}

	assert( hud );

	int currentPDA = objectiveSystem->State().GetInt( "listPDA_sel_0", "0" );
	if ( currentPDA == -1 ) {
		currentPDA = 0;
	}

	if ( updatePDASel ) {
		objectiveSystem->SetStateInt( "listPDAVideo_sel_0", 0 );
		objectiveSystem->SetStateInt( "listPDAEmail_sel_0", 0 );
		objectiveSystem->SetStateInt( "listPDAAudio_sel_0", 0 );
	}

	if ( currentPDA > 0 ) {
		currentPDA = inventory.pdas.Num() - currentPDA;
	}

	// Mark in the bit array that this pda has been read
	if ( currentPDA < 128 ) {
		inventory.pdasViewed[currentPDA >> 5] |= 1 << (currentPDA & 31);
	}

	pdaAudio = "";
	pdaVideo = "";
	pdaVideoWave = "";
	idStr name, data, preview, info, wave;
	for ( j = 0; j < MAX_PDAS; j++ ) {
		objectiveSystem->SetStateString( va( "listPDA_item_%i", j ), "" );
	}
	for ( j = 0; j < MAX_PDA_ITEMS; j++ ) {
		objectiveSystem->SetStateString( va( "listPDAVideo_item_%i", j ), "" );
		objectiveSystem->SetStateString( va( "listPDAAudio_item_%i", j ), "" );
		objectiveSystem->SetStateString( va( "listPDAEmail_item_%i", j ), "" );
		objectiveSystem->SetStateString( va( "listPDASecurity_item_%i", j ), "" );
	}
	for ( j = 0; j < inventory.pdas.Num(); j++ ) {

		const idDeclPDA *pda = static_cast< const idDeclPDA* >( declManager->FindType( DECL_PDA, inventory.pdas[j], false ) );

		if ( pda == NULL ) {
			continue;
		}

		int index = inventory.pdas.Num() - j;
		if ( j == 0 ) {
			// Special case for the first PDA
			index = 0;
		}

		if ( j != currentPDA && j < 128 && inventory.pdasViewed[j >> 5] & (1 << (j & 31)) ) {
			// This pda has been read already, mark in gray
			objectiveSystem->SetStateString( va( "listPDA_item_%i", index), va(S_COLOR_GRAY "%s", pda->GetPdaName()) );
		} else {
			// This pda has not been read yet
			objectiveSystem->SetStateString( va( "listPDA_item_%i", index), pda->GetPdaName() );
		}

		const char *security = pda->GetSecurity();
		if ( j == currentPDA || (currentPDA == 0 && security && *security ) ) {
			if ( *security == 0 ) {
				security = common->GetLanguageDict()->GetString( "#str_00066" );
			}
			objectiveSystem->SetStateString( "PDASecurityClearance", security );
		}

		if ( j == currentPDA ) {

			objectiveSystem->SetStateString( "pda_icon", pda->GetIcon() );
			objectiveSystem->SetStateString( "pda_id", pda->GetID() );
			objectiveSystem->SetStateString( "pda_title", pda->GetTitle() );

			if ( j == 0 ) {
				// Selected, personal pda
				// Add videos
				if ( updatePDASel || !inventory.pdaOpened ) {
					objectiveSystem->HandleNamedEvent( "playerPDAActive" );
					objectiveSystem->SetStateString( "pda_personal", "1" );
					inventory.pdaOpened = true;
				}
				objectiveSystem->SetStateString( "pda_location", hud->State().GetString("location") );
				objectiveSystem->SetStateString( "pda_name", cvarSystem->GetCVarString( "ui_name") );
				AddGuiPDAData( DECL_VIDEO, "listPDAVideo", pda, objectiveSystem );
				sel = objectiveSystem->State().GetInt( "listPDAVideo_sel_0", "0" );
				const idDeclVideo *vid = NULL;
				if ( sel >= 0 && sel < inventory.videos.Num() ) {
					vid = static_cast< const idDeclVideo * >( declManager->FindType( DECL_VIDEO, inventory.videos[ sel ], false ) );
				}
				if ( vid ) {
					pdaVideo = vid->GetRoq();
					pdaVideoWave = vid->GetWave();
					objectiveSystem->SetStateString( "PDAVideoTitle", vid->GetVideoName() );
					objectiveSystem->SetStateString( "PDAVideoVid", vid->GetRoq() );
					objectiveSystem->SetStateString( "PDAVideoIcon", vid->GetPreview() );
					objectiveSystem->SetStateString( "PDAVideoInfo", vid->GetInfo() );
				} else {
					//FIXME: need to precache these in the player def
					objectiveSystem->SetStateString( "PDAVideoVid", "sound/vo/video/welcome.tga" );
					objectiveSystem->SetStateString( "PDAVideoIcon", "sound/vo/video/welcome.tga" );
					objectiveSystem->SetStateString( "PDAVideoTitle", "" );
					objectiveSystem->SetStateString( "PDAVideoInfo", "" );
				}
			} else {
				// Selected, non-personal pda
				// Add audio logs
				if ( updatePDASel ) {
					objectiveSystem->HandleNamedEvent( "playerPDANotActive" );
					objectiveSystem->SetStateString( "pda_personal", "0" );
					inventory.pdaOpened = true;
				}
				objectiveSystem->SetStateString( "pda_location", pda->GetPost() );
				objectiveSystem->SetStateString( "pda_name", pda->GetFullName() );
				int audioCount = AddGuiPDAData( DECL_AUDIO, "listPDAAudio", pda, objectiveSystem );
				objectiveSystem->SetStateInt( "audioLogCount", audioCount );
				sel = objectiveSystem->State().GetInt( "listPDAAudio_sel_0", "0" );
				const idDeclAudio *aud = NULL;
				if ( sel >= 0 ) {
					aud = pda->GetAudioByIndex( sel );
				}
				if ( aud ) {
					pdaAudio = aud->GetWave();
					objectiveSystem->SetStateString( "PDAAudioTitle", aud->GetAudioName() );
					objectiveSystem->SetStateString( "PDAAudioIcon", aud->GetPreview() );
					objectiveSystem->SetStateString( "PDAAudioInfo", aud->GetInfo() );
				} else {
					objectiveSystem->SetStateString( "PDAAudioIcon", "sound/vo/video/welcome.tga" );
					objectiveSystem->SetStateString( "PDAAutioTitle", "" );
					objectiveSystem->SetStateString( "PDAAudioInfo", "" );
				}
			}
			// add emails
			name = "";
			data = "";
			int numEmails = pda->GetNumEmails();
			if ( numEmails > 0 ) {
				AddGuiPDAData( DECL_EMAIL, "listPDAEmail", pda, objectiveSystem );
				sel = objectiveSystem->State().GetInt( "listPDAEmail_sel_0", "-1" );
				if ( sel >= 0 && sel < numEmails ) {
					const idDeclEmail *email = pda->GetEmailByIndex( sel );
					name = email->GetSubject();
					data = email->GetBody();
				}
			}
			objectiveSystem->SetStateString( "PDAEmailTitle", name );
			objectiveSystem->SetStateString( "PDAEmailText", data );
		}
	}
	if ( objectiveSystem->State().GetInt( "listPDA_sel_0", "-1" ) == -1 ) {
		objectiveSystem->SetStateInt( "listPDA_sel_0", 0 );
	}
	objectiveSystem->StateChanged( gameLocal.time );
}

/*
==============
idPlayer::TogglePDA
==============
*/
void idPlayer::TogglePDA( void ) {
	if ( objectiveSystem == NULL ) {
		return;
	}

	if ( inventory.pdas.Num() == 0 ) {
		ShowTip( spawnArgs.GetString( "text_infoTitle" ), spawnArgs.GetString( "text_noPDA" ), true );
		return;
	}

	assert( hud );

	if ( !objectiveSystemOpen ) {
		int j, c = inventory.items.Num();
		objectiveSystem->SetStateInt( "inv_count", c );
		for ( j = 0; j < MAX_INVENTORY_ITEMS; j++ ) {
			objectiveSystem->SetStateString( va( "inv_name_%i", j ), "" );
			objectiveSystem->SetStateString( va( "inv_icon_%i", j ), "" );
			objectiveSystem->SetStateString( va( "inv_text_%i", j ), "" );
		}
		for ( j = 0; j < c; j++ ) {
			idDict *item = inventory.items[j];
			if ( !item->GetBool( "inv_pda" ) ) {
				const char *iname = item->GetString( "inv_name" );
				const char *iicon = item->GetString( "inv_icon" );
				const char *itext = item->GetString( "inv_text" );
				objectiveSystem->SetStateString( va( "inv_name_%i", j ), iname );
				objectiveSystem->SetStateString( va( "inv_icon_%i", j ), iicon );
				objectiveSystem->SetStateString( va( "inv_text_%i", j ), itext );
				const idKeyValue *kv = item->MatchPrefix( "inv_id", NULL );
				if ( kv ) {
					objectiveSystem->SetStateString( va( "inv_id_%i", j ), kv->GetValue() );
				}
			}
		}

		for ( j = 0; j < MAX_WEAPONS; j++ ) {
			const char *weapnum = va( "def_weapon%d", j );
			const char *hudWeap = va( "weapon%d", j );
			int weapstate = 0;
			if ( inventory.weapons & ( 1 << j ) ) {
				const char *weap = spawnArgs.GetString( weapnum );
				if ( weap && *weap ) {
					weapstate++;
				}
			}
			objectiveSystem->SetStateInt( hudWeap, weapstate );
		}

		objectiveSystem->SetStateInt( "listPDA_sel_0", inventory.selPDA );
		objectiveSystem->SetStateInt( "listPDAVideo_sel_0", inventory.selVideo );
		objectiveSystem->SetStateInt( "listPDAAudio_sel_0", inventory.selAudio );
		objectiveSystem->SetStateInt( "listPDAEmail_sel_0", inventory.selEMail );
		UpdatePDAInfo( false );
		UpdateObjectiveInfo();
		objectiveSystem->Activate( true, gameLocal.time );
		hud->HandleNamedEvent( "pdaPickupHide" );
		hud->HandleNamedEvent( "videoPickupHide" );
	} else {
		inventory.selPDA = objectiveSystem->State().GetInt( "listPDA_sel_0" );
		inventory.selVideo = objectiveSystem->State().GetInt( "listPDAVideo_sel_0" );
		inventory.selAudio = objectiveSystem->State().GetInt( "listPDAAudio_sel_0" );
		inventory.selEMail = objectiveSystem->State().GetInt( "listPDAEmail_sel_0" );
		objectiveSystem->Activate( false, gameLocal.time );
	}
	objectiveSystemOpen ^= 1;
}

/*
==============
idPlayer::ToggleScoreboard
==============
*/
void idPlayer::ToggleScoreboard( void ) {
	scoreBoardOpen ^= 1;
}

/*
==============
idPlayer::Spectate
==============
*/
void idPlayer::Spectate( bool spectate ) {
	idBitMsg	msg;
	byte		msgBuf[MAX_EVENT_PARAM_SIZE];

	// track invisible player bug
	// all hiding and showing should be performed through Spectate calls
	// except for the private camera view, which is used for teleports
	assert( ( teleportEntity.GetEntity() != NULL ) || ( IsHidden() == spectating ) );

	if ( spectating == spectate ) {
		return;
	}

	spectating = spectate;

	if ( gameLocal.isServer ) {
		msg.Init( msgBuf, sizeof( msgBuf ) );
		msg.WriteBits( spectating, 1 );
		ServerSendEvent( EVENT_SPECTATE, &msg, false, -1 );
	}

	if ( spectating ) {
		// join the spectators
		ClearPowerUps();
		spectator = this->entityNumber;
		Init( false );
		StopRagdoll();
		SetPhysics( &physicsObj );
		physicsObj.DisableClip();
		Hide();
		Event_DisableWeapon();
		if ( hud ) {
			hud->HandleNamedEvent( "aim_clear" );
			MPAimFadeTime = 0;
		}
	} else {
		// put everything back together again
		currentWeapon = -1;	// to make sure the def will be loaded if necessary
		Show();
		Event_EnableWeapon();
	}
	SetClipModel();
}

/*
==============
idPlayer::SetClipModel
==============
*/
void idPlayer::SetClipModel( void ) {
	idBounds bounds;

	if ( spectating ) {
		bounds = idBounds( vec3_origin ).Expand( pm_spectatebbox.GetFloat() * 0.5f );
	} else {
		bounds[0].Set( -pm_bboxwidth.GetFloat() * 0.5f, -pm_bboxwidth.GetFloat() * 0.5f, 0 );
		bounds[1].Set( pm_bboxwidth.GetFloat() * 0.5f, pm_bboxwidth.GetFloat() * 0.5f, pm_normalheight.GetFloat() );
	}
	// the origin of the clip model needs to be set before calling SetClipModel
	// otherwise our physics object's current origin value gets reset to 0
	idClipModel *newClip;
	if ( pm_usecylinder.GetBool() ) {
		newClip = new idClipModel( idTraceModel( bounds, 8 ) );
		newClip->Translate( physicsObj.PlayerGetOrigin() );
		physicsObj.SetClipModel( newClip, 1.0f );
	} else {
		newClip = new idClipModel( idTraceModel( bounds ) );
		newClip->Translate( physicsObj.PlayerGetOrigin() );
		physicsObj.SetClipModel( newClip, 1.0f );
	}
}

/*
==============
idPlayer::UseVehicle
==============
*/
void idPlayer::UseVehicle( void ) {
	trace_t	trace;
	idVec3 start, end;
	idEntity *ent;

	if ( GetBindMaster() && GetBindMaster()->IsType( idAFEntity_Vehicle::Type ) ) {
		Show();
		static_cast<idAFEntity_Vehicle*>(GetBindMaster())->Use( this );
	} else {
		start = GetEyePosition();
		end = start + viewAngles.ToForward() * 80.0f;
		gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_RENDERMODEL, this );
		if ( trace.fraction < 1.0f ) {
			ent = gameLocal.entities[ trace.c.entityNum ];
			if ( ent && ent->IsType( idAFEntity_Vehicle::Type ) ) {
				Hide();
				static_cast<idAFEntity_Vehicle*>(ent)->Use( this );
			}
		}
	}
}

/*
==============
idPlayer::PerformImpulse
==============
*/
void idPlayer::PerformImpulse( int impulse ) {
	int prevIdealWeap; //un noted change from original sdk

	if ( gameLocal.isClient ) {
		idBitMsg	msg;
		byte		msgBuf[MAX_EVENT_PARAM_SIZE];

		assert( entityNumber == gameLocal.localClientNum );
		msg.Init( msgBuf, sizeof( msgBuf ) );
		msg.BeginWriting();
		msg.WriteBits( impulse, 6 );
		ClientSendEvent( EVENT_IMPULSE, &msg );
	}

	//ivan start
	//no weapon changes while full body anim is playing
	if( force_torso_override ){ return; } 

	//if( forcedMovState != FORCEDMOVE_STATE_DISABLED || blendModelYaw ){ return; } //no weapon changes while forced mov
	//ivan end	

	if ( impulse == IMPULSE_21 || (impulse >= IMPULSE_23 && impulse <= IMPULSE_27) ) {

		int weap;
		if ( spawnArgs.GetInt ( va("impulse%d", impulse), "0", weap ) ) {
			prevIdealWeap = idealWeapon;
			SelectWeapon( weap, false);
			if( idealWeapon != prevIdealWeap ) {
				quickWeapon = prevIdealWeap;
			}
		}
		return;
	}

	if ( impulse >= IMPULSE_0 && impulse <= IMPULSE_12 ) {
		WeaponToggle_t* weaponToggle; //un noted change from original sdk
// This loop works as a small bug fix for toggle weapons -By Clone JC Denton
// It simply increments the impulse value if there are multiple weapons under one weapon slot.

		for (int i=0; i<impulse; i++) {
			if (weaponToggles.Get(va("weapontoggle%d", i), &weaponToggle))
				impulse += weaponToggle->toggleList.Num() - 1;
		}

		prevIdealWeap = idealWeapon;
		SelectWeapon( impulse, false, true);
		if( idealWeapon != prevIdealWeap ) {
			quickWeapon = prevIdealWeap;
		}
		return;
	}


	switch( impulse ) {
		case IMPULSE_13: {
			//ivan start
			//was: Reload();
			Interact();
			//ivan end
			break;
						 }
		case IMPULSE_14: {
			prevIdealWeap = idealWeapon; //un noted change from original sdk
			NextWeapon();
			if( idealWeapon != prevIdealWeap ) { //un noted change from original sdk
				quickWeapon = prevIdealWeap;
			}
			break;
						 }
		case IMPULSE_15: {
			prevIdealWeap = idealWeapon; //un noted change from original sdk
			PrevWeapon();
			if( idealWeapon != prevIdealWeap ) { //un noted change from original sdk
				quickWeapon = prevIdealWeap;
			}
			break;
						 }
		case IMPULSE_16: {  //un noted change from original sdk
//rev 2020 never drop weapons
			//ivan start - drop the weapon only if it is in a slot.
			//if( currentSlot >= 0 ){ 
			//	DropWeapon( false );
			//}
			break;
			
			/*
			//This was Denton's code. We don't need it.
			//was:
			// New, for half-life style quick weapon
			if ( quickWeapon == -1 ) {
				return;
			}
			prevIdealWeap = idealWeapon;
			SelectWeapon( quickWeapon, false );
			quickWeapon = prevIdealWeap;
			break;
			*/

			//ivan end
						 }
		case IMPULSE_17: {
			if ( gameLocal.isClient || entityNumber == gameLocal.localClientNum ) {
				gameLocal.mpGame.ToggleReady();
			}
			break;
						 }
		case IMPULSE_18: {
			centerView.Init(gameLocal.time, 200, viewAngles.pitch, 0);
			break;
						 }
		case IMPULSE_19: {
			// when we're not in single player, IMPULSE_19 is used for showScores
			// otherwise it opens the pda
			if ( !gameLocal.isMultiplayer ) {
				if ( objectiveSystemOpen ) {
					TogglePDA();
				} else if ( weapon_pda >= 0 ) {
					SelectWeapon( weapon_pda, true );
				}
			}
			break;
						 }
		case IMPULSE_20: {
			if ( gameLocal.isClient || entityNumber == gameLocal.localClientNum ) {
				gameLocal.mpGame.ToggleTeam();
			}
			break;
						 }
		case IMPULSE_22: {
			if ( gameLocal.isClient || entityNumber == gameLocal.localClientNum ) {
				gameLocal.mpGame.ToggleSpectate();
			}
			break;
						 }
		case IMPULSE_28: {
			if ( gameLocal.isClient || entityNumber == gameLocal.localClientNum ) {
				gameLocal.mpGame.CastVote( gameLocal.localClientNum, true );
			}
			break;
						 }
		case IMPULSE_29: {
			if ( gameLocal.isClient || entityNumber == gameLocal.localClientNum ) {
				gameLocal.mpGame.CastVote( gameLocal.localClientNum, false );
			}
			break;
						 }
						 /*	case IMPULSE_30: { //un noted change from original sdk
						 WeaponSpecialFunction ();
						 break;
						 }*/
		case IMPULSE_40: {
			UseVehicle();
			break;
						 }
	}
}

bool idPlayer::HandleESC( void ) {
	if ( gameLocal.inCinematic ) {
		return SkipCinematic();
	}

	if ( objectiveSystemOpen ) {
		TogglePDA();
		return true;
	}

	skipMouseUpd = true; //ivan
	return false;
}

bool idPlayer::SkipCinematic( void ) {
	StartSound( "snd_skipcinematic", SND_CHANNEL_ANY, 0, false, NULL );
	return gameLocal.SkipCinematic();
}

/*
==============
idPlayer::EvaluateControls
==============
*/
void idPlayer::EvaluateControls( void ) {
	// check for respawning
	if ( health <= 0 ) {
		if ( ( gameLocal.time > minRespawnTime ) && ( usercmd.buttons & BUTTON_ATTACK ) ) {
			forceRespawn = true;
		} else if ( gameLocal.time > maxRespawnTime ) {
			forceRespawn = true;
		}
	}

	// in MP, idMultiplayerGame decides spawns
	if ( forceRespawn && !gameLocal.isMultiplayer && !g_testDeath.GetBool() ) {
		//ivan start - respawn or end 
		//gameLocal.Printf( "forceRespawn = true; \n");

		if( numLives > 0 ){
			numLives--;
			Hq2QuickRespawn();
		}else{
			// in single player, we let the session handle restarting the level or loading a game
			gameLocal.sessionCommand = "died";
		}
		//ivan end
	}

	if ( ( usercmd.flags & UCF_IMPULSE_SEQUENCE ) != ( oldFlags & UCF_IMPULSE_SEQUENCE ) ) {
		PerformImpulse( usercmd.impulse );
	}

	scoreBoardOpen = ( ( usercmd.buttons & BUTTON_SCORES ) != 0 || forceScoreBoard );

	oldFlags = usercmd.flags;

	AdjustSpeed();

	// update the viewangles
	UpdateViewAngles();
}

/*
==============
idPlayer::AdjustSpeed
==============
*/
void idPlayer::AdjustSpeed( void ) {
	float speed;
	float rate;

	if ( spectating ) {
		speed = pm_spectatespeed.GetFloat();
		//bobFrac = 0.0f; //un noted change from original sdk
	} else if ( noclip ) {
		speed = pm_noclipspeed.GetFloat();
		//bobFrac = 0.0f; //un noted change from original sdk
	} 
	/*
	//ivan - commented out: run disabled
	else if ( !physicsObj.OnLadder() && ( usercmd.buttons & BUTTON_RUN ) && ( usercmd.forwardmove || usercmd.rightmove ) && ( usercmd.upmove >= 0 ) ) {
		if ( !gameLocal.isMultiplayer && !physicsObj.IsCrouching() && !PowerUpActive( ADRENALINE ) ) {
			stamina -= MS2SEC( gameLocal.msec );
		}
		if ( stamina < 0 ) {
			stamina = 0;
		}
		if ( ( !pm_stamina.GetFloat() ) || ( stamina > pm_staminathreshold.GetFloat() ) ) {
			bobFrac = 1.0f;
		} else if ( pm_staminathreshold.GetFloat() <= 0.0001f ) {
			bobFrac = 0.0f;
		} else {
			bobFrac = stamina / pm_staminathreshold.GetFloat();
		}
		speed = pm_walkspeed.GetFloat() * ( 1.0f - bobFrac ) + pm_runspeed.GetFloat() * bobFrac;
	}
	*/
	else {
		rate = pm_staminarate.GetFloat();

		// increase 25% faster when not moving
		if ( ( usercmd.forwardmove == 0 ) && ( usercmd.rightmove == 0 ) && ( !physicsObj.OnLadder() || ( usercmd.upmove == 0 ) ) ) {
			rate *= 1.25f;
		}

		stamina += rate * MS2SEC( gameLocal.msec );
		if ( stamina > pm_stamina.GetFloat() ) {
			stamina = pm_stamina.GetFloat();
		}
		speed = pm_walkspeed.GetFloat();
		//bobFrac = 0.0f; //un noted change from original sdk
	}

	speed *= PowerUpModifier(SPEED);

	if ( influenceActive == INFLUENCE_LEVEL3 ) {
		speed *= 0.33f;
	}

	physicsObj.SetSpeed( speed, pm_crouchspeed.GetFloat() );
}

/*
==============
idPlayer::AdjustBodyAngles
==============
*/
void idPlayer::AdjustBodyAngles( void ) {
	idMat3	lookAxis; //rev 2019 note this commented out in rivensin fyi
	idMat3	legsAxis;
	bool	blend;
	float	diff;
	float	frac;
	float	upBlend;
	float	forwardBlend;
	float	downBlend;

	upBlend = forwardBlend = downBlend = 0.0f; // DG: just make sure they're initialized

	if ( health < 0 ) {
		return;
	}

	if(force_torso_override){ //ivan 
		legsForward = false;
		AI_TURN_LEFT = false; //anim legs in script
		AI_TURN_RIGHT = false; //anim legs in script
		
		//test only
		if(legsYaw > 0.1f){
			legsYaw = legsYaw * 0.9f;
			legsAxis = idAngles( 0.0f, legsYaw, 0.0f ).ToMat3();
			animator.SetJointAxis( hipJoint, JOINTMOD_WORLD, legsAxis );
		}
	}else{ //old

		blend = true;

		if ( !physicsObj.HasGroundContacts() ) {
			idealLegsYaw = 0.0f;
			legsForward = true;
		} else if ( usercmd.forwardmove < 0 ) {
			idealLegsYaw = idMath::AngleNormalize180( idVec3( -usercmd.forwardmove, usercmd.rightmove, 0.0f ).ToYaw() );
			legsForward = false;
		} else if ( usercmd.forwardmove > 0 ) {
			idealLegsYaw = idMath::AngleNormalize180( idVec3( usercmd.forwardmove, -usercmd.rightmove, 0.0f ).ToYaw() );
			legsForward = true;
		} else if ( ( usercmd.rightmove != 0 ) && physicsObj.IsCrouching() ) {
			if ( !legsForward ) {
				idealLegsYaw = idMath::AngleNormalize180( idVec3( idMath::Abs( usercmd.rightmove ), usercmd.rightmove, 0.0f ).ToYaw() );
			} else {
				idealLegsYaw = idMath::AngleNormalize180( idVec3( idMath::Abs( usercmd.rightmove ), -usercmd.rightmove, 0.0f ).ToYaw() );
			}
		} else if ( usercmd.rightmove != 0 ) {
			idealLegsYaw = 0.0f;
			legsForward = true;
		} else {
			legsForward = true;
			diff = idMath::Fabs( idealLegsYaw - legsYaw );
			idealLegsYaw = idealLegsYaw - idMath::AngleNormalize180( viewAngles.yaw - oldViewYaw );
			if ( diff < 0.1f ) {
				legsYaw = idealLegsYaw;
				blend = false;
			}
		}

		if ( !physicsObj.IsCrouching() ) {
			legsForward = true;
		}

		oldViewYaw = viewAngles.yaw;

		AI_TURN_LEFT = false;
		AI_TURN_RIGHT = false;
		if ( idealLegsYaw < -45.0f ) {
			idealLegsYaw = 0;
			AI_TURN_RIGHT = true;
			blend = true;
		} else if ( idealLegsYaw > 45.0f ) {
			idealLegsYaw = 0;
			AI_TURN_LEFT = true;
			blend = true;
		}

		if ( blend ) {
			legsYaw = legsYaw * 0.9f + idealLegsYaw * 0.1f;
		}
		legsAxis = idAngles( 0.0f, legsYaw, 0.0f ).ToMat3();
		animator.SetJointAxis( hipJoint, JOINTMOD_WORLD, legsAxis );

		// calculate the blending between down, straight, and up
		frac = viewAngles.pitch / 90.0f;
		if ( frac > 0.0f ) {
			downBlend		= frac;
			forwardBlend	= 1.0f - frac;
			upBlend			= 0.0f;
		} else {
			downBlend		= 0.0f;
			forwardBlend	= 1.0f + frac;
			upBlend			= -frac;
		}
    }

	if(force_torso_override){ //ivan 
		//no looking up/down
		downBlend		= 0.0f;
		forwardBlend	= 1.0f;
		upBlend			= 0.0f;
	}

	animator.CurrentAnim( ANIMCHANNEL_TORSO )->SetSyncedAnimWeight( 0, downBlend );
	animator.CurrentAnim( ANIMCHANNEL_TORSO )->SetSyncedAnimWeight( 1, forwardBlend );
	animator.CurrentAnim( ANIMCHANNEL_TORSO )->SetSyncedAnimWeight( 2, upBlend );

	if(force_torso_override){ //ivan
		//otherwise we cannot get the real animation offset in idAnimBlend::BlendDelta
		downBlend		= 1.0f; //needed!!
		forwardBlend	= 0.0f; //1.0f; also works?
		upBlend			= 0.0f; //1.0f; also works?
	}

	animator.CurrentAnim( ANIMCHANNEL_LEGS )->SetSyncedAnimWeight( 0, downBlend );
	animator.CurrentAnim( ANIMCHANNEL_LEGS )->SetSyncedAnimWeight( 1, forwardBlend );
	animator.CurrentAnim( ANIMCHANNEL_LEGS )->SetSyncedAnimWeight( 2, upBlend );
}

/*
==============
idPlayer::InitAASLocation
==============
*/
void idPlayer::InitAASLocation( void ) {
	int		i;
	int		num;
	idVec3	size;
	idBounds bounds;
	idAAS	*aas;
	idVec3	origin;

	GetFloorPos( 64.0f, origin );

	num = gameLocal.NumAAS();
	aasLocation.SetGranularity( 1 );
	aasLocation.SetNum( num );
	for( i = 0; i < aasLocation.Num(); i++ ) {
		aasLocation[ i ].areaNum = 0;
		aasLocation[ i ].pos = origin;
		aas = gameLocal.GetAAS( i );
		if ( aas && aas->GetSettings() ) {
			size = aas->GetSettings()->boundingBoxes[0][1];
			bounds[0] = -size;
			size.z = 32.0f;
			bounds[1] = size;

			aasLocation[ i ].areaNum = aas->PointReachableAreaNum( origin, bounds, AREA_REACHABLE_WALK );
		}
	}
}

/*
==============
idPlayer::SetAASLocation
==============
*/
void idPlayer::SetAASLocation( void ) {
	int		i;
	int		areaNum;
	idVec3	size;
	idBounds bounds;
	idAAS	*aas;
	idVec3	origin;

	if ( !GetFloorPos( 64.0f, origin ) ) {
		return;
	}

	for( i = 0; i < aasLocation.Num(); i++ ) {
		aas = gameLocal.GetAAS( i );
		if ( !aas ) {
			continue;
		}

		size = aas->GetSettings()->boundingBoxes[0][1];
		bounds[0] = -size;
		size.z = 32.0f;
		bounds[1] = size;

		areaNum = aas->PointReachableAreaNum( origin, bounds, AREA_REACHABLE_WALK );
		if ( areaNum ) {
			aasLocation[ i ].pos = origin;
			aasLocation[ i ].areaNum = areaNum;
		}
	}
}

/*
==============
idPlayer::GetAASLocation
==============
*/
void idPlayer::GetAASLocation( idAAS *aas, idVec3 &pos, int &areaNum ) const {
	int i;

	if ( aas != NULL ) {
		for( i = 0; i < aasLocation.Num(); i++ ) {
			if ( aas == gameLocal.GetAAS( i ) ) {
				areaNum = aasLocation[ i ].areaNum;
				pos = aasLocation[ i ].pos;
				return;
			}
		}
	}

	areaNum = 0;
	pos = physicsObj.GetOrigin();
}

/*
==============
idPlayer::Move
==============
*/
void idPlayer::Move( void ) {
	float newEyeOffset;
	idVec3 oldOrigin;
	idVec3 oldVelocity;
	idVec3 pushVelocity;

	idVec3 delta; //ivan

	// save old origin and velocity for crashlanding
	oldOrigin = physicsObj.GetOrigin();
	oldVelocity = physicsObj.GetLinearVelocity();
	pushVelocity = physicsObj.GetPushedLinearVelocity();

	// set physics variables
	physicsObj.SetMaxStepHeight( pm_stepsize.GetFloat() );
	physicsObj.SetMaxJumpHeight( pm_jumpheight.GetFloat() );
	physicsObj.SetMaxJumpHeight( pm_jumpheight.GetFloat() ); //un noted change from original sdk

	//ivan start
	physicsObj.SetFwInverted( fw_inverted );

	waitForDamage  = spawnArgs.GetInt( "waitfordamage" ); //rev 2020 used for pass through enemies during invulnerability
	noDamage  = spawnArgs.GetBool( "nodamage" ); //rev 2020 used for pass through enemies
			
	//ivan start
	if ( animMoveNoGravity ){
		physicsObj.SetContents( CONTENTS_BODY );
		physicsObj.SetMovementType( PM_FREEZE );
	}else if ( animMoveType ){
	    physicsObj.SetContents( CONTENTS_BODY );
		if( animMoveType == ANIMMOVE_ALWAYS){
			physicsObj.SetMovementType( PM_ANIM_ALWAYS );
			GetMoveDelta( viewAxis, viewAxis, delta ); //to: check what happens to viewAxis 
			physicsObj.SetDelta( delta );
		}else if( animMoveType == ANIMMOVE_GROUND){
			physicsObj.SetMovementType( PM_ANIM_GROUND );
			GetMoveDelta( viewAxis, viewAxis, delta ); //to: check what happens to viewAxis  
			physicsObj.SetDelta( delta );
		}else if( animMoveType == ANIMMOVE_PHYSICS){
			physicsObj.SetMovementType( PM_PHYSICS_ONLY );
		}else { //that is, ANIMMOVE_FREEZE
			physicsObj.SetMovementType( PM_FREEZE );
		}
	} else 
	//ivan end

	if ( noclip ) {
		physicsObj.SetContents( 0 );
		physicsObj.SetMovementType( PM_NOCLIP );
	} else if ( spectating ) {
		physicsObj.SetContents( 0 );
		physicsObj.SetMovementType( PM_SPECTATOR );
	} else if ( health <= 0 ) {
		physicsObj.SetContents( CONTENTS_CORPSE | CONTENTS_MONSTERCLIP );
		physicsObj.SetMovementType( PM_DEAD );
	} else if ( gameLocal.inCinematic || gameLocal.GetCamera() || privateCameraView || ( influenceActive == INFLUENCE_LEVEL2 ) ) {
		physicsObj.SetContents( CONTENTS_BODY );
		physicsObj.SetMovementType( PM_FREEZE );
	} 
	//ivan start
	else if ( animBasedMovement ){
	    physicsObj.SetContents( CONTENTS_BODY );
		physicsObj.SetMovementType( PM_ANIM_CROUCH );
		GetMoveDelta( viewAxis, viewAxis, delta ); //no changes to Axis since no turning is allowed
		physicsObj.SetDelta( delta );
	}	
	//ivan en

//rev 2020		allow the player to pass through enemies when invulnerability is active.  also prevents standing on heads.	
	else if ( waitForDamage == 1 ) {
	physicsObj.SetContents( CONTENTS_CORPSE );
	physicsObj.SetMovementType( PM_NORMAL );
		}
	else if ( noDamage ) {
	physicsObj.SetContents( CONTENTS_CORPSE );
	physicsObj.SetMovementType( PM_NORMAL );
		}
//rev 2020 end
	else {
		physicsObj.SetContents( CONTENTS_BODY );
		physicsObj.SetMovementType( PM_NORMAL );
	}

	if ( spectating ) {
		physicsObj.SetClipMask( MASK_DEADSOLID );
	//} else if ( health <= 0 ) {
	} else if ( health <= 0 || noDamage  || waitForDamage == 1 ) {
		physicsObj.SetClipMask( MASK_DEADSOLID );
	} else {
		physicsObj.SetClipMask( MASK_PLAYERSOLID );
	}

	physicsObj.SetDebugLevel( g_debugMove.GetBool() );
	physicsObj.SetPlayerInput( usercmd, viewAngles );

	// FIXME: physics gets disabled somehow
	BecomeActive( TH_PHYSICS );
	RunPhysics();

	//ivan start - lock the X position 
	if( noclip ){ //fix for noclip
		if( usercmd.forwardmove == 0 ){
			if( ( usercmd.buttons & BUTTON_7 ) != 0 ){ fastXpos -= 5; }	//aim down
			if( ( usercmd.buttons & BUTTON_6 ) != 0 ){ fastXpos += 5; }	//aim up
		}
		physicsObj.SetOrigin( idVec3( fastXpos, physicsObj.GetOrigin().y, physicsObj.GetOrigin().z ) ); 
	}else if( isXlocked ){
		physicsObj.SetOrigin( idVec3( fastXpos, physicsObj.GetOrigin().y, physicsObj.GetOrigin().z ) ); 
	}else{
		fastXpos = physicsObj.GetOrigin().x; //always upd so others can easily know our X pos
	}
	//ivan end

	// update our last valid AAS location for the AI
	SetAASLocation();

	if ( spectating ) {
		newEyeOffset = 0.0f;
	} else if ( health <= 0 ) {
		newEyeOffset = pm_deadviewheight.GetFloat();
	} 

	else if ( physicsObj.IsCrouching() ) {
		newEyeOffset = pm_crouchviewheight.GetFloat();
	} 
	
	else if ( GetBindMaster() && GetBindMaster()->IsType( idAFEntity_Vehicle::Type ) ) {
		newEyeOffset = 0.0f;
	} else {
		newEyeOffset = pm_normalviewheight.GetFloat();
	}

	if ( EyeHeight() != newEyeOffset ) {
		if ( spectating ) {
			SetEyeHeight( newEyeOffset );
		} else {
			// smooth out duck height changes
			SetEyeHeight( EyeHeight() * pm_crouchrate.GetFloat() + newEyeOffset * ( 1.0f - pm_crouchrate.GetFloat() ) );
		}
	}

	if ( noclip || gameLocal.inCinematic || ( influenceActive == INFLUENCE_LEVEL2 ) ) {
		AI_CROUCH	= false;
		AI_ONGROUND	= ( influenceActive == INFLUENCE_LEVEL2 );
		AI_ONLADDER	= false;
		AI_JUMP		= false;
	} else {
		AI_CROUCH	= physicsObj.IsCrouching();
		AI_ONGROUND	= physicsObj.HasGroundContacts();
		AI_ONLADDER	= physicsObj.OnLadder();
		AI_JUMP		= physicsObj.HasJumped();

//REV 2020 START.  THIS ISN'T NEEDED.  CHECKED IN PLAYER::COLLIDE
/*
//rev 2018 start. damage the player when touching enemies from above
		// check if we're standing on top of a monster and give a push if we are
		idEntity *groundEnt = physicsObj.GetGroundEntity();
		if ( groundEnt && groundEnt->IsType( idAI::Type ) ) {
			idVec3 vel = physicsObj.GetLinearVelocity();
			if ( vel.ToVec2().LengthSqr() < 0.1f ) {
				vel.ToVec2() = physicsObj.GetOrigin().ToVec2() - groundEnt->GetPhysics()->GetAbsBounds().GetCenter().ToVec2();
				vel.ToVec2().NormalizeFast();
				vel.ToVec2() *= pm_walkspeed.GetFloat();
			} else {
				// give em a push in the direction they're going
				vel *= 1.1f;
				touchofdeathy = groundEnt->spawnArgs.GetInt( "touchofdeath" );
				if ( touchofdeathy > 0 ) {					
				Damage( NULL, NULL, vec3_origin, "damage_touchofdeath", 1.0f, 0 );				
					}
//rev 2018 end
			}
			physicsObj.SetLinearVelocity( vel );
		
		}
*/
//REV 2020 END		
	}

	if ( AI_JUMP ) {
		// bounce the view weapon
		loggedAccel_t	*acc = &loggedAccel[currentLoggedAccel&(NUM_LOGGED_ACCELS-1)];
		currentLoggedAccel++;
		acc->time = gameLocal.time;
		acc->dir[2] = 200;
		acc->dir[0] = acc->dir[1] = 0;
	}

	if ( AI_ONLADDER ) {
		int old_rung = oldOrigin.z / LADDER_RUNG_DISTANCE;
		int new_rung = physicsObj.GetOrigin().z / LADDER_RUNG_DISTANCE;

		if ( old_rung != new_rung ) {
			StartSound( "snd_stepladder", SND_CHANNEL_ANY, 0, false, NULL );
		}
	}

	BobCycle( pushVelocity );
	CrashLand( oldOrigin, oldVelocity );
}

/*
==============
idPlayer::UpdateHud
==============
*/
void idPlayer::UpdateHud( void ) {
	idPlayer *aimed;

	if ( !hud ) {
		return;
	}

	if ( entityNumber != gameLocal.localClientNum ) {
		return;
	}
	
	int c = inventory.pickupItemNames.Num();
	if ( c > 0 ) {
		if ( gameLocal.time > inventory.nextItemPickup ) {
			if ( inventory.nextItemPickup && gameLocal.time - inventory.nextItemPickup > 2000 ) {
				inventory.nextItemNum = 1;
			}
			int i;
			for ( i = 0; i < 5 && i < c; i++ ) {
				hud->SetStateString( va( "itemtext%i", inventory.nextItemNum ), inventory.pickupItemNames[0].name );
				hud->SetStateString( va( "itemicon%i", inventory.nextItemNum ), inventory.pickupItemNames[0].icon );
				hud->HandleNamedEvent( va( "itemPickup%i", inventory.nextItemNum++ ) );
				inventory.pickupItemNames.RemoveIndex( 0 );
				if (inventory.nextItemNum == 1 ) {
					inventory.onePickupTime = gameLocal.time;
				} else	if ( inventory.nextItemNum > 5 ) {
					inventory.nextItemNum = 1;
					inventory.nextItemPickup = inventory.onePickupTime + 2000;
				} else {
					inventory.nextItemPickup = gameLocal.time + 400;
				}
			}
		}
	}

	if ( gameLocal.realClientTime == lastMPAimTime ) {
		if ( MPAim != -1 && gameLocal.gameType == GAME_TDM
			&& gameLocal.entities[ MPAim ] && gameLocal.entities[ MPAim ]->IsType( idPlayer::Type )
			&& static_cast< idPlayer * >( gameLocal.entities[ MPAim ] )->team == team ) {
				aimed = static_cast< idPlayer * >( gameLocal.entities[ MPAim ] );
				hud->SetStateString( "aim_text", gameLocal.userInfo[ MPAim ].GetString( "ui_name" ) );
				hud->SetStateFloat( "aim_color", aimed->colorBarIndex );
				hud->HandleNamedEvent( "aim_flash" );
				MPAimHighlight = true;
				MPAimFadeTime = 0;	// no fade till loosing focus
		} else if ( MPAimHighlight ) {
			hud->HandleNamedEvent( "aim_fade" );
			MPAimFadeTime = gameLocal.realClientTime;
			MPAimHighlight = false;
		}
	}
	if ( MPAimFadeTime ) {
		assert( !MPAimHighlight );
		if ( gameLocal.realClientTime - MPAimFadeTime > 2000 ) {
			MPAimFadeTime = 0;
		}
	}

	hud->SetStateInt( "g_showProjectilePct", g_showProjectilePct.GetInteger() );
	if ( numProjectilesFired ) {
		hud->SetStateString( "projectilepct", va( "Hit %% %.1f", ( (float) numProjectileHits / numProjectilesFired ) * 100 ) );
	} else {
		hud->SetStateString( "projectilepct", "Hit % 0.0" );
	}

	if ( isLagged && gameLocal.isMultiplayer && gameLocal.localClientNum == entityNumber ) {
		hud->SetStateString( "hudLag", "1" );
	} else {
		hud->SetStateString( "hudLag", "0" );
	}
}

/*
==============
idPlayer::UpdateDeathSkin
==============
*/
void idPlayer::UpdateDeathSkin( bool state_hitch ) {
	if ( !( gameLocal.isMultiplayer || g_testDeath.GetBool() ) ) {
		return;
	}
	if ( health <= 0 ) {
		if ( !doingDeathSkin ) {
			deathClearContentsTime = spawnArgs.GetInt( "deathSkinTime" );
			doingDeathSkin = true;
			renderEntity.noShadow = true;
			if ( state_hitch ) {
				renderEntity.shaderParms[ SHADERPARM_TIME_OF_DEATH ] = gameLocal.time * 0.001f - 2.0f;
			} else {
				renderEntity.shaderParms[ SHADERPARM_TIME_OF_DEATH ] = gameLocal.time * 0.001f;
			}
			UpdateVisuals();
		}

		// wait a bit before switching off the content
		if ( deathClearContentsTime && gameLocal.time > deathClearContentsTime ) {
			SetCombatContents( false );
			deathClearContentsTime = 0;
		}
	} else {
		renderEntity.noShadow = false;
		renderEntity.shaderParms[ SHADERPARM_TIME_OF_DEATH ] = 0.0f;
		UpdateVisuals();
		doingDeathSkin = false;
	}
}

/*
==============
idPlayer::StartFxOnBone
==============
*/
void idPlayer::StartFxOnBone( const char *fx, const char *bone ) {
	idVec3 offset;
	idMat3 axis;
	jointHandle_t jointHandle = GetAnimator()->GetJointHandle( bone );

	if ( jointHandle == INVALID_JOINT ) {
		gameLocal.Printf( "Cannot find bone %s\n", bone );
		return;
	}

	if ( GetAnimator()->GetJointTransform( jointHandle, gameLocal.time, offset, axis ) ) {
		offset = GetPhysics()->GetOrigin() + offset * GetPhysics()->GetAxis();
		axis = axis * GetPhysics()->GetAxis();
	}

	idEntityFx::StartFx( fx, &offset, &axis, this, true );
}


//ivan start
/*
==============
idPlayer::SetSlideMoveState
==============
*/
void idPlayer::SetSlideMoveState( void ) {
	//gameLocal.Printf("SetSlideMoveState\n");
	const function_t *newstate = GetScriptFunction( "SlideMove" );
	if ( newstate ) {
			SetState( newstate );
			UpdateScript();
	}
}
//ivan end

//REV 2020 START DASH MOVE
/*
==============
idPlayer::SetChargeMoveState
==============
*/
void idPlayer::SetChargeMoveState( void ) {
	//const function_t *newstate = GetScriptFunction( "ChargeMoveF" );
	const function_t *newstate = NULL; // DG: make sure this is initialized, so if(newstate) check below works reliably
	if (chargeDir == 1){
		newstate = GetScriptFunction( "ChargeMoveU" );		
	}
	else if (chargeDir == 2){
		newstate = GetScriptFunction( "ChargeMoveUf" );		
	}
	else if (chargeDir == 3){
		newstate = GetScriptFunction( "ChargeMoveF" );		
	}
	else if (chargeDir == 4){
		newstate = GetScriptFunction( "ChargeMoveDf" );		
	}
	else if (chargeDir == 5){
		newstate = GetScriptFunction( "ChargeMoveD" );		
	}
	
	if ( newstate ) {
		lastChargeTime = gameLocal.time;	// start the clock to possibly give charges if not at maximum
		SetState( newstate );
		UpdateScript();
	}
}
//REV 2020 charge

/*
==============
idPlayer::Think

Called every tic for each player
==============
*/
void idPlayer::Think( void ) {
	renderEntity_t *headRenderEnt;

	UpdatePlayerIcons();
	
	// latch button actions
	oldButtons = usercmd.buttons;

	// ivan start 

	/* old code:
	// grab out usercmd
	usercmd_t oldCmd = usercmd;
	usercmd = gameLocal.usercmds[ entityNumber ];
	buttonMask &= usercmd.buttons;
	usercmd.buttons &= ~buttonMask;
	
	if ( gameLocal.inCinematic && gameLocal.skipCinematic ) {
		return;
	}
	*/
	
// rev 2020 check the last time we were damaged, if too long, run a script to enable damage again.
		waitForDamage  = spawnArgs.GetInt( "waitfordamage" );
		if (( gameLocal.time > lastDmgTime + 1500 ) && ( waitForDamage > 0 ) ) {	//check wait for damage or the slide move will not work
			SetState( "BlinkOff" ); 
			UpdateScript();
		}
		
	// grab out usercmd
	usercmd_t oldCmd = usercmd;
	usercmd = gameLocal.usercmds[ entityNumber ];
	
	//cinematic
	if ( gameLocal.inCinematic && gameLocal.skipCinematic ) {
		return;
	}

	//upd forced movement
	if( forcedMovState != FORCEDMOVE_STATE_DISABLED ){
		UpdForcedMov();  //upd dir and destination 
	}

	//upd camera distance in case someone or something changed the Cvar
	if ( pm_thirdPersonRange.IsModified() ){ //this allows the user or scripts to update the distance thanks to the Cvar. 
		//gameLocal.Printf("pm_thirdPersonRange.IsModified()\n");
		SetCameraDistance( pm_thirdPersonRange.GetFloat(), true );
		pm_thirdPersonRange.ClearModified(); //remove the modified flag
	}

	//upd camera height in case someone or somethingchanged the Cvar
	if ( pm_thirdPersonHeight.IsModified() ){ //this allows the user or scripts to update the distance thanks to the Cvar. 
		//gameLocal.Printf("pm_thirdPersonHeight.IsModified()\n");
		SetCameraHeight( pm_thirdPersonHeight.GetFloat(), true );
		pm_thirdPersonHeight.ClearModified(); //remove the modified flag
	}

	/*
	if ( pm_thirdPersonZ.IsModified() ){ //this allows the user or scripts to update the distance thanks to the Cvar. 
		gameLocal.Printf("pm_thirdPersonZ.IsModified()\n");
		if ( pm_thirdPersonZ.GetBool() ) {
			UnlockZCamera();
		} else {
			LockZCamera( renderView->vieworg.z );
		}
		pm_thirdPersonZ.ClearModified(); //remove the modified flag
	}
	*/
	

	//force/allow/disable inputs
	if( inhibitInputTime > gameLocal.time ){ //used by teleport
		usercmd.forwardmove		= 0;
		usercmd.upmove			= 0; //disable jumping/crouching
		usercmd.buttons			= 0; //disable all buttons (ex: fire)
	}else if( blendModelYaw ){ //don't give control until blending is done
		usercmd.forwardmove		= 0;
		usercmd.upmove			= 0; //disable jumping/crouching
		//usercmd.buttons		= 0; //disable all buttons (ex: fire)
	}else if( forcedMovState != FORCEDMOVE_STATE_DISABLED ){ //forced movement
		usercmd.forwardmove		= ( forcedMovState == FORCEDMOVE_STATE_WAITING || forcedMovState == FORCEDMOVE_STATE_BLOCKED )? 0 : 127; //forcedMovWaiting ? 0 : 127; //wait or move fw
		if( forcedMovTotalForce ){
			usercmd.upmove		= 0; //disable jumping/crouching
			usercmd.buttons		= 0; //disable all buttons (ex: fire)
		}
	}

	//always remove right/left buttons
	usercmd.rightmove = 0;

	//buttonMask
	buttonMask &= usercmd.buttons;
	usercmd.buttons &= ~buttonMask;

	// new run button feature
	if ( ( usercmd.buttons ^ oldCmd.buttons ) & BUTTON_RUN ) {
		if ( usercmd.buttons & BUTTON_RUN ) {
			save_walk_dir = true;
			keep_walk_dir = true;
		}else{
			keep_walk_dir = false;	
		}
	}

	//compare current fw with the old one.
	if( fw_inverted ){
		fw_toggled = ( usercmd.forwardmove != -oldCmd.forwardmove );
	}else{
		fw_toggled = ( usercmd.forwardmove != oldCmd.forwardmove );
	}

//REV 2020 CHARGE ATTACK START
	if ( chargeAmount < 2 ) {	// 2 is the maximum amount we can do in a row
		//if ( gameLocal.time > lastChargeTime + spawnArgs.GetInt( "charge_time" ) ) {	//was 1500
		if ( gameLocal.time > lastChargeTime + 4000 ) {	//was 1500
			if ( chargeAmount == 1 ) {				
				spawnArgs.Set( "charge_amount", "2" );	//this method proved be the most reliable in setting the amount.
				//gameLocal.Printf(" c1 + 1 ");
				lastChargeTime = gameLocal.time;	//added here to reset the time of when to check if we need another or else it will spam it.
			} else {
				spawnArgs.Set( "charge_amount", "1" );
				//gameLocal.Printf(" c0 + 1 ");
				lastChargeTime = gameLocal.time;
			}
		}
	}	

	if ( chargeAmount > 0 && !force_torso_override && ( forcedMovState == FORCEDMOVE_STATE_DISABLED ) ) {
//Set the direction to charge in. 1 up, 2 upward, 3 forward, 4 downward, 5 down
//charge attacking forward MUST come after the others or it might over ride upward/downward charge attacks
		if( ( usercmd.buttons & BUTTON_5 ) && ( usercmd.buttons & BUTTON_6 ) && ( !usercmd.forwardmove ) ){ 
			chargeDir = 1;
			SetChargeMoveState();
		}
		else if( ( usercmd.forwardmove ) && ( usercmd.buttons & BUTTON_5 ) && ( usercmd.buttons & BUTTON_6 ) ){ 
			chargeDir = 2;
			SetChargeMoveState();
		}

		if( !AI_ONGROUND	) {		//only do downward, and down when in the air	
			if( ( usercmd.forwardmove ) && ( usercmd.buttons & BUTTON_5 ) && ( usercmd.buttons & BUTTON_7 ) ){ 
				chargeDir = 4;
				SetChargeMoveState();
			}
			
			else if( ( usercmd.buttons & BUTTON_5 ) && ( usercmd.buttons & BUTTON_7 ) && ( !usercmd.forwardmove ) ){ 
				chargeDir = 5;
				SetChargeMoveState();
			}
		}
		if( ( usercmd.forwardmove ) && ( usercmd.buttons & BUTTON_5 ) ){ 
			chargeDir = 3;
			SetChargeMoveState();
		}		
		else { 
			chargeDir = 0; //reset it just incase
		}
	}
//REV 2020 CHARGE ATTACK END	
	
	//onground and ready --> do stuff
	if( AI_ONGROUND && !force_torso_override && ( forcedMovState == FORCEDMOVE_STATE_DISABLED ) ){ //note: we check "forcedMovState" instead of "isXlocked" so player can still be unlocked and do stuff 

#ifdef AUTOUPD_RESPAWN_POS
		//-- safe respawn pos --
		if( ( nextRespPosTime < gameLocal.time ) && ( lastDmgTime < gameLocal.time - 2000 ) && ( health > 0 ) ){
			nextRespPosTime = gameLocal.time + 1000; //1 sec

			//respawn pos is the last known safe position which is at least 200 units far from player.
			if( (GetPhysics()->GetOrigin() - tempRespawnPos ).LengthFast() > 200 ){
				lastCheckPoint.spawnPos = tempRespawnPos;
				tempRespawnPos = GetPhysics()->GetOrigin();
				//gameRenderWorld->DebugBounds( colorBlue, idBounds( vec3_origin ).Expand( 15.0f ) , lastCheckPoint.spawnPos, 5000 );
				//gameLocal.Printf("lastCheckPoint.spawnPos upd: %s\n", lastCheckPoint.spawnPos.ToString() );
			}
		}
#endif

		// -- slide start --
		if( ( oldCmd.upmove < 0 ) && ( usercmd.upmove > 0 ) && ( landTime + 200 < gameLocal.time ) ){ //AI_CROUCH
				SetSlideMoveState();
		}

		else{ //don't interact or crouch if slided

			// -- interact upd/down start  -- 
			if( ( usercmd.forwardmove == 0 ) && ( usercmd.upmove == 0 ) ){
				if( ( usercmd.buttons & BUTTON_7 ) != 0 ){ //aim down
					if( InteractTouchingTriggers( INTERACT_DOWN ) ){ //try interact
						usercmd.upmove = 0; //dont' jump or crouch
						usercmd.buttons &= ~BUTTON_7; //remove aim down button - don't aim down
						inhibitAimCrouchTime = gameLocal.time + 100;
					}
				}else if( ( usercmd.buttons & BUTTON_6 ) != 0 ){ //aim up
					if( InteractTouchingTriggers( INTERACT_UP ) ){ //try interact
						usercmd.upmove = 0; //dont' jump or crouch
						usercmd.buttons &= ~BUTTON_6; //remove aim up button - don't aim up
						inhibitAimCrouchTime = gameLocal.time + 100;
					}
				}
			}
			// -- interact upd/down end --  

			// -- crouch with "aim down" start -- 
			if( (( usercmd.buttons & BUTTON_7 ) != 0 )			//aim down
				&& ( inhibitAimCrouchTime < gameLocal.time )	//crouch allowed
				&& ( usercmd.upmove == 0 )){					
//rev 2019 start allow aiming downwards contra style when not using mouse aim.						
				if ( g_mouselook.GetBool() ){
					usercmd.upmove = -127; //force crouch
					usercmd.buttons &= ~BUTTON_7; //remove aim down button - don't aim down					
				} else if (!usercmd.forwardmove){
					usercmd.upmove = -127; //force crouch
					usercmd.buttons &= ~BUTTON_7; //remove aim down button - don't aim down
					}
//rev 2019 end					
				}
			// -- crouch with "aim down" end -- 

		} //end else slide
	} //end if onground and ready

	//ivan end

	// clear the ik before we do anything else so the skeleton doesn't get updated twice
	walkIK.ClearJointMods();

	// if this is the very first frame of the map, set the delta view angles
	// based on the usercmd angles
	if ( !spawnAnglesSet && ( gameLocal.GameState() != GAMESTATE_STARTUP ) ) {
		spawnAnglesSet = true;
		SetViewAngles( spawnAngles );
		oldFlags = usercmd.flags;
	}

	if ( objectiveSystemOpen || gameLocal.inCinematic || influenceActive ) { 
		if ( objectiveSystemOpen && AI_PAIN ) {
			TogglePDA();
		}
		usercmd.forwardmove = 0;
		usercmd.rightmove = 0;
		usercmd.upmove = 0;
	}	
	
	// log movement changes for weapon bobbing effects
	//ivan start
	//was: if ( usercmd.forwardmove != oldCmd.forwardmove ) {
	if(fw_toggled){
	//ivan end
		loggedAccel_t	*acc = &loggedAccel[currentLoggedAccel&(NUM_LOGGED_ACCELS-1)];
		currentLoggedAccel++;
		acc->time = gameLocal.time;
		acc->dir[0] = usercmd.forwardmove - oldCmd.forwardmove;
		acc->dir[1] = acc->dir[2] = 0;
	}

	if ( usercmd.rightmove != oldCmd.rightmove ) {
		loggedAccel_t	*acc = &loggedAccel[currentLoggedAccel&(NUM_LOGGED_ACCELS-1)];
		currentLoggedAccel++;
		acc->time = gameLocal.time;
		acc->dir[1] = usercmd.rightmove - oldCmd.rightmove;
		acc->dir[0] = acc->dir[2] = 0;
	}

	// freelook centering
	if ( ( usercmd.buttons ^ oldCmd.buttons ) & BUTTON_MLOOK ) {
		centerView.Init( gameLocal.time, 200, viewAngles.pitch, 0 );
	}

	// zooming
	if ( ( usercmd.buttons ^ oldCmd.buttons ) & BUTTON_ZOOM ) {
		if ( ( usercmd.buttons & BUTTON_ZOOM ) && weapon.GetEntity() ) {
			zoomFov.Init( gameLocal.time, 200.0f, CalcFov( false ), weapon.GetEntity()->GetZoomFov() );
		} else {
			zoomFov.Init( gameLocal.time, 200.0f, zoomFov.GetCurrentValue( gameLocal.time ), DefaultFov() );
		}
	}

#ifdef _DENTONMOD_PLAYER_CPP
	// zooming, initiated by weapon script 
	if ( ( weaponZoom.oldZoomStatus ^ weaponZoom.startZoom ) ) {
		if ( weaponZoom.startZoom && weapon.GetEntity() ) {
			weaponZoom.oldZoomStatus = true;
			zoomFov.Init( gameLocal.time, 200.0f, CalcFov( false ), weapon.GetEntity()->GetZoomFov() );
		} else {
			weaponZoom.oldZoomStatus = false;
			zoomFov.Init( gameLocal.time, 200.0f, zoomFov.GetCurrentValue( gameLocal.time ), DefaultFov() );
		}
	}
#endif //_DENTONMOD_PLAYER_CPP

	// if we have an active gui, we will unrotate the view angles as
	// we turn the mouse movements into gui events
	idUserInterface *gui = ActiveGui();
	if ( gui && gui != focusUI ) {
		RouteGuiMouse( gui );
	}

	// set the push velocity on the weapon before running the physics
	if ( weapon.GetEntity() ) {
		weapon.GetEntity()->SetPushVelocity( physicsObj.GetPushedLinearVelocity() );
	}

	EvaluateControls();

	if ( !af.IsActive() ) {
		AdjustBodyAngles();
		CopyJointsFromBodyToHead();
	}

	Move();

	if ( !g_stopTime.GetBool() ) {

		if ( !noclip && !spectating && ( health > 0 ) && !IsHidden() ) {
			TouchTriggers();
		}

		// not done on clients for various reasons. don't do it on server and save the sound channel for other things
		if ( !gameLocal.isMultiplayer ) {
			SetCurrentHeartRate();
			float scale = g_damageScale.GetFloat();
			if ( g_useDynamicProtection.GetBool() && scale < 1.0f && gameLocal.time - lastDmgTime > 500 ) {
				if ( scale < 1.0f ) {
					scale += 0.05f;
				}
				if ( scale > 1.0f ) {
					scale = 1.0f;
				}
				g_damageScale.SetFloat( scale );
			}
		}

		// update GUIs, Items, and character interactions
		UpdateFocus();

		UpdateLocation();

		// update player script
		UpdateScript();

		// service animations
		if ( !spectating && !af.IsActive() && !gameLocal.inCinematic ) {
			UpdateConditions();
			UpdateAnimState();
			CheckBlink();
		}

		// clear out our pain flag so we can tell if we recieve any damage between now and the next time we think
		AI_PAIN = false;
	}

	// calculate the exact bobbed view position, which is used to
	// position the view weapon, among other things
	CalculateFirstPersonView();

	// this may use firstPersonView, or a thirdPeroson / camera view
	CalculateRenderView();

	inventory.UpdateArmor();

	if ( spectating ) {
		UpdateSpectating();
	} else if ( health > 0 ) {
		UpdateWeapon();
	}

	//UpdateAir();

#ifdef _WATER_PHYSICS
	UpdateWaterAir();
#endif

	UpdateHud();

	//ivan start
	if( interactFlag ){
		ShowPossibleInteract();
	}
	//ivan end

	UpdatePowerUps();

	UpdateDeathSkin( false );

	if ( gameLocal.isMultiplayer ) {
		DrawPlayerIcons();
	}

	if ( head.GetEntity() ) {
		headRenderEnt = head.GetEntity()->GetRenderEntity();
	} else {
		headRenderEnt = NULL;
	}

	if ( headRenderEnt ) {
		if ( influenceSkin ) {
			headRenderEnt->customSkin = influenceSkin;
		} else {
			headRenderEnt->customSkin = NULL;
		}
	}

	if ( gameLocal.isMultiplayer || g_showPlayerShadow.GetBool() ) {
		renderEntity.suppressShadowInViewID	= 0;
		if ( headRenderEnt ) {
			headRenderEnt->suppressShadowInViewID = 0;
		}
	} else {
		renderEntity.suppressShadowInViewID	= entityNumber+1;
		if ( headRenderEnt ) {
			headRenderEnt->suppressShadowInViewID = entityNumber+1;
		}
	}
	// never cast shadows from our first-person muzzle flashes
	renderEntity.suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + entityNumber;
	if ( headRenderEnt ) {
		headRenderEnt->suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + entityNumber;
	}

	if ( !g_stopTime.GetBool() ) {
		UpdateAnimation();

		Present();

		UpdateDamageEffects();

		LinkCombat();

		playerView.CalculateShake();
	}

	if ( !( thinkFlags & TH_THINK ) ) {
		gameLocal.Printf( "player %d not thinking?\n", entityNumber );
	}

	if ( g_showEnemies.GetBool() ) {
		idActor *ent;
		int num = 0;
		for( ent = enemyList.Next(); ent != NULL; ent = ent->enemyNode.Next() ) {
			gameLocal.Printf( "enemy (%d)'%s'\n", ent->entityNumber, ent->name.c_str() );
			gameRenderWorld->DebugBounds( colorRed, ent->GetPhysics()->GetBounds().Expand( 2 ), ent->GetPhysics()->GetOrigin() );
			num++;
		}
		gameLocal.Printf( "%d: enemies\n", num );
	}
#ifdef _PORTALSKY //un noted change from original sdk
	// determine if portal sky is in pvs
	gameLocal.portalSkyActive = gameLocal.pvs.CheckAreasForPortalSky( gameLocal.GetPlayerPVS(), GetPhysics()->GetOrigin() );
#endif

	//ivan start
	//kick
	if ( kickEnabled && !noclip && !spectating && ( health > 0 ) && !IsHidden() ) {
		EvaluateKick();
	}
	//ivan end
	
}

/*
=================
idPlayer::RouteGuiMouse
=================
*/
void idPlayer::RouteGuiMouse( idUserInterface *gui ) {
	sysEvent_t ev;

	if ( usercmd.mx != oldMouseX || usercmd.my != oldMouseY ) {
		ev = sys->GenerateMouseMoveEvent( usercmd.mx - oldMouseX, usercmd.my - oldMouseY );
		gui->HandleEvent( &ev, gameLocal.time );
		oldMouseX = usercmd.mx;
		oldMouseY = usercmd.my;
	}
}

/*
==================
idPlayer::LookAtKiller
==================
*/
void idPlayer::LookAtKiller( idEntity *inflictor, idEntity *attacker ) {
	idVec3 dir;

	if ( attacker && attacker != this ) {
		dir = attacker->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin();
	} else if ( inflictor && inflictor != this ) {
		dir = inflictor->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin();
	} else {
		dir = viewAxis[ 0 ];
	}

	idAngles ang( 0, dir.ToYaw(), 0 );
	SetViewAngles( ang );
}

/*
==============
idPlayer::Kill
==============
*/
void idPlayer::Kill( bool delayRespawn, bool nodamage ) {
	if ( spectating ) {
		SpectateFreeFly( false );
	} else if ( health > 0 ) {
		godmode = false;
		if ( nodamage ) {
			ServerSpectate( true );
			forceRespawn = true;
		} else {
			Damage( this, this, vec3_origin, "damage_suicide", 1.0f, INVALID_JOINT );
			if ( delayRespawn ) {
				forceRespawn = false;
				int delay = spawnArgs.GetFloat( "respawn_delay" );
				minRespawnTime = gameLocal.time + SEC2MS( delay );
				maxRespawnTime = minRespawnTime + MAX_RESPAWN_TIME;
			}
		}
	}
}

/*
==================
idPlayer::Killed
==================
*/
void idPlayer::Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location ) {
	float delay;

	assert( !gameLocal.isClient );

	// stop taking knockback once dead
	fl.noknockback = true;
	if ( health < -999 ) {
		health = -999;
	}

	if ( AI_DEAD ) {
		AI_PAIN = true;
		return;
	}

	heartInfo.Init( 0, 0, 0, BASE_HEARTRATE );
	AdjustHeartRate( DEAD_HEARTRATE, 10.0f, 0.0f, true );


	//was: if ( !g_testDeath.GetBool() ) { //un noted change from original sdk
	if ( !g_testDeath.GetBool() && numLives == 0 ) {
		playerView.Fade( colorBlack, 12000 );
	}

	AI_DEAD = true;
	SetAnimState( ANIMCHANNEL_LEGS, "Legs_Death", 4 );
	SetAnimState( ANIMCHANNEL_TORSO, "Torso_Death", 4 );
	SetWaitState( "" );

	animator.ClearAllJoints();

	if ( StartRagdoll() ) {
		pm_modelView.SetInteger( 0 );
		minRespawnTime = gameLocal.time + RAGDOLL_DEATH_TIME;
		maxRespawnTime = minRespawnTime + MAX_RESPAWN_TIME;
	} else {
		// don't allow respawn until the death anim is done
		// g_forcerespawn may force spawning at some later time
		delay = spawnArgs.GetFloat( "respawn_delay" );
		minRespawnTime = gameLocal.time + SEC2MS( delay );
		maxRespawnTime = minRespawnTime + MAX_RESPAWN_TIME;
	}

	physicsObj.SetMovementType( PM_DEAD );
	StartSound( "snd_death", SND_CHANNEL_VOICE, 0, false, NULL );
	StopSound( SND_CHANNEL_BODY2, false );

	fl.takedamage = true;		// can still be gibbed

	// get rid of weapon
	weapon.GetEntity()->OwnerDied();

//rev 2020 don't drop the weapon! Doom Slayer don't drop weapons... they are the weapon! lol
	// drop the weapon as an item
	//ivan start -drop the weapon only if it is in a slot
	//if( currentSlot >= 0 ){
	//	DropWeapon( true );
	//}
	//ivan end

	if ( !g_testDeath.GetBool() ) {
		LookAtKiller( inflictor, attacker );
	}

	if ( gameLocal.isMultiplayer || g_testDeath.GetBool() ) {
		idPlayer *killer = NULL;
		// no gibbing in MP. Event_Gib will early out in MP
		if ( attacker->IsType( idPlayer::Type ) ) {
			killer = static_cast<idPlayer*>(attacker);
			if ( health < -20 || killer->PowerUpActive( BERSERK ) ) {
				gibDeath = true;
				gibsDir = dir;
				gibsLaunched = false;
			}
		}
		gameLocal.mpGame.PlayerDeath( this, killer, isTelefragged );
	} else {
		physicsObj.SetContents( CONTENTS_CORPSE | CONTENTS_MONSTERCLIP );
	}

	ClearPowerUps();

	UpdateVisuals();

	isChatting = false;
}

/*
=====================
idPlayer::GetAIAimTargets

Returns positions for the AI to aim at.
=====================
*/
void idPlayer::GetAIAimTargets( const idVec3 &lastSightPos, idVec3 &headPos, idVec3 &chestPos ) {
	idVec3 offset;
	idMat3 axis;
	idVec3 origin;

	origin = lastSightPos - physicsObj.GetOrigin();

	GetJointWorldTransform( chestJoint, gameLocal.time, offset, axis );
	headPos = offset + origin;

	GetJointWorldTransform( headJoint, gameLocal.time, offset, axis );
	chestPos = offset + origin;
}

/*
================
idPlayer::DamageFeedback

callback function for when another entity received damage from this entity.  damage can be adjusted and returned to the caller.
================
*/
void idPlayer::DamageFeedback( idEntity *victim, idEntity *inflictor, int &damage ) {
	assert( !gameLocal.isClient );
	damage *= PowerUpModifier( BERSERK );
	if ( damage && ( victim != this ) && victim->IsType( idActor::Type ) ) {
		SetLastHitTime( gameLocal.time );
	}
}

/*
=================
idPlayer::CalcDamagePoints

Calculates how many health and armor points will be inflicted, but
doesn't actually do anything with them.  This is used to tell when an attack
would have killed the player, possibly allowing a "saving throw"
=================
*/
void idPlayer::CalcDamagePoints( idEntity *inflictor, idEntity *attacker, const idDict *damageDef,
								const float damageScale, const int location, int *health, int *armor ) {
									int		damage;
									int		armorSave;

									damageDef->GetInt( "damage", "20", damage );
									damage = GetDamageForLocation( damage, location );

									idPlayer *player = attacker->IsType( idPlayer::Type ) ? static_cast<idPlayer*>(attacker) : NULL;
									if ( !gameLocal.isMultiplayer ) {
										if ( inflictor != gameLocal.world ) {
											switch ( g_skill.GetInteger() ) {
				case 0:
					damage *= 0.80f;
					if ( damage < 1 ) {
						damage = 1;
					}
					break;
				case 2:
					damage *= 1.70f;
					break;
				case 3:
					damage *= 3.5f;
					break;
				default:
					break;
											}
										}
									}

									damage *= damageScale;

									// always give half damage if hurting self
									if ( attacker == this ) {
										if ( gameLocal.isMultiplayer ) {
											// only do this in mp so single player plasma and rocket splash is very dangerous in close quarters
											damage *= damageDef->GetFloat( "selfDamageScale", "0.5" );
										} else {
											damage *= damageDef->GetFloat( "selfDamageScale", "1" );
										}
									}

									// check for completely getting out of the damage
									if ( !damageDef->GetBool( "noGod" ) ) {
										// check for godmode
										if ( godmode ) { 
											damage = 0;
										}
									}

									//ivan start - no damage during full body anim
									//if ( force_torso_override ) { 
										//damage = 0;
									//}
									//ivan end
									//Rev 2018 no damage for a brief amount of time after being hit
									waitForDamage  = spawnArgs.GetInt( "waitfordamage" );
									if ( waitForDamage > 0 ) {
										damage = 0;
									}
									//rev 2020
									noDamage  = spawnArgs.GetInt( "nodamage" );
									if ( noDamage ) {
										damage = 0;
									}									

									// inform the attacker that they hit someone
									attacker->DamageFeedback( this, inflictor, damage );

									// save some from armor
									if ( !damageDef->GetBool( "noArmor" ) ) {
										float armor_protection;

										armor_protection = ( gameLocal.isMultiplayer ) ? g_armorProtectionMP.GetFloat() : g_armorProtection.GetFloat();

										armorSave = ceil( damage * armor_protection );
										if ( armorSave >= inventory.armor ) {
											armorSave = inventory.armor;
										}

										if ( !damage ) {
											armorSave = 0;
										} else if ( armorSave >= damage ) {
											armorSave = damage - 1;
											damage = 1;
										} else {
											damage -= armorSave;
										}
									} else {
										armorSave = 0;
									}

									// check for team damage
									if ( gameLocal.gameType == GAME_TDM
										&& !gameLocal.serverInfo.GetBool( "si_teamDamage" )
										&& !damageDef->GetBool( "noTeam" )
										&& player
										&& player != this		// you get self damage no matter what
										&& player->team == team ) {
											damage = 0;
									}

									*health = damage;
									*armor = armorSave;
}

/*
============
Damage

this		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
example: this=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback in global space

damageDef	an idDict with all the options for damage effects

inflictor, attacker, dir, and point can be NULL for environmental effects
============
*/
void idPlayer::Damage( idEntity *inflictor, idEntity *attacker, const idVec3 &dir,
					   const char *damageDefName, const float damageScale, const int location ) {

  idVec3		kick;
  int			damage;
  int			armorSave;
  int			knockback;
  idVec3		damage_from;
  idVec3		localDamageVector;	
  float		attackerPushScale;
  float		playerDamageScale; //ivan

  // damage is only processed on server
  if ( gameLocal.isClient ) {
	  return;
  }

  if ( !fl.takedamage || noclip || spectating || gameLocal.inCinematic ) {
	  return;
  }

  if ( !inflictor ) {
	  inflictor = gameLocal.world;
  }
  if ( !attacker ) {
	  attacker = gameLocal.world;
  }

  if ( attacker->IsType( idAI::Type ) ) {
	  if ( PowerUpActive( BERSERK ) ) {
		  return;
	  }
	  // don't take damage from monsters during influences
	  if ( influenceActive != 0 ) {
		  return;
	  }
  }

  const idDeclEntityDef *damageDef = gameLocal.FindEntityDef( damageDefName, false );
  if ( !damageDef ) {
	  gameLocal.Warning( "Unknown damageDef '%s'", damageDefName );
	  return;
  }

  if ( damageDef->dict.GetBool( "ignore_player" ) ) {
	  return;
  }

  //ivan start
  if(damageDef->dict.GetBool( "ignore_friends" )){
	  if(team == attacker->spawnArgs.GetInt("team","0")){
		  return;
	  }
  }

  playerDamageScale = damageDef->dict.GetFloat( "playerDamageScale","1");
  //ivan end

  //ivan start - damaging fx
  CheckDamageFx( &damageDef->dict );
  //ivan end

  CalcDamagePoints( inflictor, attacker, &damageDef->dict, damageScale*playerDamageScale , location, &damage, &armorSave ); //ivan: added *playerDamageScale

  // determine knockback
  damageDef->dict.GetInt( "knockback", "20", knockback );

  if ( knockback != 0 && !fl.noknockback ) {
	  if ( attacker == this ) {
		  damageDef->dict.GetFloat( "attackerPushScale", "0", attackerPushScale );
	  } else {
		  attackerPushScale = 1.0f;
	  }

	  kick = dir;
	  kick.Normalize();
	  kick *= g_knockback.GetFloat() * knockback * attackerPushScale / 200.0f;
	  physicsObj.SetLinearVelocity( physicsObj.GetLinearVelocity() + kick );

	  // set the timer so that the player can't cancel out the movement immediately
	  physicsObj.SetKnockBack( idMath::ClampInt( 50, 200, knockback * 2 ) );
  }

  // give feedback on the player view and audibly when armor is helping
  if ( armorSave ) {
	  inventory.armor -= armorSave;

	  if ( gameLocal.time > lastArmorPulse + 200 ) {
		  StartSound( "snd_hitArmor", SND_CHANNEL_ITEM, 0, false, NULL );
	  }
	  lastArmorPulse = gameLocal.time;
  }

  if ( damageDef->dict.GetBool( "burn" ) ) {
	  StartSound( "snd_burn", SND_CHANNEL_BODY3, 0, false, NULL );
  } else if ( damageDef->dict.GetBool( "no_air" ) ) {
	  if ( !armorSave && health > 0 ) {
		  StartSound( "snd_airGasp", SND_CHANNEL_ITEM, 0, false, NULL );
	  }
  }

  if ( g_debugDamage.GetInteger() ) {
	  gameLocal.Printf( "client:%i health:%i damage:%i armor:%i\n", 
		  entityNumber, health, damage, armorSave );
  }

  // move the world direction vector to local coordinates
  damage_from = dir;
  damage_from.Normalize();

  viewAxis.ProjectVector( damage_from, localDamageVector );

  // add to the damage inflicted on a player this frame
  // the total will be turned into screen blends and view angle kicks
  // at the end of the frame
  //if ( health > 0 ) {
	waitForDamage  = spawnArgs.GetInt( "waitfordamage" );	//rev 2020 no dmg effect on screen when invulnerability is on
	noDamage  = spawnArgs.GetBool( "nodamage" );	//rev 2020 no dmg effect on screen when invulnerability is on
	if (( health > 0 ) && ( waitForDamage < 1 || noDamage )) {	//rev 2020 updated
  //if ( health > 0 ) {
	  playerView.DamageImpulse( localDamageVector, &damageDef->dict );
  }

  // do the damage
  if ( damage > 0 ) {

	  if ( !gameLocal.isMultiplayer ) {
		  float scale = g_damageScale.GetFloat();
		  if ( g_useDynamicProtection.GetBool() && g_skill.GetInteger() < 2 ) {
			  if ( gameLocal.time > lastDmgTime + 500 && scale > 0.25f ) {
				  scale -= 0.05f;
				  g_damageScale.SetFloat( scale );
			  }
		  }

		  if ( scale > 0.0f ) {
			  damage *= scale;
		  }
	  }

	  if ( damage < 1 ) {
		  damage = 1;
	  }

	  int oldHealth = health;
	  health -= damage;

	  //ivan start
	  if ( oldHealth > 0 ) { //don't add damage if dead
		  health_lost += ( health > 0 ) ? damage : oldHealth; //if we are dying only add the health we have 
	  }
	  //ivan end

	  if ( health <= 0 ) {

		  if ( health < -999 ) {
			  health = -999;
		  }

		  isTelefragged = damageDef->dict.GetBool( "telefrag" );

		  lastDmgTime = gameLocal.time;
		  Killed( inflictor, attacker, damage, dir, location );

	  } else {
		  // force a blink
		  blink_time = 0;

		  // let the anim script know we took damage
		  AI_PAIN = Pain( inflictor, attacker, damage, dir, location );
		  if ( !g_testDeath.GetBool() ) {
			  lastDmgTime = gameLocal.time;
		  }
	  }
	  
	  
		SetState( "BlinkOn" );	// rev 2018 triggers the player's script to start no damage for a bit.
		UpdateScript();	
		
  } else {
	  // don't accumulate impulses
	  if ( af.IsLoaded() ) {
		  // clear impacts
		  af.Rest();

		  // physics is turned off by calling af.Rest()
		  BecomeActive( TH_PHYSICS );
	  }
  }

  lastDamageDef = damageDef->Index();
  lastDamageDir = damage_from;
  lastDamageLocation = location;
}

/*
===========
idPlayer::Teleport
============
*/
void idPlayer::Teleport( const idVec3 &origin, const idAngles &angles, idEntity *destination ) {
	idVec3 org;

	if ( weapon.GetEntity() ) {
		weapon.GetEntity()->LowerWeapon();
	}

	//ivan start - set the right position & upd camera
	StopForcedMov(); //fix - make sure
	fastXpos = origin.x; //upd fastXpos!
	playerView.Fade( idVec4(0, 0, 0, 0), 500 ); //fade in
	skipCameraZblend = true; //force instant update next frame
	enableCameraYblend = false; //make sure no Y blending

	inhibitInputTime = gameLocal.time + 500; //disable inputs for a while so player can't fire/turn immediately
	//ivan end

	SetOrigin( origin + idVec3( 0, 0, CM_CLIP_EPSILON ) );

	if ( !gameLocal.isMultiplayer && GetFloorPos( 16.0f, org ) ) { 
		fastXpos = org.x; //ivan - upd fastXpos! 
		SetOrigin( org );
	}

	/*
	//ivan - save pos after teleport
	SaveCheckPointPos();
	*/

	// clear the ik heights so model doesn't appear in the wrong place
	walkIK.EnableAll();

	//ivan start

	/* was:
	GetPhysics()->SetLinearVelocity( vec3_origin );
	SetViewAngles( angles );
	*/

	if( destination ){
		if( !destination->spawnArgs.GetBool("keepSpeed","0") ){ 
			GetPhysics()->SetLinearVelocity( vec3_origin );
			SetViewAngles( angles );
			reqDefaultCrossPos = true; //reset crosshair pos to default
		}

		UpdateCameraSettingsFromEntity( destination );
	} //ivan

	//ivan start - face the right direction
	if( angles.yaw > 0 && angles.yaw < 180){ //from 0 to 180 -> set 90 (left)
		viewPos = VIEWPOS_LEFT_MAX;
	}else{ //from -180 to 0 -> set -90 (right)
		viewPos = -VIEWPOS_LEFT_MAX;
	}
	//ivan end

	legsYaw = 0.0f;
	idealLegsYaw = 0.0f;
	oldViewYaw = viewAngles.yaw;

	if ( gameLocal.isMultiplayer ) {
		playerView.Flash( colorWhite, 140 );
	}

	UpdateVisuals();

	teleportEntity = destination;

	if ( !gameLocal.isClient && !noclip ) {
		if ( gameLocal.isMultiplayer ) {
			// kill anything at the new position or mark for kill depending on immediate or delayed teleport
			gameLocal.KillBox( this, destination != NULL );
		} else {
			// kill anything at the new position
			gameLocal.KillBox( this, true );
		}
	}
}

/*
====================
idPlayer::SetPrivateCameraView
====================
*/
void idPlayer::SetPrivateCameraView( idCamera *camView ) {
	privateCameraView = camView;
	if ( camView ) {
		StopFiring();
		Hide();
	} else {
		if ( !spectating ) {
			Show();
		}
	}
}

/*
====================
idPlayer::DefaultFov

Returns the base FOV
====================
*/
float idPlayer::DefaultFov( void ) const {
	float fov;

	fov = g_fov.GetFloat();
	if ( gameLocal.isMultiplayer ) {
		if ( fov < 90.0f ) {
			return 90.0f;
		} else if ( fov > 110.0f ) {
			return 110.0f;
		}
	}

	return fov;
}

/*
====================
idPlayer::CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
float idPlayer::CalcFov( bool honorZoom ) {
	float fov;

	if ( fxFov ) {
		return DefaultFov() + 10.0f + cos( ( gameLocal.time + 2000 ) * 0.01 ) * 10.0f;
	}

	if ( influenceFov ) {
		return influenceFov;
	}

#ifdef _DENTONMOD_PLAYER_CPP
	if ( zoomFov.IsDone( gameLocal.time ) ) {
		fov = ( honorZoom && ((usercmd.buttons & BUTTON_ZOOM) || weaponZoom.startZoom )) && weapon.GetEntity() ? weapon.GetEntity()->GetZoomFov() : DefaultFov(); // Updated By Clone JCD
#else
	if ( zoomFov.IsDone( gameLocal.time ) ) {
		fov = ( honorZoom && (usercmd.buttons & BUTTON_ZOOM)) && weapon.GetEntity() ? weapon.GetEntity()->GetZoomFov() : DefaultFov(); // Updated By Clone JCD
#endif// _DENTONMOD_PLAYER_CPP
	} else {
		fov = zoomFov.GetCurrentValue( gameLocal.time );
	}

	// bound normal viewsize
	if ( fov < 1 ) {
		fov = 1;
	} else if ( fov > 179 ) {
		fov = 179;
	}

	return fov;
}

/*
==============
idPlayer::GunTurningOffset

generate a rotational offset for the gun based on the view angle
history in loggedViewAngles
==============
*/
idAngles idPlayer::GunTurningOffset( void ) {
	idAngles	a;

	a.Zero();

	if ( gameLocal.framenum < NUM_LOGGED_VIEW_ANGLES ) {
		return a;
	}

	idAngles current = loggedViewAngles[ gameLocal.framenum & (NUM_LOGGED_VIEW_ANGLES-1) ];

	idAngles	av, base;
	int weaponAngleOffsetAverages;
	float weaponAngleOffsetScale, weaponAngleOffsetMax;

	weapon.GetEntity()->GetWeaponAngleOffsets( &weaponAngleOffsetAverages, &weaponAngleOffsetScale, &weaponAngleOffsetMax );

	av = current;

	// calcualte this so the wrap arounds work properly
	for ( int j = 1 ; j < weaponAngleOffsetAverages ; j++ ) {
		idAngles a2 = loggedViewAngles[ ( gameLocal.framenum - j ) & (NUM_LOGGED_VIEW_ANGLES-1) ];

		idAngles delta = a2 - current;

		if ( delta[1] > 180 ) {
			delta[1] -= 360;
		} else if ( delta[1] < -180 ) {
			delta[1] += 360;
		}

		av += delta * ( 1.0f / weaponAngleOffsetAverages );
	}

	a = ( av - current ) * weaponAngleOffsetScale;

	for ( int i = 0 ; i < 3 ; i++ ) {
		if ( a[i] < -weaponAngleOffsetMax ) {
			a[i] = -weaponAngleOffsetMax;
		} else if ( a[i] > weaponAngleOffsetMax ) {
			a[i] = weaponAngleOffsetMax;
		}
	}

	return a;
}

/*
==============
idPlayer::GunAcceleratingOffset

generate a positional offset for the gun based on the movement
history in loggedAccelerations
==============
*/
idVec3	idPlayer::GunAcceleratingOffset( void ) {
	idVec3	ofs;

	float weaponOffsetTime, weaponOffsetScale;

	ofs.Zero();

	weapon.GetEntity()->GetWeaponTimeOffsets( &weaponOffsetTime, &weaponOffsetScale );

	int stop = currentLoggedAccel - NUM_LOGGED_ACCELS;
	if ( stop < 0 ) {
		stop = 0;
	}
	for ( int i = currentLoggedAccel-1 ; i > stop ; i-- ) {
		loggedAccel_t	*acc = &loggedAccel[i&(NUM_LOGGED_ACCELS-1)];

		float	f;
		float	t = gameLocal.time - acc->time;
		if ( t >= weaponOffsetTime ) {
			break;	// remainder are too old to care about
		}

		f = t / weaponOffsetTime;
		f = ( cos( f * 2.0f * idMath::PI ) - 1.0f ) * 0.5f;
		ofs += f * weaponOffsetScale * acc->dir;
	}

	return ofs;
}

/*
==============
idPlayer::CalculateViewWeaponPos

Calculate the bobbing position of the view weapon
==============
*/
/* //un noted change from original sdk
void idPlayer::CalculateViewWeaponPos( idVec3 &origin, idMat3 &axis ) {
	float		scale;
	float		fracsin;
	idAngles	angles;
	int			delta;

	// CalculateRenderView must have been called first
	const idVec3 &viewOrigin = firstPersonViewOrigin;
	const idMat3 &viewAxis = firstPersonViewAxis;

	// these cvars are just for hand tweaking before moving a value to the weapon def
	idVec3	gunpos( g_gun_x.GetFloat(), g_gun_y.GetFloat(), g_gun_z.GetFloat() );

	// as the player changes direction, the gun will take a small lag
	idVec3	gunOfs = GunAcceleratingOffset();
	origin = viewOrigin + ( gunpos + gunOfs ) * viewAxis;

	// on odd legs, invert some angles
	if ( bobCycle & 128 ) {
		scale = -xyspeed;
	} else {
		scale = xyspeed;
	}

	// gun angles from bobbing
	angles.roll		= scale * bobfracsin * 0.005f;
	angles.yaw		= scale * bobfracsin * 0.01f;
	angles.pitch	= xyspeed * bobfracsin * 0.005f;

	// gun angles from turning
	if ( gameLocal.isMultiplayer ) {
		idAngles offset = GunTurningOffset();
		offset *= g_mpWeaponAngleScale.GetFloat();
		angles += offset;
	} else {
		angles += GunTurningOffset();
	}

	idVec3 gravity = physicsObj.GetGravityNormal();

	// drop the weapon when landing after a jump / fall
	delta = gameLocal.time - landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		origin -= gravity * ( landChange*0.25f * delta / LAND_DEFLECT_TIME );
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		origin -= gravity * ( landChange*0.25f * (LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME );
	}

	// speed sensitive idle drift
	scale = xyspeed + 40.0f;
	fracsin = scale * sin( MS2SEC( gameLocal.time ) ) * 0.01f;
	angles.roll		+= fracsin;
	angles.yaw		+= fracsin;
	angles.pitch	+= fracsin;

	axis = angles.ToMat3() * viewAxis;
}
*/

/*
===============
idPlayer::OffsetThirdPersonView
===============
*/
void idPlayer::OffsetThirdPersonView( float angle, float range, float height, bool clip ) {
	idVec3			view;
	idVec3			focusAngles;
	trace_t			trace;
	idVec3			focusPoint;
	float			focusDist;
	float			forwardScale, sideScale;
	idVec3			origin;
	idAngles		angles;
	idMat3			axis;
	idBounds		bounds;

	//ivan start
	float			delta;
	float			speed;

	//stop the screen from turning
    viewAngles.yaw = 90;
    //viewAngles.pitch = 0; //commented out because it is set in ::UpdateViewAngles	
	//ivan end

	angles = viewAngles;
	GetViewPos( origin, axis );

	if ( angle ) {
		angles.pitch = 0.0f;
	}

	if ( angles.pitch > 45.0f ) {
		angles.pitch = 45.0f;		// don't go too far overhead
	}

	focusPoint = origin + angles.ToForward() * THIRD_PERSON_FOCUS_DISTANCE;
	focusPoint.z += height;
	view = origin;
	view.z += 8 + height;

	angles.pitch *= 0.5f;
	renderView->viewaxis = angles.ToMat3() * physicsObj.GetGravityAxis();

	idMath::SinCos( DEG2RAD( angle ), sideScale, forwardScale );
	view -= range * forwardScale * renderView->viewaxis[ 0 ];
	view += range * sideScale * renderView->viewaxis[ 1 ];

	
	//moved below //un noted change from original sdk
	if ( clip ) {
		// trace a ray from the origin to the viewpoint to make sure the view isn't
		// in a solid block.  Use an 8 by 8 block to prevent the view from near clipping anything
		bounds = idBounds( idVec3( -4, -4, -4 ), idVec3( 4, 4, 4 ) );
		gameLocal.clip.TraceBounds( trace, origin, view, bounds, MASK_SOLID, this );
		if ( trace.fraction != 1.0f ) {
			view = trace.endpos;
			view.z += ( 1.0f - trace.fraction ) * 32.0f;

			// try another trace to this position, because a tunnel may have the ceiling
			// close enough that this is poking out
			gameLocal.clip.TraceBounds( trace, origin, view, bounds, MASK_SOLID, this );
			view = trace.endpos;
		}
	}

	// select pitch to look at focus point from vieword
	focusPoint -= view;
	focusDist = idMath::Sqrt( focusPoint[0] * focusPoint[0] + focusPoint[1] * focusPoint[1] );
	if ( focusDist < 1.0f ) {
		focusDist = 1.0f;	// should never happen
	}

	angles.pitch = - RAD2DEG( atan2( focusPoint.z, focusDist ) );
	angles.yaw -= angle;

	//ivan start - interpolate Z position

	// --- Y pos --- (horiziontal)

	if( enableCameraYblend ){
		//delta
		if( cameraSettings.lockYaxis ){
			delta = cameraSettings.lockedYpos - oldCameraPos.y; //(ideal camera position) - (where camera is)
		}else{
			delta = view.y - oldCameraPos.y; //(ideal camera position) - (where camera is)
		}
		
		//interpolate toward the ideal position

		/*
		if( delta > 0 ){
			speed = delta / CAMERA_MAX_Y_DELTA_POS;
		}else{ // < 0
			speed = delta / CAMERA_MAX_Y_DELTA_NEG;	
		}
		*/
		speed = delta / CAMERA_MAX_Y_DELTA;
		if( speed < 0.0f ){ speed = -speed; } //this may happen, it depends on the delta.

		//min speed
		if( speed < CAMERA_MIN_Y_SPEED ){
			speed = CAMERA_MIN_Y_SPEED;
		}

		//new y pos
		view.y = oldCameraPos.y + ( delta * speed * CAMERA_SENSIBILITY );

		//stop blending if near enough
		if( delta < 1.0f && delta > -1.0f ){ 
			//gameLocal.Printf("stop Y blending\n");
			enableCameraYblend = false;
		}
	}else if( cameraSettings.lockYaxis ){ //forced pos, but no blend
		view.y = cameraSettings.lockedYpos; 
	}

	// --- Z pos --- (vertical)

	if ( skipCameraZblend ) { //no blend this frame: instant update
		skipCameraZblend = false;
		if( cameraSettings.lockZaxis ){ //forced pos, but no blend
			view.z = cameraSettings.lockedZpos; 
		}
	} else { //&& (gameLocal.time > 500)
		if ( cameraSettings.lockZaxis ){ //Rev 2020 lock the z position of the camera so that it does not move. 
			delta = cameraSettings.lockedZpos - oldCameraPos.z; //(ideal camera position) - (where camera is)
		} else {
			delta = view.z - oldCameraPos.z; //(ideal camera position) - (where camera is)
		}
		/*
		if(delta > CAMERA_MAX_Z_DELTA_POS){ //don't let it run away UP
			view.z = view.z - CAMERA_MAX_Z_DELTA_POS;
			//gameLocal.Printf("don't run away up\n");
		}
		
		else if(delta < CAMERA_MAX_Z_DELTA_NEG){ //don't let it run away DOWN
			view.z = view.z - CAMERA_MAX_Z_DELTA_NEG;
			gameLocal.Printf("don't run away down. delta: %f\n", delta);
		}
		else 
		*/

		if ( delta != 0.0f ) {
			//interpolate toward the ideal position
			if ( delta > 0 ) {
				speed = delta / CAMERA_MAX_Z_DELTA_POS;
			} else { // < 0
				speed = delta / CAMERA_MAX_Z_DELTA_NEG;	

				//min negative speed
				if( speed < CAMERA_MIN_Z_SPEED_NEG ){
					speed = CAMERA_MIN_Z_SPEED_NEG;
				}
			}

			//new z pos
			view.z = oldCameraPos.z + ( delta * speed * CAMERA_SENSIBILITY );
		}
	}

	// --- X pos --- (distance)
	if( enableCameraXblend ){
		//delta
		delta = view.x - oldCameraPos.x; //(ideal camera position) - (where camera is)
		
		//interpolate toward the ideal position
		speed = delta / CAMERA_MAX_X_DELTA;
		if( speed < 0 ){ speed = -speed; } //this may happen, it depends on the delta.

		//min speed
		if( speed < CAMERA_MIN_X_SPEED ){
			speed = CAMERA_MIN_X_SPEED;
		}

		//new y pos
		view.x = oldCameraPos.x + ( delta * speed * CAMERA_SENSIBILITY );

		//stop blending if near enough
		if( delta < 1.0f && delta > -1.0f ){ 
			//gameLocal.Printf("stop X blending\n");
			enableCameraXblend = false;
		}
	}

	// --- remember old pos ---
	oldCameraPos = view;

	//ivan end 

	renderView->vieworg = view;
	renderView->viewaxis = angles.ToMat3() * physicsObj.GetGravityAxis();
	renderView->viewID = 0;
}

/*
===============
idPlayer::GetEyePosition
===============
*/
idVec3 idPlayer::GetEyePosition( void ) const {
   idVec3 org;

   // use the smoothed origin if spectating another player in multiplayer
   if ( gameLocal.isClient && entityNumber != gameLocal.localClientNum ) {
      org = smoothedOrigin;
   } 
   
   //ivan start 

   //old code restored:
	else {
      org = GetPhysics()->GetOrigin();
   }

   //Rev code commented out - replaced by new code in ::OffsetThirdPersonView

	/*
////START  REVILITY STOP THE CAMERA FROM MOVING VERTICALLY   <- no need of "6th venom" here ;)     
   else {
      org = GetPhysics()->GetOrigin();

      // if camera can go up/down, update thirdperson camera height
      if ( pm_thirdPersonUp.GetBool() ) {
         pm_thirdPersonCamHeight.SetFloat( org.z );
      }
      // if camera can go left right, update thirdperson camera position
      if ( pm_thirdPersonGo.GetBool() ) {
         pm_thirdPersonCamWay.SetFloat( org.y );
      }	  

      // force the Z position to thirdperson camera height
      org.z =  pm_thirdPersonCamHeight.GetFloat(); 
      // force the Y position to thirdperson camera position  
      org.y =  pm_thirdPersonCamWay.GetFloat(); 
   }
////END  REVILITY STOP THE CAMERA FROM MOVING VERTICALLY  
	*/

	//ivan end

	return org + ( GetPhysics()->GetGravityNormal() * -eyeOffset.z );
}

/*
===============
idPlayer::GetViewPos
===============
*/
void idPlayer::GetViewPos( idVec3 &origin, idMat3 &axis ) const {
	idAngles angles;

	// if dead, fix the angle and don't add any kick
	if ( health <= 0 ) {
		angles.yaw = viewAngles.yaw;
		angles.roll = 40;
		angles.pitch = -15;
		axis = angles.ToMat3();
		origin = GetEyePosition();
	} else {
		////REVILITY BOB FIX We don't want any bobbing in the game
		////origin = GetEyePosition() + viewBob;
		origin = GetEyePosition();
		////REVILITY BOB FIX  We don't want any bobbing in the game
		////angles = viewAngles + viewBobAngles + playerView.AngleOffset();
		angles = viewAngles; //ivan: no kick offset // + playerView.AngleOffset();

		axis = angles.ToMat3() * physicsObj.GetGravityAxis();

		// Stop the gun from moving left and right
		if( isXlocked ){
			axis[0].x = 0.0f;
		}
		
		/* 
		//ivan - commented out: useless and dangerous
		// adjust the origin based on the camera nodal distance (eye distance from neck)
		origin += physicsObj.GetGravityNormal() * g_viewNodalZ.GetFloat();
		origin += axis[0] * g_viewNodalX.GetFloat() + axis[2] * g_viewNodalZ.GetFloat();
		*/
	}
}

/*
===============
idPlayer::CalculateFirstPersonView
===============
*/
void idPlayer::CalculateFirstPersonView( void ) {
	if ( ( pm_modelView.GetInteger() == 1 ) || ( ( pm_modelView.GetInteger() == 2 ) && ( health <= 0 ) ) ) {
		//	Displays the view from the point of view of the "camera" joint in the player model

		idMat3 axis;
		idVec3 origin;
		idAngles ang;

		ang = playerView.AngleOffset(); //+ viewBobAngles //un noted change from original sdk
		ang.yaw += viewAxis[ 0 ].ToYaw();

		jointHandle_t joint = animator.GetJointHandle( "camera" );
		animator.GetJointTransform( joint, gameLocal.time, origin, axis );
		firstPersonViewOrigin = ( origin + modelOffset ) * ( viewAxis * physicsObj.GetGravityAxis() ) + physicsObj.GetOrigin(); // + viewBob;
		firstPersonViewAxis = axis * ang.ToMat3() * physicsObj.GetGravityAxis();
	} else {
		// offset for local bobbing and kicks
		GetViewPos( firstPersonViewOrigin, firstPersonViewAxis );
#if 0
		// shakefrom sound stuff only happens in first person
		firstPersonViewAxis = firstPersonViewAxis * playerView.ShakeAxis();
#endif
	}
}

/*
==================
idPlayer::GetRenderView

Returns the renderView that was calculated for this tic
==================
*/
renderView_t *idPlayer::GetRenderView( void ) {
	return renderView;
}

/*
==================
idPlayer::CalculateRenderView

create the renderView for the current tic
==================
*/
void idPlayer::CalculateRenderView( void ) {
	int i;
	//float range; //rev 2019 commented out in rivensin fyi

	if ( !renderView ) {
		renderView = new renderView_t;
	}
	memset( renderView, 0, sizeof( *renderView ) );

	// copy global shader parms
	for( i = 0; i < MAX_GLOBAL_SHADER_PARMS; i++ ) {
		renderView->shaderParms[ i ] = gameLocal.globalShaderParms[ i ];
	}
	renderView->globalMaterial = gameLocal.GetGlobalMaterial();
	renderView->time = gameLocal.time;

	// calculate size of 3D view
	renderView->x = 0;
	renderView->y = 0;
	renderView->width = SCREEN_WIDTH;
	renderView->height = SCREEN_HEIGHT;
	renderView->viewID = 0;

	// check if we should be drawing from a camera's POV
	if ( !noclip && (gameLocal.GetCamera() || privateCameraView) ) {
		// get origin, axis, and fov
		if ( privateCameraView ) {
			privateCameraView->GetViewParms( renderView );
		} else {
			gameLocal.GetCamera()->GetViewParms( renderView );
		}
	} else {
		if ( g_stopTime.GetBool() ) {
			renderView->vieworg = firstPersonViewOrigin;
			renderView->viewaxis = firstPersonViewAxis;

			/*
			//ivan - commented out
			if ( !pm_thirdPerson.GetBool() ) {
				// set the viewID to the clientNum + 1, so we can suppress the right player bodies and
				// allow the right player view weapons
				renderView->viewID = entityNumber + 1;
			}
			*/
		} else {
			
			//ivan start
			OffsetThirdPersonView( pm_thirdPersonAngle.GetFloat(), cameraSettings.distance, cameraSettings.height, false ); //never clip

			/*
			//was:
			else if ( pm_thirdPerson.GetBool() ) {		
				OffsetThirdPersonView( pm_thirdPersonAngle.GetFloat(), pm_thirdPersonRange.GetFloat(), pm_thirdPersonHeight.GetFloat(), pm_thirdPersonClip.GetBool() );
			} else if ( pm_thirdPersonDeath.GetBool() ) {
				range = gameLocal.time < minRespawnTime ? ( gameLocal.time + RAGDOLL_DEATH_TIME - minRespawnTime ) * ( 120.0f / RAGDOLL_DEATH_TIME ) : 120.0f;
				OffsetThirdPersonView( 0.0f, 20.0f + range, 0.0f, false );
			} else {
				renderView->vieworg = firstPersonViewOrigin;
				renderView->viewaxis = firstPersonViewAxis;

				// set the viewID to the clientNum + 1, so we can suppress the right player bodies and
				// allow the right player view weapons
				renderView->viewID = entityNumber + 1; //un noted change from original sdk
			}
			*/
		}

		// field of view
		gameLocal.CalcFov( CalcFov( true ), renderView->fov_x, renderView->fov_y );
	}

	if ( renderView->fov_y == 0 ) {
		common->Error( "renderView->fov_y == 0" );
	}

	if ( g_showviewpos.GetBool() ) {
		gameLocal.Printf( "%s : %s\n", renderView->vieworg.ToString(), renderView->viewaxis.ToAngles().ToString() );
	}
}

/*
=============
idPlayer::AddAIKill
=============
*/
void idPlayer::AddAIKill( void ) {
	int max_souls;
	int ammo_souls;

	if ( ( weapon_soulcube < 0 ) || ( inventory.weapons & ( 1 << weapon_soulcube ) ) == 0 ) {
		return;
	}

	assert( hud );

	ammo_souls = idWeapon::GetAmmoNumForName( "ammo_souls" );
	max_souls = inventory.MaxAmmoForAmmoClass( this, "ammo_souls" );
	if ( inventory.ammo[ ammo_souls ] < max_souls ) {
		inventory.ammo[ ammo_souls ]++;
		if ( inventory.ammo[ ammo_souls ] >= max_souls ) {
			hud->HandleNamedEvent( "soulCubeReady" );
			StartSound( "snd_soulcube_ready", SND_CHANNEL_ANY, 0, false, NULL );
		}
	}
}

/*
=============
idPlayer::SetSoulCubeProjectile
=============
*/
void idPlayer::SetSoulCubeProjectile( idProjectile *projectile ) {
	soulCubeProjectile = projectile;
}

/*
=============
idPlayer::AddProjectilesFired
=============
*/
void idPlayer::AddProjectilesFired( int count ) {
	numProjectilesFired += count;
}

/*
=============
idPlayer::AddProjectileHites
=============
*/
void idPlayer::AddProjectileHits( int count ) {
	numProjectileHits += count;
}

/*
=============
idPlayer::SetLastHitTime
=============
*/
void idPlayer::SetLastHitTime( int time ) {
	idPlayer *aimed = NULL;

	if ( time && lastHitTime != time ) {
		lastHitToggle ^= 1;
	}
	lastHitTime = time;
	if ( !time ) {
		// level start and inits
		return;
	}
	if ( gameLocal.isMultiplayer && ( time - lastSndHitTime ) > 10 ) {
		lastSndHitTime = time;
		StartSound( "snd_hit_feedback", SND_CHANNEL_ANY, SSF_PRIVATE_SOUND, false, NULL );
	}
	if ( cursor ) {
		cursor->HandleNamedEvent( "hitTime" );
	}
	if ( hud ) {
		if ( MPAim != -1 ) {
			if ( gameLocal.entities[ MPAim ] && gameLocal.entities[ MPAim ]->IsType( idPlayer::Type ) ) {
				aimed = static_cast< idPlayer * >( gameLocal.entities[ MPAim ] );
			}
			assert( aimed );
			// full highlight, no fade till loosing aim
			hud->SetStateString( "aim_text", gameLocal.userInfo[ MPAim ].GetString( "ui_name" ) );
			if ( aimed ) {
				hud->SetStateFloat( "aim_color", aimed->colorBarIndex );
			}
			hud->HandleNamedEvent( "aim_flash" );
			MPAimHighlight = true;
			MPAimFadeTime = 0;
		} else if ( lastMPAim != -1 ) {
			if ( gameLocal.entities[ lastMPAim ] && gameLocal.entities[ lastMPAim ]->IsType( idPlayer::Type ) ) {
				aimed = static_cast< idPlayer * >( gameLocal.entities[ lastMPAim ] );
			}
			assert( aimed );
			// start fading right away
			hud->SetStateString( "aim_text", gameLocal.userInfo[ lastMPAim ].GetString( "ui_name" ) );
			if ( aimed ) {
				hud->SetStateFloat( "aim_color", aimed->colorBarIndex );
			}
			hud->HandleNamedEvent( "aim_flash" );
			hud->HandleNamedEvent( "aim_fade" );
			MPAimHighlight = false;
			MPAimFadeTime = gameLocal.realClientTime;
		}
	}
}

/*
=============
idPlayer::SetInfluenceLevel
=============
*/
void idPlayer::SetInfluenceLevel( int level ) {
	if ( level != influenceActive ) {
		if ( level ) {
			for ( idEntity *ent = gameLocal.spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
				if ( ent->IsType( idProjectile::Type ) ) {
					// remove all projectiles
					ent->PostEventMS( &EV_Remove, 0 );
				}
			}
			if ( weaponEnabled && weapon.GetEntity() ) {
				weapon.GetEntity()->EnterCinematic();
			}
		} else {
			physicsObj.SetLinearVelocity( vec3_origin );
			if ( weaponEnabled && weapon.GetEntity() ) {
				weapon.GetEntity()->ExitCinematic();
			}
		}
		influenceActive = level;
	}
}

/*
=============
idPlayer::SetInfluenceView
=============
*/
void idPlayer::SetInfluenceView( const char *mtr, const char *skinname, float radius, idEntity *ent ) {
	influenceMaterial = NULL;
	influenceEntity = NULL;
	influenceSkin = NULL;
	if ( mtr && *mtr ) {
		influenceMaterial = declManager->FindMaterial( mtr );
	}
	if ( skinname && *skinname ) {
		influenceSkin = declManager->FindSkin( skinname );
		if ( head.GetEntity() ) {
			head.GetEntity()->GetRenderEntity()->shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.time );
		}
		UpdateVisuals();
	}
	influenceRadius = radius;
	if ( radius > 0.0f ) {
		influenceEntity = ent;
	}
}

/*
=============
idPlayer::SetInfluenceFov
=============
*/
void idPlayer::SetInfluenceFov( float fov ) {
	influenceFov = fov;
}

/*
================
idPlayer::OnLadder
================
*/
bool idPlayer::OnLadder( void ) const {
	return physicsObj.OnLadder();
}

/*
==================
idPlayer::Event_GetButtons
==================
*/
void idPlayer::Event_GetButtons( void ) {
	idThread::ReturnInt( usercmd.buttons );
}

/*
==================
idPlayer::Event_GetMove
==================
*/
void idPlayer::Event_GetMove( void ) {
	idVec3 move( usercmd.forwardmove, usercmd.rightmove, usercmd.upmove );
	idThread::ReturnVector( move );
}

/*
================
idPlayer::Event_GetViewAngles
================
*/
void idPlayer::Event_GetViewAngles( void ) {
	idThread::ReturnVector( idVec3( viewAngles[0], viewAngles[1], viewAngles[2] ) );
}

/*
==================
idPlayer::Event_StopFxFov
==================
*/
void idPlayer::Event_StopFxFov( void ) {
	fxFov = false;
}

/*
==================
idPlayer::StartFxFov
==================
*/
void idPlayer::StartFxFov( float duration ) {
	fxFov = true;
	PostEventSec( &EV_Player_StopFxFov, duration );
}

/*
==================
idPlayer::Event_EnableWeapon
==================
*/
void idPlayer::Event_EnableWeapon( void ) {
	hiddenWeapon = gameLocal.world->spawnArgs.GetBool( "no_Weapons" );
	weaponEnabled = true;
	if ( weapon.GetEntity() ) {
		weapon.GetEntity()->ExitCinematic();
	}
}

/*
==================
idPlayer::Event_DisableWeapon
==================
*/
void idPlayer::Event_DisableWeapon( void ) {
	hiddenWeapon = gameLocal.world->spawnArgs.GetBool( "no_Weapons" );
	weaponEnabled = false;
	if ( weapon.GetEntity() ) {
		weapon.GetEntity()->EnterCinematic();
	}
}

/*
==================
idPlayer::Event_GetCurrentWeapon
==================
*/
void idPlayer::Event_GetCurrentWeapon( void ) {
	const char *weapon;

	if ( currentWeapon >= 0 ) {
		weapon = spawnArgs.GetString( va( "def_weapon%d", currentWeapon ) );
		idThread::ReturnString( weapon );
	} else {
		idThread::ReturnString( "" );
	}
}

/*
==================
idPlayer::Event_GetPreviousWeapon
==================
*/
void idPlayer::Event_GetPreviousWeapon( void ) {
	const char *weapon;

	if ( previousWeapon >= 0 ) {
		int pw = ( gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) ) ? 0 : previousWeapon;
		weapon = spawnArgs.GetString( va( "def_weapon%d", pw) );
		idThread::ReturnString( weapon );
	} else {	
		idThread::ReturnString( spawnArgs.GetString( "def_weapon0" ) );
	}
}

/*
==================
idPlayer::Event_SelectWeapon
==================
*/
void idPlayer::Event_SelectWeapon( const char *weaponName ) {
	int i;
	int weaponNum;

	if ( gameLocal.isClient ) {
		gameLocal.Warning( "Cannot switch weapons from script in multiplayer" );
		return;
	}

	if ( hiddenWeapon && gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) ) {
#ifdef _DENTONMOD
		idealWeapon = quickWeapon = weapon_fists;
#else
		idealWeapon = weapon_fists;
#endif
		weapon.GetEntity()->HideWeapon();
		return;
	}

	weaponNum = -1;
	for( i = 0; i < MAX_WEAPONS; i++ ) {
		if ( inventory.weapons & ( 1 << i ) ) {
			const char *weap = spawnArgs.GetString( va( "def_weapon%d", i ) );
			if ( !idStr::Cmp( weap, weaponName ) ) {
				weaponNum = i;
				break;
			}
		}
	}

	if ( weaponNum < 0 ) {
		gameLocal.Warning( "%s is not carrying weapon '%s'", name.c_str(), weaponName );
		return;
	}

	hiddenWeapon = false;

#ifdef _DENTONMOD
	if( idealWeapon != weaponNum ) {
		quickWeapon = idealWeapon;
	}
	idealWeapon = weaponNum;
#endif

	UpdateHudWeapon();
} 

/*
==================
idPlayer::Event_GetWeaponEntity
==================
*/
void idPlayer::Event_GetWeaponEntity( void ) {
	idThread::ReturnEntity( weapon.GetEntity() );
}

/*
==================
idPlayer::Event_OpenPDA
==================
*/
void idPlayer::Event_OpenPDA( void ) {
	if ( !gameLocal.isMultiplayer ) {
		TogglePDA();
	}
}

/*
==================
idPlayer::Event_InPDA
==================
*/
void idPlayer::Event_InPDA( void ) {
	idThread::ReturnInt( objectiveSystemOpen );
}

/*
==================
idPlayer::TeleportDeath
==================
*/
void idPlayer::TeleportDeath( int killer ) {
	teleportKiller = killer;
}

/*
==================
idPlayer::Event_ExitTeleporter
==================
*/
void idPlayer::Event_ExitTeleporter( void ) {
	idEntity	*exitEnt;
	float		pushVel;

	// verify and setup
	exitEnt = teleportEntity.GetEntity();
	if ( !exitEnt ) {
		common->DPrintf( "Event_ExitTeleporter player %d while not being teleported\n", entityNumber );
		return;
	}

	pushVel = exitEnt->spawnArgs.GetFloat( "push", "300" );

	if ( gameLocal.isServer ) {
		ServerSendEvent( EVENT_EXIT_TELEPORTER, NULL, false, -1 );
	}

	SetPrivateCameraView( NULL );
	// setup origin and push according to the exit target
	SetOrigin( exitEnt->GetPhysics()->GetOrigin() + idVec3( 0, 0, CM_CLIP_EPSILON ) );
	SetViewAngles( exitEnt->GetPhysics()->GetAxis().ToAngles() );
	physicsObj.SetLinearVelocity( exitEnt->GetPhysics()->GetAxis()[ 0 ] * pushVel );
	physicsObj.ClearPushedVelocity();
	// teleport fx
	playerView.Flash( colorWhite, 120 );

	// clear the ik heights so model doesn't appear in the wrong place
	walkIK.EnableAll();

	UpdateVisuals();

	StartSound( "snd_teleport_exit", SND_CHANNEL_ANY, 0, false, NULL );

	if ( teleportKiller != -1 ) {
		// we got killed while being teleported
		Damage( gameLocal.entities[ teleportKiller ], gameLocal.entities[ teleportKiller ], vec3_origin, "damage_telefrag", 1.0f, INVALID_JOINT );
		teleportKiller = -1;
	} else {
		// kill anything that would have waited at teleport exit
		gameLocal.KillBox( this );
	}
	teleportEntity = NULL;
}

/*
================
idPlayer::ClientPredictionThink
================
*/
void idPlayer::ClientPredictionThink( void ) {
	renderEntity_t *headRenderEnt;

	oldFlags = usercmd.flags;
	oldButtons = usercmd.buttons;

	usercmd = gameLocal.usercmds[ entityNumber ];

	if ( entityNumber != gameLocal.localClientNum ) {
		// ignore attack button of other clients. that's no good for predictions
		usercmd.buttons &= ~BUTTON_ATTACK;
	}

	buttonMask &= usercmd.buttons;
	usercmd.buttons &= ~buttonMask;

	if ( objectiveSystemOpen ) {
		usercmd.forwardmove = 0;
		usercmd.rightmove = 0;
		usercmd.upmove = 0;
	}

	// clear the ik before we do anything else so the skeleton doesn't get updated twice
	walkIK.ClearJointMods();

	if ( gameLocal.isNewFrame ) {
		if ( ( usercmd.flags & UCF_IMPULSE_SEQUENCE ) != ( oldFlags & UCF_IMPULSE_SEQUENCE ) ) {
			PerformImpulse( usercmd.impulse );
		}
	}

	scoreBoardOpen = ( ( usercmd.buttons & BUTTON_SCORES ) != 0 || forceScoreBoard );

	AdjustSpeed();

	UpdateViewAngles();

	// update the smoothed view angles
	if ( gameLocal.framenum >= smoothedFrame && entityNumber != gameLocal.localClientNum ) {
		idAngles anglesDiff = viewAngles - smoothedAngles;
		anglesDiff.Normalize180();
		if ( idMath::Fabs( anglesDiff.yaw ) < 90.0f && idMath::Fabs( anglesDiff.pitch ) < 90.0f ) {
			// smoothen by pushing back to the previous angles
			viewAngles -= gameLocal.clientSmoothing * anglesDiff;
			viewAngles.Normalize180();
		}
		smoothedAngles = viewAngles;
	}
	smoothedOriginUpdated = false;

	if ( !af.IsActive() ) {
		AdjustBodyAngles();
	}

	if ( !isLagged ) {
		// don't allow client to move when lagged
		Move();
	}

	// update GUIs, Items, and character interactions
	UpdateFocus();

	// service animations
	if ( !spectating && !af.IsActive() ) {
		UpdateConditions();
		UpdateAnimState();
		CheckBlink();
	}

	// clear out our pain flag so we can tell if we recieve any damage between now and the next time we think
	AI_PAIN = false;

	// calculate the exact bobbed view position, which is used to
	// position the view weapon, among other things
	CalculateFirstPersonView();

	// this may use firstPersonView, or a thirdPerson / camera view
	CalculateRenderView();

	if ( !gameLocal.inCinematic && weapon.GetEntity() && ( health > 0 ) && !( gameLocal.isMultiplayer && spectating ) ) {
		UpdateWeapon();
	}

	UpdateHud();

	if ( gameLocal.isNewFrame ) {
		UpdatePowerUps();
	}

	UpdateDeathSkin( false );

	if ( head.GetEntity() ) {
		headRenderEnt = head.GetEntity()->GetRenderEntity();
	} else {
		headRenderEnt = NULL;
	}

	if ( headRenderEnt ) {
		if ( influenceSkin ) {
			headRenderEnt->customSkin = influenceSkin;
		} else {
			headRenderEnt->customSkin = NULL;
		}
	}

	if ( gameLocal.isMultiplayer || g_showPlayerShadow.GetBool() ) {
		renderEntity.suppressShadowInViewID	= 0;
		if ( headRenderEnt ) {
			headRenderEnt->suppressShadowInViewID = 0;
		}
	} else {
		renderEntity.suppressShadowInViewID	= entityNumber+1;
		if ( headRenderEnt ) {
			headRenderEnt->suppressShadowInViewID = entityNumber+1;
		}
	}
	// never cast shadows from our first-person muzzle flashes
	renderEntity.suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + entityNumber;
	if ( headRenderEnt ) {
		headRenderEnt->suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + entityNumber;
	}

	if ( !gameLocal.inCinematic ) {
		UpdateAnimation();
	}

	if ( gameLocal.isMultiplayer ) {
		DrawPlayerIcons();
	}

	Present();

	UpdateDamageEffects();

	LinkCombat();

	if ( gameLocal.isNewFrame && entityNumber == gameLocal.localClientNum ) {
		playerView.CalculateShake();
	}

#ifdef _PORTALSKY //un noted change from original sdk
	// determine if portal sky is in pvs
	pvsHandle_t	clientPVS = gameLocal.pvs.SetupCurrentPVS( GetPVSAreas(), GetNumPVSAreas() );
	gameLocal.portalSkyActive = gameLocal.pvs.CheckAreasForPortalSky( clientPVS, GetPhysics()->GetOrigin() );
	gameLocal.pvs.FreeCurrentPVS( clientPVS );
#endif
}

/*
================
idPlayer::GetPhysicsToVisualTransform
================
*/
bool idPlayer::GetPhysicsToVisualTransform( idVec3 &origin, idMat3 &axis ) {
	if ( af.IsActive() ) {
		af.GetPhysicsToVisualTransform( origin, axis );
		return true;
	}

	// smoothen the rendered origin and angles of other clients
	// smooth self origin if snapshots are telling us prediction is off
	if ( gameLocal.isClient && gameLocal.framenum >= smoothedFrame && ( entityNumber != gameLocal.localClientNum || selfSmooth ) ) {
		// render origin and axis
		idMat3 renderAxis = viewAxis * GetPhysics()->GetAxis();
		idVec3 renderOrigin = GetPhysics()->GetOrigin() + modelOffset * renderAxis;

		// update the smoothed origin
		if ( !smoothedOriginUpdated ) {
			idVec2 originDiff = renderOrigin.ToVec2() - smoothedOrigin.ToVec2();
			if ( originDiff.LengthSqr() < Square( 100.0f ) ) {
				// smoothen by pushing back to the previous position
				if ( selfSmooth ) {
					assert( entityNumber == gameLocal.localClientNum );
					renderOrigin.ToVec2() -= net_clientSelfSmoothing.GetFloat() * originDiff;
				} else {
					renderOrigin.ToVec2() -= gameLocal.clientSmoothing * originDiff;
				}
			}
			smoothedOrigin = renderOrigin;

			smoothedFrame = gameLocal.framenum;
			smoothedOriginUpdated = true;
		}

		axis = idAngles( 0.0f, smoothedAngles.yaw, 0.0f ).ToMat3();
		origin = ( smoothedOrigin - GetPhysics()->GetOrigin() ) * axis.Transpose();

	} else {

		axis = viewAxis;
		origin = modelOffset;
	}
	return true;
}

/*
================
idPlayer::GetPhysicsToSoundTransform
================
*/
bool idPlayer::GetPhysicsToSoundTransform( idVec3 &origin, idMat3 &axis ) {
	idCamera *camera;

	if ( privateCameraView ) {
		camera = privateCameraView;
	} else {
		camera = gameLocal.GetCamera();
	}

	if ( camera ) {
		renderView_t view;

		memset( &view, 0, sizeof( view ) );
		camera->GetViewParms( &view );
		origin = view.vieworg;
		axis = view.viewaxis;
		return true;
	} else {
		return idActor::GetPhysicsToSoundTransform( origin, axis );
	}
}

/*
================
idPlayer::WriteToSnapshot
================
*/
void idPlayer::WriteToSnapshot( idBitMsgDelta &msg ) const {
	physicsObj.WriteToSnapshot( msg );
	WriteBindToSnapshot( msg );
	msg.WriteDeltaFloat( 0.0f, deltaViewAngles[0] );
	msg.WriteDeltaFloat( 0.0f, deltaViewAngles[1] );
	msg.WriteDeltaFloat( 0.0f, deltaViewAngles[2] );
	msg.WriteShort( health );
	msg.WriteBits( gameLocal.ServerRemapDecl( -1, DECL_ENTITYDEF, lastDamageDef ), gameLocal.entityDefBits );
	msg.WriteDir( lastDamageDir, 9 );
	msg.WriteShort( lastDamageLocation );
	msg.WriteBits( idealWeapon, idMath::BitsForInteger( MAX_WEAPONS ) );
	msg.WriteBits( inventory.weapons, MAX_WEAPONS );
	msg.WriteBits( weapon.GetSpawnId(), 32 );
	msg.WriteBits( spectator, idMath::BitsForInteger( MAX_CLIENTS ) );
	msg.WriteBits( lastHitToggle, 1 );
	msg.WriteBits( weaponGone, 1 );
	msg.WriteBits( isLagged, 1 );
	msg.WriteBits( isChatting, 1 );
}

/*
================
idPlayer::ReadFromSnapshot
================
*/
void idPlayer::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	int		i, oldHealth, newIdealWeapon, weaponSpawnId;
	bool	newHitToggle, stateHitch;

	if ( snapshotSequence - lastSnapshotSequence > 1 ) {
		stateHitch = true;
	} else {
		stateHitch = false;
	}
	lastSnapshotSequence = snapshotSequence;

	oldHealth = health;

	physicsObj.ReadFromSnapshot( msg );
	ReadBindFromSnapshot( msg );
	deltaViewAngles[0] = msg.ReadDeltaFloat( 0.0f );
	deltaViewAngles[1] = msg.ReadDeltaFloat( 0.0f );
	deltaViewAngles[2] = msg.ReadDeltaFloat( 0.0f );
	health = msg.ReadShort();
	lastDamageDef = gameLocal.ClientRemapDecl( DECL_ENTITYDEF, msg.ReadBits( gameLocal.entityDefBits ) );
	lastDamageDir = msg.ReadDir( 9 );
	lastDamageLocation = msg.ReadShort();
	newIdealWeapon = msg.ReadBits( idMath::BitsForInteger( MAX_WEAPONS ) );
	inventory.weapons = msg.ReadBits( MAX_WEAPONS );
	weaponSpawnId = msg.ReadBits( 32 );
	spectator = msg.ReadBits( idMath::BitsForInteger( MAX_CLIENTS ) );
	newHitToggle = msg.ReadBits( 1 ) != 0;
	weaponGone = msg.ReadBits( 1 ) != 0;
	isLagged = msg.ReadBits( 1 ) != 0;
	isChatting = msg.ReadBits( 1 ) != 0;

	// no msg reading below this

	if ( weapon.SetSpawnId( weaponSpawnId ) ) {
		if ( weapon.GetEntity() ) {
			// maintain ownership locally
			weapon.GetEntity()->SetOwner( this );
		}
		currentWeapon = -1;
	}
	// if not a local client assume the client has all ammo types
	if ( entityNumber != gameLocal.localClientNum ) {
		for( i = 0; i < AMMO_NUMTYPES; i++ ) {
			inventory.ammo[ i ] = 999;
		}
	}

	if ( oldHealth > 0 && health <= 0 ) {
		if ( stateHitch ) {
			// so we just hide and don't show a death skin
			UpdateDeathSkin( true );
		}
		// die
		AI_DEAD = true;
		ClearPowerUps();
		SetAnimState( ANIMCHANNEL_LEGS, "Legs_Death", 4 );
		SetAnimState( ANIMCHANNEL_TORSO, "Torso_Death", 4 );
		SetWaitState( "" );
		animator.ClearAllJoints();
		if ( entityNumber == gameLocal.localClientNum ) {
			playerView.Fade( colorBlack, 12000 );
		}
		StartRagdoll();
		physicsObj.SetMovementType( PM_DEAD );
		if ( !stateHitch ) {
			StartSound( "snd_death", SND_CHANNEL_VOICE, 0, false, NULL );
		}
		if ( weapon.GetEntity() ) {
			weapon.GetEntity()->OwnerDied();
		}
	} else if ( oldHealth <= 0 && health > 0 ) {
		// respawn
		Init( false );
		StopRagdoll();
		SetPhysics( &physicsObj );
		physicsObj.EnableClip();
		SetCombatContents( true );
	} else if ( health < oldHealth && health > 0 ) {
		if ( stateHitch ) {
			lastDmgTime = gameLocal.time;
		} else {
			// damage feedback
			const idDeclEntityDef *def = static_cast<const idDeclEntityDef *>( declManager->DeclByIndex( DECL_ENTITYDEF, lastDamageDef, false ) );
			if ( def ) {
				playerView.DamageImpulse( lastDamageDir * viewAxis.Transpose(), &def->dict );
				AI_PAIN = Pain( NULL, NULL, oldHealth - health, lastDamageDir, lastDamageLocation );
				lastDmgTime = gameLocal.time;
			} else {
				common->Warning( "NET: no damage def for damage feedback '%d'\n", lastDamageDef );
			}
		}
	} else if ( health > oldHealth && PowerUpActive( MEGAHEALTH ) && !stateHitch ) {
		// just pulse, for any health raise
		healthPulse = true;
	}

	// If the player is alive, restore proper physics object
	if ( health > 0 && IsActiveAF() ) {
		StopRagdoll();
		SetPhysics( &physicsObj );
		physicsObj.EnableClip();
		SetCombatContents( true );
	}

	if ( idealWeapon != newIdealWeapon ) {
		if ( stateHitch ) {
			weaponCatchup = true;
		}
		idealWeapon = newIdealWeapon;
		UpdateHudWeapon();
	}

	if ( lastHitToggle != newHitToggle ) {
		SetLastHitTime( gameLocal.realClientTime );
	}

	if ( msg.HasChanged() ) {
		UpdateVisuals();
	}
}

/*
================
idPlayer::WritePlayerStateToSnapshot
================
*/
void idPlayer::WritePlayerStateToSnapshot( idBitMsgDelta &msg ) const {
	int i;

	/* //un noted change from original sdk
	msg.WriteByte( bobCycle );
	msg.WriteInt( stepUpTime );
	msg.WriteFloat( stepUpDelta );
	*/
	msg.WriteInt( inventory.weapons );//new
	//	msg.WriteShort( inventory.weapons );
	msg.WriteByte( inventory.armor );

	for( i = 0; i < AMMO_NUMTYPES; i++ ) {
		msg.WriteBits( inventory.ammo[i], ASYNC_PLAYER_INV_AMMO_BITS );
	}
	for( i = 0; i < MAX_WEAPONS; i++ ) {
		msg.WriteBits( inventory.clip[i], ASYNC_PLAYER_INV_CLIP_BITS );
	}
}

/*
================
idPlayer::ReadPlayerStateFromSnapshot
================
*/
void idPlayer::ReadPlayerStateFromSnapshot( const idBitMsgDelta &msg ) {
	int i, ammo;

	/* //un noted change from original sdk
	bobCycle = msg.ReadByte();
	stepUpTime = msg.ReadInt();
	stepUpDelta = msg.ReadFloat();
	*/
	inventory.weapons = msg.ReadInt();//new
	//	inventory.weapons = msg.ReadShort();
	inventory.armor = msg.ReadByte();

	for( i = 0; i < AMMO_NUMTYPES; i++ ) {
		ammo = msg.ReadBits( ASYNC_PLAYER_INV_AMMO_BITS );
		if ( gameLocal.time >= inventory.ammoPredictTime ) {
			inventory.ammo[ i ] = ammo;
		}
	}
	for( i = 0; i < MAX_WEAPONS; i++ ) {
		inventory.clip[i] = msg.ReadBits( ASYNC_PLAYER_INV_CLIP_BITS );
	}
}

/*
================
idPlayer::ServerReceiveEvent
================
*/
bool idPlayer::ServerReceiveEvent( int event, int time, const idBitMsg &msg ) {

	if ( idEntity::ServerReceiveEvent( event, time, msg ) ) {
		return true;
	}

	// client->server events
	switch( event ) {
		case EVENT_IMPULSE: {
			PerformImpulse( msg.ReadBits( 6 ) );
			return true;
							}
		default: {
			return false;
				 }
	}
}

/*
================
idPlayer::ClientReceiveEvent
================
*/
bool idPlayer::ClientReceiveEvent( int event, int time, const idBitMsg &msg ) {
	int powerup;
	bool start;

	switch ( event ) {
		case EVENT_EXIT_TELEPORTER:
			Event_ExitTeleporter();
			return true;
		case EVENT_ABORT_TELEPORTER:
			SetPrivateCameraView( NULL );
			return true;
		case EVENT_POWERUP: {
			powerup = msg.ReadShort();
			start = msg.ReadBits( 1 ) != 0;
			if ( start ) {
				GivePowerUp( powerup, 0 );
			} else {
				ClearPowerup( powerup );
			}
			return true;
							}
		case EVENT_PICKUPNAME: { //New, Not so sure what it does
			char buf[MAX_EVENT_PARAM_SIZE];
			msg.ReadString(buf, MAX_EVENT_PARAM_SIZE);
			inventory.AddPickupName(buf, "", this); //New from_D3XP
			return true;
							   }
		case EVENT_SPECTATE: {
			bool spectate = ( msg.ReadBits( 1 ) != 0 );
			Spectate( spectate );
			return true;
							 }
		case EVENT_ADD_DAMAGE_EFFECT: {
			if ( spectating ) {
				// if we're spectating, ignore
				// happens if the event and the spectate change are written on the server during the same frame (fraglimit)
				return true;
			}
			break;
		}
		default:
			break;
	}

	return idActor::ClientReceiveEvent( event, time, msg );
}

/*
================
idPlayer::Hide
================
*/
void idPlayer::Hide( void ) {
	idWeapon *weap;

	idActor::Hide();
	weap = weapon.GetEntity();
	if ( weap ) {
		weap->HideWorldModel();
	}
}

/*
================
idPlayer::Show
================
*/
void idPlayer::Show( void ) {
	idWeapon *weap;

	idActor::Show();
	weap = weapon.GetEntity();
	if ( weap ) {
		weap->ShowWorldModel();
	}
}

/*
===============
idPlayer::StartAudioLog
===============
*/
void idPlayer::StartAudioLog( void ) {
	if ( hud ) {
		hud->HandleNamedEvent( "audioLogUp" );
	}
}

/*
===============
idPlayer::StopAudioLog
===============
*/
void idPlayer::StopAudioLog( void ) {
	if ( hud ) {
		hud->HandleNamedEvent( "audioLogDown" );
	}
}

/*
===============
idPlayer::ShowTip
===============
*/
void idPlayer::ShowTip( const char *title, const char *tip, bool autoHide ) {
	if ( tipUp ) {
		return;
	}
	hud->SetStateString( "tip", tip );
	hud->SetStateString( "tiptitle", title );
	hud->HandleNamedEvent( "tipWindowUp" );
	if ( autoHide ) {
		CancelEvents( &EV_Player_HideTip ); //ivan - make sure there are no other events that could hide this too soon.
		PostEventSec( &EV_Player_HideTip, 10.0f ); //ivan - was 5.0f 
	}
	tipUp = true;
}

/*
===============
idPlayer::HideTip
===============
*/
void idPlayer::HideTip( void ) {
	hud->HandleNamedEvent( "tipWindowDown" );
	tipUp = false;
}

/*
===============
idPlayer::Event_HideTip
===============
*/
void idPlayer::Event_HideTip( void ) {
	HideTip();
}

//ivan start
/*
===============
idPlayer::ShowInfo
===============
*/
void idPlayer::ShowInfo( const char *text, float time ) {
	hud->SetStateString( "info", text );
	hud->HandleNamedEvent( "infoWindowUp" ); 
		
	CancelEvents( &EV_Player_HideInfo ); //make sure there are no other events that could hide this too soon.
	PostEventSec( &EV_Player_HideInfo, time ); 
}

/*
===============
idPlayer::HideInfo
===============
*/
void idPlayer::HideInfo( void ) {
	hud->HandleNamedEvent( "infoWindowDown" ); 
}

/*
===============
idPlayer::Event_HideInfo
===============
*/
void idPlayer::Event_HideInfo( void ) {
	HideInfo();
}

/*
===============
idPlayer::Event_GetWaterLevel
===============
*/
void idPlayer::Event_GetWaterLevel( void ) {
	idThread::ReturnInt( (int) physicsObj.GetWaterLevel() );
}

//ivan end

/*
===============
idPlayer::ShowObjective
===============
*/
void idPlayer::ShowObjective( const char *obj ) {
	hud->HandleNamedEvent( obj );
	objectiveUp = true;
}

/*
===============
idPlayer::HideObjective
===============
*/
void idPlayer::HideObjective( void ) {
	hud->HandleNamedEvent( "closeObjective" );
	objectiveUp = false;
}

/*
===============
idPlayer::Event_StopAudioLog
===============
*/
void idPlayer::Event_StopAudioLog( void ) {
	StopAudioLog();
}

/*
===============
idPlayer::SetSpectateOrigin
===============
*/
void idPlayer::SetSpectateOrigin( void ) {
	idVec3 neworig;

	neworig = GetPhysics()->GetOrigin();
	neworig[ 2 ] += EyeHeight();
	neworig[ 2 ] += 25;
	SetOrigin( neworig );
}

/*
===============
idPlayer::RemoveWeapon
===============
*/
void idPlayer::RemoveWeapon( const char *weap ) {
	if ( weap && *weap ) {
		inventory.Drop( spawnArgs, spawnArgs.GetString( weap ), -1 );
	}
}

/*
===============
idPlayer::CanShowWeaponViewmodel
===============
*/
bool idPlayer::CanShowWeaponViewmodel( void ) const {
	return showWeaponViewModel;
}

/*
===============
idPlayer::SetLevelTrigger
===============
*/
void idPlayer::SetLevelTrigger( const char *levelName, const char *triggerName ) {
	if ( levelName && *levelName && triggerName && *triggerName ) {
		idLevelTriggerInfo lti;
		lti.levelName = levelName;
		lti.triggerName = triggerName;
		inventory.levelTriggers.Append( lti );
	}
}

/*
===============
idPlayer::Event_LevelTrigger
===============
*/
void idPlayer::Event_LevelTrigger( void ) {
	idStr mapName = gameLocal.GetMapName();
	mapName.StripPath();
	mapName.StripFileExtension();
	for ( int i = inventory.levelTriggers.Num() - 1; i >= 0; i-- ) {
		if ( idStr::Icmp( mapName, inventory.levelTriggers[i].levelName) == 0 ){
			idEntity *ent = gameLocal.FindEntity( inventory.levelTriggers[i].triggerName );
			if ( ent ) {
				ent->PostEventMS( &EV_Activate, 1, this );
			}
		}
	}
}

/*
===============
idPlayer::Event_Gibbed
===============
*/
void idPlayer::Event_Gibbed( void ) {
}

/*
==================
idPlayer::Event_GetIdealWeapon
==================
*/
void idPlayer::Event_GetIdealWeapon( void ) {
	const char *weapon;

	if ( idealWeapon >= 0 ) {
		weapon = spawnArgs.GetString( va( "def_weapon%d", idealWeapon ) );
		idThread::ReturnString( weapon );
	} else {
		idThread::ReturnString( "" );
	}
}

/*
===============
idPlayer::UpdatePlayerIcons
===============
*/
void idPlayer::UpdatePlayerIcons( void ) {
	int time = networkSystem->ServerGetClientTimeSinceLastPacket( entityNumber );
	if ( time > cvarSystem->GetCVarInteger( "net_clientMaxPrediction" ) ) {
		isLagged = true;
	} else {
		isLagged = false;
	}
}

/*
===============
idPlayer::DrawPlayerIcons
===============
*/
void idPlayer::DrawPlayerIcons( void ) {
	if ( !NeedsIcon() ) {
		playerIcon.FreeIcon();
		return;
	}
	playerIcon.Draw( this, headJoint );
}

/*
===============
idPlayer::HidePlayerIcons
===============
*/
void idPlayer::HidePlayerIcons( void ) {
	playerIcon.FreeIcon();
}

/*
===============
idPlayer::NeedsIcon
==============
*/
bool idPlayer::NeedsIcon( void ) {
	// local clients don't render their own icons... they're only info for other clients
	return entityNumber != gameLocal.localClientNum && ( isLagged || isChatting );
}

/*
==================
idPlayer::Event_WeaponAvailable
==================
*/
void idPlayer::Event_WeaponAvailable( const char* name ) {

	idThread::ReturnInt( WeaponAvailable(name) ? 1 : 0 );
}

/*
==================
idPlayer::Event_GetImpulseKey
==================
*/
void idPlayer::Event_GetImpulseKey( void ) {
	idThread::ReturnInt( usercmd.impulse );
}

bool idPlayer::WeaponAvailable( const char* name ) {
	for( int i = 0; i < MAX_WEAPONS; i++ ) {
		if ( inventory.weapons & ( 1 << i ) ) {
			const char *weap = spawnArgs.GetString( va( "def_weapon%d", i ) );
			if ( !idStr::Cmp( weap, name ) ) {
				return true;
			}
		}
	}
	return false;
}


/*  
=================
idPlayer::GetCurrentWeapon			//New
=================

idStr idPlayer::GetCurrentWeapon() {
const char *weapon;

if ( currentWeapon >= 0 ) {
weapon = spawnArgs.GetString( va( "def_weapon%d", currentWeapon ) );
return weapon;
} else {
return "";
}
}
*/

//ivan start

/*
===============
idPlayer::IsComboActive
==============
*/
bool idPlayer::IsComboActive( void ) {
	return comboOn;
}


/*
===============
idPlayer::Event_HudEvent
==============
*/
void idPlayer::Event_HudEvent( const char* name ) {
	if ( hud ) {
		hud->HandleNamedEvent( name );
	}
}

/*
===============
idPlayer::Event_SetGravityInAnimMove
==============
*/

void idPlayer::Event_SetGravityInAnimMove( float mult ) {
	//physicsObj.SetGravityInAnimMove( 1 ); //mult

	//never accept negative values. They means frozen movement. 
	if( mult < 0.0f  ){
		mult = 1.0f;
		animMoveNoGravity = true;
	}else{
		animMoveNoGravity = false;

		//FIX!
		//we cannot really set 0 or everything will be fucked up. Sets a value little enough.
		if( mult == 0.0f ){ 
			mult = 1e-4f;
		}
	}

	physicsObj.SetGravity( mult * gameLocal.GetGravity() );
}
	
/*
===============
idPlayer::Event_SetHudParm
==============
*/
void idPlayer::Event_SetHudParm( const char *key, const char *val ) {
	if ( hud ) {
		hud->SetStateString( key, val );
	}
}

/*
===============
idPlayer::Event_GetHudFloat
==============
*/
void idPlayer::Event_GetHudFloat( const char *key ) {
	if ( hud ) {
		idThread::ReturnFloat(hud->GetStateFloat(key));
	}else{
		idThread::ReturnFloat(0.0f);
	}
}

/*
===============
idPlayer::Event_ShowStats
==============
*/
void idPlayer::Event_ShowStats( void ) {
	int secs;
	idStr time_secs;
	idStr time_mins;
	idStr time_hrs;
	idStr time_on_hud;
	idStr killed_ratio;
	idStr secrets_ratio;
	int temp_int;
	
	if ( hud ) {
		//1 -- time --
		secs = MS2SEC( gameLocal.time );

		//secs
		temp_int = secs % 60;
		(temp_int < 10) ? sprintf( time_secs, "0%d", temp_int ) : sprintf( time_secs, "%d", temp_int );

		//mins
		temp_int = (secs/60) % 60;
		(temp_int < 10) ? sprintf( time_mins, "0%d", temp_int ) : sprintf( time_mins, "%d", temp_int );

		//hours
		temp_int = (secs/3600) % 24;
		(temp_int < 10) ? sprintf( time_hrs, "0%d", temp_int ) : sprintf( time_hrs, "%d", temp_int );

		//put together
		sprintf( time_on_hud, "%s:%s:%s", time_hrs.c_str(), time_mins.c_str(), time_secs.c_str() );
		hud->SetStateString( "time_on_hud", time_on_hud.c_str() );
		//gameLocal.Printf("time_on_hud: %s \n", time_on_hud.c_str() );

		//2 -- killed ratio --
		sprintf( killed_ratio, "%d / %d",  gameLocal.enemies_killed_counter, gameLocal.enemies_spawned_counter );
		hud->SetStateString( "killed_ratio", killed_ratio.c_str() );
		//gameLocal.Printf("killed_ratio: %s \n", killed_ratio.c_str() );

		//3 -- secrets ratio --
		sprintf( secrets_ratio, "%d / %d",  gameLocal.secrets_found_counter, gameLocal.secrets_spawned_counter );
		hud->SetStateString( "secrets_ratio", secrets_ratio.c_str() );
		//gameLocal.Printf("secrets_ratio: %s \n", secrets_ratio.c_str() );

		//4 -- health lost --
		hud->SetStateInt( "health_lost", health_lost );
		//gameLocal.Printf("health_lost: %d \n", health_lost );

		//-- show --
		hud->HandleNamedEvent( "showStats" );
	}
}

/*
===============
idPlayer::Event_DropWeapon
==============
*/
void idPlayer::Event_DropWeapon( int weapNum ) {
	//gameLocal.Printf("Event_DropWeapon\n");
	if(weapNum < 0 || weapNum > MAX_WEAPONS){
		gameLocal.Warning("Weapon %d out of range", weapNum);
		return;
	}
	if ( ( inventory.weapons & ( 1 << weapNum ) ) == 0 ) {
		gameLocal.Warning("We don't have weapon %d", weapNum);
		return;
	}

	if( weapNum == currentWeapon ){
		//gameLocal.Printf("drop current weapon\n");
		if( !DropWeapon( false, true ) ){ //parm2: if it is true, DropWeapon will select another weapon
			//weapon was not dropped, so we'll try to drop it later.
			PostEventMS( &EV_Player_DropWeapon, 100, weapNum );
		}
	}else{
		DropNotSelectedWeapon( weapNum );
	}
}

/*
==============
idPlayer::UpdateCameraCvarsFromSettings
==============
*/
void idPlayer::UpdateCameraCvarsFromSettings( void ) {
	pm_thirdPersonRange.SetFloat( cameraSettings.distance );
	pm_thirdPersonRange.ClearModified();
	pm_thirdPersonHeight.SetFloat( cameraSettings.height );
	pm_thirdPersonHeight.ClearModified();
	//pm_thirdPersonZ.SetBool( !cameraSettings.lockZaxis );
	//pm_thirdPersonZ.ClearModified();
}


/*
==============
idPlayer::UpdateCameraSettingsFromEntity
Entity could be: spawnPoint, target, teleport, ...
==============
*/
void idPlayer::UpdateCameraSettingsFromEntity( idEntity *ent ) {
	float cameraDist, cameraHeight;
	float cameraLockedYpos, cameraLockedZpos;
	bool cameraLockedY, cameraLockedZ;

	const idDict &dict = ent->spawnArgs;

	if ( dict.GetFloat( "cameraDist", "0", cameraDist ) ) {
		if ( cameraDist <= 0 ) {
			cameraDist = CAMERA_DEFAULT_DISTANCE;
		}
		SetCameraDistance( cameraDist, dict.GetBool( "cameraDistBlend" ) );
	}
	if ( dict.GetFloat( "cameraHeight", "0", cameraHeight ) ) {
		if ( cameraHeight <= 0 ) {
			cameraHeight = CAMERA_DEFAULT_HEIGHT;
		}
		SetCameraHeight( cameraHeight, dict.GetBool( "cameraHeightBlend" ) );
	}
	if ( dict.GetBool( "cameraLockedY", "0", cameraLockedY ) ) { //horizontal
		if ( cameraLockedY ) {
			if ( !dict.GetFloat( "cameraLockedYpos", "0", cameraLockedYpos ) ) {
				cameraLockedYpos = ent -> GetPhysics()->GetOrigin().y;
			}
			LockYCamera( cameraLockedYpos );
		} else {
			UnlockYCamera();
		}
	}
	if ( dict.GetBool( "cameraLockedZ", "0", cameraLockedZ ) ) { //vertical
		if ( cameraLockedZ ) {
			if ( !dict.GetFloat( "cameraLockedZpos", "0", cameraLockedZpos ) ) {
				cameraLockedZpos = ent -> GetPhysics()->GetOrigin().z;
			}
			LockZCamera( cameraLockedZpos );
		} else {
			UnlockZCamera();
		}
	}
}

/*
==============
idPlayer::LockYCamera
==============
*/
void idPlayer::LockYCamera( float ypos ) {
	if ( !cameraSettings.lockYaxis || ( cameraSettings.lockedYpos != ypos ) ) { //do this only if camera is free || new position
		cameraSettings.lockYaxis = true;
		cameraSettings.lockedYpos = ypos;
		enableCameraYblend = true; //blend forced pos
	}
}

/*
==============
idPlayer::LockZCamera
==============
*/
void idPlayer::LockZCamera( float zpos ) {
	if ( !cameraSettings.lockZaxis || ( cameraSettings.lockedZpos != zpos ) ) { //do this only if camera is free || new position
		cameraSettings.lockZaxis = true;
		cameraSettings.lockedZpos = zpos;
		//blending is enabled by default for Z axis
	}
}

/*
==============
idPlayer::UnlockYCamera
==============
*/
void idPlayer::UnlockYCamera( void ) {
	if ( cameraSettings.lockYaxis ) { //set it free only if it's currently not free
		cameraSettings.lockYaxis = false;
		enableCameraYblend = true; //blend to free pos
	}
}

/*
==============
idPlayer::UnlockZCamera
==============
*/
void idPlayer::UnlockZCamera( void ) {
	if ( cameraSettings.lockZaxis ) { //set it free only if it's currently not free
		cameraSettings.lockZaxis = false;
		//blending is enabled by default for Z axis
	}
}


/*
==============
idPlayer::SetCameraDistance
==============
*/
void idPlayer::SetCameraDistance( float distance, bool blend ){
	//gameLocal.Printf("SetCameraDistance: distance %f\n", distance );
	if( distance > 0.0f && cameraSettings.distance != distance ){ 
		cameraSettings.distance = distance; 
		enableCameraXblend = blend; //blend to new distance
		gameLocal.UpdateSeeDistances( distance );
	}
	//else{ gameLocal.Printf("SetCameraDistance: upd ignored\n" ); }
}

/*
==============
idPlayer::SetCameraHeight
==============
*/
void idPlayer::SetCameraHeight( float height, bool blend ){
	//gameLocal.Printf("SetCameraHeight: height %f\n", height );
	if( cameraSettings.height != height ){ 
		cameraSettings.height = height; 
		skipCameraZblend = !blend; //skip blending if blend is false
	}
	//else{ gameLocal.Printf("SetCameraHeight: upd ignored\n" ); }
}

/*
==============
idPlayer::Event_DoubleJumpEnabled
==============
*/
void idPlayer::Event_DoubleJumpEnabled( int on ){
	physicsObj.SetDoubleJumpEnabled( (on != 0) );
}

/*
==============
idPlayer::Event_WallJumpEnabled
==============
*/
void idPlayer::Event_WallJumpEnabled( int on ){
	physicsObj.SetWallJumpEnabled( (on != 0) );
}



/*
==============
idPlayer::Event_SetSkin
==============
*/
void idPlayer::Event_SetSkin( const char *skinname ) {
	skin = declManager->FindSkin( skinname );
}

/*
==============
idPlayer::StartForcedMov
==============
*/
bool idPlayer::StartForcedMov( idEntity *destinationEntity, int inhibitInputDelay, bool canAbort, bool totalForce, bool forceStart ){
	
	inhibitAimCrouchTime = gameLocal.time + 100;
	//gameLocal.Printf("StartForcedMov\n");

	//do nothing if already enabled or invalid target
	if( forcedMovState != FORCEDMOVE_STATE_DISABLED || !destinationEntity ){
		return false; 
	}

	if( !forceStart ){
		if( totalForce ){
			if( !AI_ONGROUND ) return false; //if totalForce, we must be touching the ground 
		}else{
			if( usercmd.forwardmove != 0 )	return false; //don't do it if also walking fw/bw
			if( usercmd.upmove > 0 )		return false; //don't change pos if jumping
		}
	}
	
	//set target stuff
	forcedMovTarget = destinationEntity;
	forcedMovOldOrg = GetPhysics()->GetOrigin();
	forcedMovDelta = forcedMovTarget->GetPhysics()->GetOrigin() - forcedMovOldOrg;
	forcedMovDelta.z = 0; //ignore height

	//upd flags and vars
	forcedMovWasLocked = isXlocked; //remember the old value
	//ignoredByAI = true; //AI cannot attack us during a forced movement (NOTE: melee attacks are still allowed) 
	isXlocked = false; //unlock to move
	blendModelYaw = true; //blend toward the destination Entity
	forcedMovCanBeAborted = canAbort;
	forcedMovTotalForce = totalForce; //controls disabled, exact destination pos checks
	forcedMovIncreasingX = ( forcedMovDelta.x > 0 ); //are we moving toward the camera or the opposite?
	if( inhibitInputDelay > 0 ){ //useful after a teleport
		inhibitInputTime = gameLocal.time + inhibitInputDelay; 
	}

	//set state
	forcedMovState = FORCEDMOVE_STATE_GOINGFW;
	return true;
}

/*
==============
idPlayer::BlockForcedMov

Called by triggers that can block the movement
==============
*/
void idPlayer::BlockForcedMov( void ){ 
	
	//do nothing if disabled
	if( forcedMovState == FORCEDMOVE_STATE_DISABLED ){ 
		return; 
	}

	//don't block if aborting. This prevents the trigger from blocking us forever.
	if( forcedMovState != FORCEDMOVE_STATE_ABORTING ){ 
		forcedMovState = FORCEDMOVE_STATE_BLOCKED;
	}

	if( !forcedMovCanBeAborted ){
		gameLocal.Warning( "BlockForcedMov but cannot be aborted! trapped!" );
		//forcedMovCanBeAborted = true; //fix
	}
}

/*
==============
idPlayer::UpdForcedMov
==============
*/
void idPlayer::UpdForcedMov( void ){ //this in the only function that can ALWAYS see REAL inputs because it's called at the beginning of ::Think
	
	//noclip fix
	if ( noclip ) {
		StopForcedMov();
		return;
	}

	inhibitAimCrouchTime = gameLocal.time + 100;
	//gameLocal.Printf("UpdForcedMov\n");

	//abort or resume?
	if( forcedMovCanBeAborted && !blendModelYaw ){ //cannot abort/resume while blending
		if( ( usercmd.buttons & BUTTON_7 ) != 0 ){ //button down
			//if we are going up --> abort, else resume.
			if( forcedMovState == FORCEDMOVE_STATE_BLOCKED ){
				if( forcedMovIncreasingX ){ forcedMovState = FORCEDMOVE_STATE_ABORTING; } //only allow abort
			}else{
				forcedMovState = forcedMovIncreasingX ? FORCEDMOVE_STATE_ABORTING : FORCEDMOVE_STATE_GOINGFW;
			}
		}else if(( usercmd.buttons & BUTTON_6 ) != 0 ){ //button up
			//if we are going up --> resume, else abort.
			if( forcedMovState == FORCEDMOVE_STATE_BLOCKED ){
				if( !forcedMovIncreasingX ){ forcedMovState = FORCEDMOVE_STATE_ABORTING; } //only allow abort
			}else{
				forcedMovState = forcedMovIncreasingX ? FORCEDMOVE_STATE_GOINGFW : FORCEDMOVE_STATE_ABORTING;
			}
		}
	}

	//always upd delta, even if waiting state (in fact, player could still be moving)
	if( forcedMovState == FORCEDMOVE_STATE_ABORTING ){ 
		forcedMovDelta = forcedMovOldOrg - GetPhysics()->GetOrigin(); //delta from initial pos and current one
	}else if( forcedMovTarget ){	
		forcedMovDelta = forcedMovTarget->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin(); //delta from target and player
	}else{
		gameLocal.Error("Forced movement but no forcedMovTarget!");
	}
	forcedMovDelta.z = 0; //ignore height - otherwise player would look to target origin


	//do nothing if waiting
	if( forcedMovState == FORCEDMOVE_STATE_WAITING || forcedMovState == FORCEDMOVE_STATE_BLOCKED ){ 
		return;
	}

	// Y dist (only if TotalForce)
	if( forcedMovTotalForce && ( forcedMovDelta.y > 5.0f || forcedMovDelta.y < -5.0f ) ){ //check also Y axis
		return; //too far
	}

	// X dist
	if( forcedMovDelta.x < 5.0f && forcedMovDelta.x > -5.0f ){ //near enough on X axis... be a bit tolerant.		
		
		//target destinaton reached
		if( forcedMovState == FORCEDMOVE_STATE_GOINGFW ){ 		
			//stop walking if destination target is reached but it doesn't allow lock
			if ( !forcedMovTarget->spawnArgs.GetBool("allowLock","1") ){ //allow by default
				if( !AI_ONGROUND ){
					physicsObj.SetLinearVelocity( vec3_origin );
				}
				forcedMovState = FORCEDMOVE_STATE_WAITING; //forcedMovWaiting = true; 
				return;
			}
			
			//trigger target - NOTE: this could reactivate forcedMov mode next tic
			if( !forcedMovTarget->spawnArgs.GetBool( "noTrigWhenReached", "0" ) ){ //triggered by default
				if ( forcedMovTarget->RespondsTo( EV_Activate ) ) {
					//forcedMovTarget->ProcessEvent( &EV_Activate, this ); //this is like a function call
					forcedMovTarget->PostEventMS( &EV_Activate, 0, this ); //next tic
				}
			}
		} //end fw

		//movement completed (aborted or not, we don't care now)
		StopForcedMov();

		//set the direction suggested by the player, if any
		if( usercmd.forwardmove > 0 ){
			viewPos = -VIEWPOS_LEFT_MAX;
		}else if( usercmd.forwardmove < 0 ){
			viewPos = VIEWPOS_LEFT_MAX;
		}
	} //end near enough 
}

/*
==============
idPlayer::StopForcedMov
==============
*/
void idPlayer::StopForcedMov( void ){
	//gameLocal.Printf("StopForcedMov\n");

	if( forcedMovState == FORCEDMOVE_STATE_DISABLED ) return; //already disabled 

	//new X pos - we'll be pushed in place if isXlocked is true
	if ( noclip ) {
		fastXpos = GetPhysics()->GetOrigin().x; //where we are now
	}else if( forcedMovState == FORCEDMOVE_STATE_ABORTING ){ 
		fastXpos = forcedMovOldOrg.x; //old origin
	}else{
		fastXpos = forcedMovTarget->GetPhysics()->GetOrigin().x; //target origin
	}

	//clear target & infos
	forcedMovTarget = NULL;
	forcedMovDelta = vec3_origin;
	forcedMovOldOrg = vec3_origin;
	
	//set flags
	isXlocked = forcedMovWasLocked; //restore old value
	//ignoredByAI = false; //AI can attack us again
	blendModelYaw = true; //blend toward usual left or right position
	forcedMovCanBeAborted = false;
	forcedMovTotalForce = false;

	//set state
	forcedMovState = FORCEDMOVE_STATE_DISABLED;
}

/*
==============
idPlayer::Interact
==============
*/
void idPlayer::Interact( void ){
	if( !InteractTouchingTriggers( INTERACT_IMPULSE ) ){ //try to interact
		StartSound( "snd_noInteraction", SND_CHANNEL_ITEM, 0, false, NULL );
	}
}

/*
==============
idPlayer::AddPossibleInteract
==============
*/
void idPlayer::AddPossibleInteract( int flags ){
	interactFlag |= flags; //add the flag
}

/*
==============
idPlayer::AddWeaponInteract
==============
*/
void idPlayer::AddWeaponInteract( int flags, int weaponNum ){ //, const char * weaponName
	interactFlag |= flags; //add the flag
	if( interactShownWeaponNum == -1 ){ //set only the first one
		interactShownWeaponNum = weaponNum;
		//interactShownWeaponName = weaponName;
	}
}

/*
==============
idPlayer::ShowPossibleInteract
==============
*/
void idPlayer::ShowPossibleInteract( void ){
	if( hud ){
		if( interactFlag & INTERACT_IMPULSE ){ 
			hud->HandleNamedEvent( "interactImpulse" );
		}
		if( interactFlag & INTERACT_UP ){ 
			hud->HandleNamedEvent( "interactUp" );
		}
		if( interactFlag & INTERACT_DOWN ){ 
			hud->HandleNamedEvent( "interactDown" );
		} 
		if( interactShownWeaponNum != -1 ){
			hud->SetStateInt( "weapon_interactNum", interactShownWeaponNum );
			//hud->SetStateString( "weapon_interactName", interactShownWeaponName );
			hud->HandleNamedEvent( "interactWeapon" );
		}
	}
	interactFlag = 0; //clear the flags so we don't find this again next tic
	interactShownWeaponNum = -1;
	
}

//smart AI start
/*
===============
idPlayer::Event_ForceUpdateNpcStatus
==============
*/
void idPlayer::Event_ForceUpdateNpcStatus( void ) { //ff1.1
	if ( focusCharacter && hud ) {
		if(focusCharacter->spawnArgs.GetBool( "showStatus", "0" )){  
			hud->SetStateString( "npc", "Status:" );
			hud->SetStateString( "npc_action", focusCharacter->spawnArgs.GetString( "shownState", "Inactive" ) );
		}
	}
}

/*
===============
idPlayer::Event_SetCommonEnemy
==============
*/
void idPlayer::Event_SetCommonEnemy( idEntity *enemy ) { 
	if ( enemy && enemy->IsType( idActor::Type ) ) {
		friendsCommonEnemy = static_cast<idActor *>( enemy );
	}else{
		friendsCommonEnemy = NULL;
	}
}

/*
===============
idPlayer::Event_GetCommonEnemy
==============
*/
void idPlayer::Event_GetCommonEnemy( void ) { 
	idActor *ent = friendsCommonEnemy.GetEntity();
	if ( ent && ent->health <= 0 ) {
		friendsCommonEnemy = NULL;
		ent = NULL;

		//gameLocal.Printf("CommonEnemy Health <= 0!\n");
	}

	/*
	else if ( !ent ){ //test only
		gameLocal.Printf("CommonEnemy not valid!\n");
	}
	*/

	idThread::ReturnEntity( ent );
}

//smart AI end

//ivan start - anim move


/*
===============
idPlayer::Event_SetFullBodyAnimOn
==============
*/
void idPlayer::Event_SetFullBodyAnimOn( int anim_movement, int allow_turn, int iscombo ) {
/* replaced with rivensin version rev 2019
	blendModelYaw = false;
	force_torso_override = true;
	animBasedMovement = ( anim_movement != 0 );
*/

	blendModelYaw = false;
	force_torso_override = true;

	if ( anim_movement >= 0 && anim_movement < ANIMMOVE_NUMTYPES ) {
		animMoveType = anim_movement;
	}else{
		gameLocal.Error("Unknown animMoveType!");
	}

	allowTurn = ( allow_turn != 0 );
	comboOn = ( iscombo != 0 );

}


/*
===============
idPlayer::Event_SetFullBodyAnimOff
==============
*/
void idPlayer::Event_SetFullBodyAnimOff( void ) {
/* replaced with rivensin version rev 2019:
	blendModelYaw = false; //directly go to the dir
	force_torso_override = false;
	animBasedMovement = false;
*/
	blendModelYaw = true;
	force_torso_override = false;

	animMoveNoGravity = false;
	animMoveType = ANIMMOVE_NONE;
	allowTurn = true;
	comboOn = false;

	//reset gravity
	animMoveNoGravity = false;
	physicsObj.SetGravity( gameLocal.GetGravity() );
}

/*
=====================
idAI::GetMoveDelta
=====================
*/
void idPlayer::GetMoveDelta( const idMat3 &oldaxis, const idMat3 &axis, idVec3 &delta ) {
	idVec3 oldModelOrigin;
	idVec3 modelOrigin;

	animator.GetDelta( gameLocal.time - gameLocal.msec, gameLocal.time, delta );
	//gameLocal.Printf( "delta %f %f %f\n", delta[0],delta[1],delta[2] );

	delta = axis * delta;

	if ( modelOffset != vec3_zero ) {
		oldModelOrigin = modelOffset * oldaxis;
		modelOrigin = modelOffset * axis;
		delta += oldModelOrigin - modelOrigin;
	}

	delta *= physicsObj.GetGravityAxis();
}

//ivan end - anim move

//ivan start - cheat fix
/*
==============
idPlayer::SpawnAllWeapons
==============
*/
void idPlayer::SpawnAllWeapons( void ) {
	int i;
	int offsetYmult = 0;
	int offsetZmult = 0;
	const char *weapon_classname;
	for( i = 0; i < MAX_WEAPONS; i++ ) {
		weapon_classname = spawnArgs.GetString( va( "def_weapon%d", i ) );
		if ( !weapon_classname ) {
			continue;
		}
		const idDeclEntityDef *decl = gameLocal.FindEntityDef( weapon_classname, false );
		if ( !decl ) {
			continue;
		}
		
		//spawn it
		if ( SpawnInsteadOfGiving( weapon_classname, offsetYmult, offsetZmult ) ){
			if( offsetZmult == 0){
				offsetZmult = 1;
			}else{
				offsetZmult = 0;
				offsetYmult = -offsetYmult;
				if( offsetYmult >= 0 ){ offsetYmult++; }
			}
		}
	}
}


/*
===================
idPlayer::SpawnInsteadOfGiving
===================
*/
bool idPlayer::SpawnInsteadOfGiving( const char *classname, int offsetYmult, int offsetZmult ) {
	float		yaw;
	idVec3		org;
	idDict		dict;

	yaw = viewAngles.yaw;

	dict.Set( "classname", classname );
	dict.Set( "angle", va( "%f", yaw + 180 ) );

	org = GetPhysics()->GetOrigin() + idAngles( 0, yaw, 0 ).ToForward() * 60 * offsetYmult + idVec3( 0, 0, 5 + 120 * offsetZmult  ); 
	dict.Set( "origin", org.ToString() );
	dict.Set( "spin", "1" );

	return gameLocal.SpawnEntityDef( dict );
}
//ivan end
//rev 2019 from rivensin start
/*
=====================
idPlayer::Event_StartAutoMelee
=====================
*/
void idPlayer::Event_StartAutoMelee( float dmgMult, int trailNum ) {  
	if ( weapon.GetEntity() ) {
		weapon.GetEntity()->StartAutoMelee( dmgMult, trailNum );
	}
}

/*
=====================
idPlayer::Event_StopAutoMelee
=====================
*/
void idPlayer::Event_StopAutoMelee(void) {
	if (weapon.GetEntity()) {
		weapon.GetEntity()->StopAutoMelee();
	}
}
/*
=====================
idPlayer::Event_StartKick
=====================
*/
void idPlayer::Event_StartKick( const char *meleeDefName, float dmgMult ) {  
	kickDmgMultiplier = dmgMult;
	
	//get the def
	kickDef = gameLocal.FindEntityDef( meleeDefName, false );
	if ( !kickDef ) {
		gameLocal.Error( "No kickDef" );
	}

	//setup vars
	kickDefName = idStr( kickDef->GetName() ); //TODO: new???

	fromJointKick = GetAnimator()->GetJointHandle( kickDef->dict.GetString("from_joint") ); //"Rloleg"
	toJointKick = GetAnimator()->GetJointHandle( kickDef->dict.GetString("to_joint") ); //"Rankle_r"

	kickBox.Zero();
	kickBox.ExpandSelf( kickDef->dict.GetFloat( "kick_tracerWidth", "1" ) );

	kickDistance = kickDef->dict.GetFloat( "kick_distance" );

	lastKickedEnt = NULL; //reset it so that can be hit again
    kickEnabled = true;
	
	nextKickFx = gameLocal.time + 100; //delay snd+prt for LOW priority entities after the beginning of the attack
	nextKickSnd = gameLocal.time + 100; //don't play snd on world too early - this could not be used
}

/*
=====================
idPlayer::Event_StopKick
=====================
*/
void idPlayer::Event_StopKick( void ) {
    kickDmgMultiplier = 1.0f;
	lastKickedEnt = NULL; //don't remember it in the future  	
	kickEnabled = false;
}

/*
=====================
idPlayer::EvaluateKick
=====================
*/
bool idPlayer::EvaluateKick( void ) {  
	idEntity *ent;
	trace_t tr;

	//temp vars
	idVec3 start;
	idVec3 end;
	idVec3 dir;
	idMat3 useless;
	bool damaged;
	//end temp

	if ( !kickDef ) {
		gameLocal.Error( "No kickDef" );
	}

	if ( gameLocal.isClient ) {
		return false;
	}

	//init vars
	damaged = false;
	GetJointWorldTransform( fromJointKick, gameLocal.time, start, useless );
	GetJointWorldTransform( toJointKick, gameLocal.time, end, useless ); 
	dir = end - start; //get dir
	dir.Normalize(); //normalize
	end = start + dir * kickDistance * PowerUpModifier( MELEE_DISTANCE );

	gameLocal.clip.TraceBounds( tr, start, end, kickBox, MASK_SHOT_RENDERMODEL, this ); //ignore player

	if ( tr.fraction < 1.0f ) {	
		ent = gameLocal.entities[ tr.c.entityNum ]; //fix the headshot bug with melee attacks
		if(( ent ) && !(ent->IsType( idAFAttachment::Type))){ //only if it's not an idAFAttachment
			ent = gameLocal.GetTraceEntity( tr );
		}
	} else {
		ent = NULL;
	}

	if ( g_debugWeapon.GetBool() ) {
		gameRenderWorld->DebugLine( colorYellow, start, end, 100 );
		gameRenderWorld->DebugBounds( colorBlue, kickBox, start, 100 );
		gameRenderWorld->DebugBounds( colorBlue, kickBox, end, 100 );

		if ( ent ) {
			gameRenderWorld->DebugBounds( colorRed, ent->GetPhysics()->GetBounds(), ent->GetPhysics()->GetOrigin(), 100 );
		}
	}

	if ( ent ) {  //something hit

		//is it the last one?
		if(ent == lastKickedEnt){ //ignore the last entity hit
			//gameLocal.Printf( "idPlayer::EvaluateKick - entity ignored\n" );
			return true; //we hit the same thing again... do nothing now.
		}
		//gameLocal.Printf( "idPlayer::EvaluateKick - ent = %s \n",ent->GetName());

//rev 2021 make sure to not trigger any damage to jump through platforms.  Could cause charge attack damage to end early.
		if ( ( ent->fl.takedamage ) && !ent->spawnArgs.GetBool( "platform" ) ) {
			idVec3 kickDir, globalKickDir;
			kickDef->dict.GetVector( "kickDir", "0 0 0", kickDir );
			globalKickDir = renderEntity.axis * kickDir; //TODO: check if this is always the dir we want...

			//Ivan fix - transform clipmodel to joint handle to correctly get the damage zone in idActor::Damage
			//was: ent->Damage( owner, owner, globalKickDir, kickDefName, owner->PowerUpModifier( MELEE_DAMAGE ), tr.c.id );
			ent->Damage( this, this, globalKickDir, kickDefName, (kickDmgMultiplier * PowerUpModifier( MELEE_DAMAGE )) , CLIPMODEL_ID_TO_JOINT_HANDLE( tr.c.id ) );
			lastKickedEnt = ent; //remember this to avoid hitting it consecutively
			damaged = true; //hit and damaged!
		}

		//push it
		float push = kickDef->dict.GetFloat( "push" );
		//idVec3 impulse = -push * PowerUpModifier( SPEED ) * tr.c.normal;
		idVec3 impulse = push * PowerUpModifier( SPEED ) * dir;

		//extra push for AFs
		if( (ent->health <= 0) && (ent->IsType(idAFEntity_Base::Type)) ){
			idAFEntity_Base *p = static_cast< idAFEntity_Base * >( ent );

			if ( p->IsActiveAF() ){
				//gameLocal.Printf( "p->IsActiveAF()\n" );
				impulse *= kickDef->dict.GetInt( "pushAFMult", "1" );
			}
		}

		ent->ApplyImpulse( this, tr.c.id, tr.c.point, impulse );

		//case 1/3: (HIGH priority entities) AND (can bleed) -> ALWAYS play the snd and the prt on them, unless 'bleed' key is set to '0'. 
		if ( (ent->IsType(idBrittleFracture::Type) || ent->IsType(idAnimatedEntity::Type) || ent->IsType(idMoveable::Type) || ent->IsType(idMoveableItem::Type)) && ent->spawnArgs.GetBool( "bleed", "1" ) ) {	 
			nextKickFx = gameLocal.time + 500; ///delay snd+prt for LOW priority entities after an hit on HIGH priority entity
			//hitSound = kickDef->dict.GetString( PowerUpActive( BERSERK ) ? "snd_hit_berserk" : "snd_hit" );
			ent->AddDamageEffect( tr, impulse, kickDef->dict.GetString( "classname" ) ); //play the sound from the entity hit!
			//addDamageEffect already plays its own sound
		}
		//case 2/3: (LOW priority entities) AND (can bleed) -> play the snd and the prt less frequently - (example: sword on LOW priority entities)
		else if ( ent->spawnArgs.GetBool( "bleed", "1" )){ // Again, this is not done if 'bleed' key is set to '0'.
			if (( gameLocal.time > nextKickFx )  ){ //this is usually the worldspawn... don't play too much snd and prt on it!
				nextKickFx = gameLocal.time + 300; //delay snd+prt  for LOW priority entities after an hit on LOW priority entity
				//hitSound = kickDef->dict.GetString( PowerUpActive( BERSERK ) ? "snd_hit_berserk" : "snd_hit" );
				ent->AddDamageEffect( tr, impulse, kickDef->dict.GetString( "classname" ), this ); //play the sound from the player itself!
				//AddDamageEffect already plays its own sound from the player
			}				
		} 
		//case 3/3: (LOW or HIGH priority entities) AND (cannot bleed) -> do nothing... only sound?
		else { 
			int type = tr.c.material->GetSurfaceType();
			if ( type == SURFTYPE_NONE ) {
				type = GetDefaultSurfaceType();
			}
			const char *materialType = gameLocal.sufaceTypeNames[ type ];

			// start impact sound based on material type
			const char *hitSound = kickDef->dict.GetString( va( "snd_%s", materialType ) );
			if ( *hitSound == '\0' ) {
				hitSound = kickDef->dict.GetString( "snd_metal" );
			}

			if ( gameLocal.time > nextKickFx ) { //don't play it too frequently
				nextKickFx = gameLocal.time + 200;

				//play sound if (we damaged something ) or (hit something even not damaged, as world, and we are beyond the min time)
				if( (damaged) || ( gameLocal.time > nextKickSnd )) {
					if ( *hitSound != '\0' ) {
						const idSoundShader *snd = declManager->FindSound( hitSound );
						StartSoundShader( snd, SND_CHANNEL_BODY2, 0, true, NULL );
						nextKickSnd = gameLocal.time + 1000;
					}
				}
			} 
		}
	} //end something hit

	return damaged;
}
//rev 2019 from rivensin end
