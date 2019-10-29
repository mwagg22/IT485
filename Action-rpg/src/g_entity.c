#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "simple_logger.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "g_entity.h"
float framechange;
//float frame;
typedef struct
{
    Entity *entity_list;
    Uint32  entity_max;
}EntityManager;

static EntityManager gf3d_entity_manager = {0};
SDL_Event event;

void gf3d_entity_manager_close()
{
    if(gf3d_entity_manager.entity_list != NULL)
    {
        free(gf3d_entity_manager.entity_list);
    }
    memset(&gf3d_entity_manager,0,sizeof(EntityManager));
}

void gf3d_entity_manager_init(Uint32 entity_max)
{
    gf3d_entity_manager.entity_list = (Entity*)gfc_allocate_array(sizeof(Entity),entity_max);
	gf3d_entity_manager.entity_max = entity_max;
    if (!gf3d_entity_manager.entity_list)
    {
        slog("failed to allocate entity list");
        return;
    }
    atexit(gf3d_entity_manager_close);
}

Entity *gf3d_entity_new()
{
    Entity *ent = NULL;
    int i;
    for (i = 0; i < gf3d_entity_manager.entity_max; i++)
    {
        if (gf3d_entity_manager.entity_list[i]._inuse)continue;
        //. found a free entity
        memset(&gf3d_entity_manager.entity_list[i],0,sizeof(Entity));
        gf3d_entity_manager.entity_list[i]._inuse = 1;
		slog("Entity arr :%i", i);
		//slog("Size of ents list:%i", sizeof(gf3d_entity_manager.entity_list) / sizeof(int));
        return &gf3d_entity_manager.entity_list[i];
    }
    slog("request for entity failed: all full up");
    return NULL;
}

void gf3d_entity_free(Entity *self)
{
    if (!self)
    {
        slog("self pointer is not valid");
        return;
    }
    self->_inuse = 0;
    gf3d_model_free(self->model);
    if (self->data != NULL)
    {
        slog("warning: data not freed at entity free!");
    }
}

Entity *gf3d_return_list(){
	return gf3d_entity_manager.entity_list;
}

void gf3d_set_boundbox(Entity *self, Vector3D min, Vector3D max){
	self->box.m_vecMin = min;
	self->box.m_vecMax = max;
}
void gf3d_set_hitbox(Entity *self, Vector3D min, Vector3D max){
	self->Hitbox.m_vecMin = min;
	self->Hitbox.m_vecMax = max;
}
void collision_check(Entity *ents,Uint32 entity_max){
	size_t z = entity_max;
	for (int p = 0; p < z; p++){
		for (int j = p + 1; j <z; j++){
			if (!ents[p]._inuse)continue;
			if (!ents[j]._inuse)continue;
			if (check_collision(&ents[p], &ents[j]) == true){
				if (ents[p].type == ES_Projectile&&ents[j].type == ES_Enemy || ents[p].type == ES_Hitbox&&ents[j].type == ES_Enemy&&ents[p].parent->type != ES_Enemy || ents[j].type == ES_Projectile&&ents[p].type == ES_Enemy || ents[j].type == ES_Hitbox&&ents[p].type == ES_Enemy&&ents[j].parent->type != ES_Enemy)
				{
					//slog("projectile collision");
					//handle_projectile_collision(&ents[p], &ents[j]);
					handle_projectile_collision(&ents[j], &ents[p]);
				}
				else if (ents[p].type == ES_Hitbox&&ents[j].type == ES_Player&&ents[p].parent->type == ES_Enemy || ents[j].type == ES_Hitbox&&ents[p].type == ES_Player&&ents[j].parent->type == ES_Enemy)
				{
					//slog("enemy hitbox collision");
					handle_projectile_collision(&ents[p], &ents[j]);
					handle_projectile_collision(&ents[j], &ents[p]);
				}
				else{
					//slog("COLLISION");
					handle_collision(&ents[p], &ents[j]);
					handle_collision(&ents[j], &ents[p]);
				}
			}
			else{
				;
				ents[p].collision = false;
				ents[j].collision = false;
				//slog("No Coliison");
			}
		}
	};
}
bool check_collision(Entity *self, Entity *other){
	return(
		self->position.x + self->box.m_vecMax.x  >= other->position.x + other->box.m_vecMin.x &&
		self->position.x + self->box.m_vecMin.x  < other->position.x + other->box.m_vecMax.x &&
		self->position.y + self->box.m_vecMax.y  >= other->position.y + other->box.m_vecMin.y &&
		self->position.y + self->box.m_vecMin.y  < other->position.y + other->box.m_vecMax.y &&
		self->position.z + self->box.m_vecMax.z  >= other->position.z + other->box.m_vecMin.z &&
		self->position.z + self->box.m_vecMin.z  < other->position.z + other->box.m_vecMax.z);
}
void handle_collision(Entity *self, Entity *other){
	self->collision = true;
	other->collision = true;
	if (self->state == ES_Running){
		if (self->position.x >= other->position.x) { displacement(self, vector3d(self->movementspeed * 2, 0, 0)); displacement(other, vector3d(0, 0, 0)); slog("right"); }
		if (self->position.x  < other->position.x) { displacement(self, vector3d(-self->movementspeed * 2, 0, 0)); displacement(other, vector3d(0, 0, 0)); slog("left"); }
		if (self->position.y >= other->position.y) { displacement(self, vector3d(0, -self->movementspeed * 2, 0)); displacement(other, vector3d(0, 0, 0)); slog("up"); }
		if (self->position.y  < other->position.y) { displacement(self, vector3d(0, self->movementspeed * 2, 0)); displacement(other, vector3d(0, 0, 0)); slog("down"); }
		//if (self->position.z >= other->position.z) { displacement(self, vector3d(0, 0, 1)); displacement(other, vector3d(0, 0, 1)); }
		//else if (self->position.z <= other->position.z) { displacement(self, vector3d(0, 0, -1)); displacement(other, vector3d(0, 0, -1)); }
	}

}
bool check_hitbox_collision(Entity *hitlist, Entity *other){
	bool found=false;
	for (int i = 0; i < 10; i++){
		//slog(":%i,:%i", hitlist[i].Ent_ID, other->Ent_ID);
		if (!hitlist[i]._inuse)continue;
		if (hitlist[i].Ent_ID == other->Ent_ID)
		{
			found = true;
		}
	}
	return found;
}

