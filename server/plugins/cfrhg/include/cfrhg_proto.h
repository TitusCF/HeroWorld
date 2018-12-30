/* cfrhg.c */
int cfrhg_globalEventListener(int *type, ...);
int eventListener(int *type, ...);
int initPlugin(const char *iversion, f_plug_api gethooksptr);
void *getPluginProperty(int *type, ...);
int cfrhg_runPluginCommand(object *op, char *params);
int postInitPlugin(void);
int closePlugin(void);
