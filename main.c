#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#define PHILOSOPER_AMOUNT 1
#define OPERATION_TIME 1
#define RESULTAT "resultat.txt"

int code = 0;
int fd = -1;

typedef enum  {
	THINKING, HUNGRY, EATING
} State;

typedef struct {
	pthread_mutex_t mutex;
	int philosophers[PHILOSOPER_AMOUNT];
	State states[PHILOSOPER_AMOUNT];
	pthread_cond_t conditions[PHILOSOPER_AMOUNT];
	int total_wait_time[PHILOSOPER_AMOUNT];
	int meals[PHILOSOPER_AMOUNT];
	int total_meals;
} TheDiningPhilosophers;

TheDiningPhilosophers diningPhilosophers;

void *philosopher(void*);
void pickup_forks(int);
void return_forks(int);
void check_forks(int);
void valider();

int main(int argc, char const *argv[]) {
	srand(time(NULL));

	fd = open(RESULTAT, O_WRONLY | O_CREAT | O_TRUNC);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	pthread_t tids[PHILOSOPER_AMOUNT];

	for (int i = 0; i < PHILOSOPER_AMOUNT; i += 1) {
		diningPhilosophers.philosophers[i] = i;
		diningPhilosophers.states[i] = THINKING;
		diningPhilosophers.meals[i] = 0;
		diningPhilosophers.total_wait_time[i] = 0;
		if (pthread_cond_init(&diningPhilosophers.conditions[i], NULL)) {
			perror("pthread_cond_init");
			exit(EXIT_FAILURE);
		}
	}

	diningPhilosophers.total_meals = 0;
	if (pthread_mutex_init(&diningPhilosophers.mutex, NULL)) {
		perror("pthread_mutex_init");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < PHILOSOPER_AMOUNT; i += 1) {
		if (pthread_create(&tids[i], NULL, philosopher, &diningPhilosophers.philosophers[i])) {
			perror("pthread_crcheck_forkse");
			exit(EXIT_FAILURE);
		}
	}
	for (int i = 0; i < PHILOSOPER_AMOUNT; i += 1) {
		if (pthread_join(tids[i], NULL)) {
			perror("pthread_join");
			exit(EXIT_FAILURE);
		}
	}

	if (pthread_mutex_destroy(&diningPhilosophers.mutex)) {
		perror("pthread_mutex_destroy");
		exit(EXIT_FAILURE);
	}

	valider();

	exit(EXIT_SUCCESS);
}

void *philosopher(void *param) {
	int philosopher = *(int*) param;

	while (1) {
		int think_time = (rand() % OPERATION_TIME) + 1;
		int check_forks_time = (rand() % OPERATION_TIME) + 1;
		char buf[100];
		char tmp[100];

		sprintf(buf, "%d", code++);
		strcat(buf, "\tPhilosopher ");
		sprintf(tmp, "%d", philosopher);
		strcat(buf, tmp);
		strcat(buf, "\tTHINKING\n\0");
		write(fd, buf, strlen(buf));

		sleep(think_time);
		pickup_forks(philosopher);
	
		sprintf(buf, "%d", code++);
		strcat(buf, "\tPhilosopher ");
		sprintf(tmp, "%d", philosopher);
		strcat(buf, tmp);
		strcat(buf, "\tEATING\n\0");
		write(fd, buf, strlen(buf));

		sleep(check_forks_time);
		return_forks(philosopher);

		if (diningPhilosophers.meals[philosopher] == PHILOSOPER_AMOUNT) {
			pthread_exit(NULL);
		}
	}
}

void pickup_forks(int philosopher) {
	if (pthread_mutex_lock(&diningPhilosophers.mutex)) {
		perror("pthread_mutex_lock");
		exit(EXIT_FAILURE);
	}

	diningPhilosophers.states[philosopher] = HUNGRY;
	// printf("Philosopher %d HUNGRY\n", philosopher);

	check_forks(philosopher);
	if (diningPhilosophers.states[philosopher] != EATING) {
		if (pthread_cond_wait(&diningPhilosophers.conditions[philosopher], &diningPhilosophers.mutex)) {
			perror("pthread_cond_wait");
			exit(EXIT_FAILURE);
		}
	}

	if (pthread_mutex_unlock(&diningPhilosophers.mutex)) {
		perror("pthread_mutex_unlock");
		exit(EXIT_FAILURE);
	}
}

void return_forks(int philosopher) {
	if (pthread_mutex_lock(&diningPhilosophers.mutex)) {
		perror("pthread_mutex_lock");
		exit(EXIT_FAILURE);
	}

	diningPhilosophers.meals[philosopher] += 1;
	diningPhilosophers.total_meals += 1;
	diningPhilosophers.states[philosopher] = THINKING;

	int left_philosopher = (philosopher + PHILOSOPER_AMOUNT - 1) % PHILOSOPER_AMOUNT;
	int right_philosopher = (philosopher + 1) % PHILOSOPER_AMOUNT;
	check_forks(left_philosopher);
	check_forks(right_philosopher);

	if (pthread_mutex_unlock(&diningPhilosophers.mutex)) {
		perror("pthread_mutex_unlock");
		exit(EXIT_FAILURE);
	}
}

void check_forks(int philosopher) {
	int left_philosopher = (philosopher + PHILOSOPER_AMOUNT - 1) % PHILOSOPER_AMOUNT;
	int right_philosopher = (philosopher + 1) % PHILOSOPER_AMOUNT;

	if (diningPhilosophers.states[philosopher] == HUNGRY && 
		diningPhilosophers.states[left_philosopher] != EATING && 
		diningPhilosophers.states[right_philosopher] != EATING) {

		diningPhilosophers.states[philosopher] = EATING;

		if (pthread_cond_signal(&diningPhilosophers.conditions[philosopher])) {
			perror("pthread_cond_signal");
			exit(EXIT_FAILURE);
		}
	}
}

void valider() {
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
					execlp("less", "less", RESULTAT, (char*)NULL);
				}
				if (waitpid(pid, NULL, 0) == -1) {
					perror("waitpid()");
					exit(EXIT_FAILURE);
				}
				break;
			}
			case '2': {

				break;
			}
			case '3': {

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
}
