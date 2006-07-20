/********************************************************************** 
 Freeciv - Copyright (C) 1996 - A Kjeldberg, L Gregersen, P Unold
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
***********************************************************************/
#ifndef FC__UNIT_H
#define FC__UNIT_H

#include "fc_types.h"
#include "terrain.h"		/* enum tile_special_type */
#include "unittype.h"

struct unit_order;

#define BARBARIAN_LIFE    5

/* Changing this enum will break savegame and network compatability. */
enum unit_activity {
  ACTIVITY_IDLE, ACTIVITY_POLLUTION, ACTIVITY_ROAD, ACTIVITY_MINE,
  ACTIVITY_IRRIGATE, ACTIVITY_FORTIFIED, ACTIVITY_FORTRESS, ACTIVITY_SENTRY,
  ACTIVITY_RAILROAD, ACTIVITY_PILLAGE, ACTIVITY_GOTO, ACTIVITY_EXPLORE,
  ACTIVITY_TRANSFORM, ACTIVITY_UNKNOWN, ACTIVITY_AIRBASE, ACTIVITY_FORTIFYING,
  ACTIVITY_FALLOUT,
  ACTIVITY_PATROL_UNUSED, /* Needed for savegame compatability. */
  ACTIVITY_LAST   /* leave this one last */
};

/* Changing this enum will break network compatability. */
enum unit_orders {
  ORDER_MOVE, ORDER_ACTIVITY,
  ORDER_FULL_MP, ORDER_BUILD_CITY, ORDER_DISBAND, ORDER_BUILD_WONDER,
  ORDER_TRADEROUTE, ORDER_HOMECITY,
  /* and plenty more for later... */
  ORDER_LAST
};

enum unit_focus_status {
  FOCUS_AVAIL, FOCUS_WAIT, FOCUS_DONE  
};

enum diplomat_actions {
  DIPLOMAT_BRIBE, DIPLOMAT_EMBASSY, DIPLOMAT_SABOTAGE,
  DIPLOMAT_STEAL, DIPLOMAT_INCITE, SPY_POISON, 
  DIPLOMAT_INVESTIGATE, SPY_SABOTAGE_UNIT,
  SPY_GET_SABOTAGE_LIST,
  DIPLOMAT_MOVE,	/* move onto city square - only for allied cities */
  DIPLOMAT_ANY_ACTION   /* leave this one last */
};

enum ai_unit_task { AIUNIT_NONE, AIUNIT_AUTO_SETTLER, AIUNIT_BUILD_CITY,
                    AIUNIT_DEFEND_HOME, AIUNIT_ATTACK, AIUNIT_ESCORT, 
                    AIUNIT_EXPLORE, AIUNIT_RECOVER, AIUNIT_HUNTER };

enum goto_move_restriction {
  GOTO_MOVE_ANY,
  GOTO_MOVE_CARDINAL_ONLY, /* No diagonal moves.  */
  GOTO_MOVE_STRAIGHTEST
};

enum goto_route_type {
  ROUTE_GOTO, ROUTE_PATROL
};

enum unit_move_result {
  MR_OK, MR_BAD_TYPE_FOR_CITY_TAKE_OVER, MR_NO_WAR, MR_ZOC,
  MR_BAD_ACTIVITY, MR_BAD_DESTINATION, MR_BAD_MAP_POSITION,
  MR_DESTINATION_OCCUPIED_BY_NON_ALLIED_UNIT,
  MR_NO_TRANSPORTER_CAPACITY,
  MR_DESTINATION_OCCUPIED_BY_NON_ALLIED_CITY
};

enum add_build_city_result {
  AB_BUILD_OK,			/* Unit OK to build city */
  AB_ADD_OK,			/* Unit OK to add to city */
  AB_NOT_BUILD_LOC,		/* City is not allowed to be built at
				   this location */
  AB_NOT_ADDABLE_UNIT,		/* Unit is not one that can be added
				   to cities */
  AB_NOT_BUILD_UNIT,		/* Unit is not one that can build
				   cities */
  AB_NO_MOVES_BUILD,		/* Unit does not have moves left to
				   build a city */
  AB_NO_MOVES_ADD,		/* Unit does not have moves left to
				   add to city */
  AB_NOT_OWNER,			/* Owner of unit is not owner of
				   city */
  AB_TOO_BIG,			/* City is too big to be added to */
  AB_NO_SPACE			/* Adding takes city past limit */
};

