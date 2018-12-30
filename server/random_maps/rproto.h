#ifndef _RPROTO_H
#define _RPROTO_H

/* random_map.c */
extern void dump_layout(char **layout, RMParms *RP);
extern mapstruct *generate_random_map(const char *OutFileName, RMParms *RP, char **use_layout);
extern char **layoutgen(RMParms *RP);
extern char **symmetrize_layout(char **maze, int sym, RMParms *RP);
extern char **rotate_layout(char **maze, int rotation, RMParms *RP);
extern void roomify_layout(char **maze, RMParms *RP);
extern int can_make_wall(char **maze, int dx, int dy, int dir, RMParms *RP);
extern int make_wall(char **maze, int x, int y, int dir);
extern void doorify_layout(char **maze, RMParms *RP);
extern StringBuffer *write_map_parameters_to_string(RMParms *RP);
/* room_gen_onion.c */
extern char **map_gen_onion(int xsize, int ysize, int option, int layers);
extern void centered_onion(char **maze, int xsize, int ysize, int option, int layers);
extern void bottom_centered_onion(char **maze, int xsize, int ysize, int option, int layers);
extern void draw_onion(char **maze, float *xlocations, float *ylocations, int layers);
extern void make_doors(char **maze, float *xlocations, float *ylocations, int layers, int options);
extern void bottom_right_centered_onion(char **maze, int xsize, int ysize, int option, int layers);
/* room_gen_spiral.c */
extern char **map_gen_spiral(int xsize, int ysize, int option);
extern void connect_spirals(int xsize, int ysize, int sym, char **layout);
/* maze_gen.c */
extern char **maze_gen(int xsize, int ysize, int option);
/* floor.c */
extern mapstruct *make_map_floor(char **layout, char *floorstyle, RMParms *RP);
/* wall.c */
extern int surround_flag(char **layout, int i, int j, RMParms *RP);
extern int surround_flag2(char **layout, int i, int j, RMParms *RP);
extern int surround_flag3(mapstruct *map, int i, int j, RMParms *RP);
extern int surround_flag4(mapstruct *map, int i, int j, RMParms *RP);
extern void make_map_walls(mapstruct *map, char **layout, char *w_style, RMParms *RP);
extern object *pick_joined_wall(object *the_wall, char **layout, int i, int j, RMParms *RP);
extern object *retrofit_joined_wall(mapstruct *the_map, int i, int j, int insert_flag, RMParms *RP);
/* monster.c */
extern void insert_multisquare_ob_in_map(object *new_obj, mapstruct *map);
extern void place_monsters(mapstruct *map, char *monsterstyle, int difficulty, RMParms *RP);
/* door.c */
extern int surround_check2(char **layout, int i, int j, int Xsize, int Ysize);
extern void put_doors(mapstruct *the_map, char **maze, const char *doorstyle, RMParms *RP);
/* decor.c */
extern int obj_count_in_map(mapstruct *map, int x, int y);
extern void put_decor(mapstruct *map, char **maze, char *decorstyle, int decor_option, RMParms *RP);
/* exit.c */
extern void find_in_layout(int mode, char target, int *fx, int *fy, char **layout, RMParms *RP);
extern void place_exits(mapstruct *map, char **maze, char *exitstyle, int orientation, RMParms *RP);
extern void unblock_exits(mapstruct *map, char **maze, RMParms *RP);
/* treasure.c */
extern int wall_blocked(mapstruct *m, int x, int y);
extern void place_treasure(mapstruct *map, char **layout, char *treasure_style, int treasureoptions, RMParms *RP);
extern object *place_chest(int treasureoptions, int x, int y, mapstruct *map, mapstruct *style_map, int n_treasures, RMParms *RP);
extern object *find_closest_monster(mapstruct *map, int x, int y, RMParms *RP);
extern int keyplace(mapstruct *map, int x, int y, char *keycode, int door_flag, int n_keys, RMParms *RP);
extern object *find_monster_in_room_recursive(char **layout, mapstruct *map, int x, int y, RMParms *RP);
extern object *find_monster_in_room(mapstruct *map, int x, int y, RMParms *RP);
extern int find_spot_in_room(mapstruct *map, int x, int y, int *kx, int *ky, RMParms *RP);
extern void find_enclosed_spot(mapstruct *map, int *cx, int *cy, RMParms *RP);
extern void remove_monsters(int x, int y, mapstruct *map);
extern void find_doors_in_room_recursive(char **layout, mapstruct *map, int x, int y, object **doorlist, int *ndoors, RMParms *RP);
extern object **find_doors_in_room(mapstruct *map, int x, int y, RMParms *RP);
extern void lock_and_hide_doors(object **doorlist, mapstruct *map, int opts, RMParms *RP);
/* special.c */
extern void nuke_map_region(mapstruct *map, int xstart, int ystart, int xsize, int ysize);
extern void include_map_in_map(mapstruct *dest_map, const mapstruct *in_map, int x, int y);
extern int find_spot_for_submap(mapstruct *map, char **layout, int *ix, int *iy, int xsize, int ysize);
extern void place_fountain_with_specials(mapstruct *map);
extern void place_special_exit(mapstruct *map, int hole_type, const RMParms *RP);
extern void place_specials_in_map(mapstruct *map, char **layout, RMParms *RP);
/* style.c */
extern int load_dir(const char *dir, char ***namelist, int skip_dirs);
extern mapstruct *load_style_map(char *style_name);
extern mapstruct *find_style(const char *dirname, const char *stylename, int difficulty);
extern object *pick_random_object(mapstruct *style);
extern void free_style_maps(void);
/* rogue_layout.c */
extern int surround_check(char **layout, int i, int j, int Xsize, int Ysize);
extern char **roguelike_layout_gen(int xsize, int ysize, int options);
/* snake.c */
extern char **make_snake_layout(int xsize, int ysize);
/* square_spiral.c */
extern void find_top_left_corner(char **maze, int *cx, int *cy);
extern char **make_square_spiral_layout(int xsize, int ysize);
/* expand2x.c */
extern char **expand2x(char **layout, int xsize, int ysize);

#endif