void handle_projectile_collision(Entity *self, Entity *other){
	
	if (self->type == ES_Projectile&&other->type == ES_Enemy || self->type == ES_Hitbox&&other->type == ES_Enemy&&self->parent->type == ES_Player){
		//self->ProjectileData.Hitarray[0] = *other;
		bool found = check_hitbox_collision(self->ProjectileData.Hitarray, other);
		//slog("found %i",found);
		if (!found){

			//slog("hitbox COLLISION");
			other->is_hit = true;
			other->state = ES_Hit;
			other->update_model(other);
			for (int i = 0; i < 10; i++){
				if (!self->ProjectileData.Hitarray[i]._inuse){
					//slog("added in:%i", i);
					self->ProjectileData.Hitarray[i] = *other;
					return;
				}
			}
			
			//self->_inuse = 0;
		}
		if (self->ProjectileData.destroyOncollision != 0){
			if (self->ProjectileData.effect != 0 && self->ProjectileData.destroyOncollision != 2){
				update_ent_model(self);
			}
			else if (self->ProjectileData.destroyOncollision == 2){
				//slog("dos");
			}
			else{
				self->_inuse = 0;
				self->model->_inuse = 0;
				
			}
		}
	}
	if (other->type == ES_Projectile&&self->type == ES_Enemy || other->type == ES_Hitbox&&self->type == ES_Enemy&&other->parent->type == ES_Player){
		bool found = check_hitbox_collision(&other->ProjectileData.Hitarray, &self);
		//slog("hitbox COLLISION");
		if (other->ProjectileData.effect == 1 && other->ProjectileData.created == false){
			other->ProjectileData.created = true;
			update_ent_model(other);
		}
		self->is_hit = true;
		self->state = ES_Hit;
		self->update_model(self);
		//other->_inuse = 0;
	}
	if (self->type == ES_Hitbox&&other->type == ES_Player&&self->parent->type == ES_Enemy){
		//slog("hitbox COLLISION");
		if (other->state != ES_Blocking){
			other->is_hit = true;
			other->state = ES_Hit;
			other->update_model(other);
		}
		else{
			displacement(other, vector3d(self->up.x, self->up.y, 0));
		}
		self->_inuse = 0;
		self->model->_inuse = 0;
	}
	if (other->type == ES_Hitbox&&self->type == ES_Player&&other->parent->type == ES_Enemy){
		//slog("hitbox COLLISION");
		if (self->state != ES_Blocking){
			self->is_hit = true;
			self->state = ES_Hit;
			self->update_model(self);
		}
		else{
			displacement(self, vector3d(other->up.x, other->up.y,0));
		}
		other->_inuse = 0;
	}
}
void handle_hitbox_collision(Entity *self, Entity *other, Vector3D kick){
	displacement(other, kick);
}
void set_position(Entity *self, Matrix4 mat){
	self->position.x =  mat[3][0];
	self->position.y =  mat[3][1];
	self->position.z =  mat[3][2];
}
void displacement(Entity *self, Vector3D disp){
		//vector3d_normalize(&disp);
		vector3d_rotate_about_z(&disp, self->rotated);
		float angle;
		if (disp.x == 0 && disp.y == 0 && disp.z == 0){
			//slog("framecnt :%i",self->model->frameCount);
			return;
		}
		else{
			//slog("rotaiton");
			//slog("movemnt x:%f y:%f z:%f", disp.x, disp.y, disp.z);
			angle = vector3d_angle_between_vectors(self->up, disp);
			//slog("angle %f", angle);
			//slog("up before x:%f y:%f z:%f frame:%f", self->up.x, self->up.y, self->up.z, framechange);
			if (!self->is_hit&&!self->collision&&self->state!=ES_Blocking){
				gfc_matrix_rotate(self->EntMatx, self->EntMatx, angle, vector3d(0, 0, 1));
				self->up.x = disp.x;
				self->up.y = disp.y;
				self->up.z = disp.z;
			}
			gfc_matrix_translate(self->EntMatx, disp);
			
			//slog("up after x:%f y:%f z:%f frame:%f", self->up.x, self->up.y, self->up.z, framechange);
		}
	}

