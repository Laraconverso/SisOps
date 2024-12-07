#include "fs.h"
#include <stdlib.h>

struct super_block super_b = {};

// FS
// inicializar FILE SYS
// definir tamanio maximo
//  y cantidad max. de directorios anidados
int
initialize_fs()
{
	// inicializo el bloque de memoria en 0
	memset(super_b.inodes, 0, sizeof(super_b.inodes));
	memset(super_b.bitmap_inodes, 0, sizeof(super_b.bitmap_inodes));

	struct inode *root = &super_b.inodes[0];
	root->type = FS_DIR;
	root->mode = MODE_DIR;
	root->size = MAX_DIR_SIZE;
	root->id_user = getuid();
	root->id_grup = getgid();
	root->stats_info.creation = time(NULL);
	root->stats_info.last_acc = time(NULL);
	root->stats_info.last_mod = time(NULL);
	strcpy(root->path, ROOT_PATH);
	memset(root->content, 0, sizeof(root->content));
	strcpy(root->directory_path, "");
	super_b.bitmap_inodes[0] = 1;


	return 0;
}

// guardar
int
save_fs(char *save_file)
{
	return 0;
}

// FILE OPERTATIONS
// read file
int
read_file(char *path)
{
	return 0;
}
// funcion para eliminar el slash del path y devolver solo el nombre del
// archivo o del directorio
char *
remove_slash(const char *path)
{
	size_t len = strlen(path);
	char *new_path = malloc(len);
	if (!new_path) {
		return NULL;
	}

	memcpy(new_path, path + 1, len - 1);
	new_path[len - 1] = '\0';

	const char *ultimo = strrchr(path, '/');
	if (ultimo == NULL) {
		return new_path;
	}

	size_t final_len = strlen(ultimo + 1);
	char *final_path = malloc(final_len + 1);
	if (!final_path) {
		free(new_path);
		return NULL;
	}

	memcpy(final_path, ultimo + 1, final_len);
	final_path[final_len] = '\0';

	free(new_path);
	return final_path;
}

// funcion para obtener el indice del inodo
int get_index_inodo(const char *path){
	if(strcmp(path,ROOT_PATH)==0){
		return 0;
	}
	char *new_path = remove_slash(path);
	if(!new_path){
		return -1;
	}
	for(int i=0;i<MAX_INODES;i++){
		if(strcmp(new_path,super_b.inodes[i].path)==0){
			return i;
		}
	}
	free(new_path);
	return -1;
}

// funcion para modificar el path del inodo y cambio el '/' por '\0'
void
get_path_padre(char *path_padre)
{
	char *ultimo = strrchr(path_padre, '/');
	if (ultimo != NULL) {
		*ultimo = '\0';
	} else {
		path_padre[0] = '\0';
	}
}

// Funcion para buscar el proximo inodo libre
//  ENOSPC si no hay espacio
//  EEXIST si ya existe nodo en el path
int
next_free_inodo(const char *path)
{
	bool existe = false;
	int next_free = -ENOSPC;
	for (int i = 0; i < MAX_INODES && !existe; i++) {
		if (super_b.bitmap_inodes[i] == 0 && next_free < 0) {
			// me quedo con el primer indice libre
			next_free = i;
		}
		if (strcmp(super_b.inodes[i].path, path) == 0) {
			existe = true;
		}
	}
	if (existe) {
		fprintf(stderr,
		        "[Debug] Error next_free_iodo: %s\n",
		        strerror(errno));
		errno = EEXIST;
		return -EEXIST;
	} else {
		return next_free;
	}
}


// create file
/*crea un nuevo inodo con el path, mode y tipo que se le pasa.
lo guarda en el superbloque.-
En caso de error devuelve: ENAMETOOLONG, si el path es muy largo
        ENOSPC, si no hay más espacio
        EEXIST, si ya existe un inodo a ese path*/
