/* cfnewspaper.c */
int initPlugin(const char *iversion, f_plug_api gethooksptr);
void *getPluginProperty(int *type, ...);
int cfnewspaper_runPluginCommand(object *op, char *params);
int cfnewspaper_globalEventListener(int *type, ...);
int postInitPlugin(void);
int eventListener(int *type, ...);
int closePlugin(void);
