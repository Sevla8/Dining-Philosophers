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

int fd = -1;
int NB_LINES = 0;

void function1();
void function2();
void function3();
void function4();

int main(int argc, char const *argv[]) {

	fd = open(RESULTAT, O_RDWR);	// on ouvre le fichier
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}
	char c[1] = "0";	// on compte le nombre de lignes
	while (read(fd, c, sizeof(char)) != 0) {
		if (c[0] == '\n') NB_LINES += 1;
	}

	char input = '0';
	int tour = 0;	// utilisé pour gérer l'affichage

	while (input != '5') {

		printf("\n \
			1 - Consulter résultat\n \
			2 - Modifier le nom d'un philosophe\n \
			3 - Supprimer le nom d'un philosophe\n \
			4 - Modifier le nom et l'action d'un philosophe\n \
			5 - Quitter\n");
		if (tour) while ((input = getchar()) != '\n' && input != EOF) ;
		printf("Veuillez taper '1' ou '2' ou '3' ou '4' ou '5' svp\n");
		scanf("%c", &input);

		while (input != '1' && input != '2' && input != '3' && input != '4' && input != '5') {
			while ((input = getchar()) != '\n' && input != EOF) ;	// vidage du buffer
			printf("Veuillez taper '1' ou '2' ou '3' ou '4' ou '5' svp\n");
			scanf("%c", &input);
		}

		switch (input) {
			case '1': {
				function1();
				break;
			}
			case '2': {
				function2();
				break;
			}
			case '3': {
				function3();
				break;
			}
			case '4': {
				function4();
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

void function1() {
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
}

void function2() {
	printf("Veuillez choisir un code de ligne svp (entre 0 et %d)\n", NB_LINES-1);
	int line;
	scanf("%d", &line);

	if (line >= NB_LINES) {
		fprintf(stderr, "Saisie invalide\n");
		exit(EXIT_FAILURE);
	}

	if (lseek(fd, 0, SEEK_SET) == -1) {	// on met le curseur au début
		perror("lseek");
		exit(EXIT_FAILURE);
	}

	char c[1];
	int line_nb = 0;
	while (line_nb < line) {	// on se place à la ligne correspondante
		memset(c, 0, 1);

		if (read(fd, c, sizeof(char)) == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		if (c[0] == '\n') line_nb += 1;
	}

	int nb = 2;
	int size = 0;
	while (nb) {	// on calcule la taille du champ nom
		memset(c, 0, 1);

		if (read(fd, c, sizeof(char)) == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		size += 1;

		if (c[0] == '\t') nb -= 1;
	}
	size -= 3; // on enlève le code et les 2 tabulations

	if (lockf(fd, F_TEST, size) == -1) {
		printf("Veuillez reessayer plus tard svp\n");
	}
	else {
		if (lockf(fd, F_LOCK, size) == -1) {
			perror("lockf");
			exit(EXIT_FAILURE);
		}

		printf("Veuillez choisir un nouveau nom svp\n");
		char string[100];
		scanf("%s", string);

		int fd_tmp = open("tmp2.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
		if (fd_tmp == -1) {
			perror("open");
			exit(EXIT_FAILURE);
		}

		if (lseek(fd, 0, SEEK_SET) == -1) {	// on met le curseur au début
			perror("lseek");
			exit(EXIT_FAILURE);
		}

		char c[1];
		int line_nb = 0;
		while (line_nb < line) {	// on copie tout jusqu'à la ligne correspondante
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

		while (c[0] != '\t') {	 // on copie tout jusqu'au début du champ nom
			memset(c, 0, 1);

			if (read(fd, c, sizeof(char)) == -1) {
				perror("read");
				exit(EXIT_FAILURE);
			}
			if (write(fd_tmp, c, sizeof(char)) == -1) {
				perror("write");
				exit(EXIT_FAILURE);
			}
		}

		if (read(fd, c, sizeof(char)) == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		while (c[0] != '\t') {	// on se place à la fin du champ nom
			memset(c, 0, 1);

			if (read(fd, c, sizeof(char)) == -1) {
				perror("read");
				exit(EXIT_FAILURE);
			}
		}

		if (write(fd_tmp, string, sizeof(char)*strlen(string)) == -1) {	// on écrit le nom
			perror("write");
			exit(EXIT_FAILURE);
		}

		if (write(fd_tmp, "\t", sizeof(char)) == -1) {
			perror("write");
			exit(EXIT_FAILURE);
		}

		while (line_nb < NB_LINES) {	// on écrit le reste
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

		if (unlink(RESULTAT) == -1) {	// on supprime le fichier initiale
			perror("unlink");
			exit(EXIT_FAILURE);
		}		

		if (rename("tmp2.txt", RESULTAT) == -1) {	// on renomme le fichier tmp
			perror("rename");
			exit(EXIT_FAILURE);
		}
	}
}

void function3() {
	struct stat bufStat;
	if (fstat(fd, &bufStat) == -1) {
		perror("stat()");
		exit(EXIT_FAILURE);
	}
	int size = (int) bufStat.st_size;	// on attrape la taille du fichier

	if (lockf(fd, F_TEST, size) == -1) {
		printf("Veuillez reessayer plus tard svp\n");
	}
	else {
		if (lockf(fd, F_LOCK, size) == -1) {
			perror("lockf");
			exit(EXIT_FAILURE);
		}

		printf("Veuillez choisir un code de ligne svp (entre 0 et %d)\n", NB_LINES-1);
		int line;
		scanf("%d", &line);

		if (line >= NB_LINES) {
			fprintf(stderr, "Saisie invalide\n");
			exit(EXIT_FAILURE);
		}

		int fd_tmp = open("tmp3.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
		if (fd_tmp == -1) {
			perror("open");
			exit(EXIT_FAILURE);
		}

		if (lseek(fd, 0, SEEK_SET) == -1) {	// on se place au début
			perror("lseek");
			exit(EXIT_FAILURE);
		}

		char c[1];
		int line_nb = 0;
		while (line_nb < line) {	// on copie tout jusqu'à la ligne correspondante
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

		while (c[0] != '\n') {	// on saute la ligne en question
			memset(c, 0, 1);

			if (read(fd, c, sizeof(char)) == -1) {
				perror("read");
				exit(EXIT_FAILURE);
			}
		}

		while (line_nb < NB_LINES - 1) {	// on copie le reste
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

		if (unlink(RESULTAT) == -1) {	// on supprime le fichier initiale
			perror("unlink");
			exit(EXIT_FAILURE);
		}		

		if (rename("tmp3.txt", RESULTAT) == -1) {	// on renomme le fichier tmp
			perror("rename");
			exit(EXIT_FAILURE);
		}
	}
}

void function4() {
	printf("Veuillez choisir un code de ligne svp (entre 0 et %d)\n", NB_LINES-1);
	int line;
	scanf("%d", &line);

	if (line >= NB_LINES) {
		fprintf(stderr, "Saisie invalide\n");
		exit(EXIT_FAILURE);
	}

	if (lseek(fd, 0, SEEK_SET) == -1) {	// on met le curseur au début
		perror("lseek");
		exit(EXIT_FAILURE);
	}

	char c[1];
	int line_nb = 0;
	while (line_nb < line) {	// on se place à la ligne correspondante
		memset(c, 0, 1);

		if (read(fd, c, sizeof(char)) == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		if (c[0] == '\n') line_nb += 1;
	}

	int size = 0;
	do {	// on calcule la taille de l'enregistrement
		if (read(fd, c, sizeof(char)) == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}
		size += 1;
	} while (c[0] != '\n');


	if (lockf(fd, F_TEST, size) == -1) {
		printf("Veuillez reessayer plus tard svp\n");
	}
	else {
		if (lockf(fd, F_LOCK, size) == -1) {
			perror("lockf");
			exit(EXIT_FAILURE);
		}

		printf("Veuillez choisir un nouveau nom svp\n");
		char string_nom[100];
		scanf("%s", string_nom);
		printf("Veuillez choisir une nouvelle action svp\n");
		char string_action[100];
		scanf("%s", string_action);

		int fd_tmp = open("tmp4.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
		if (fd_tmp == -1) {
			perror("open");
			exit(EXIT_FAILURE);
		}

		if (lseek(fd, 0, SEEK_SET) == -1) {	// on met le curseur au début
			perror("lseek");
			exit(EXIT_FAILURE);
		}

		char c[1];
		int line_nb = 0;
		while (line_nb < line) {	// on copie tout jusqu'à la ligne correspondante
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

		while (c[0] != '\t') {	 // on copie tout jusqu'au début du champ nom
			memset(c, 0, 1);

			if (read(fd, c, sizeof(char)) == -1) {
				perror("read");
				exit(EXIT_FAILURE);
			}
			if (write(fd_tmp, c, sizeof(char)) == -1) {
				perror("write");
				exit(EXIT_FAILURE);
			}
		}

		while (c[0] != '\n') {	// on se place à la fin de l'enregistrement
			memset(c, 0, 1);

			if (read(fd, c, sizeof(char)) == -1) {
				perror("read");
				exit(EXIT_FAILURE);
			}
		}

		if (write(fd_tmp, string_nom, sizeof(char)*strlen(string_nom)) == -1) {	// on écrit le nom
			perror("write");
			exit(EXIT_FAILURE);
		}

		if (write(fd_tmp, "\t", sizeof(char)) == -1) {
			perror("write");
			exit(EXIT_FAILURE);
		}

		if (write(fd_tmp, string_action, sizeof(char)*strlen(string_action)) == -1) {	// on écrit l'action'
			perror("write");
			exit(EXIT_FAILURE);
		}

		if (write(fd_tmp, "\n", sizeof(char)) == -1) {
			perror("write");
			exit(EXIT_FAILURE);
		}

		while (line_nb < NB_LINES - 1) {	// on écrit le reste
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

		if (unlink(RESULTAT) == -1) {	// on supprime le fichier initiale
			perror("unlink");
			exit(EXIT_FAILURE);
		}		

		if (rename("tmp4.txt", RESULTAT) == -1) {	// on renomme le fichier tmp
			perror("rename");
			exit(EXIT_FAILURE);
		}
	}
}