void init_ent(Entity *self,int cont){
	self->state = ES_Idle;
	self->dr = Up;
	self->controling = cont;
	self->position = vector3d(0, 0, 0);
	self->movementspeed = 30.f;
	self->up = vector3d(0, 1, 0);
	self->update_ent = update_ent;
	self->get_inputs = get_inputs;
	//self->update_model(self);
	gf3d_set_boundbox(self, self->model->mesh[0]->minv, self->model->mesh[0]->maxv);
}

void update_ent(Entity *self){
	
	set_position(self, self->EntMatx);
	self->frame = self->frame + 0.9;
	if (self->position.x > 100 || self->position.x < -100){
		self->_inuse = 0;
		self->model->_inuse = 0;
	}
	if (self->position.y > 100 || self->position.y < -100){
		self->_inuse = 0;
		self->model->_inuse = 0;
	}
	if (self->ProjectileData.destroyOncollision == 2){
		if (round(self->position.z) == -1){
			if (self->ProjectileData.effect != 0&&self->ProjectileData.created==0){
				self->ProjectileData.created = 1;
				update_ent_model(self);
			}
			else{
				//self->_inuse = 0;
			}
		}
	}
	if (self->frame >= self->model->frameCount)
	{
		if (self->type==ES_Effect){
			self->_inuse = 0;
			self->model->_inuse = 0;
		}
		if (self->type == ES_Hitbox){
			self->_inuse = 0;
			self->model->_inuse = 0;
		}
		self->frame = 0;
	}
	if (self->type == ES_Projectile){
		displacement(self, vector3d(self->up.x, self->up.y, self->up.z));
	}
}
void update_ent_model(Entity *self){
	self->frame = 0;
	if (self->ProjectileData.effect == 1){
		self->type = ES_Effect;
		self->model = gf3d_model_load_animated("//other//effects//explosion//explosion", "other//effects//magma", 0, 39);
		self->up.x = 0;
		self->up.y = 0;
		self->up.z = 0;
		
	}
}
void get_inputs(Entity *self, const Uint8 *keys,float delta){
	framechange = delta;
}


/*projectile_inputs*/
void get_proj_inputs(Entity *self, const Uint8 *keys, float delta){
	return;
}


