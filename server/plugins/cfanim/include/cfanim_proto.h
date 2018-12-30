/* cfanim.c */
CF_PLUGIN int initPlugin(const char *iversion, f_plug_api gethooksptr);
CF_PLUGIN void *getPluginProperty(int *type, ...);
CF_PLUGIN anim_move_result cfanim_runPluginCommand(object *op, char *params);
CF_PLUGIN int postInitPlugin(void);
CF_PLUGIN int cfanim_globalEventListener(int *type, ...);
CF_PLUGIN int eventListener(int *type, ...);
CF_PLUGIN int closePlugin(void);
