#ifndef __GF3D_ENTITY_H__
#define __GF3D_ENTITY_H__
#include "gf3d_model.h"
#include "gf3d_commands.h"
//#include "gf3d_vgraphics.h"
#include "gf3d_obj_load.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"

typedef enum
{
    ES_Idle = 0,
	ES_Running = 1,
	ES_Attacking = 2,
	ES_Blocking = 3,
	ES_HPSkill = 4,
	ES_Special = 5,
	ES_UsingItem = 6,
	ES_Hit = 7,
    ES_Dying = 8,
    ES_Dead = 9
}EntityState;
typedef enum
{
	ES_Player = 0,
	ES_Item = 1,
	ES_Weapon = 2,
	ES_Projectile = 3,
	ES_Hitbox = 4,
	ES_Effect = 5,
	ES_NPC = 6,
	ES_Enemy = 7,
	ES_Stage=8
}EntityType;
typedef enum
{
	W_Sword = 0,
	W_Staff = 1,
	W_Arrow = 2,
	W_Projectile = 3
}WeaponType;
typedef enum
{
	attack = 0,
	movement = 1,
	special = 2,
	hpskill = 3,
	block = 4,
	menu = 5,
	none = 6
}Actioninput;

typedef struct BoundBox
{
	//Entity *self;
	Vector3D m_vecMin;
	Vector3D m_vecMax;
}BoundBox;
//for projectiles
typedef enum{
	Right,
	Left,
	Up,
	Down,
	RightUp,
	LeftUp,
	RightDown,
	LeftDown
}dir;
typedef struct Projectiles_s
{
	//Entity *self;
	struct Entity_S *Projectile;
	struct Entity_S *Hitarray;
	EntityType parenttype;
	int effect;
	int destroyOncollision;
	bool created;
}Projectiles;
typedef struct Entity_S
{
	Matrix4			EntMatx;		//matrix of position n stuff for draw
    Uint8           _inuse;         /**<flag to keep track if this isntance is in use and should not be reassigned*/
	int				Ent_ID;			//idx in list of ents
    Model           *model;          /**<the 3d model for this entity*/
	float			frame;
    Vector3D        position;       /**<position of the entity in 3d space*/
    Vector3D        velocity;       /**<velocity of the entity in 3d space*/
    Vector3D        acceleration;   /**<acceleration of the entity in 3d space*/
    Vector3D        up;       /**<up vector*/
    Vector3D        scale;          /**<*please default to 1,1,1*/
    EntityState     state;          /**<current state of the entity*/
	BoundBox		box;			//bounding box
	BoundBox		Hitbox;			//hitbox box
	Projectiles		ProjectileData; //for projectiles/hitboxes
	dir				dr;
	Actioninput		action;			//keep track of input
	Actioninput		prev_action;	//prev action
	EntityType		type;			//check entity type
	//Entity		*parent;			//if weapon have parent
	//WeaponType	weap;				//type of weapon if entity is a weapon
    void (*think)(struct Entity_S* self);   /**<function called on entity think*/
    void (*update_model)(struct Entity_S* self);   /**<function called on entity update*/
    void (*touch)(struct Entity_S* self,struct Entity_S* other);   /**<function called on entity think*/
	void (*update_ent)(struct Entity *self);
	void			(*move)(struct Entity_S* self);
	void			(*attack)(struct Entity_S* self);
	void			(*block)(struct Entity_S* self);
	void			(*special)(struct Entity_S* self,int n);
	void			(*hpSkill)(struct Entity_S* self);
	void			(*useItem)(struct Entity_S* self);
	void (*get_inputs)(struct Entity *self, const Uint8 * keys, float delta);
	//void			(*get_inputs)(struct Entity_S* self, const Uint8 * keys, float delta);
    float           health;
    float           healthmax;
    float           defense;
	float           attackdmg;
	float           mp;
    float           experience;
    float           level;
    float           movementspeed;
	float			rotated;
	int 			controling;	
	int				specialnum;
	int attacknum;
	int cast;//for magic casting
	bool			can_attack;
	bool			can_block;
	bool			can_hpskill;
	bool			can_special;
	bool			is_hit;
	bool			in_action;
	bool			collision;
	int				comboNum;
	struct Entity_S *target;//for enemies
	struct Entity_S *parent;//for projectiles
    void *data;                     /**<additional entity specific data*/
    
}Entity;

/**
 * @brief initializes the entity subsystem
 * @param entity_max maximum number of simultaneous entities you wish to support
 */
void gf3d_entity_manager_init(Uint32 entity_max);

/**
 * @brief get an empty entity from the system
 * @return NULL on out of space or a pointer to an entity otherwise
 */
Entity *gf3d_entity_new();

Entity *gf3d_return_list();
void spawn_Entity2(Entity *entsToAdd);
/**
 * @brief free an active entity
 * @param self the entity to free
 */
//extern Entity *ents[4];
void gf3d_entity_free(Entity *self);
void gf3d_set_boundbox(Entity *self, Vector3D min, Vector3D max);
void collision_check(Entity *ents, Uint32 entity_max);
bool check_collision(Entity *self, Entity *other);
bool check_hitbox_collision(Entity *hitlist, Entity *other);
void handle_hitbox_collision(Entity *self, Entity *other,Vector3D kick);
void handle_projectile_collision(Entity *self, Entity *other);
void set_position(Entity *self, Matrix4 mat);
void displacement(Entity *self, Vector3D disp);
void handle_collision(Entity *self, Entity *other);
void init_ent(Entity *self, int cont);
void get_inputs(Entity *self, const Uint8 * keys,float delta);
//void get_inputs(Entity *self, SDL_Event *e);
void update_model(Entity *self);
void update_ent_model(Entity *self);
void update_ent(Entity *self);
void create_projectile(Entity *self,float speed,float dmg);
void get_proj_inputs(Entity *self, const Uint8 *keys, float delta);
void create_hitbox(Entity *self, Entity *other, Vector3D minboundbox, Vector3D maxboundbox, Vector3D kickback);
Entity *get_nearest_target(Entity *self,Entity *other);
void rotate_towards_target(Entity *self, Vector3D disp, Vector3D *weap_up);
void gf3d_set_hitbox(Entity *self, Vector3D min, Vector3D max);
//type whether stationary or spawn at target
void create_projectile_e(Entity *self, Entity *other, char *model, char *texture, int startf, int endf, float dmg, float kick, int type, int bboxframe, Vector3D up);
#endif