#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>

int newTasksToBeAdded;

/* Struct for list nodes */
typedef struct list_node_s {
int data;
struct list_node_s* next;
} list_node;

list_node *list_node_head = NULL;

/* Struct for task nodes */
typedef struct task_node_s {
int task_num;
int task_type; // insert:0, delete:1, search:2
int value;
struct task_node_s* next;
} task_node;

task_node *task_node_head = NULL;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;

void searchv2(int data, list_node **list_node_head);
void deletev2(int data, list_node **list_node_head);
void insertv2(int data, list_node **list_node_head);

/* Task queue functions */

void Task_queue(int n); //generate random tasks for the task queue
void Task_enqueue(int task_num, int task_type, int value); //insert a new task into task queue
void Task_dequeue(long my_rank, int task_num_p, int task_type_p, int value_p); //take a task from task queue

void *thread_func(void *arg);

void Task_queue(int n) { // generate random task
    int value = rand() % 501;
    int task_type = rand() % 3;
    Task_enqueue(n, task_type, value);
}

void Task_enqueue(int task_num, int task_type, int value) { // insert the task into the queue

    task_node *new_task_node;

    new_task_node = (task_node*)malloc(sizeof(task_node));
    // get the fields of the head note
    new_task_node->task_num = task_num;
    new_task_node->task_type = task_type;
    new_task_node->value = value;
    new_task_node->next == NULL;
    
    if (task_node_head == NULL) {

        task_node_head = new_task_node;
    }
    else {
        task_node *current;

        current = task_node_head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_task_node;
        return;
    }
}


void *thread_func(void *arg) {
    long my_rank = pthread_self();

    while (newTasksToBeAdded != 0 || task_node_head != NULL) {
        
        if (newTasksToBeAdded == 0 && task_node_head == NULL) {
            return NULL;
        }

        pthread_mutex_lock(&lock1);
        
        while (newTasksToBeAdded != 0) {
            pthread_cond_wait(&cond, &lock1);
        }

        if ((task_node_head != NULL)) {

            int task_num = task_node_head->task_num;
            int task_type = task_node_head->task_type;
            int value = task_node_head->value;

            Task_dequeue(my_rank, task_num, task_type, value); // take a task from the queue

        }
        

        pthread_mutex_unlock(&lock1);
    }
}

void Task_dequeue(long my_rank, int task_num_p, int task_type_p, int value_p) { // insert:0, delete:1, search:2

    if (task_type_p == 1) { // delete
        printf("Thread %ld: task %d: ", my_rank, task_num_p);
        deletev2(value_p, &list_node_head);
    }

    else if (task_type_p == 0) { // insert
        printf("Thread %ld: task %d: ", my_rank, task_num_p);
        insertv2(value_p, &list_node_head);
    }

    else if (task_type_p == 2) { // search
        printf("Thread %ld: task %d: ", my_rank, task_num_p);
        searchv2(value_p, &list_node_head);
    }

    task_node *temp = task_node_head;
    task_node_head = task_node_head->next;
    free(temp);
}

void insertv2(int data, list_node **list_node_head) { // needed for insert task only

    list_node *new_data_node;

    new_data_node = (list_node*)malloc(sizeof(list_node));
    new_data_node->data = data;
    new_data_node->next = NULL;

    if (*list_node_head == NULL) { // create head at the start of the list creation
        *list_node_head = new_data_node;
        printf("%d inserted\n", data);
        return;
    }

    list_node *current = *list_node_head;
    list_node *prev;

    int isFirstIteration = 1;
    // 2 3 6
    while (current != NULL) // propogate among the nodes until the end is found
    {
        if (new_data_node->data == current->data) { // already in the list
            printf("%d cannot be inserted, already in the list\n",data);
            free(new_data_node);
            return;
        }

        if (new_data_node->data < current->data) {
            if (isFirstIteration) { // remembering the first iteration is important for adding the new datum as the new head
                new_data_node->next = current;
                *list_node_head = new_data_node;
                printf("%d inserted\n", data);
                return;
            }
            // adding in the middle
            new_data_node->next = current;
            prev->next = new_data_node;
            printf("%d inserted\n", data);
            return;
        }

        isFirstIteration = 0;
        prev = current;
        current = current->next;
    }

    prev->next = new_data_node; // adding as the last data
    printf("%d inserted\n", data);
    return;
}

void deletev2(int data, list_node **list_node_head) { // deleting

    list_node *current = *list_node_head;
    list_node *prev;

    int isFirstIteration = 1;

    while (current != NULL) {
        if (isFirstIteration) { // remembering the first iteration is important because we don't want to lose the head while deleting the list
            if (current->data == data) {
                *list_node_head = current->next;
                list_node *temp = current;
                free(temp);
                printf("%d deleted\n", data);
                return;
            }
        }

        if (current->data == data) {
            prev->next = current->next;
            list_node *temp = current;
            free(temp);
            printf("%d deleted\n", data);
            return;
        }

        isFirstIteration = 0;
        prev = current;
        current = current->next;
    }
    
    printf("%d cannot be deleted, not in the list\n",data);

}

void searchv2(int data, list_node **list_node_head) { // searching function
    list_node *current = *list_node_head;

    while (current != NULL) { // propagate among the list
        if (current->data == data) {
            printf("%d is in the list\n",data);
            return;
        }
        current = current->next;
    }
    
    printf("%d cannot be found\n",data);
}

void print_list(list_node **head) // function to print sorted linked list
{
    list_node *current = *head;
    int i = 1;
    while (current != NULL)
    {
        printf("%d > ",(current->data));
        current = current->next;
        i++;
    }
    printf("\n");
}

// void print_tasks(task_node **head) //! I have written this function to myself to see whether the task insertion process works fine or not
// {
//     task_node *current = *head;
//     int i = 1;
//     while (current != NULL)
//     {
//         printf("%d, %d, %d\n",current->task_num, current->value, current->task_type);
//         current = current->next;
//         i++;
//     }
// }

int main(int argc, char const *argv[]) {

    clock_t start, end;
    start = clock();

    time_t t;
    srand((unsigned) time(&t));
    int number_of_threads = atoi(argv[1]);
    int number_of_tasks = atoi(argv[2]);
    newTasksToBeAdded = atoi(argv[2]);

    pthread_t thread_id[number_of_threads];

    for (int i = 0 ; i < number_of_threads ; i++) {
        pthread_create(&thread_id[i], NULL, thread_func, NULL);
    }

    for (int i = 0 ; i < number_of_tasks ; i++) {
        pthread_mutex_lock(&lock1);
        Task_queue(i);
        newTasksToBeAdded = newTasksToBeAdded-1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock1);
    }

    pthread_cond_broadcast(&cond);

    while(task_node_head != NULL) {}
    for(int i = 0 ; i < number_of_threads ; i++) {
        pthread_join(thread_id[i], NULL);
    }

    // print_tasks(&task_node_head); // ! additional function for myself to see the task nodes
    printf("Final list\n");
    print_list(&list_node_head);

    end = clock();

    double time_elapsed = (double)(end - start)/(double)(CLOCKS_PER_SEC);
    printf("Execution time of the program: %lf\n",time_elapsed);

    return 0;
}
