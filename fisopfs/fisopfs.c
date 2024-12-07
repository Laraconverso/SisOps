#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "fs.h"

#define DEFAULT_FILE_DISK "fs.fisopfs"


char filedisk[MAX_PATH] = DEFAULT_FILE_DISK;


// Función para buscar el inodo de un archivo dado un path
/*int
search_inode(const char *path)
{
        if (strcmp(path, "/") == 0) {
                return 0;
        }

        // Si el path comienza con '/' lo descarto
        const char *path_ws = path;
        if (path[0] == '/') {
                path_ws = path + 1;
        }
        char *path_copy = strdcmp(path_ws);
        //char *path_copy = strdup(path_ws);
        if (!path_copy) {
                return -1;
        }
        for (int i = 0; i < MAX_INODES; i++) {
                if (strcmp(path_copy, super_b.inodes[i].path) == 0) {
                        free(path_copy);
                        return i;
                }
        }

        free(path_copy);
        return -1;
}
*/

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	int pos = get_index_inodo(path);
	if (pos == -1) {
		fprintf(stderr, "[Debug] Error getattr: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}
	struct inode inode = super_b.inodes[pos];
	st->st_dev = 0;
	st->st_ino = pos;
	st->st_uid = inode.id_user;
	st->st_mode = __S_IFDIR | 0755;
	st->st_atime = inode.stats_info.last_acc;
	st->st_mtime = inode.stats_info.last_mod;
	st->st_ctime = inode.stats_info.creation;
	st->st_size = inode.size;
	st->st_gid = inode.id_grup;
	st->st_nlink = 2;
	// st->st_mode = inode.mode;
	if (inode.type == FS_FILE) {
		st->st_mode = __S_IFREG | 0644;
		st->st_nlink = 1;
	}

	/*
	if (strcmp(path, "/") == 0) {
	        st->st_uid = 1717;
	} else if (strcmp(path, "/fisop") == 0) {
	        st->st_uid = 1818;
	        st->st_size = 2048;
	} else {
	        return -ENOENT;
	}
	*/

	return 0;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	int pos = get_index_inodo(path);
	if (pos == -1) {
		fprintf(stderr, "[Debug] Error readdir: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}


	struct inode inodo_dir = super_b.inodes[pos];
	if (inodo_dir.type != FS_DIR) {
		fprintf(stderr, "[Debug] Error readdir: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}

	inodo_dir.stats_info.last_acc = time(NULL);
	for (int i = 1; i < MAX_INODES; i++) {
		if (super_b.bitmap_inodes[i] == 1) {
			if (strcmp(super_b.inodes[i].directory_path,
			           inodo_dir.path) == 0) {
				filler(buffer, super_b.inodes[i].path, NULL, 0);
			}
		}
	}

	return 0;
	/*
	// Si nos preguntan por el directorio raiz, solo tenemos un archivo
	if (strcmp(path, "/") == 0) {
	        filler(buffer, "fisop", NULL, 0);
	        return 0;
	}

	return -ENOENT;*/
}

#define MAX_CONTENIDO 100
// static char fisop_file_contenidos[MAX_CONTENIDO] = "hola fisopfs!\n";

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);


	if (offset < 0 || size < 0) {
		fprintf(stderr, "[Debug] Error read: %s\n", strerror(errno));
		errno = EINVAL;
		return -EINVAL;
	}
	int pos = get_index_inodo(path);
	if (pos == -1) {
		fprintf(stderr, "[Debug] Error read: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}

	struct inode *inode = &super_b.inodes[pos];
	if (inode->type == FS_DIR) {
		fprintf(stderr, "[Debug] Error read: %s\n", strerror(errno));
		errno = EISDIR;
		return -EISDIR;
	}

	char *content = inode->content;
	size_t file_size = inode->size;
	if (offset > file_size) {
		fprintf(stderr, "[Debug] Error read: %s\n", strerror(errno));
		errno = EINVAL;
		return -EINVAL;
	}
	strncpy(buffer, content + offset, size);
	inode->stats_info.last_acc = time(NULL);

	return size;
}

void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[Debug] fisop_init\n");
	FILE *file = fopen(filedisk, "r");
	if (!file) {
		initialize_fs();
	} else {
		int i = fread(&super_b, sizeof(super_b), 1, file);
		if (i != 1) {
			fprintf(stderr,
			        "[Debug] Error init: %s\n",
			        strerror(errno));
			return NULL;
		}
		fclose(file);
	}

	return 0;
}

static int
fisopfs_touch(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[Debug] fisop_touch : %s\n", path);
	return create_file(path, mode, FS_FILE);
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[Debug] fisopfs_mkdir : %s\n", path);
	// return create_dir(path, mode);
	return create_file(path, mode, FS_DIR);
}


// static int fisopfs_ls(){
// 	return 0;
// }

static int
fisopfs_write(const char *path,
              const char *buffer,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("[Debug] fisopfs_write: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);
	return write_file(path, buffer, size, offset);
}

// static int fisopfs_stat(){
// 	return 0;
// }


static int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink - path: %s\n", path);
	// int i = search_inode(path);
	return unlink(path);
}

// static int
// fisopfs_unlink(const char *path){
// 	printf("[Debug] fisopfs_unlink: %s\n", path);
// 	return delete_file(path);
// }


static int
fisopfs_rmdir(const char *path)
{
	printf("[Debug] fisopfs_rmdir: %s\n", path);
	return delete_dir(path);
}

int
fisopfs_update_time(const char *path, const struct timespec ts[2])
{
	int pos = get_index_inodo(path);
	if (pos == -1) {
		fprintf(stderr, "[Debug] Error readdir: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}
	struct inode *inode = &super_b.inodes[pos];

	inode->stats_info.last_acc = ts[0].tv_sec;
	inode->stats_info.last_mod = ts[1].tv_sec;
	return 0;
}

static int
fisopfs_truncate(const char *path, off_t size)
{
	printf("[Debug] fisopfs_truncate: %s\n", path);
	if (size > MAX_CONTENT) {
		fprintf(stderr, "[Debug] Error truncate: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}
	int i = get_index_inodo(path);
	if (i < 0) {
		fprintf(stderr, "[Debug] Error truncate: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}

	struct inode *inode = &super_b.inodes[i];
	inode->size = size;
	inode->stats_info.last_mod = time(NULL);
	return 0;
}

void
fisopfs_destroy(void *private_data)
{
	printf("[Debug] fisopfs_destroy: \n");
	FILE *file = fopen(filedisk, "w");
	if (!file) {
		fprintf(stderr, "[Debug] ERROR save fisop: %s\n", strerror(errno));
	}
	fwrite(&super_b, sizeof(super_b), 1, file);
	fflush(file);
	fclose(file);
}


static struct fuse_operations operations = { .getattr = fisopfs_getattr,  // stats
	                                     .readdir = fisopfs_readdir,  // ls
	                                     .read = fisopfs_read,
	                                     .init = fisopfs_init,
	                                     .create = fisopfs_touch,
	                                     .mkdir = fisopfs_mkdir,
	                                     .write = fisopfs_write,
	                                     .rmdir = fisopfs_rmdir,
	                                     .unlink = fisopfs_unlink,
	                                     .utimens = fisopfs_update_time,
	                                     .truncate = fisopfs_truncate,
	                                     .destroy = fisopfs_destroy };

int
main(int argc, char *argv[])
{
	// valido si me pasan -f por consola
	if (strcmp(argv[1], "-f") == 0) {
		if (argc == 4) {
			// copio el tercer argumento al fs y disminuyo un argumento
			strcpy(filedisk, argv[3]);
			argv[3] = NULL;
			argc--;
		}
	} else {
		if (argc == 3) {
			strcpy(filedisk, argv[2]);
			argv[2] = NULL;
			argc--;
		}
	}
	return fuse_main(argc, argv, &operations, NULL);
}