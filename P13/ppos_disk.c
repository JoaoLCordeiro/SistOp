
#include "disk.c"

int disk_mgr_init (int *numBlocks, int *blockSize){
	if (disk_cmd (DISK_CMD_INIT, NULL, NULL)){
		perror ("disk_mgr_init: Erro ao iniciar o disco (DISK_CMD_INIT)\n");
		return -1;
	}

	*numBlocks	=	disk_cmd (DISK_CMD_DISKSIZE, NULL, NULL);
	if (numBlocks == -1){
		perror ("disk_mgr_init: Erro ao iniciar o disco (DISK_CMD_DISKSIZE)\n");
		return -1;
	}

	*blockSize	=	disk_cmd (DISK_CMD_BLOCKSIZE, NULL, NULL);
	if (blockSize == -1){
		perror ("disk_mgr_init: Erro ao iniciar o disco (DISK_CMD_BLOCKSIZE)\n");
		return -1;
	}

	return 0;
}

int disk_block_read (int block, void *buffer){
	if (disk_cmd (DISK_CMD_READ, block, buffer)){
		perror ("disk_block_read: Erro ao ler do arquivo\n");
		return -1;
	}
}

