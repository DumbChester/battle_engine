#include <pokeagb/pokeagb.h>
#include "../battle_data/battle_state.h"
#include "moves/moves.h"
#include "../../generated/images/type_icons.h"
#include "../../generated/images/PSS_icons.h"
#include "../../generated/images/hpbox/empty_bar.h"

#define MOVE_NAMES_TAG 0x900
#define MOVE_PP_TAG 0x980
#define MOVE_PSS_TAG 0x990

const struct OamData type_icon_oam = {
                                .y = 0,
                                .affine_mode = 0,
                                .obj_mode = 0,
                                .mosaic = 0,
                                .bpp = 0,
                                .shape = 1,
                                .x = 0,
                                .matrix_num = 0,
                                .size = 2, //32x16
                                .tile_num = 0,
                                .priority = 0,
                                .palette_num = 0,
                                .affine_param = 0
};

const struct OamData pp_icon_oam = {
                                .y = 0,
                                .affine_mode = 0,
                                .obj_mode = 0,
                                .mosaic = 0,
                                .bpp = 0,
                                .shape = 0,
                                .x = 0,
                                .matrix_num = 0,
                                .size = 1, //16x16
                                .tile_num = 0,
                                .priority = 0,
                                .palette_num = 0,
                                .affine_param = 0
};

const struct OamData text_oam = {
                                .y = 0,
                                .affine_mode = 0,
                                .obj_mode = 0,
                                .mosaic = 0,
                                .bpp = 0,
                                .shape = 1,
                                .x = 0,
                                .matrix_num = 0,
                                .size = 3, //64x32
                                .tile_num = 0,
                                .priority = 0, // above BG background, below entry layer
                                .palette_num = 0,
                                .affine_param = 0
};


static const struct Frame (**nullframe)[] = (const struct Frame (**)[])0x8231CF0;
static const struct RotscaleFrame (**nullrsf)[] = (const struct RotscaleFrame (**)[])0x8231CFC;

extern void oac_nullsub(struct Object*);


u8 load_dmg_type_icon(u8 type, s16 x, s16 y, u8 tag)
{
    struct SpritePalette icon_pal = {(void*)type_iconsPal, 0x750};
    struct SpriteTiles icon_gfx = {(void*)(type_iconsTiles + (type * 256)), 256, 0x750 + tag};
    struct Template icon_temp = {0x750 + tag, 0x750, &type_icon_oam, nullframe, &icon_gfx, nullrsf,
                                (ObjectCallback)oac_nullsub};
    gpu_tile_obj_alloc_tag_and_upload(&icon_gfx);
    gpu_pal_obj_alloc_tag_and_apply(&icon_pal);
    return template_instanciate_forward_search(&icon_temp, x, y, 0);
}

u8 load_dmg_category_icon(u8 category, s16 x, s16 y, u8 tag)
{
    struct SpritePalette icon_pal = {(void*)PSS_iconsPal, MOVE_PSS_TAG};
    struct SpriteTiles icon_gfx = {(void*)(PSS_iconsTiles + (category * 128)), 256, MOVE_PSS_TAG + tag};
    struct Template icon_temp = {MOVE_PSS_TAG + tag, MOVE_PSS_TAG, &pp_icon_oam, nullframe, &icon_gfx, nullrsf,
                                (ObjectCallback)oac_nullsub};
    gpu_tile_obj_alloc_tag_and_upload(&icon_gfx);
    gpu_pal_obj_alloc_tag_and_apply(&icon_pal);
    return template_instanciate_forward_search(&icon_temp, x, y, 0);
}

void draw_text_obj(u8 font, u8 x_shift, u8 y_shift, pchar* string_buff, void* vram_addr, u8 background_pal)
{
    u8 r_id = rboxid_init((struct Textbox*)0x8260564);
    rboxid_clear_pixels(r_id, background_pal);
    u32 arrays[] = {2, 1, 3};
    rboxid_add_812E5A4(r_id, font, x_shift, y_shift, 0, 0, (u32*)&arrays[0], 0xFF, string_buff);
    u32 pixels = rboxid_get_field(r_id, 7);
    rbox_to_vram(vram_addr, (void*)(pixels), 8);
    rboxid_free(r_id);
}