int
create_file(const char *path, mode_t mode, int type)
{
	if (strlen(path) - 1 > MAX_CONTENT) {
		fprintf(stderr, "[Debug] Error create_file: %s\n", strerror(errno));
		errno = ENAMETOOLONG;
		return -ENAMETOOLONG;
	}

	char *final_path = remove_slash(path);
	if (!final_path) {
		return -1;
	}
	int i = next_free_inodo(final_path);
	if (i < 0) {
		return i;
	}

	struct inode new_inodo;
	new_inodo.type = type;
	new_inodo.mode = mode;
	new_inodo.size = 0;  // debería arranca vacío
	new_inodo.id_user = getuid();
	new_inodo.id_grup = getgid();
	new_inodo.stats_info.last_acc = time(NULL);
	new_inodo.stats_info.last_mod = time(NULL);
	strcpy(new_inodo.path, final_path);

	if (type == FS_FILE) {
		char path_padre[MAX_PATH];
		memcpy(path_padre, path + 1, strlen(path) - 1);
		path_padre[strlen(path) - 1] = '\0';
		get_path_padre(path_padre);

		if (strlen(path_padre) == 0) {
			strcpy(path_padre, ROOT_PATH);
		}

		strcpy(new_inodo.directory_path, path_padre);
	} else {
		strcpy(new_inodo.directory_path, ROOT_PATH);
	}

	memset(new_inodo.content, 0, sizeof(new_inodo.content));
	super_b.inodes[i] = new_inodo;
	super_b.bitmap_inodes[i] = 1;
	free(final_path);

	return 0;
}
// delete file
int
delete_file(char *path)
{
	for (int i = 0; i < MAX_INODES; i++) {
		if (super_b.bitmap_inodes[i] &&
		    strcmp(super_b.inodes[i].path, path) == 0) {
			struct inode *file = &super_b.inodes[i];
			if (file->type != FS_FILE) {
				fprintf(stderr,
				        "[Debug] Error: %s is not a file.\n",
				        path);
				return -1;
			}
			memset(file, 0, sizeof(struct inode));
			super_b.bitmap_inodes[i] = 0;
			return 0;
		}
	}
	fprintf(stderr, "[Debug] Error: File %s not found.\n", path);
	return -1;
}

// write file
int
write_file(const char *path, const char *buffer, size_t size, off_t offset)
{
	if (offset + size > MAX_CONTENT) {
		fprintf(stderr, "Error: File write exceeds max content size.\n");
		return -EFBIG;
	}
	int inode_index = get_index_inodo(path);
	if(inode_index<0){
		int  new_file = create_file(path,0644,FS_FILE);
		if (new_file<0){
			return new_file;
		}
		inode_index=get_index_inodo(path);
	}
	
	/*for (int i = 0; i < MAX_INODES; i++) {
		if (super_b.bitmap_inodes[i] &&
		    strcmp(super_b.inodes[i].path, path) == 0) {
			inode_index = i;
			break;
		}
	}*/
	if (inode_index == -1) {
		fprintf(stderr, "Error: File not found.\n");
		return -ENOENT;
	}
	struct inode *file_inode = &super_b.inodes[inode_index];
	if(file_inode->size < offset){
		fprintf(stderr,"[Debug] Error write: %s\n",strerror(errno));
		errno = EINVAL;
		return -EINVAL;
	}	
	
	if (file_inode->type == FS_DIR) {
		fprintf(stderr, "Error: Cannot write to a directory.\n");
		return -EACCES;
	}
	/*
	if (offset + size > MAX_CONTENT) {
		size = MAX_CONTENT - offset;
	}
	memcpy(file_inode->content + offset, buffer, size);
	*/
	strncpy(file_inode->content + offset, buffer,size);
	file_inode->size = strlen(file_inode->content);
	file_inode->stats_info.last_mod = time(NULL);
	file_inode->stats_info.last_acc = time(NULL);
	file_inode->content[file_inode->size] ='\0';

	return (int) size;
}

