/* altar/altar.c */
void init_type_altar(void);
/* armour_improver/armour_improver.c */
void init_type_armour_improver(void);
/* arrow/arrow.c */
void init_type_arrow(void);
/* blindness/blindness.c */
void init_type_blindness(void);
/* book/book.c */
void init_type_book(void);
/* button/button.c */
void init_type_button(void);
/* cf_handle/cf_handle.c */
void init_type_cf_handle(void);
/* check_inv/check_inv.c */
void init_type_check_inv(void);
/* clock/clock.c */
void init_type_clock(void);
/* container/container.c */
void init_type_container(void);
/* converter/converter.c */
void init_type_converter(void);
/* creator/creator.c */
void init_type_creator(void);
/* deep_swamp/deep_swamp.c */
void init_type_deep_swamp(void);
/* detector/detector.c */
void init_type_detector(void);
/* director/director.c */
void init_type_director(void);
/* dragon_focus/dragon_focus.c */
void init_type_dragon_focus(void);
/* duplicator/duplicator.c */
void init_type_duplicator(void);
/* exit/exit.c */
void init_type_exit(void);
/* food/food.c */
void init_type_food(void);
/* gate/gate.c */
void init_type_gate(void);
/* hole/hole.c */
void init_type_hole(void);
/* identify_altar/identify_altar.c */
void init_type_identify_altar(void);
/* lamp/lamp.c */
void init_type_lamp(void);
/* lighter/lighter.c */
void init_type_lighter(void);
/* marker/marker.c */
void init_type_marker(void);
/* mood_floor/mood_floor.c */
void init_type_mood_floor(void);
/* peacemaker/peacemaker.c */
void init_type_peacemaker(void);
/* pedestal/pedestal.c */
void init_type_pedestal(void);
/* player_changer/player_changer.c */
void init_type_player_changer(void);
/* player_mover/player_mover.c */
void init_type_player_mover(void);
/* poison/poison.c */
void init_type_poison(void);
/* poisoning/poisoning.c */
void init_type_poisoning(void);
/* potion/potion.c */
void init_type_potion(void);
/* power_crystal/power_crystal.c */
void init_type_power_crystal(void);
/* savebed/savebed.c */
void init_type_savebed(void);
/* scroll/scroll.c */
void init_type_scroll(void);
/* shop_inventory/shop_inventory.c */
void init_type_shop_inventory(void);
/* shop_mat/shop_mat.c */
void init_type_shop_mat(void);
/* sign/sign.c */
void init_type_sign(void);
/* skillscroll/skillscroll.c */
void init_type_skillscroll(void);
/* spell_effect/spell_effect.c */
void init_type_spell_effect(void);
/* spellbook/spellbook.c */
void init_type_spellbook(void);
/* spinner/spinner.c */
void init_type_spinner(void);
/* teleporter/teleporter.c */
void init_type_teleporter(void);
/* thrown_object/thrown_object.c */
void init_type_thrown_object(void);
/* transport/transport.c */
void init_type_transport(void);
/* trap/common_trap.c */
method_ret common_trap_type_move_on(ob_methods *context, object *trap, object *victim, object *originator);
method_ret common_trap_type_process(ob_methods *context, object *op);
/* trap/trap.c */
void init_type_trap(void);
/* trap/rune.c */
void init_type_rune(void);
/* trapdoor/trapdoor.c */
void init_type_trapdoor(void);
/* treasure/treasure.c */
void init_type_treasure(void);
/* trigger/trigger.c */
void init_type_trigger(void);
/* trigger_altar/trigger_altar.c */
void init_type_trigger_altar(void);
/* trigger_button/trigger_button.c */
void init_type_trigger_button(void);
/* trigger_pedestal/trigger_pedestal.c */
void init_type_trigger_pedestal(void);
/* weapon_improver/weapon_improver.c */
void init_type_weapon_improver(void);
/* common/common_apply.c */
method_ret common_ob_move_on(ob_methods *context, object *trap, object *victim, object *originator);
method_ret common_pre_ob_move_on(object *trap, object *victim, object *originator);
void common_post_ob_move_on(object *trap, object *victim, object *originator);
/* common/describe.c */
void common_ob_describe(const ob_methods *context, const object *op, const object *observer, char *buf, size_t size);
/* common/projectile.c */
void stop_projectile(object *op);
method_ret common_process_projectile(ob_methods *context, object *op);
method_ret common_projectile_move_on(ob_methods *context, object *trap, object *victim, object *originator);
/* legacy/apply.c */
method_ret legacy_ob_apply(ob_methods *context, object *op, object *applier, int aflags);
/* legacy/legacy_describe.c */
void legacy_ob_describe(const ob_methods *context, const object *op, const object *observer, char *buf, size_t size);
/* legacy/process.c */
method_ret legacy_ob_process(ob_methods *context, object *op);