u8 draw_pp(u8 bank, u8 index)
{
    pchar prefix[] = _("{HIGHLIGHT 1}{COLOR 2}{SHADOW 3} ");
    pstrcpy(string_buffer, prefix);
    u8 pp = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_PP1 + index, NULL);
    fmt_int_10(pstrlen(string_buffer) + string_buffer - ((pp < 10) ? 0 : 1), pp, 0, 3);
    
    /* Make canvas object */
    struct SpritePalette text_pal = {(void*)stdpal_get(1), MOVE_PP_TAG};
    struct SpriteTiles text_gfx = {(void*)empty_barTiles, 1024, MOVE_PP_TAG + index};
    struct Template text_temp = {MOVE_PP_TAG + index, MOVE_PP_TAG, &text_oam, nullframe, &text_gfx, nullrsf, (ObjectCallback)oac_nullsub};
    gpu_tile_obj_decompress_alloc_tag_and_upload(&text_gfx);
    gpu_pal_obj_alloc_tag_and_apply(&text_pal);
    
    void* vram_addr;
    u8 objid;
 
     switch (index) {
        case 0:
            objid = template_instanciate_forward_search(&text_temp, 110, 131, 0);
            vram_addr = (void*)((objects[objid].final_oam.tile_num * 32) + 0x6010000);   
            break;
        case 1:
            objid = template_instanciate_forward_search(&text_temp, 212, 131, 0);
            vram_addr = (void*)((objects[objid].final_oam.tile_num * 32) + 0x6010000);
            break;
        case 2:
            objid = template_instanciate_forward_search(&text_temp, 110, 150, 0);
            vram_addr = (void*)((objects[objid].final_oam.tile_num * 32) + 0x6010000);
            break;
        default:
            objid = template_instanciate_forward_search(&text_temp, 212, 150, 0);
            vram_addr = (void*)((objects[objid].final_oam.tile_num * 32) + 0x6010000);
            break;
    };
    draw_text_obj(0, 0, 4, string_buffer, vram_addr, 0);
    return objid;
}

void load_names_moves(u8 bank)
{
    u8 i;
    pchar prefix[] = _("{HIGHLIGHT 1}{COLOR 2}{SHADOW 3}");
    u16 p_moves[4];
    for (i = 0; i < 4; i++) {
        /* figure out move string */
        p_moves[i] = pokemon_getattr(p_bank[bank]->this_pkmn, (0xD + i), NULL);
        pstrcpy(string_buffer, prefix);
        pstrcat(string_buffer, moves[p_moves[i]].name);
        
        /* Make canvas object */
        struct SpritePalette text_pal = {(void*)stdpal_get(1), MOVE_NAMES_TAG};
        struct SpriteTiles text_gfx = {(void*)empty_barTiles, 1024, MOVE_NAMES_TAG + i};
        struct Template text_temp = {MOVE_NAMES_TAG + i, MOVE_NAMES_TAG, &text_oam, nullframe, &text_gfx, nullrsf, (ObjectCallback)oac_nullsub};
        gpu_tile_obj_decompress_alloc_tag_and_upload(&text_gfx);
        gpu_pal_obj_alloc_tag_and_apply(&text_pal);
        
        
        void* vram_addr;
        u8 objid;        
        switch (i) {
            case 0:
                objid = template_instanciate_forward_search(&text_temp, 43, 131, 0);
                vram_addr = (void*)((objects[objid].final_oam.tile_num * 32) + 0x6010000);                
                break;
            case 1:
                objid = template_instanciate_forward_search(&text_temp, 145, 131, 0);
                vram_addr = (void*)((objects[objid].final_oam.tile_num * 32) + 0x6010000);
                break;
            case 2:
                objid = template_instanciate_forward_search(&text_temp, 43, 150, 0);
                vram_addr = (void*)((objects[objid].final_oam.tile_num * 32) + 0x6010000);
                break;
            default:
                objid = template_instanciate_forward_search(&text_temp, 145, 150, 0);
                vram_addr = (void*)((objects[objid].final_oam.tile_num * 32) + 0x6010000);
                break;
        };
        draw_text_obj(0, 0, 3, string_buffer, vram_addr, 0);
        battle_master->move_name_objid[i] = objid;
            // 0, 0
    
    }    
    
}

void load_icons_moves(u8 bank)
{
    u16 p_moves[4];
    u8 i;
    for (i = 0; i < 4; i++) {
        p_moves[i] = pokemon_getattr(p_bank[bank]->this_pkmn, (0xD + i), NULL);
        s16 x = (!i || (i == 2)) ? 92 : 194;
        s16 y = (i < 2) ? 126 : 145;
        u8 type = moves[p_moves[i]].type;
        if (type == MTYPE_EGG) {
            battle_master->type_objid[i] = 0x3F;
            battle_master->move_pp_objid[i] = 0x3F;
            battle_master->move_pss_objid[i] = 0x3F;
        } else {
            u8 objid;
            battle_master->type_objid[i] = load_dmg_type_icon(type, x, y, i);
            objid = draw_pp(bank, i);
            objects[objid].final_oam.affine_mode = 2;
            battle_master->move_pp_objid[i] = objid;
            objid = load_dmg_category_icon(MOVE_CATEGORY(p_moves[i]), x + 8, y, i);
            battle_master->move_pss_objid[i] = objid;
            objects[objid].final_oam.affine_mode = 2;
        }
    }
    load_names_moves(bank);
}















