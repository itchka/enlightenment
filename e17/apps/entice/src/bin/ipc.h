#ifndef ENTICE_IPC_H
#define ENTICE_IPC_H

/**
 * entice_ipc_init - entialize entice's ipc handlers
 * @argc - the number or arguments
 * @argv - pointer to the argument strings
 * Lengthy descriptin in entice_ipc.c
 */
int entice_ipc_init(int argc, const char **argv);
void entice_ipc_shutdown(void);
void entice_ipc_client_request_image_load(const char *file);

#endif