enum unit_upgrade_result {
  UR_OK,
  UR_NO_UNITTYPE,
  UR_NO_MONEY,
  UR_NOT_IN_CITY,
  UR_NOT_CITY_OWNER,
  UR_NOT_ENOUGH_ROOM
};
    
struct unit_ai {
  bool control; /* 0: not automated    1: automated */
  enum ai_unit_task ai_role;
  /* The following are unit ids or special indicator values (<=0) */
  int ferryboat; /* the ferryboat assigned to us */
  int passenger; /* the unit assigned to this ferryboat */
  int bodyguard; /* the unit bodyguarding us */
  int charge; /* the unit this unit is bodyguarding */

  struct tile *prev_struct, *cur_struct;
  struct tile **prev_pos, **cur_pos;

  int target; /* target we hunt */
  int hunted; /* if a player is hunting us, set by that player */
  bool done;  /* we are done controlling this unit this turn */
};

struct unit {
  struct unit_type *type;
  int id;
  struct player *owner; /* Cannot be NULL. */
  struct tile *tile;
  int homecity;
  int moves_left;
  int hp;
  int veteran;
  int unhappiness;
  int upkeep[O_MAX];
  int fuel;
  int bribe_cost;
  struct unit_ai ai;
  enum unit_activity activity;
  struct tile *goto_tile; /* May be NULL. */

  /* The amount of work that has been done on the current activity.  This
   * is counted in turns but is multiplied by ACTIVITY_COUNT (which allows
   * fractional values in some cases). */
  int activity_count;

  enum tile_special_type activity_target;
  enum unit_focus_status focus_status;
  int ord_map, ord_city;
  /* ord_map and ord_city are the order index of this unit in tile.units
     and city.units_supported; they are only used for save/reload */
  bool foul;
  bool debug;
  bool moved;
  bool paradropped;

  /* This value is set if the unit is done moving for this turn. This
   * information is used by the client.  The invariant is:
   *   - If the unit has no more moves, it's done moving.
   *   - If the unit is on a goto but is waiting, it's done moving.
   *   - Otherwise the unit is not done moving. */
  bool done_moving;

  int transported_by;
  int occupy; /* number of units that occupy transporter */

  /* The battlegroup ID: defined by the client but stored by the server. */
#define MAX_NUM_BATTLEGROUPS (4)
#define BATTLEGROUP_NONE (-1)
  int battlegroup;

  struct {
    /* Equivalent to pcity->client.color.  Only for F_CITIES units. */
    bool colored;
    int color_index;
  } client;
  struct {
    struct vision *vision; /* See explanation in maphand.h. */
  } server;

  bool has_orders;
  struct {
    int length, index;
    bool repeat;	/* The path is to be repeated on completion. */
    bool vigilant;	/* Orders should be cleared if an enemy is met. */
    struct unit_order *list;
  } orders;
};

#define SINGLE_MOVE 3
#define MOVE_COST_RIVER 1
#define MOVE_COST_RAIL 0
#define MOVE_COST_ROAD 1

/* Iterates over the types of unit activity. */
#define activity_type_iterate(act)					    \
{									    \
  Activity_type_id act;			         			    \
									    \
  for (act = 0; act < ACTIVITY_LAST; act++) {

#define activity_type_iterate_end     					    \
  }									    \
}

bool diplomat_can_do_action(const struct unit *pdiplomat,
			    enum diplomat_actions action,
			    const struct tile *ptile);
bool is_diplomat_action_available(const struct unit *pdiplomat,
				  enum diplomat_actions action,
				  const struct tile *ptile);

bool unit_can_help_build_wonder(const struct unit *punit,
				const struct city *pcity);
bool unit_can_help_build_wonder_here(const struct unit *punit);
bool unit_can_est_traderoute_here(const struct unit *punit);
bool unit_can_airlift_to(const struct unit *punit, const struct city *pcity);
bool unit_has_orders(const struct unit *punit);

