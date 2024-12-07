#ifndef FS_H
#define FS_H

// includes
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

// estructura
#define MAX_INODES 64
#define SAVE_FILE "fs.fisopsfs"
#define MAX_DIR_SIZE 1024
#define MAX_PATH 200
#define MAX_CONTENT 1024
#define ROOT_PATH "/"

// mode

#define MODE_DIR __S_IFDIR | 0755
#define MODE_FILE __S_IFREG | 0644

typedef enum inode_type { FS_DIR, FS_FILE } inode_type;


typedef struct stats {
	time_t last_acc;  // fecha de ultimo acceso
	time_t last_mod;  // fecha de ultima modificacion
	time_t creation;
} stats_t;

struct inode {
	inode_type type;     // tipo -> variantes archivo y directorio
	size_t size;         // tamanio
	uid_t id_user;       // id propietario
	gid_t id_grup;       // id grupo
	mode_t mode;         // permisos lectura, escritura o ejecutado
	stats_t stats_info;  // tiempo de acceso  //tiempo de modificacion
	nlink_t link_num;    // nro de enlaces
	char path[MAX_PATH];
	char content[MAX_CONTENT];
	char directory_path[MAX_PATH];  // punteros a los bloques de disco
};

struct super_block {
	struct inode inodes[MAX_INODES];
	int bitmap_inodes[MAX_INODES];
};

extern struct super_block super_b;

//-> puede ser por inodos u alguna otra opcion (pensar)
int get_index_inodo(const char *path);
char *remove_slash(const char *path);
void get_path_padre(char *path_padre);
int next_free_inodo(const char *path);
// inicializar FILE SYS
int initialize_fs();
// guardar fs
int save_fs(char *save_file);

// FILE OPERTATIONS
// read file
int read_file(char *path);
// create file
int create_file(const char *path, mode_t mode, int type);
// delete file
int delete_file(char *path);
// write file
int write_file(const char *path, const char *buffer, size_t size, off_t offset);
// stats
stats_t get_stats(char *path);


// DIR OPERATIONS
// create dir
int create_dir(const char *path, mode_t mode);
// get dir
char *get_dir(char *path, mode_t mode);

// delete dir

int delete_dir(const char *path);
// list dir
int list_dir(char *path);

int unlink(const char *path);


#endif