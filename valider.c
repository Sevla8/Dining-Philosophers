#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define RESULTAT "resultat.txt"
#define NB_LINES 50

int main(int argc, char const *argv[]) {

	int fd = open(RESULTAT, O_RDWR);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	char input = '0';
	int tour = 0;

	while (input != '5') {

		printf("\n \
			1 - Consulter résultat\n \
			2 - Modifier le nom d'un philosophe\n \
			3 - Supprimer le nom d'un philosophe\n \
			4 - Modifier le nom et l'action d'un philosophe\n \
			5 - Quitter\n");
		if (tour) while ((input = getchar()) != '\n' && input != EOF) {}
		printf("Veuillez taper '1' ou '2' ou '3' ou '4' ou '5' svp\n");
		scanf("%c", &input);

		while (input != '1' && input != '2' && input != '3' && input != '4' && input != '5') {
			while ((input = getchar()) != '\n' && input != EOF) {}
			printf("Veuillez taper '1' ou '2' ou '3' ou '4' ou '5' svp\n");
			scanf("%c", &input);
		}

		switch (input) {
			case '1': {
				pid_t pid = fork();
				if (pid == -1) {
					perror("fork()");
					exit(EXIT_FAILURE);
				}
				if (!pid) {
					execlp("cat", "cat", RESULTAT, (char*)NULL);
				}
				if (waitpid(pid, NULL, 0) == -1) {
					perror("waitpid()");
					exit(EXIT_FAILURE);
				}
				break;
			}
			case '2': {
				printf("Veuillez choisir un code de ligne svp (entre 0 et %d)\n", NB_LINES-1);
				int line;
				scanf("%d", &line);
				break;
			}
			case '3': {
				printf("Veuillez choisir un code de ligne svp (entre 0 et %d)\n", NB_LINES-1);
				int line;
				scanf("%d", &line);

				struct stat bufStat;
				if (fstat(fd, &bufStat) == -1) {
					perror("stat()");
					exit(EXIT_FAILURE);
				}
				int size = (int) bufStat.st_size;

				if (lockf(fd, F_TEST, size) == -1) {
					printf("Veuillez reessayer plus tard svp\n");
				}
				else {
					if (lockf(fd, F_LOCK, size) == -1) {
						perror("lockf");
						exit(EXIT_FAILURE);
					}

					int fd_tmp = open("tmp.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
					if (fd_tmp == -1) {
						perror("open");
						exit(EXIT_FAILURE);
					}

					if (lseek(fd, 0, SEEK_SET) == -1) {
						perror("lseek");
						exit(EXIT_FAILURE);
					}

					char c[1];
					int line_nb = 0;
					while (line_nb < line) {
						memset(c, 0, 1);

						if (read(fd, c, sizeof(char)) == -1) {
							perror("read");
							exit(EXIT_FAILURE);
						}

						if (write(fd_tmp, c, sizeof(char)) == -1) {
							perror("write");
							exit(EXIT_FAILURE);
						}

						if (c[0] == '\n') line_nb += 1;
					}

					if (read(fd, c, sizeof(char)) == -1) {
						perror("read");
						exit(EXIT_FAILURE);
					}
					while (c[0] != '\n') {
						memset(c, 0, 1);

						if (read(fd, c, sizeof(char)) == -1) {
							perror("read");
							exit(EXIT_FAILURE);
						}
					}

					while (line_nb < NB_LINES - 1) {
						memset(c, 0, 1);

						if (read(fd, c, sizeof(char)) == -1) {
							perror("read");
							exit(EXIT_FAILURE);
						}

						if (write(fd_tmp, c, sizeof(char)) == -1) {
							perror("write");
							exit(EXIT_FAILURE);
						}

						if (c[0] == '\n') line_nb += 1;
					}

					if (lockf(fd, F_ULOCK, size) == -1) {
						perror("lockf");
						exit(EXIT_FAILURE);
					}

					close(fd_tmp);

					if (rename("tmp.txt", RESULTAT) == -1) {
						perror("rename");
						exit(EXIT_FAILURE);
					}
				}


				break;
			}
			case '4': {
				
				break;
			}
			case '5': {
				break;
			}
		}
		tour += 1;
	}

	exit(EXIT_SUCCESS);
}