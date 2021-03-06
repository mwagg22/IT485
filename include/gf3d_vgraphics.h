#ifndef __GF3D_VGRAPHICS_H__
#define __GF3D_VGRAPHICS_H__

#include <vulkan/vulkan.h>
#include "gfc_vector.h"
#include "gfc_matrix.h"

#include "gf3d_pipeline.h"
#include "gf3d_commands.h"
#include "g_entity.h"

#define GF3D_VGRAPHICS_DISCRETE 1   //Choosing whether to use discrete [1] or integrated graphics [0]

/**
 * @brief init Vulkan / SDL, setup device and initialize infrastructure for 3d graphics
 */
void gf3d_vgraphics_init(
    char *windowName,
    int renderWidth,
    int renderHeight,
    Vector4D bgcolor,
    Bool fullscreen,
    Bool enableValidation
);

/**
 * @brief After initialization 
 */
VkDevice gf3d_vgraphics_get_default_logical_device();

VkPhysicalDevice gf3d_vgraphics_get_default_physical_device();

VkExtent2D gf3d_vgraphics_get_view_extent();

void gf3d_vgraphics_clear();

Uint32 gf3d_vgraphics_render_begin();
void gf3d_vgraphics_render_end(Uint32 imageIndex);

int gf3d_vgraphics_create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer * buffer, VkDeviceMemory * bufferMemory);

void gf3d_vgraphics_copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

uint32_t gf3d_vgraphics_find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);

void gf3d_vgraphics_rotate_camera(float degrees);

void update_cam(Entity *target,float rot);
void gf3d_copy_camera(Matrix4 *dest);
void gf3d_set_camera(Matrix4 *dest, Entity *target,float rot,float wheel);

VkBuffer gf3d_vgraphics_get_uniform_buffer_by_index(Uint32 index);
UniformBufferObject gf3d_vgraphics_get_uniform_buffer_object();

Pipeline *gf3d_vgraphics_get_graphics_pipeline();
Pipeline *gf3d_vgraphics_get_graphics_pipeline_2d();
Pipeline *gf3d_vgraphics_get_graphics_pipeline_sky();
Command *gf3d_vgraphics_get_graphics_command_pool();
Pipeline *gf3d_vgraphics_get_graphics_pipeline2d();
VkImageView gf3d_vgraphics_create_image_view(VkImage image, VkFormat format);

SDL_Window *gf3d_vgraphics_get_sdl_window();
void gf3d_vgraphics_move_camera(Vector3D disp);
void display_img_to_screen(char* imgFile);
Vector3D ArbitraryRotate(Vector3D p, double theta, Vector3D r);
#endif