// stats
stats_t
get_stats(char *path)
{
	stats_t stats = {};
	return stats;
}

// DIR OPERATIONS
// create dir
int
create_dir(const char *path, mode_t mode)
{
	if (strlen(path) - 1 > MAX_CONTENT) {
		fprintf(stderr,
		        "[Debug] Error create_dir: %s\n",
		        strerror(ENAMETOOLONG));
		errno = ENAMETOOLONG;
		return -ENAMETOOLONG;
	}

	char *final_path = remove_slash(path);
	if (!final_path) {
		return -1;
	}

	int i = next_free_inodo(final_path);
	if (i < 0) {
		free(final_path);
		return i;
	}

	struct inode new_inodo;
	new_inodo.type = FS_DIR;
	new_inodo.mode = mode;
	new_inodo.size = 0;
	new_inodo.id_user = getuid();
	new_inodo.id_grup = getgid();
	new_inodo.stats_info.creation = time(NULL);
	new_inodo.stats_info.last_acc = time(NULL);
	new_inodo.stats_info.last_mod = time(NULL);
	strcpy(new_inodo.path, final_path);

	char path_padre[MAX_PATH];
	memcpy(path_padre, path + 1, strlen(path) - 1);
	path_padre[strlen(path) - 1] = '\0';
	get_path_padre(path_padre);

	if (strlen(path_padre) == 0) {
		strcpy(path_padre, ROOT_PATH);
	}

	strcpy(new_inodo.directory_path, path_padre);
	memset(new_inodo.content, 0, sizeof(new_inodo.content));
	super_b.inodes[i] = new_inodo;
	super_b.bitmap_inodes[i] = 1;

	free(final_path);
	return 0;
}

// get dir
char *
get_dir(char *path, mode_t mode)
{
	char *re = "";
	return re;
}

// delete dir
int unlink(const char *path){
		int i = get_index_inodo(path);
	if (i < 0) {
		return -1;  // No existe el archivo
	}

	//creo el sctruc inode para poder usar luego el memset
	//struct inode *inode = &super_b.inodes[i];

	// Verificar si el archivo existe y no es un dir
	if (super_b.inodes[i].type == FS_DIR) {
		return -EISDIR;
	}

	super_b.bitmap_inodes[i] = 0;
	//memset(inode,0,sizeof(struct inode));
	for (int j = 0; j < MAX_CONTENT; j++) {
		super_b.inodes[i].content[j] = 0;
	}

	for (int k = 0; k < MAX_PATH; k++) {
		super_b.inodes[i].path[k] = 0;
	}

	return 0;
}


