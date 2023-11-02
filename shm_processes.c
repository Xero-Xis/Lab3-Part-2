#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

// Shared memory key
#define SHM_KEY 1234

// Shared memory struct
typedef struct {
  int bank_account;
  int turn;
} SharedMem;

// Parent process code
void ParentProcess(SharedMem *shared_mem) {
  // Sleep for a random amount of time
  sleep(rand() % 5);

  // Copy the bank account balance to a local variable
  int account = shared_mem->bank_account;

  // Loop until the child process has finished its turn
  while (shared_mem->turn != 0) {}

  // If the bank account balance is less than or equal to 100, deposit money
  if (account <= 100) {
    // Generate a random amount of money to deposit
    int balance = rand() % 101;

    // If the random number is even, deposit the money
    if (balance % 2 == 0) {
      account += balance;
      printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
    } else {
      printf("Dear old Dad: Doesn't have any money to give\n");
    }
  } else {
    printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
  }

  // Copy the updated bank account balance back to shared memory
  shared_mem->bank_account = account;

  // Set the child process's turn
  shared_mem->turn = 1;
}

// Child process code
void ChildProcess(SharedMem *shared_mem) {
  // Sleep for a random amount of time
  sleep(rand() % 5);

  // Copy the bank account balance to a local variable
  int account = shared_mem->bank_account;

  // Loop until the parent process has finished its turn
  while (shared_mem->turn != 1) {}

  // Generate a random amount of money to withdraw
  int balance = rand() % 51;

  // If the amount of money needed is less than or equal to the bank account balance, withdraw the money
  if (balance <= account) {
    account -= balance;
    printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
  } else {
    printf("Poor Student: Not Enough Cash ($%d)\n", account);
  }

  // Copy the updated bank account balance back to shared memory
  shared_mem->bank_account = account;

  // Set the parent process's turn
  shared_mem->turn = 0;
}

int main() {
  // Create a shared memory segment
  int shm_id = shmget(SHM_KEY, sizeof(SharedMem), IPC_CREAT | 0666);
  if (shm_id < 0) {
    perror("shmget");
    exit(1);
  }

  // Attach the shared memory segment
  SharedMem *shared_mem = shmat(shm_id, NULL, 0);
  if (shared_mem == NULL) {
    perror("shmat");
    exit(1);
  }

  // Initialize the shared memory
  shared_mem->bank_account = 0;
  shared_mem->turn = 0;

  // Fork a child process
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(1);
  }

  // Parent process
  if (pid > 0) {
    ParentProcess(shared_mem);
  }

  // Child process
  else {
    ChildProcess(shared_mem);
  }

  // Wait for the child process to finish
  wait(NULL);

  // Detach the shared memory segment
  shmdt(shared_mem);

  // Remove the shared memory segment
  shmctl(shm_id, IPC_RMID, NULL);

  return 0;
}
