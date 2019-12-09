#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define PHILOSOPER_AMOUNT 5
#define OPERATION_TIME 3

int code = 0;

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

int main(int argc, char const *argv[]) {
	srand(time(NULL));

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

	exit(EXIT_SUCCESS);
}

void *philosopher(void *param) {
	int philosopher = *(int*) param;

	while (1) {
		int think_time = (rand() % OPERATION_TIME) + 1;
		int check_forks_time = (rand() % OPERATION_TIME) + 1;

		printf("%d Philosopher %d THINKING\n", code++, philosopher);
		sleep(think_time);
		pickup_forks(philosopher);
		
		printf("%d Philosopher %d EATING\n", code++, philosopher);
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