int
delete_dir(const char *path)
{

	// Obtener el índice del inodo para el directorio.
    int pos = get_index_inodo(path);
    if (pos < 0) {
        return pos; // Retorna error si no se encuentra el inodo.
    }

    if (super_b.inodes[pos].type == FS_FILE) {
        return -ENOTDIR; // Error: El path corresponde a un archivo, no un directorio.
    }

    // Remover la barra inicial del path para la comparación.
    char *new_path = remove_slash(path);
    if (!new_path) {
        return -ENOMEM; // Error de memoria.
    }

    // Verificar si el directorio tiene contenido.
    for (int j = 0; j < MAX_INODES; j++) {
        if (strcmp(super_b.inodes[j].directory_path, new_path) == 0) {
            // Si contiene archivos o subdirectorios, retornar -ENOTEMPTY.
            free(new_path);
            return -ENOTEMPTY;
        }
    }

    // Liberar la memoria de la ruta transformada.
    free(new_path);

    // Marcar el inodo del directorio como libre y limpiar sus datos.
    super_b.bitmap_inodes[pos] = 0; // Marcar como libre.
    memset(&super_b.inodes[pos], 0, sizeof(struct inode)); // Limpiar el inodo.

    return 0; 
	/*ESTE BORRA TODO
	int pos = get_index_inodo(path);
	if(pos < 0){
		return pos;
	}
	if(super_b.inodes[pos].type==FS_FILE){
		return -1;	
	}
	char *new_path = remove_slash(path);
	for(int j=0;j<MAX_INODES; j++){
		if(strcmp(super_b.inodes[j].path,new_path)==0){
			if(super_b.inodes[j].type==FS_FILE){
				unlink(super_b.inodes[j].path);
			}
			if(super_b.inodes[j].type==FS_DIR){
				delete_dir(super_b.inodes[j].path);
			}
		}
	}

	super_b.bitmap_inodes[pos]=0;
	for (int k = 0; k < MAX_CONTENT; k++) {
		super_b.inodes[pos].content[k] = 0;
	}
	for (int k = 0; k < MAX_PATH; k++) {
		super_b.inodes[pos].path[k] = 0;
	}

	return 0;
	HASTA ACA*/
/*

	struct inode *dir = &super_b.inodes[pos];
	int tope=0;
	struct inode **file = file_dir(path, &tope);
	if(!file){
		fprintf(stderr,"[Debug] Error rmdir1 %s\n",strerror(errno));
		errno=ENOMEM;
		return -ENOMEM;
	}
	free(file);


	if(tope>0){
		fprintf(stderr,"[Debug] Error rmdir3: %s\n",strerror(errno));
		errno=ENOTEMPTY;
		return -ENOTEMPTY;	
	}
	super_b.bitmap_inodes[pos] = 0;
	memset(dir, 0, sizeof(struct inode));
	return 0;


	// Encontramos el inodo segun el path pasado por parametro
	for (int i = 0; i < MAX_INODES; i++) {
		if (strcmp(new_path,super_b.inodes[i].path) == 0) {
			if(super_b.inodes[i].type == FS_FILE){
				int j = get_index_inodo(super_b.inodes[i].path);
				if(j<0) return j;
				if(super_b.inodes[j].type==FS_DIR) return -1;
				super_b.bitmap_inodes[j]=0;
				for (int k = 0; k < MAX_INODES; k++) {
					super_b.inodes[j].content[k] = 0;
				}
				for(int k = 0; k<MAX_PATH;k++){
					super_b.inodes[j].path[k]=0;
				}
				return 0;
			}
			else{
				delete_dir(super_b.inodes[i].path);
			}
		}
	}
	free(new_path);
	super_b.bitmap_inodes[pos] = 0;

	for (int k = 0; k < MAX_CONTENT; k++) {
		super_b.inodes[pos].content[k] = 0;
	}
	for (int l = 0; l < MAX_PATH; l++) {
		super_b.inodes[pos].path[l] = 0;
	}


			// chequamos que sea un directorio
			if (dir->type != FS_DIR) {
				fprintf(stderr, "[Debug] Error: %s is not a directory.\n", path);
				return -1;
			}
				if (super_b.bitmap_inodes[j]) {
					struct inode *entry = &super_b.inodes[j];
					if (strncmp(entry->directory_path,
					            path,
					            strlen(path)) == 0) {
						// si es un dir lo borramos recursivamente
						if (entry->type == FS_DIR) {
							if (delete_dir(entry->path) !=
							    0) {
								fprintf(stderr, "[Debug] Error: Failed to delete subdirectory %s.\n", entry->path);
								return -1;
							}
						}
						// si es un archivo, lo borramos
						else if (entry->type == FS_FILE) {
							if (delete_file(entry->path) !=
							    0) {
								fprintf(stderr, "[Debug] Error: Failed to delete file %s.\n", entry->path);
								return -1;
							}
						}
					}
				}
			}
		}
	}
	fprintf(stderr, "[Debug] Error: Directory %s not found.\n", path);
	return -1;*/
}

// list dir
int
list_dir(char *path)
{
	return 0;
}