bool can_unit_load(const struct unit *punit, const struct unit *ptrans);
bool can_unit_unload(const struct unit *punit, const struct unit *ptrans);
bool can_unit_paradrop(const struct unit *punit);
bool can_unit_bombard(const struct unit *punit);
bool can_unit_change_homecity_to(const struct unit *punit,
				 const struct city *pcity);
bool can_unit_change_homecity(const struct unit *punit);
const char *get_activity_text(enum unit_activity activity);
bool can_unit_continue_current_activity(struct unit *punit);
bool can_unit_do_activity(const struct unit *punit,
			  enum unit_activity activity);
bool can_unit_do_activity_targeted(const struct unit *punit,
				   enum unit_activity activity,
				   enum tile_special_type target);
bool can_unit_do_activity_targeted_at(const struct unit *punit,
				      enum unit_activity activity,
				      enum tile_special_type target,
				      const struct tile *ptile);
void set_unit_activity(struct unit *punit, enum unit_activity new_activity);
void set_unit_activity_targeted(struct unit *punit,
				enum unit_activity new_activity,
				enum tile_special_type new_target);
int get_activity_rate(const struct unit *punit);
int get_activity_rate_this_turn(const struct unit *punit);
int get_turns_for_activity_at(const struct unit *punit,
			      enum unit_activity activity,
			      const struct tile *ptile);
bool can_unit_do_autosettlers(const struct unit *punit); 
bool is_unit_activity_on_tile(enum unit_activity activity,
			      const struct tile *ptile);
bv_special get_unit_tile_pillage_set(const struct tile *ptile);
bool is_attack_unit(const struct unit *punit);
bool is_military_unit(const struct unit *punit);           /* !set !dip !cara */
bool is_diplomat_unit(const struct unit *punit);
bool is_square_threatened(const struct player *pplayer,
			  const struct tile *ptile);
bool is_field_unit(const struct unit *punit);              /* ships+aero */
bool is_hiding_unit(const struct unit *punit);
#define COULD_OCCUPY(punit) \
  (unit_class_flag(punit->type->class, UCF_CAN_OCCUPY) && is_military_unit(punit))
bool can_unit_add_to_city (const struct unit *punit);
bool can_unit_build_city (const struct unit *punit);
bool can_unit_add_or_build_city (const struct unit *punit);
enum add_build_city_result test_unit_add_or_build_city(const struct unit *
						       punit);
bool kills_citizen_after_attack(const struct unit *punit);

const char *unit_activity_text(const struct unit *punit);
int get_transporter_capacity(const struct unit *punit);

struct player *unit_owner(const struct unit *punit);

struct unit *is_allied_unit_tile(const struct tile *ptile,
				 const struct player *pplayer);
struct unit *is_enemy_unit_tile(const struct tile *ptile,
				const struct player *pplayer);
struct unit *is_non_allied_unit_tile(const struct tile *ptile,
				     const struct player *pplayer);
struct unit *is_non_attack_unit_tile(const struct tile *ptile,
				     const struct player *pplayer);

int unit_loss_pct(const struct player *pplayer, const struct tile *ptile,
		  const struct unit *punit);
int base_trireme_loss_pct(const struct player *pplayer,
			  const struct unit *punit);
int base_unsafe_terrain_loss_pct(const struct player *pplayer,
				 const struct unit *punit);

bool is_my_zoc(const struct player *unit_owner, const struct tile *ptile);
bool unit_being_aggressive(const struct unit *punit);
bool unit_type_really_ignores_zoc(const struct unit_type *punittype);

bool is_build_or_clean_activity(enum unit_activity activity);

struct unit *create_unit_virtual(struct player *pplayer, struct city *pcity,
                                 struct unit_type *punittype,
				 int veteran_level);
void destroy_unit_virtual(struct unit *punit);
void free_unit_orders(struct unit *punit);

int get_transporter_occupancy(const struct unit *ptrans);
struct unit *find_transporter_for_unit(const struct unit *pcargo,
				       const struct tile *ptile);

enum unit_upgrade_result test_unit_upgrade(const struct unit *punit,
					   bool is_free);
enum unit_upgrade_result get_unit_upgrade_info(char *buf, size_t bufsz,
					       const struct unit *punit);

#endif  /* FC__UNIT_H */