void create_hitbox(Entity *self, Entity *other, Vector3D minboundbox, Vector3D maxboundbox, Vector3D kickback){
	size_t n = sizeof(other);
	vector3d_rotate_about_z(&kickback, self->rotated);
	for (int j = 0; j <n; j++){
		//slog("one+ size:%i",n);
		if (self == &other[j]){
			slog("me");
			continue;
		}
		else if (check_hitbox_collision(self, &other[j], minboundbox, maxboundbox) == true){
			slog("hitbox COLLISION");
			other[j].is_hit = true;
			other[j].state = ES_Hit;
			other[j].update_model(&other[j]);
			//handle_hitbox_collision(self, &other[j], kickback);
		}
		else{
		//slog("No Coliison");
		other[j].is_hit = false;
		}
	}
}
//targeting
Entity *get_nearest_target(Entity *self,Entity *other){
	Entity *nearest;
	int index=3;
	float distance;
	size_t n = sizeof(other);
	//slog("n :%i", n);
	if (self->type == ES_Player){
		distance = vector3d_magnitude_between(other[3].position, self->position);
		nearest = &other[3];
		for (int i = 3; i < 10; i++){
			if (!other[i]._inuse)continue;
			if (other[i].type != ES_Enemy)continue;			
			if (other[i].state == ES_Dead)continue;
			if (vector3d_magnitude_between(other[i].position, self->position) < distance){
				distance = vector3d_magnitude_between(other[i].position, self->position);
				nearest = &other[i];
				//slog("nearest :%i", i);
			}
		}
		return nearest;
	}
	else if (self->type == ES_Enemy){
		distance = vector3d_magnitude_between(other[0].position, self->position);
		nearest = &other[0];
		for (int i = 0; i < 3; i++){
			if (other[i].state == ES_Dead)continue;
			if (vector3d_magnitude_between(other[i].position, self->position) < distance){
				distance = vector3d_magnitude_between(other[i].position, self->position);
				nearest = &other[i];
			}
		}
		return nearest;
	}
	return nearest;
}
void rotate_towards_target(Entity *self, Vector3D disp,Vector3D *weap_up){
	float angle;
		angle = vector3d_angle_between_vectors(self->up, disp);
		//slog("turned:%f", angle);
			gfc_matrix_rotate(self->EntMatx, self->EntMatx, angle, vector3d(0, 0, 1));
			vector3d_rotate_about_z(&self->Hitbox.m_vecMin, angle);
			vector3d_rotate_about_z(&self->Hitbox.m_vecMax, angle);
			self->up.x = disp.x;
			self->up.y = disp.y;
			self->up.z = disp.z;
			weap_up->x = disp.x;
			weap_up->y = disp.y;
			weap_up->z = disp.z;
}
//
void create_projectile_e(Entity *self, Entity *other, char *model, char *texture,int startf, int endf,float dmg,float kick,bool type,int bboxframe,Vector3D up){
	Entity projEnt = *gf3d_entity_new();
	Model *proj = gf3d_model_new();
	proj = gf3d_model_load_animated(model, texture, startf, endf);
	projEnt.model = proj;
	projEnt.up.x = self->up.x;
	projEnt.up.y = self->up.y;
	projEnt.up.z = self->up.z;
	projEnt.movementspeed = 0;
	projEnt.attackdmg = dmg;
	projEnt.rotated = self->rotated;
	projEnt.state = ES_Idle;
	projEnt.update_ent = update_ent;
	projEnt.controling = 0;
	projEnt.parent = self;
	projEnt.ProjectileData.Hitarray = (Entity*)gfc_allocate_array(sizeof(Entity), 10);
	//projEnt.get_inputs = get_proj_inputs;
	projEnt.type = ES_Effect;
	if (type == 0){ 
		projEnt.type = ES_Hitbox;
		gfc_matrix_copy(projEnt.EntMatx, self->EntMatx); 
		projEnt.EntMatx[3][0] += 5 * (projEnt.up.x);
		projEnt.EntMatx[3][1] -= 5 * (projEnt.up.y);
	}
	else if(type==1){ 
		projEnt.type = ES_Hitbox;
		gfc_matrix_copy(projEnt.EntMatx, other->EntMatx); 
	}	
	else if (type == 4){
		projEnt.type = ES_Projectile;
		gfc_matrix_copy(projEnt.EntMatx, self->EntMatx);
	}
	else if (type == 6){
		projEnt.type = ES_Effect;
		gfc_matrix_copy(projEnt.EntMatx, self->EntMatx);
	}
	else{ 
	projEnt.type = ES_Projectile;
	projEnt.up.x = up.x;
	projEnt.up.y = up.y;
	projEnt.up.z = up.z;
	gfc_matrix_copy(projEnt.EntMatx, other->EntMatx);
	projEnt.EntMatx[3][2] = 30;
	projEnt.ProjectileData.parenttype = self->type;
	projEnt.ProjectileData.effect = 1;
	projEnt.ProjectileData.destroyOncollision = 2;
	projEnt.ProjectileData.Projectile = &projEnt;
	}
	gf3d_set_boundbox(&projEnt, projEnt.model->mesh[bboxframe]->minv, projEnt.model->mesh[bboxframe]->maxv);
	//slog("min x%f y:%f z:%f max x:%f y:%f z:%f", projEnt.box.m_vecMin.x, projEnt.box.m_vecMin.y, projEnt.box.m_vecMin.z, projEnt.box.m_vecMax.x, projEnt.box.m_vecMax.y, projEnt.box.m_vecMax.z);
	//
	spawn_Entity2(&projEnt);
	//free(&projEnt);
}
/*eol@eof*/