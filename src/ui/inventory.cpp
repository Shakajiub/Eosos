//	Copyright (C) 2018 Jere Oikarinen
//
//	This file is part of Eosos.
//
//	Eosos is free software : you can redistribute it and / or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	Eosos is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with Eosos. If not, see <http://www.gnu.org/licenses/>.

#include "engine.hpp"
#include "inventory.hpp"
#include "item.hpp"
#include "object.hpp"
#include "texture.hpp"
#include "button.hpp"

#include "player.hpp"
#include "camera.hpp"
#include "texture_manager.hpp"
#include "bitmap_font.hpp"
#include "ui.hpp"

std::string limit_to_string(const uint8_t limit)
{
	switch (limit)
	{
		case ET_AMMO:   return "ammo";    case ET_AMULET: return "amulet";
		case ET_ARMOR:  return "armor";   case ET_BOOTS:  return "boots";
		case ET_CLOAK:  return "cloak";   case ET_GLOVES: return "gloves";
		case ET_HAT:    return "hat";     case ET_LIGHT:  return "light";
		case ET_SHIELD: return "shield";  case ET_TOOL:   return "tool";
		case ET_RING:   return "ring";    case ET_WEAPON: return "weapon";
		default:        return "unknown";
	}
	return "unknown"; // Should not ever get here
}
Inventory::Inventory() :
	render_flag(false), overlap_flag(false), inv_texture(nullptr),
	slot_highlight(nullptr), close_button(nullptr), inv_source(nullptr)
{

}
Inventory::~Inventory()
{
	free();
}
void Inventory::free()
{
	if (inv_texture != nullptr)
	{
		SDL_DestroyTexture(inv_texture);
		inv_texture = nullptr;
	}
	if (slot_highlight != nullptr)
	{
		engine.get_texture_manager()->free_texture(slot_highlight->get_name());
		slot_highlight = nullptr;
	}
	if (close_button != nullptr)
	{
		delete close_button;
		close_button = nullptr;
	}
	for (uint8_t i = 0; i < inventory.capacity(); i++)
	{
		if (inventory[i] != nullptr)
			delete inventory[i];
	}
	inventory.clear();
	limits.clear();
}
void Inventory::init(uint8_t id, const std::string &title, uint8_t inv_width, uint8_t inv_height)
{
	free(); ID = id;

	const uint8_t size = inv_width * inv_height;
	inventory.reserve(size);

	for (uint8_t i = 0; i < size; i++)
		inventory[i] = nullptr;

	width = inv_width + 1;
	height = inv_height + 2;

	const uint8_t prev_scale = ui.get_bitmap_font()->get_scale();
	ui.get_bitmap_font()->set_scale(2);

	uint8_t title_width = (title.length() * ui.get_bitmap_font()->get_width()) / 32;
	if (title_width > width - 1)
	{
		std::cout << "warning! title '" << title << "' is too long!" << std::endl;
		title_width = width - 1;
	}
	button_offset = (title_width + 1) * 32;

	inv_texture = SDL_CreateTexture(engine.get_renderer(),
		SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width * 32, height * 32
	);
	if (inv_texture == NULL)
	{
		inv_texture = nullptr;
		std::cout << "unable to create blank texture! SDL Error: " << SDL_GetError() << std::endl;
		return;
	}
	slot_highlight = engine.get_texture_manager()->load_texture("core/texture/ui/highlight.png", true);
	if (slot_highlight != nullptr)
		slot_highlight->set_color(COLOR_BERRY);

	SDL_SetRenderTarget(engine.get_renderer(), inv_texture);

	// Fill the texture with "nothing" (full transparency)
	SDL_SetTextureBlendMode(inv_texture, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(engine.get_renderer(), 0, 0, 0, 0);
	SDL_RenderClear(engine.get_renderer());
	SDL_SetRenderDrawColor(engine.get_renderer(), 20, 12, 28, 255);

	ui.get_bitmap_font()->set_color(COLOR_PEPPERMINT);
	ui.draw_box(0, 0, title_width, 1); // Draw the title bar
	ui.get_bitmap_font()->render_text(8, 8, title); // Title text
	ui.draw_box(0, 32, width, height - 1); // Main inventory box
	ui.get_bitmap_font()->set_scale(prev_scale);

	Texture *sep_h = engine.get_texture_manager()->load_texture("core/texture/ui/separator_horizontal.png");
	Texture *sep_v = engine.get_texture_manager()->load_texture("core/texture/ui/separator_vertical.png");

	if (sep_h != nullptr && sep_v != nullptr)
	{
		sep_h->set_color(COLOR_PLUM);
		sep_v->set_color(COLOR_PLUM);

		start_x = ((width * 16) - 16) - (34 * (inv_width / 2));
		start_y = (height * 16) - (34 * (inv_height / 2));

		if (inv_height % 2 == 0)
			start_y += 17;

		for (uint16_t temp_y = start_y; temp_y < start_y + (inv_height * 34); temp_y += 34)
		{
			for (uint16_t temp_x = start_x; temp_x < start_x + (inv_width * 32); temp_x += 34)
			{
				if (temp_x != start_x)
					sep_h->render(temp_x - 2, temp_y);
				if (temp_y != start_y)
					sep_v->render(temp_x, temp_y - 2);
			}
		}
	}
	SDL_SetRenderTarget(engine.get_renderer(), NULL);
}
void Inventory::render() const
{
	if (inv_texture != nullptr)
	{
		if (!render_flag)
		{
			if (ID == INV_TEMP) return; // Temporary inventory should not have a minimized version

			const int8_t offset = (ID == INV_BACKPACK) ? -96 : 0;

			const SDL_Rect clip = { 0, 0, width * 32, 32 };
			const SDL_Rect quad = { x + offset, camera.get_cam_h() - 32, width * 32, 32 };

			SDL_RenderCopyEx(engine.get_renderer(), inv_texture, &clip, &quad, 0.0, nullptr, SDL_FLIP_NONE);

			if (close_button != nullptr)
				close_button->render();
			return;
		}
		if (close_button != nullptr)
			close_button->render();

		const SDL_Rect clip = { 0, 0, width * 32, height * 32 };
		const SDL_Rect quad = { x, y, width * 32, height * 32 };

		SDL_RenderCopyEx(engine.get_renderer(), inv_texture, &clip, &quad, 0.0, nullptr, SDL_FLIP_NONE);

		const uint8_t real_width = width - 1;
		const uint8_t real_heigth = height - 2;

		for (uint8_t temp_y = 0; temp_y < real_heigth; temp_y++)
		{
			for (uint8_t temp_x = 0; temp_x < real_width; temp_x++)
			{
				const uint8_t slot = temp_y * real_width + temp_x;
				const uint16_t xpos = x + (temp_x * 34) + (18 - width);
				const uint16_t ypos = y + ((temp_y + 1) * 34) + (17 - height);
				const bool info_flag = (overlap_flag && temp_x == slot_x && temp_y == slot_y - 1);

				if (inv_source != nullptr) // If we have an external inventory source, render it instead
					inv_source->render_inventory_item(slot, xpos, ypos, info_flag);

				else if (inventory.capacity() >= slot && inventory[slot] != nullptr)
				{
					inventory[slot]->render(xpos, ypos, info_flag);

					if (info_flag && ui.get_temp_item() != nullptr)
						ui.get_temp_item()->render_info(xpos - 160, ypos);
				}
			}
		}
		if (overlap_flag && slot_highlight != nullptr)
		{
			slot_highlight->render(x + (slot_x * 34) + (18 - width), y + (slot_y * 34) + (17 - height));

			ui.get_bitmap_font()->render_text(16, 176 + ui.get_bitmap_font()->get_height() * 2,
				"Slot: " + std::to_string(slot_x) + ", " + std::to_string(slot_y - 1)
			);
			const uint8_t slot = (slot_y - 1) * (width - 1) + slot_x;
			for (auto &it : limits)
			{
				if (it.first == slot)
				{
					if (it.second != ET_NONE)
						ui.get_bitmap_font()->render_text(
							x + (slot_x * 32) + (slot_x * 2) + 16, y + (slot_y * 32), limit_to_string(it.second)
						);
					break;
				}
			}
		}
	}
}
void Inventory::clear()
{
	for (uint8_t i = 0; i < inventory.capacity(); i++)
	{
		if (inventory[i] != nullptr)
			delete inventory[i];
	}
	inventory.clear();
	limits.clear();
}
void Inventory::push_item(Item *item)
{
	if (item == nullptr)
	{
		std::cout << "can't push nullptr into inventory!" << std::endl;
		return;
	}
	bool success = false;
	for (uint8_t i = 0; i < inventory.capacity() - 1; i++)
	{
		if (inventory[i] == nullptr)
		{
			uint8_t limit = ET_NONE;
			for (auto &it : limits)
			{
				if (it.first == i)
				{
					limit = it.second;
					break;
				}
			}
			if (limit == ET_NONE || limit == item->get_equip_type())
			{
				inventory[i] = item;
				success = true;
				break;
			}
		}
	}
	if (!success)
	{
		std::cout << "no room in inventory to push item! deleting ..." << std::endl;
		delete item;
	}
}
void Inventory::limit_slot(uint8_t slot, uint8_t type, const std::string &custom_texture)
{
	if (inv_texture != nullptr && inventory.capacity() >= slot)
	{
		bool already_limited = false;
		for (auto &it : limits)
		{
			if (it.first == slot)
			{
				it.second = type;
				already_limited = true;
				break;
			}
		}
		if (!already_limited)
			limits.push_back(std::make_pair(slot, type));

		std::string texture_name = "core/texture/ui/icon/cross.png";

		if (custom_texture.length() > 0)
			texture_name = custom_texture;
		else switch (type)
		{
			case ET_AMMO: texture_name = "core/texture/ui/icon/arrow.png"; break;
			case ET_AMULET: texture_name = "core/texture/ui/icon/amulet.png"; break;
			case ET_ARMOR: texture_name = "core/texture/ui/icon/armor.png"; break;
			case ET_BOOTS: texture_name = "core/texture/ui/icon/boots.png"; break;
			case ET_CLOAK: texture_name = "core/texture/ui/icon/cloak.png"; break;
			case ET_GLOVES: texture_name = "core/texture/ui/icon/gloves.png"; break;
			case ET_HAT: texture_name = "core/texture/ui/icon/cap.png"; break;
			case ET_LIGHT: texture_name = "core/texture/ui/icon/torch.png"; break;
			case ET_SHIELD: texture_name = "core/texture/ui/icon/shield.png"; break;
			case ET_TOOL: texture_name = "core/texture/ui/icon/pickaxe.png"; break;
			case ET_RING: texture_name = "core/texture/ui/icon/ring.png"; break;
			case ET_WEAPON: texture_name = "core/texture/ui/icon/sword.png"; break;
			default: break;
		}
		Texture *temp_texture = engine.get_texture_manager()->load_texture(texture_name, true, false);
		if (temp_texture != nullptr)
		{
			SDL_SetRenderTarget(engine.get_renderer(), inv_texture);

			const uint8_t xpos = slot == 0 ? 0 : slot % (width - 1);
			const uint8_t ypos = slot == 0 ? 0 : slot / (width - 1);

			temp_texture->set_color(COLOR_PLUM);
			temp_texture->render((xpos * 34) + (18 - width), ((ypos + 1) * 34) + (17 - height));

			SDL_SetRenderTarget(engine.get_renderer(), NULL);
			engine.get_texture_manager()->free_texture(temp_texture->get_name());
		}
	}
}
void Inventory::modify_durability(uint8_t item_type, int8_t value)
{
	// If we're modifying light sources, we need to update the player's visibility range
	uint8_t visibility = 2;

	for (uint8_t i = 0; i < inventory.capacity(); i++)
	{
		Item *item = inventory[i];
		if (item != nullptr && item->get_equip_type() == item_type)
		{
			const uint8_t dur = item->get_durability();

			if (dur + value > 0 && dur + value < 256)
				item->set_durability(dur + value);
			else if (value < 0)
				item->set_durability(0);
			else item->set_durability(255);

			if (item_type == ET_LIGHT) // Keep track of the strongest light source
			{
				const uint8_t light_strength = std::min(item->get_value(0), item->get_durability());
				if (light_strength > visibility)
					visibility = light_strength;
			}
		}
	}
	if (item_type == ET_LIGHT)
		player->set_visibility_range(visibility);
}
bool Inventory::input_controller(uint8_t index, uint8_t value)
{
	if (!render_flag || !overlap_flag)
		return false;

	bool got_input = true;
	bool click = false;
	bool pass_control = false;

	if (value == 1) switch (index)
	{
		case 4:
			if (slot_y > 1) slot_y -= 1;
			else if (ID == INV_BACKPACK)
			{
				ui.give_controller_input(INV_EQUIP);
				pass_control = true;
			}
			else slot_y = height - 2;
			break;

		case 6:
			if (slot_y < height - 2) slot_y += 1;
			else if (ID == INV_EQUIP)
			{
				ui.give_controller_input(INV_BACKPACK);
				pass_control = true;
			}
			else slot_y = 1;
			break;

		case 7: (slot_x > 0) ? slot_x -= 1 : slot_x = width - 2; break;
		case 5: (slot_x < width - 2) ? slot_x += 1 : slot_x = 0; break;
		case 14: click = true; break;
		default: got_input = false; break;
	}
	if (got_input)
	{
		if (!pass_control)
			engine.set_mouse_pos(x + (slot_x * 34) + (20 - width), y + (slot_y * 34) + (19 - height));
		else overlap_flag = false;

		if (click)
			get_click(engine.get_mouse_x(), engine.get_mouse_y());
	}
	return got_input;
}
void Inventory::give_controller_input()
{
	//std::cout << "received controller input on inventory ID " << (int)ID << std::endl;
	slot_x = 0; slot_y = 0;
	set_render_flag(true);
}
bool Inventory::get_overlap(int16_t xpos, int16_t ypos)
{
	// NOTE - This function is called from a const function, so the variable changes here are not desired

	if (close_button != nullptr)
		close_button->get_overlap(xpos, ypos);

	if (!render_flag) return false;

	if (xpos > x && xpos < (x + width * 32) && ypos > (y + 16) && ypos < (y + height * 32))
	{
		if ((ypos - y - start_y) < -16) // Mouse is over the title
		{
			overlap_flag = false;
			return false;
		}
		overlap_flag = true;
		slot_x = (xpos - x - start_x) / 34;
		slot_y = (ypos - y - start_y) / 34 + 1;

		if (slot_y < 1) slot_y = 1;
		else if (slot_y > height - 2) slot_y = height - 2;
		if (slot_x < 0) slot_x = 0;
		else if (slot_x > width - 2) slot_x = width - 2;
	}
	else overlap_flag = false;
	return overlap_flag;
}
bool Inventory::get_click(int16_t xpos, int16_t ypos)
{
	// NOTE - This function is called from a const function, so any variable changes here are not desired

	if (!render_flag)
	{
		if (ID != INV_TEMP && close_button != nullptr && close_button->get_click())
		{
			set_render_flag(true);
			return true;
		}
		return false;
	}
	if (overlap_flag)
	{
		//std::cout << "mouse click at slot: " << (int)(slot_x) << ", " << (int)(slot_y - 1) << std::endl;
		const uint8_t slot = (slot_y - 1) * (width - 1) + slot_x;
		if (inventory.capacity() >= slot)
		{
			bool done_something = false;

			// Switch the temporary item held under the mouse with the item in the clicked slot
			// It could, of course, be an empty slot, in which case the temp item just gets cleared
			Item *temp_item = (inv_source != nullptr) ? inv_source->get_inventory_item(slot) : inventory[slot];
			if (ui.get_temp_item() != nullptr)
			{
				uint8_t limit = ET_NONE;
				for (auto &it : limits)
				{
					if (it.first == slot)
					{
						limit = it.second;
						break;
					}
				}
				if (limit == ET_NONE || ui.get_temp_item()->get_equip_type() == limit)
				{
					if (inv_source != nullptr)
						inv_source->set_inventory_item(slot, ui.get_temp_item());
					else inventory[slot] = ui.get_temp_item();

					ui.set_temp_item(temp_item);
					done_something = true;
				}
			}
			else if (temp_item != nullptr)
			{
				if (inv_source != nullptr)
					inv_source->set_inventory_item(slot, ui.get_temp_item());
				else inventory[slot] = ui.get_temp_item();

				ui.set_temp_item(temp_item);
				done_something = true;
			}
			// Update the player's equipped item values
			if (done_something && ID == INV_EQUIP && player != nullptr)
				player->update_item_values(inventory);
		}
		return true;
	}
	else if (close_button != nullptr && close_button->get_click())
	{
		set_render_flag(false);
		return true;
	}
	return false;
}
void Inventory::set_render_flag(bool flag)
{
	if (close_button != nullptr)
	{
		if (!flag)
		{
			const int8_t offset = (ID == INV_BACKPACK) ? -96 : 0;
			close_button->set_position(x + button_offset + offset, camera.get_cam_h() - 32);
			close_button->set_state(1);
			inv_source = nullptr;
		}
		else
		{
			close_button->set_position(x + button_offset, y);
			close_button->set_state(0);

			engine.set_mouse_pos(x + (slot_x * 34) + (20 - width), y + 34 + (slot_y * 34) + (19 - height));
		}
	}
	render_flag = flag;
}
void Inventory::set_position(int16_t xpos, int16_t ypos)
{
	x = xpos;
	y = ypos;

	if (close_button == nullptr)
	{
		close_button = new Button;
		close_button->init(BTN_WIDGET_TOGGLE);
	}
	set_render_flag(render_flag); // This is here just to update the button position
}
